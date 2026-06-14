#include "plugins/data/ExternalIntegration/PythonRuntime.h"

#include <functional>
#include <memory>
#include <regex>
#include <sstream>
#include <utility>

#include "../../../kernel/simulator/model/ModelSimulation.h"
#include "../../../kernel/simulator/SimulatorFacade.h"
#include "../../../kernel/simulator/essentialPlugins/Entity.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#if GENESYS_HAS_PYTHON_INTEGRATION
#include <Python.h>
#endif

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PythonRuntime::GetPluginInformation;
}
#endif

#if GENESYS_HAS_PYTHON_INTEGRATION
namespace {

struct PyGenesysSimulatorObject {
	PyObject_HEAD
	SimulatorFacade* facade = nullptr;
	bool ownsFacade = false;
};

struct PyGenesysContextObject {
	PyObject_HEAD
	SimulatorFacade* facade = nullptr;
	Model* model = nullptr;
	ModelComponent* component = nullptr;
	Entity* entity = nullptr;
};

struct PyGenesysEntityObject {
	PyObject_HEAD
	Entity* entity = nullptr;
};

struct PyGenesysEventObject {
	PyObject_HEAD
	Event* event = nullptr;
};

struct PyGenesysComponentObject {
	PyObject_HEAD
	ModelComponent* component = nullptr;
};

struct PyGenesysDataDefinitionObject {
	PyObject_HEAD
	ModelDataDefinition* dataDefinition = nullptr;
};

struct PyGenesysModelObject {
	PyObject_HEAD
	Model* model = nullptr;
};

PyTypeObject PyGenesysSimulatorType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysContextType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysEntityType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysEventType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysComponentType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysDataDefinitionType = {PyVarObject_HEAD_INIT(nullptr, 0)};
PyTypeObject PyGenesysModelType = {PyVarObject_HEAD_INIT(nullptr, 0)};

bool g_pythonTypesReady = false;

std::string formatPythonException() {
	PyObject* excType = nullptr;
	PyObject* excValue = nullptr;
	PyObject* excTraceback = nullptr;
	PyErr_Fetch(&excType, &excValue, &excTraceback);
	PyErr_NormalizeException(&excType, &excValue, &excTraceback);

	std::string message = "Unknown Python error";
	PyObject* tracebackModule = PyImport_ImportModule("traceback");
	if (tracebackModule != nullptr) {
		PyObject* formatted = PyObject_CallMethod(tracebackModule,
		                                         "format_exception",
		                                         "OOO",
		                                         excType != nullptr ? excType : Py_None,
		                                         excValue != nullptr ? excValue : Py_None,
		                                         excTraceback != nullptr ? excTraceback : Py_None);
		if (formatted != nullptr) {
			PyObject* separator = PyUnicode_FromString("");
			PyObject* joined = separator != nullptr ? PyUnicode_Join(separator, formatted) : nullptr;
			if (joined != nullptr) {
				const char* utf8 = PyUnicode_AsUTF8(joined);
				if (utf8 != nullptr) {
					message = utf8;
				}
			}
			Py_XDECREF(joined);
			Py_XDECREF(separator);
			Py_DECREF(formatted);
		}
		Py_DECREF(tracebackModule);
	}

	Py_XDECREF(excType);
	Py_XDECREF(excValue);
	Py_XDECREF(excTraceback);
	PyErr_Clear();
	return message;
}

void simulatorDealloc(PyObject* self) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	if (object->ownsFacade) {
		delete object->facade;
		object->facade = nullptr;
	}
	Py_TYPE(self)->tp_free(self);
}

PyObject* createSimulatorObject(SimulatorFacade* facade, bool ownsFacade = false);
PyObject* createContextObject(SimulatorFacade* facade, Model* model, ModelComponent* component, Entity* entity);
PyObject* createEntityObject(Entity* entity);
PyObject* createEventObject(Event* event);
PyObject* createComponentObject(ModelComponent* component);
PyObject* createDataDefinitionObject(ModelDataDefinition* dataDefinition);
PyObject* createModelObject(Model* model);

ModelDataDefinition* extractModelDataDefinition(PyObject* object) {
	if (PyObject_TypeCheck(object, &PyGenesysDataDefinitionType)) {
		return reinterpret_cast<PyGenesysDataDefinitionObject*>(object)->dataDefinition;
	}
	if (PyObject_TypeCheck(object, &PyGenesysEntityType)) {
		return reinterpret_cast<PyGenesysEntityObject*>(object)->entity;
	}
	if (PyObject_TypeCheck(object, &PyGenesysComponentType)) {
		return reinterpret_cast<PyGenesysComponentObject*>(object)->component;
	}
	return nullptr;
}

ModelComponent* extractModelComponent(PyObject* object) {
	if (PyObject_TypeCheck(object, &PyGenesysComponentType)) {
		return reinterpret_cast<PyGenesysComponentObject*>(object)->component;
	}
	return nullptr;
}

template <typename Container, typename Converter>
PyObject* createPythonListFromContainer(const Container* values, Converter converter) {
	PyObject* result = PyList_New(0);
	if (result == nullptr) {
		return nullptr;
	}
	if (values == nullptr) {
		return result;
	}
	for (const auto& value : *values) {
		PyObject* item = converter(value);
		if (item == nullptr || PyList_Append(result, item) != 0) {
			Py_XDECREF(item);
			Py_DECREF(result);
			return nullptr;
		}
		Py_DECREF(item);
	}
	return result;
}

PyObject* createStringList(const List<std::string>* values) {
	return values != nullptr ? createPythonListFromContainer(values->list(), [](const std::string& value) { return PyUnicode_FromString(value.c_str()); }) : PyList_New(0);
}

PyObject* createStdStringList(const std::list<std::string>* values) {
	return createPythonListFromContainer(values, [](const std::string& value) { return PyUnicode_FromString(value.c_str()); });
}

PyObject* createDoubleList(const List<double>* values) {
	return values != nullptr ? createPythonListFromContainer(values->list(), [](double value) { return PyFloat_FromDouble(value); }) : PyList_New(0);
}

PyObject* createModelList(const std::vector<Model*>& models) {
	return createPythonListFromContainer(&models, [](Model* model) { return createModelObject(model); });
}

PyObject* createDataDefinitionList(const std::list<ModelDataDefinition*>* values) {
	return createPythonListFromContainer(values, [](ModelDataDefinition* dataDefinition) { return createDataDefinitionObject(dataDefinition); });
}

PyObject* createDataDefinitionList(const List<ModelDataDefinition*>* values) {
	return values != nullptr ? createDataDefinitionList(values->list()) : PyList_New(0);
}

PyObject* createComponentList(const std::list<ModelComponent*>* values) {
	return createPythonListFromContainer(values, [](ModelComponent* component) { return createComponentObject(component); });
}

PyObject* createComponentList(const List<ModelComponent*>* values) {
	return values != nullptr ? createComponentList(values->list()) : PyList_New(0);
}

PyObject* createEventList(const std::list<Event*>* values) {
	return createPythonListFromContainer(values, [](Event* event) { return createEventObject(event); });
}

PyObject* createEventList(const List<Event*>* values) {
	return values != nullptr ? createEventList(values->list()) : PyList_New(0);
}

PyObject* simulatorGetVersion(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyUnicode_FromString(object->facade != nullptr ? object->facade->getVersion().c_str() : "");
}

PyObject* simulatorGetVersionNumber(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->getVersionNumber() : 0u);
}

PyObject* simulatorGetName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyUnicode_FromString(object->facade != nullptr ? object->facade->getName().c_str() : "");
}

PyObject* simulatorSimGetSimulatedTime(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyFloat_FromDouble(object->facade != nullptr ? object->facade->simGetSimulatedTime() : 0.0);
}

PyObject* simulatorModelCheck(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->modelCheck());
}

PyObject* simulatorModelShowLanguage(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->modelShowLanguage() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoGetDescription(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoGetDescription() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoSetDescription(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* description = nullptr;
	if (!PyArg_ParseTuple(args, "s", &description)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetDescription(description);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorTrace(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* text = nullptr;
	if (!PyArg_ParseTuple(args, "s", &text)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->trace(text);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorTraceReport(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* text = nullptr;
	if (!PyArg_ParseTuple(args, "s", &text)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->traceReport(text);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorTraceError(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* text = nullptr;
	if (!PyArg_ParseTuple(args, "s", &text)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->traceError(text);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorComponentGetNumberOfComponents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->componentGetNumberOfComponents() : 0u);
}

PyObject* simulatorDataGetNumberOfDataDefinitions(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->dataGetNumberOfDataDefinitions() : 0u);
}

PyObject* simulatorCurrentModel(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createModelObject(object->facade != nullptr ? object->facade->currentModel() : nullptr);
}

PyObject* simulatorNewModel(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createModelObject(object->facade != nullptr ? object->facade->newModel() : nullptr);
}

PyObject* simulatorModelCount(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->modelCount() : 0u);
}

PyObject* simulatorModelAt(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	unsigned long index = 0ul;
	if (!PyArg_ParseTuple(args, "k", &index)) {
		return nullptr;
	}
	return createModelObject(object->facade != nullptr ? object->facade->modelAt(static_cast<unsigned int>(index)) : nullptr);
}

PyObject* simulatorModels(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createModelList(object->facade != nullptr ? object->facade->models() : std::vector<Model*>{});
}

PyObject* simulatorSetCurrentModel(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	if (candidate == Py_None) {
		return PyBool_FromLong(0);
	}
	if (!PyObject_TypeCheck(candidate, &PyGenesysModelType)) {
		PyErr_SetString(PyExc_TypeError, "setCurrentModel expects a genesys.Model object");
		return nullptr;
	}
	Model* model = reinterpret_cast<PyGenesysModelObject*>(candidate)->model;
	return PyBool_FromLong(object->facade != nullptr && object->facade->setCurrentModel(model));
}

PyObject* simulatorInfoGetName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoGetName() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoSetName(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* value = nullptr;
	if (!PyArg_ParseTuple(args, "s", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetName(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorInfoGetAnalystName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoGetAnalystName() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoSetAnalystName(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* value = nullptr;
	if (!PyArg_ParseTuple(args, "s", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetAnalystName(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorInfoGetProjectTitle(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoGetProjectTitle() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoSetProjectTitle(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* value = nullptr;
	if (!PyArg_ParseTuple(args, "s", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetProjectTitle(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorInfoGetVersion(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoGetVersion() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoSetVersion(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* value = nullptr;
	if (!PyArg_ParseTuple(args, "s", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetVersion(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorInfoShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->infoShow() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorInfoHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	Model* current = object->facade != nullptr ? object->facade->currentModel() : nullptr;
	return PyBool_FromLong(current != nullptr && current->getInfos() != nullptr && current->getInfos()->hasChanged());
}

PyObject* simulatorInfoSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->infoSetHasChanged(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimSetNumberOfReplications(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	unsigned long value = 0ul;
	if (!PyArg_ParseTuple(args, "k", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetNumberOfReplications(static_cast<unsigned int>(value));
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetNumberOfReplications(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->simGetNumberOfReplications() : 0u);
}

PyObject* simulatorSimSetReplicationLength(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	double value = 0.0;
	if (!PyArg_ParseTuple(args, "d", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetReplicationLength(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetReplicationLength(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyFloat_FromDouble(object->facade != nullptr ? object->facade->simGetReplicationLength() : 0.0);
}

PyObject* simulatorSimSetWarmUpPeriod(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	double value = 0.0;
	if (!PyArg_ParseTuple(args, "d", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetWarmUpPeriod(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetWarmUpPeriod(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyFloat_FromDouble(object->facade != nullptr ? object->facade->simGetWarmUpPeriod() : 0.0);
}

PyObject* simulatorSimSetReplicationLengthTimeUnit(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	unsigned long value = 0ul;
	if (!PyArg_ParseTuple(args, "k", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetReplicationLengthTimeUnit(static_cast<Util::TimeUnit>(value));
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetReplicationLengthTimeUnit(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? static_cast<unsigned long>(object->facade->simGetReplicationLengthTimeUnit()) : 0u);
}

PyObject* simulatorSimSetReplicationReportBaseTimeUnit(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	unsigned long value = 0ul;
	if (!PyArg_ParseTuple(args, "k", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetReplicationReportBaseTimeUnit(static_cast<Util::TimeUnit>(value));
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetReplicationBaseTimeUnit(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? static_cast<unsigned long>(object->facade->simGetReplicationBaseTimeUnit()) : 0u);
}

PyObject* simulatorSimSetWarmUpPeriodTimeUnit(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	unsigned long value = 0ul;
	if (!PyArg_ParseTuple(args, "k", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetWarmUpPeriodTimeUnit(static_cast<Util::TimeUnit>(value));
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetWarmUpPeriodTimeUnit(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? static_cast<unsigned long>(object->facade->simGetWarmUpPeriodTimeUnit()) : 0u);
}

PyObject* simulatorSimSetTerminatingCondition(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* value = nullptr;
	if (!PyArg_ParseTuple(args, "s", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetTerminatingCondition(value);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimGetTerminatingCondition(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->simGetTerminatingCondition() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorSimIsRunning(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsRunning());
}

PyObject* simulatorSimIsPaused(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsPaused());
}

PyObject* simulatorSimGetCurrentReplicationNumber(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->simGetCurrentReplicationNumber() : 0u);
}

PyObject* simulatorSimGetBreakpointsOnTime(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createDoubleList(object->facade != nullptr ? object->facade->simGetBreakpointsOnTime() : nullptr);
}

PyObject* simulatorSimGetBreakpointsOnEntity(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const List<Entity*>* values = object->facade != nullptr ? object->facade->simGetBreakpointsOnEntity() : nullptr;
	return createPythonListFromContainer(values != nullptr ? values->list() : nullptr,
	                                    [](Entity* entity) { return createEntityObject(entity); });
}

PyObject* simulatorSimGetBreakpointsOnComponent(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const List<ModelComponent*>* values = object->facade != nullptr ? object->facade->simGetBreakpointsOnComponent() : nullptr;
	return createPythonListFromContainer(values != nullptr ? values->list() : nullptr,
	                                    [](ModelComponent* component) { return createComponentObject(component); });
}

PyObject* simulatorSimGetSimulationStatisticsAggregates(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createDataDefinitionList(object->facade != nullptr ? object->facade->simGetSimulationStatisticsAggregates() : nullptr);
}

PyObject* simulatorSimGetCurrentEvent(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createEventObject(object->facade != nullptr ? object->facade->simGetCurrentEvent() : nullptr);
}

PyObject* simulatorSimShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::string value = object->facade != nullptr ? object->facade->simShow() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* simulatorSimSetPauseOnEvent(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetPauseOnEvent(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsPauseOnEvent(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsPauseOnEvent());
}

PyObject* simulatorSimSetStepByStep(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetStepByStep(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsStepByStep(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsStepByStep());
}

PyObject* simulatorSimSetInitializeStatistics(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetInitializeStatistics(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsInitializeStatistics(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsInitializeStatistics());
}

PyObject* simulatorSimSetInitializeSystem(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetInitializeSystem(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsInitializeSystem(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsInitializeSystem());
}

PyObject* simulatorSimSetPauseOnReplication(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetPauseOnReplication(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsPauseOnReplication(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsPauseOnReplication());
}

PyObject* simulatorSimSetShowReportsAfterReplication(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetShowReportsAfterReplication(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsShowReportsAfterReplication(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsShowReportsAfterReplication());
}

PyObject* simulatorSimSetShowReportsAfterSimulation(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetShowReportsAfterSimulation(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsShowReportsAfterSimulation(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsShowReportsAfterSimulation());
}

PyObject* simulatorSimSetShowSimulationResponsesInReport(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetShowSimulationResposesInReport(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsShowSimulationResponsesInReport(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsShowSimulationResposesInReport());
}

PyObject* simulatorSimSetShowSimulationControlsInReport(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->simSetShowSimulationControlsInReport(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorSimIsShowSimulationControlsInReport(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->simIsShowSimulationControlsInReport());
}

PyObject* simulatorTraceGetErrorMessages(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createStringList(object->facade != nullptr ? object->facade->traceGetErrorMessages() : nullptr);
}

PyObject* simulatorModelGetId(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLongLong(object->facade != nullptr ? object->facade->modelGetId() : 0u);
}

PyObject* simulatorModelHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyBool_FromLong(object->facade != nullptr && object->facade->modelHasChanged());
}

PyObject* simulatorModelSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->modelSetHasChanged(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorModelGetLevel(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return PyLong_FromUnsignedLong(object->facade != nullptr ? object->facade->modelGetLevel() : 0u);
}

PyObject* simulatorModelShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	if (object->facade != nullptr) {
		object->facade->modelShow();
	}
	Py_RETURN_NONE;
}

PyObject* simulatorModelClear(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	if (object->facade != nullptr) {
		object->facade->modelClear();
	}
	Py_RETURN_NONE;
}

PyObject* simulatorModelSave(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* filename = nullptr;
	if (!PyArg_ParseTuple(args, "s", &filename)) {
		return nullptr;
	}
	return PyBool_FromLong(object->facade != nullptr && object->facade->modelSave(filename));
}

PyObject* simulatorModelLoad(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* filename = nullptr;
	if (!PyArg_ParseTuple(args, "s", &filename)) {
		return nullptr;
	}
	return PyBool_FromLong(object->facade != nullptr && object->facade->modelLoad(filename));
}

PyObject* simulatorModelInsert(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelDataDefinition* dataDefinition = extractModelDataDefinition(candidate);
	if (dataDefinition == nullptr) {
		PyErr_SetString(PyExc_TypeError, "modelInsert expects a genesys.ModelDataDefinition, genesys.Entity, or genesys.Component object");
		return nullptr;
	}
	return PyBool_FromLong(object->facade != nullptr && object->facade->modelInsert(dataDefinition));
}

PyObject* simulatorModelRemove(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelDataDefinition* dataDefinition = extractModelDataDefinition(candidate);
	if (dataDefinition == nullptr) {
		PyErr_SetString(PyExc_TypeError, "modelRemove expects a genesys.ModelDataDefinition, genesys.Entity, or genesys.Component object");
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->modelRemove(dataDefinition);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorModelCollectDataDefinitionsRemovedWith(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* sequence = nullptr;
	if (!PyArg_ParseTuple(args, "O", &sequence)) {
		return nullptr;
	}
	PyObject* fastSequence = PySequence_Fast(sequence, "modelCollectDataDefinitionsRemovedWith expects a sequence");
	if (fastSequence == nullptr) {
		return nullptr;
	}
	std::list<ModelDataDefinition*> roots;
	const Py_ssize_t count = PySequence_Fast_GET_SIZE(fastSequence);
	for (Py_ssize_t index = 0; index < count; ++index) {
		ModelDataDefinition* dataDefinition = extractModelDataDefinition(PySequence_Fast_GET_ITEM(fastSequence, index));
		if (dataDefinition == nullptr) {
			Py_DECREF(fastSequence);
			PyErr_SetString(PyExc_TypeError, "modelCollectDataDefinitionsRemovedWith expects a sequence of genesys.ModelDataDefinition, genesys.Entity, or genesys.Component objects");
			return nullptr;
		}
		roots.push_back(dataDefinition);
	}
	Py_DECREF(fastSequence);
	const std::list<ModelDataDefinition*> values = object->facade != nullptr ? object->facade->modelCollectDataDefinitionsRemovedWith(roots) : std::list<ModelDataDefinition*>{};
	return createDataDefinitionList(&values);
}

PyObject* simulatorModelGetFutureEvents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	return createEventList(object->facade != nullptr ? object->facade->modelGetFutureEvents() : nullptr);
}

PyObject* simulatorModelGetControls(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const List<SimulationControl*>* values = object->facade != nullptr ? object->facade->modelGetControls() : nullptr;
	return createPythonListFromContainer(values != nullptr ? values->list() : nullptr,
	                                    [](SimulationControl* control) {
		                                    return PyUnicode_FromString(control != nullptr ? control->show().c_str() : "");
	                                    });
}

PyObject* simulatorModelGetResponses(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const List<SimulationResponse*>* values = object->facade != nullptr ? object->facade->modelGetResponses() : nullptr;
	return createPythonListFromContainer(values != nullptr ? values->list() : nullptr,
	                                    [](SimulationResponse* response) {
		                                    return PyUnicode_FromString(response != nullptr ? response->show().c_str() : "");
	                                    });
}

PyObject* simulatorDataInsert(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelDataDefinition* dataDefinition = extractModelDataDefinition(candidate);
	if (dataDefinition == nullptr || PyObject_TypeCheck(candidate, &PyGenesysComponentType)) {
		PyErr_SetString(PyExc_TypeError, "dataInsert expects a genesys.ModelDataDefinition or genesys.Entity object");
		return nullptr;
	}
	return PyBool_FromLong(object->facade != nullptr && object->facade->dataInsert(dataDefinition));
}

PyObject* simulatorDataRemove(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelDataDefinition* dataDefinition = extractModelDataDefinition(candidate);
	if (dataDefinition == nullptr || PyObject_TypeCheck(candidate, &PyGenesysComponentType)) {
		PyErr_SetString(PyExc_TypeError, "dataRemove expects a genesys.ModelDataDefinition or genesys.Entity object");
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->dataRemove(dataDefinition);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorDataClear(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	if (object->facade != nullptr) {
		object->facade->dataClear();
	}
	Py_RETURN_NONE;
}

PyObject* simulatorDataHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	Model* current = object->facade != nullptr ? object->facade->currentModel() : nullptr;
	return PyBool_FromLong(current != nullptr && current->getDataManager() != nullptr && current->getDataManager()->hasChanged());
}

PyObject* simulatorDataSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->dataSetHasChanged(value != 0);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorDataGetDataDefinitionClassnames(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::list<std::string> values = object->facade != nullptr ? object->facade->dataGetDataDefinitionClassnames() : std::list<std::string>{};
	return createStdStringList(&values);
}

PyObject* simulatorDataGetDataDefinitionList(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* typeName = nullptr;
	if (!PyArg_ParseTuple(args, "s", &typeName)) {
		return nullptr;
	}
	const List<ModelDataDefinition*>* values = object->facade != nullptr ? object->facade->dataGetDataDefinitionList(typeName) : nullptr;
	return createDataDefinitionList(values);
}

PyObject* simulatorDataGetRankOf(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const char* typeName = nullptr;
	const char* name = nullptr;
	if (!PyArg_ParseTuple(args, "ss", &typeName, &name)) {
		return nullptr;
	}
	return PyLong_FromLong(object->facade != nullptr ? object->facade->dataGetRankOf(typeName, name) : -1);
}

PyObject* simulatorComponentGetSourceComponents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::list<SourceModelComponent*>* values = object->facade != nullptr ? object->facade->componentGetSourceComponents() : nullptr;
	return createPythonListFromContainer(values, [](SourceModelComponent* component) { return createComponentObject(component); });
}

PyObject* simulatorComponentGetTransferInComponents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::list<ModelComponent*>* values = object->facade != nullptr ? object->facade->componentGetTransferInComponents() : nullptr;
	return createComponentList(values);
}

PyObject* simulatorComponentGetAllComponents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	const std::list<ModelComponent*>* values = object->facade != nullptr ? object->facade->componentGetAllComponents() : nullptr;
	return createComponentList(values);
}

PyObject* simulatorComponentInsert(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelComponent* component = extractModelComponent(candidate);
	if (component == nullptr) {
		PyErr_SetString(PyExc_TypeError, "componentInsert expects a genesys.Component object");
		return nullptr;
	}
	return PyBool_FromLong(object->facade != nullptr && object->facade->componentInsert(component));
}

PyObject* simulatorComponentRemove(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	PyObject* candidate = nullptr;
	if (!PyArg_ParseTuple(args, "O", &candidate)) {
		return nullptr;
	}
	ModelComponent* component = extractModelComponent(candidate);
	if (component == nullptr) {
		PyErr_SetString(PyExc_TypeError, "componentRemove expects a genesys.Component object");
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->componentRemove(component);
	}
	Py_RETURN_NONE;
}

PyObject* simulatorComponentClear(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	if (object->facade != nullptr) {
		object->facade->componentClear();
	}
	Py_RETURN_NONE;
}

PyObject* simulatorComponentHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	Model* current = object->facade != nullptr ? object->facade->currentModel() : nullptr;
	return PyBool_FromLong(current != nullptr && current->getComponentManager() != nullptr && current->getComponentManager()->hasChanged());
}

PyObject* simulatorComponentSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(self);
	int value = 0;
	if (!PyArg_ParseTuple(args, "p", &value)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->componentSetHasChanged(value != 0);
	}
	Py_RETURN_NONE;
}

PyMethodDef simulatorMethods[] = {
	{"getVersion", simulatorGetVersion, METH_NOARGS, nullptr},
	{"getVersionNumber", simulatorGetVersionNumber, METH_NOARGS, nullptr},
	{"getName", simulatorGetName, METH_NOARGS, nullptr},
	{"currentModel", simulatorCurrentModel, METH_NOARGS, nullptr},
	{"newModel", simulatorNewModel, METH_NOARGS, nullptr},
	{"modelCount", simulatorModelCount, METH_NOARGS, nullptr},
	{"modelAt", simulatorModelAt, METH_VARARGS, nullptr},
	{"models", simulatorModels, METH_NOARGS, nullptr},
	{"setCurrentModel", simulatorSetCurrentModel, METH_VARARGS, nullptr},
	{"simGetSimulatedTime", simulatorSimGetSimulatedTime, METH_NOARGS, nullptr},
	{"modelCheck", simulatorModelCheck, METH_NOARGS, nullptr},
	{"modelShowLanguage", simulatorModelShowLanguage, METH_NOARGS, nullptr},
	{"infoGetName", simulatorInfoGetName, METH_NOARGS, nullptr},
	{"infoSetName", simulatorInfoSetName, METH_VARARGS, nullptr},
	{"infoGetAnalystName", simulatorInfoGetAnalystName, METH_NOARGS, nullptr},
	{"infoSetAnalystName", simulatorInfoSetAnalystName, METH_VARARGS, nullptr},
	{"infoGetDescription", simulatorInfoGetDescription, METH_NOARGS, nullptr},
	{"infoSetDescription", simulatorInfoSetDescription, METH_VARARGS, nullptr},
	{"infoGetProjectTitle", simulatorInfoGetProjectTitle, METH_NOARGS, nullptr},
	{"infoSetProjectTitle", simulatorInfoSetProjectTitle, METH_VARARGS, nullptr},
	{"infoGetVersion", simulatorInfoGetVersion, METH_NOARGS, nullptr},
	{"infoSetVersion", simulatorInfoSetVersion, METH_VARARGS, nullptr},
	{"infoShow", simulatorInfoShow, METH_NOARGS, nullptr},
	{"infoHasChanged", simulatorInfoHasChanged, METH_NOARGS, nullptr},
	{"infoSetHasChanged", simulatorInfoSetHasChanged, METH_VARARGS, nullptr},
	{"simSetNumberOfReplications", simulatorSimSetNumberOfReplications, METH_VARARGS, nullptr},
	{"simGetNumberOfReplications", simulatorSimGetNumberOfReplications, METH_NOARGS, nullptr},
	{"simSetReplicationLength", simulatorSimSetReplicationLength, METH_VARARGS, nullptr},
	{"simGetReplicationLength", simulatorSimGetReplicationLength, METH_NOARGS, nullptr},
	{"simSetReplicationLengthTimeUnit", simulatorSimSetReplicationLengthTimeUnit, METH_VARARGS, nullptr},
	{"simGetReplicationLengthTimeUnit", simulatorSimGetReplicationLengthTimeUnit, METH_NOARGS, nullptr},
	{"simSetReplicationReportBaseTimeUnit", simulatorSimSetReplicationReportBaseTimeUnit, METH_VARARGS, nullptr},
	{"simGetReplicationBaseTimeUnit", simulatorSimGetReplicationBaseTimeUnit, METH_NOARGS, nullptr},
	{"simSetWarmUpPeriod", simulatorSimSetWarmUpPeriod, METH_VARARGS, nullptr},
	{"simGetWarmUpPeriod", simulatorSimGetWarmUpPeriod, METH_NOARGS, nullptr},
	{"simSetWarmUpPeriodTimeUnit", simulatorSimSetWarmUpPeriodTimeUnit, METH_VARARGS, nullptr},
	{"simGetWarmUpPeriodTimeUnit", simulatorSimGetWarmUpPeriodTimeUnit, METH_NOARGS, nullptr},
	{"simSetTerminatingCondition", simulatorSimSetTerminatingCondition, METH_VARARGS, nullptr},
	{"simGetTerminatingCondition", simulatorSimGetTerminatingCondition, METH_NOARGS, nullptr},
	{"simShow", simulatorSimShow, METH_NOARGS, nullptr},
	{"simSetPauseOnEvent", simulatorSimSetPauseOnEvent, METH_VARARGS, nullptr},
	{"simIsPauseOnEvent", simulatorSimIsPauseOnEvent, METH_NOARGS, nullptr},
	{"simSetStepByStep", simulatorSimSetStepByStep, METH_VARARGS, nullptr},
	{"simIsStepByStep", simulatorSimIsStepByStep, METH_NOARGS, nullptr},
	{"simSetInitializeStatistics", simulatorSimSetInitializeStatistics, METH_VARARGS, nullptr},
	{"simIsInitializeStatistics", simulatorSimIsInitializeStatistics, METH_NOARGS, nullptr},
	{"simSetInitializeSystem", simulatorSimSetInitializeSystem, METH_VARARGS, nullptr},
	{"simIsInitializeSystem", simulatorSimIsInitializeSystem, METH_NOARGS, nullptr},
	{"simSetPauseOnReplication", simulatorSimSetPauseOnReplication, METH_VARARGS, nullptr},
	{"simIsPauseOnReplication", simulatorSimIsPauseOnReplication, METH_NOARGS, nullptr},
	{"simSetShowReportsAfterReplication", simulatorSimSetShowReportsAfterReplication, METH_VARARGS, nullptr},
	{"simIsShowReportsAfterReplication", simulatorSimIsShowReportsAfterReplication, METH_NOARGS, nullptr},
	{"simSetShowReportsAfterSimulation", simulatorSimSetShowReportsAfterSimulation, METH_VARARGS, nullptr},
	{"simIsShowReportsAfterSimulation", simulatorSimIsShowReportsAfterSimulation, METH_NOARGS, nullptr},
	{"simSetShowSimulationResponsesInReport", simulatorSimSetShowSimulationResponsesInReport, METH_VARARGS, nullptr},
	{"simIsShowSimulationResponsesInReport", simulatorSimIsShowSimulationResponsesInReport, METH_NOARGS, nullptr},
	{"simSetShowSimulationControlsInReport", simulatorSimSetShowSimulationControlsInReport, METH_VARARGS, nullptr},
	{"simIsShowSimulationControlsInReport", simulatorSimIsShowSimulationControlsInReport, METH_NOARGS, nullptr},
	{"simIsRunning", simulatorSimIsRunning, METH_NOARGS, nullptr},
	{"simIsPaused", simulatorSimIsPaused, METH_NOARGS, nullptr},
	{"simGetCurrentReplicationNumber", simulatorSimGetCurrentReplicationNumber, METH_NOARGS, nullptr},
	{"simGetBreakpointsOnTime", simulatorSimGetBreakpointsOnTime, METH_NOARGS, nullptr},
	{"simGetBreakpointsOnEntity", simulatorSimGetBreakpointsOnEntity, METH_NOARGS, nullptr},
	{"simGetBreakpointsOnComponent", simulatorSimGetBreakpointsOnComponent, METH_NOARGS, nullptr},
	{"simGetSimulationStatisticsAggregates", simulatorSimGetSimulationStatisticsAggregates, METH_NOARGS, nullptr},
	{"simGetCurrentEvent", simulatorSimGetCurrentEvent, METH_NOARGS, nullptr},
	{"modelGetId", simulatorModelGetId, METH_NOARGS, nullptr},
	{"modelHasChanged", simulatorModelHasChanged, METH_NOARGS, nullptr},
	{"modelSetHasChanged", simulatorModelSetHasChanged, METH_VARARGS, nullptr},
	{"modelGetLevel", simulatorModelGetLevel, METH_NOARGS, nullptr},
	{"modelShow", simulatorModelShow, METH_NOARGS, nullptr},
	{"modelClear", simulatorModelClear, METH_NOARGS, nullptr},
	{"modelSave", simulatorModelSave, METH_VARARGS, nullptr},
	{"modelLoad", simulatorModelLoad, METH_VARARGS, nullptr},
	{"modelInsert", simulatorModelInsert, METH_VARARGS, nullptr},
	{"modelRemove", simulatorModelRemove, METH_VARARGS, nullptr},
	{"modelCollectDataDefinitionsRemovedWith", simulatorModelCollectDataDefinitionsRemovedWith, METH_VARARGS, nullptr},
	{"modelGetFutureEvents", simulatorModelGetFutureEvents, METH_NOARGS, nullptr},
	{"modelGetControls", simulatorModelGetControls, METH_NOARGS, nullptr},
	{"modelGetResponses", simulatorModelGetResponses, METH_NOARGS, nullptr},
	{"dataInsert", simulatorDataInsert, METH_VARARGS, nullptr},
	{"dataRemove", simulatorDataRemove, METH_VARARGS, nullptr},
	{"dataClear", simulatorDataClear, METH_NOARGS, nullptr},
	{"dataHasChanged", simulatorDataHasChanged, METH_NOARGS, nullptr},
	{"dataSetHasChanged", simulatorDataSetHasChanged, METH_VARARGS, nullptr},
	{"dataGetDataDefinitionClassnames", simulatorDataGetDataDefinitionClassnames, METH_NOARGS, nullptr},
	{"dataGetDataDefinitionList", simulatorDataGetDataDefinitionList, METH_VARARGS, nullptr},
	{"dataGetRankOf", simulatorDataGetRankOf, METH_VARARGS, nullptr},
	{"componentInsert", simulatorComponentInsert, METH_VARARGS, nullptr},
	{"componentRemove", simulatorComponentRemove, METH_VARARGS, nullptr},
	{"componentClear", simulatorComponentClear, METH_NOARGS, nullptr},
	{"componentHasChanged", simulatorComponentHasChanged, METH_NOARGS, nullptr},
	{"componentSetHasChanged", simulatorComponentSetHasChanged, METH_VARARGS, nullptr},
	{"componentGetSourceComponents", simulatorComponentGetSourceComponents, METH_NOARGS, nullptr},
	{"componentGetTransferInComponents", simulatorComponentGetTransferInComponents, METH_NOARGS, nullptr},
	{"componentGetAllComponents", simulatorComponentGetAllComponents, METH_NOARGS, nullptr},
	{"trace", simulatorTrace, METH_VARARGS, nullptr},
	{"traceReport", simulatorTraceReport, METH_VARARGS, nullptr},
	{"traceError", simulatorTraceError, METH_VARARGS, nullptr},
	{"traceGetErrorMessages", simulatorTraceGetErrorMessages, METH_NOARGS, nullptr},
	{"componentGetNumberOfComponents", simulatorComponentGetNumberOfComponents, METH_NOARGS, nullptr},
	{"dataGetNumberOfDataDefinitions", simulatorDataGetNumberOfDataDefinitions, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* entityGetName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	return PyUnicode_FromString(object->entity != nullptr ? object->entity->getName().c_str() : "");
}

PyObject* entityGetNumber(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	return PyLong_FromUnsignedLongLong(object->entity != nullptr ? object->entity->entityNumber() : 0u);
}

PyObject* entityGetId(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	return PyLong_FromUnsignedLongLong(object->entity != nullptr ? object->entity->getId() : 0u);
}

PyObject* entityGetEntityTypeName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	const std::string value = object->entity != nullptr ? object->entity->getEntityTypeName() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* entityGetAttributeValue(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	const char* attributeName = nullptr;
	const char* index = "";
	if (!PyArg_ParseTuple(args, "s|s", &attributeName, &index)) {
		return nullptr;
	}
	const double value = object->entity != nullptr ? object->entity->getAttributeValue(attributeName, index) : 0.0;
	return PyFloat_FromDouble(value);
}

PyObject* entitySetAttributeValue(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	const char* attributeName = nullptr;
	double value = 0.0;
	const char* index = "";
	int createIfNotFound = 0;
	if (!PyArg_ParseTuple(args, "sd|sp", &attributeName, &value, &index, &createIfNotFound)) {
		return nullptr;
	}
	if (object->entity != nullptr) {
		object->entity->setAttributeValue(attributeName, value, index, createIfNotFound != 0);
	}
	Py_RETURN_NONE;
}

PyObject* entityShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(self);
	const std::string value = object->entity != nullptr ? object->entity->show() : "";
	return PyUnicode_FromString(value.c_str());
}

PyMethodDef entityMethods[] = {
	{"getName", entityGetName, METH_NOARGS, nullptr},
	{"getId", entityGetId, METH_NOARGS, nullptr},
	{"entityNumber", entityGetNumber, METH_NOARGS, nullptr},
	{"getEntityTypeName", entityGetEntityTypeName, METH_NOARGS, nullptr},
	{"getAttributeValue", entityGetAttributeValue, METH_VARARGS, nullptr},
	{"setAttributeValue", entitySetAttributeValue, METH_VARARGS, nullptr},
	{"show", entityShow, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* eventGetTime(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEventObject*>(self);
	return PyFloat_FromDouble(object->event != nullptr ? object->event->getTime() : 0.0);
}

PyObject* eventGetComponent(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEventObject*>(self);
	return createComponentObject(object->event != nullptr ? object->event->getComponent() : nullptr);
}

PyObject* eventGetEntity(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEventObject*>(self);
	return createEntityObject(object->event != nullptr ? object->event->getEntity() : nullptr);
}

PyObject* eventGetComponentinputPortNumber(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEventObject*>(self);
	return PyLong_FromUnsignedLong(object->event != nullptr ? object->event->getComponentinputPortNumber() : 0u);
}

PyObject* eventShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysEventObject*>(self);
	const std::string value = object->event != nullptr ? object->event->show() : "";
	return PyUnicode_FromString(value.c_str());
}

PyMethodDef eventMethods[] = {
	{"getTime", eventGetTime, METH_NOARGS, nullptr},
	{"getComponent", eventGetComponent, METH_NOARGS, nullptr},
	{"getEntity", eventGetEntity, METH_NOARGS, nullptr},
	{"getComponentinputPortNumber", eventGetComponentinputPortNumber, METH_NOARGS, nullptr},
	{"show", eventShow, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* componentGetName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	return PyUnicode_FromString(object->component != nullptr ? object->component->getName().c_str() : "");
}

PyObject* componentGetClassname(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	return PyUnicode_FromString(object->component != nullptr ? object->component->getClassname().c_str() : "");
}

PyObject* componentGetId(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	return PyLong_FromUnsignedLongLong(object->component != nullptr ? object->component->getId() : 0u);
}

PyObject* componentGetDescription(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	const std::string value = object->component != nullptr ? object->component->getDescription() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* componentSetDescription(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	const char* description = nullptr;
	if (!PyArg_ParseTuple(args, "s", &description)) {
		return nullptr;
	}
	if (object->component != nullptr) {
		object->component->setDescription(description);
	}
	Py_RETURN_NONE;
}

PyObject* componentHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	return PyBool_FromLong(object->component != nullptr && object->component->hasChanged());
}

PyObject* componentSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	int hasChanged = 0;
	if (!PyArg_ParseTuple(args, "p", &hasChanged)) {
		return nullptr;
	}
	if (object->component != nullptr) {
		object->component->setHasChanged(hasChanged != 0);
	}
	Py_RETURN_NONE;
}

PyObject* componentShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(self);
	const std::string value = object->component != nullptr ? object->component->show() : "";
	return PyUnicode_FromString(value.c_str());
}

PyMethodDef componentMethods[] = {
	{"getName", componentGetName, METH_NOARGS, nullptr},
	{"getClassname", componentGetClassname, METH_NOARGS, nullptr},
	{"getId", componentGetId, METH_NOARGS, nullptr},
	{"getDescription", componentGetDescription, METH_NOARGS, nullptr},
	{"setDescription", componentSetDescription, METH_VARARGS, nullptr},
	{"hasChanged", componentHasChanged, METH_NOARGS, nullptr},
	{"setHasChanged", componentSetHasChanged, METH_VARARGS, nullptr},
	{"show", componentShow, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* dataDefinitionGetName(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	return PyUnicode_FromString(object->dataDefinition != nullptr ? object->dataDefinition->getName().c_str() : "");
}

PyObject* dataDefinitionGetClassname(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	return PyUnicode_FromString(object->dataDefinition != nullptr ? object->dataDefinition->getClassname().c_str() : "");
}

PyObject* dataDefinitionGetId(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	return PyLong_FromUnsignedLongLong(object->dataDefinition != nullptr ? object->dataDefinition->getId() : 0u);
}

PyObject* dataDefinitionHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	return PyBool_FromLong(object->dataDefinition != nullptr && object->dataDefinition->hasChanged());
}

PyObject* dataDefinitionSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	int hasChanged = 0;
	if (!PyArg_ParseTuple(args, "p", &hasChanged)) {
		return nullptr;
	}
	if (object->dataDefinition != nullptr) {
		object->dataDefinition->setHasChanged(hasChanged != 0);
	}
	Py_RETURN_NONE;
}

PyObject* dataDefinitionShow(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(self);
	const std::string value = object->dataDefinition != nullptr ? object->dataDefinition->show() : "";
	return PyUnicode_FromString(value.c_str());
}

PyMethodDef dataDefinitionMethods[] = {
	{"getName", dataDefinitionGetName, METH_NOARGS, nullptr},
	{"getClassname", dataDefinitionGetClassname, METH_NOARGS, nullptr},
	{"getId", dataDefinitionGetId, METH_NOARGS, nullptr},
	{"hasChanged", dataDefinitionHasChanged, METH_NOARGS, nullptr},
	{"setHasChanged", dataDefinitionSetHasChanged, METH_VARARGS, nullptr},
	{"show", dataDefinitionShow, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* modelShowLanguage(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const std::string value = object->model != nullptr ? object->model->showLanguage() : "";
	return PyUnicode_FromString(value.c_str());
}

PyObject* modelGetId(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	return PyLong_FromUnsignedLongLong(object->model != nullptr ? object->model->getId() : 0u);
}

PyObject* modelHasChanged(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	return PyBool_FromLong(object->model != nullptr && object->model->hasChanged());
}

PyObject* modelSetHasChanged(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	int hasChanged = 0;
	if (!PyArg_ParseTuple(args, "p", &hasChanged)) {
		return nullptr;
	}
	if (object->model != nullptr) {
		object->model->setHasChanged(hasChanged != 0);
	}
	Py_RETURN_NONE;
}

PyObject* modelCheck(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	return PyBool_FromLong(object->model != nullptr && object->model->check());
}

PyObject* modelClear(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	if (object->model != nullptr) {
		object->model->clear();
	}
	Py_RETURN_NONE;
}

PyObject* modelSave(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* filename = nullptr;
	if (!PyArg_ParseTuple(args, "s", &filename)) {
		return nullptr;
	}
	return PyBool_FromLong(object->model != nullptr && object->model->save(filename));
}

PyObject* modelLoad(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* filename = nullptr;
	if (!PyArg_ParseTuple(args, "s", &filename)) {
		return nullptr;
	}
	return PyBool_FromLong(object->model != nullptr && object->model->load(filename));
}

PyObject* modelParseExpression(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* expression = nullptr;
	if (!PyArg_ParseTuple(args, "s", &expression)) {
		return nullptr;
	}
	const double value = object->model != nullptr ? object->model->parseExpression(expression) : 0.0;
	return PyFloat_FromDouble(value);
}

PyObject* modelParseExpressionDetailed(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* expression = nullptr;
	if (!PyArg_ParseTuple(args, "s", &expression)) {
		return nullptr;
	}
	bool success = false;
	std::string errorMessage;
	const double value = object->model != nullptr ? object->model->parseExpression(expression, success, errorMessage) : 0.0;
	PyObject* result = Py_BuildValue("{s:d,s:O,s:s}",
	                                 "value", value,
	                                 "success", success ? Py_True : Py_False,
	                                 "error", errorMessage.c_str());
	return result;
}

PyObject* modelCreateEntity(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* name = nullptr;
	int insertIntoModel = 1;
	if (!PyArg_ParseTuple(args, "s|p", &name, &insertIntoModel)) {
		return nullptr;
	}
	return createEntityObject(object->model != nullptr ? object->model->createEntity(name, insertIntoModel != 0) : nullptr);
}

PyObject* modelComponentFind(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* name = nullptr;
	if (!PyArg_ParseTuple(args, "s", &name)) {
		return nullptr;
	}
	ModelComponent* component = nullptr;
	if (object->model != nullptr && object->model->getComponentManager() != nullptr) {
		component = object->model->getComponentManager()->find(name);
	}
	return createComponentObject(component);
}

PyObject* modelDataGetDataDefinition(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const char* typeName = nullptr;
	const char* name = nullptr;
	if (!PyArg_ParseTuple(args, "ss", &typeName, &name)) {
		return nullptr;
	}
	ModelDataDefinition* data = nullptr;
	if (object->model != nullptr && object->model->getDataManager() != nullptr) {
		data = object->model->getDataManager()->getDataDefinition(typeName, name);
	}
	return createDataDefinitionObject(data);
}

PyObject* modelCollectDataDefinitionsRemovedWith(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	PyObject* sequence = nullptr;
	if (!PyArg_ParseTuple(args, "O", &sequence)) {
		return nullptr;
	}
	PyObject* fastSequence = PySequence_Fast(sequence, "modelCollectDataDefinitionsRemovedWith expects a sequence");
	if (fastSequence == nullptr) {
		return nullptr;
	}
	std::list<ModelDataDefinition*> roots;
	const Py_ssize_t count = PySequence_Fast_GET_SIZE(fastSequence);
	for (Py_ssize_t index = 0; index < count; ++index) {
		ModelDataDefinition* dataDefinition = extractModelDataDefinition(PySequence_Fast_GET_ITEM(fastSequence, index));
		if (dataDefinition == nullptr) {
			Py_DECREF(fastSequence);
			PyErr_SetString(PyExc_TypeError, "modelCollectDataDefinitionsRemovedWith expects a sequence of genesys.ModelDataDefinition, genesys.Entity, or genesys.Component objects");
			return nullptr;
		}
		roots.push_back(dataDefinition);
	}
	Py_DECREF(fastSequence);
	const std::list<ModelDataDefinition*> values = object->model != nullptr ? object->model->collectDataDefinitionsRemovedWith(roots) : std::list<ModelDataDefinition*>{};
	return createDataDefinitionList(&values);
}

PyObject* modelGetComponentCount(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const unsigned int count = (object->model != nullptr && object->model->getComponentManager() != nullptr)
		? object->model->getComponentManager()->getNumberOfComponents()
		: 0u;
	return PyLong_FromUnsignedLong(count);
}

PyObject* modelGetDataDefinitionCount(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	const unsigned int count = (object->model != nullptr && object->model->getDataManager() != nullptr)
		? object->model->getDataManager()->getNumberOfDataDefinitions()
		: 0u;
	return PyLong_FromUnsignedLong(count);
}

PyObject* modelGetFutureEvents(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysModelObject*>(self);
	return createEventList(object->model != nullptr ? object->model->getFutureEvents() : nullptr);
}

PyMethodDef modelMethods[] = {
	{"getId", modelGetId, METH_NOARGS, nullptr},
	{"hasChanged", modelHasChanged, METH_NOARGS, nullptr},
	{"setHasChanged", modelSetHasChanged, METH_VARARGS, nullptr},
	{"check", modelCheck, METH_NOARGS, nullptr},
	{"clear", modelClear, METH_NOARGS, nullptr},
	{"save", modelSave, METH_VARARGS, nullptr},
	{"load", modelLoad, METH_VARARGS, nullptr},
	{"showLanguage", modelShowLanguage, METH_NOARGS, nullptr},
	{"parseExpression", modelParseExpression, METH_VARARGS, nullptr},
	{"parseExpressionDetailed", modelParseExpressionDetailed, METH_VARARGS, nullptr},
	{"createEntity", modelCreateEntity, METH_VARARGS, nullptr},
	{"componentFind", modelComponentFind, METH_VARARGS, nullptr},
	{"dataGetDataDefinition", modelDataGetDataDefinition, METH_VARARGS, nullptr},
	{"collectDataDefinitionsRemovedWith", modelCollectDataDefinitionsRemovedWith, METH_VARARGS, nullptr},
	{"getFutureEvents", modelGetFutureEvents, METH_NOARGS, nullptr},
	{"getComponentCount", modelGetComponentCount, METH_NOARGS, nullptr},
	{"getDataDefinitionCount", modelGetDataDefinitionCount, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

PyObject* contextLog(PyObject* self, PyObject* args) {
	auto* object = reinterpret_cast<PyGenesysContextObject*>(self);
	const char* text = nullptr;
	if (!PyArg_ParseTuple(args, "s", &text)) {
		return nullptr;
	}
	if (object->facade != nullptr) {
		object->facade->trace(text);
	}
	Py_RETURN_NONE;
}

PyObject* contextSimulatorFacade(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysContextObject*>(self);
	return createSimulatorObject(object->facade);
}

PyObject* contextEntity(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysContextObject*>(self);
	return createEntityObject(object->entity);
}

PyObject* contextComponent(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysContextObject*>(self);
	return createComponentObject(object->component);
}

PyObject* contextModel(PyObject* self, PyObject*) {
	auto* object = reinterpret_cast<PyGenesysContextObject*>(self);
	return createModelObject(object->model);
}

PyMethodDef contextMethods[] = {
	{"log", contextLog, METH_VARARGS, nullptr},
	{"simulator_facade", contextSimulatorFacade, METH_NOARGS, nullptr},
	{"entity", contextEntity, METH_NOARGS, nullptr},
	{"component", contextComponent, METH_NOARGS, nullptr},
	{"model", contextModel, METH_NOARGS, nullptr},
	{nullptr, nullptr, 0, nullptr}
};

bool ensurePythonTypesReady() {
	if (g_pythonTypesReady) {
		return true;
	}

	PyGenesysSimulatorType.tp_name = "genesys.SimulatorFacade";
	PyGenesysSimulatorType.tp_basicsize = sizeof(PyGenesysSimulatorObject);
	PyGenesysSimulatorType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysSimulatorType.tp_methods = simulatorMethods;
	PyGenesysSimulatorType.tp_new = PyType_GenericNew;
	PyGenesysSimulatorType.tp_dealloc = simulatorDealloc;

	PyGenesysContextType.tp_name = "genesys.Context";
	PyGenesysContextType.tp_basicsize = sizeof(PyGenesysContextObject);
	PyGenesysContextType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysContextType.tp_methods = contextMethods;
	PyGenesysContextType.tp_new = PyType_GenericNew;

	PyGenesysEntityType.tp_name = "genesys.Entity";
	PyGenesysEntityType.tp_basicsize = sizeof(PyGenesysEntityObject);
	PyGenesysEntityType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysEntityType.tp_methods = entityMethods;
	PyGenesysEntityType.tp_new = PyType_GenericNew;

	PyGenesysEventType.tp_name = "genesys.Event";
	PyGenesysEventType.tp_basicsize = sizeof(PyGenesysEventObject);
	PyGenesysEventType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysEventType.tp_methods = eventMethods;
	PyGenesysEventType.tp_new = PyType_GenericNew;

	PyGenesysComponentType.tp_name = "genesys.Component";
	PyGenesysComponentType.tp_basicsize = sizeof(PyGenesysComponentObject);
	PyGenesysComponentType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysComponentType.tp_methods = componentMethods;
	PyGenesysComponentType.tp_new = PyType_GenericNew;

	PyGenesysDataDefinitionType.tp_name = "genesys.ModelDataDefinition";
	PyGenesysDataDefinitionType.tp_basicsize = sizeof(PyGenesysDataDefinitionObject);
	PyGenesysDataDefinitionType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysDataDefinitionType.tp_methods = dataDefinitionMethods;
	PyGenesysDataDefinitionType.tp_new = PyType_GenericNew;

	PyGenesysModelType.tp_name = "genesys.Model";
	PyGenesysModelType.tp_basicsize = sizeof(PyGenesysModelObject);
	PyGenesysModelType.tp_flags = Py_TPFLAGS_DEFAULT;
	PyGenesysModelType.tp_methods = modelMethods;
	PyGenesysModelType.tp_new = PyType_GenericNew;

	if (PyType_Ready(&PyGenesysSimulatorType) < 0 ||
	    PyType_Ready(&PyGenesysContextType) < 0 ||
	    PyType_Ready(&PyGenesysEntityType) < 0 ||
	    PyType_Ready(&PyGenesysEventType) < 0 ||
	    PyType_Ready(&PyGenesysComponentType) < 0 ||
	    PyType_Ready(&PyGenesysDataDefinitionType) < 0 ||
	    PyType_Ready(&PyGenesysModelType) < 0) {
		return false;
	}

	g_pythonTypesReady = true;
	return true;
}

template<typename T>
PyObject* allocateObject(PyTypeObject* type) {
	auto* object = PyObject_New(T, type);
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createSimulatorObject(SimulatorFacade* facade, bool ownsFacade) {
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysSimulatorObject*>(allocateObject<PyGenesysSimulatorObject>(&PyGenesysSimulatorType));
	if (object == nullptr) {
		return nullptr;
	}
	object->facade = facade;
	object->ownsFacade = ownsFacade;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createContextObject(SimulatorFacade* facade, Model* model, ModelComponent* component, Entity* entity) {
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysContextObject*>(allocateObject<PyGenesysContextObject>(&PyGenesysContextType));
	if (object == nullptr) {
		return nullptr;
	}
	object->facade = facade;
	object->model = model;
	object->component = component;
	object->entity = entity;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createEntityObject(Entity* entity) {
	if (entity == nullptr) {
		Py_RETURN_NONE;
	}
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysEntityObject*>(allocateObject<PyGenesysEntityObject>(&PyGenesysEntityType));
	if (object == nullptr) {
		return nullptr;
	}
	object->entity = entity;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createEventObject(Event* event) {
	if (event == nullptr) {
		Py_RETURN_NONE;
	}
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysEventObject*>(allocateObject<PyGenesysEventObject>(&PyGenesysEventType));
	if (object == nullptr) {
		return nullptr;
	}
	object->event = event;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createComponentObject(ModelComponent* component) {
	if (component == nullptr) {
		Py_RETURN_NONE;
	}
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysComponentObject*>(allocateObject<PyGenesysComponentObject>(&PyGenesysComponentType));
	if (object == nullptr) {
		return nullptr;
	}
	object->component = component;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createDataDefinitionObject(ModelDataDefinition* dataDefinition) {
	if (dataDefinition == nullptr) {
		Py_RETURN_NONE;
	}
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysDataDefinitionObject*>(allocateObject<PyGenesysDataDefinitionObject>(&PyGenesysDataDefinitionType));
	if (object == nullptr) {
		return nullptr;
	}
	object->dataDefinition = dataDefinition;
	return reinterpret_cast<PyObject*>(object);
}

PyObject* createModelObject(Model* model) {
	if (model == nullptr) {
		Py_RETURN_NONE;
	}
	if (!ensurePythonTypesReady()) {
		return nullptr;
	}
	auto* object = reinterpret_cast<PyGenesysModelObject*>(allocateObject<PyGenesysModelObject>(&PyGenesysModelType));
	if (object == nullptr) {
		return nullptr;
	}
	object->model = model;
	return reinterpret_cast<PyObject*>(object);
}

class PythonGilLock {
public:
	PythonGilLock() : _state(PyGILState_Ensure()) {}
	~PythonGilLock() { PyGILState_Release(_state); }
private:
	PyGILState_STATE _state;
};

bool ensureInterpreterInitialized(std::string& errorMessage) {
	if (Py_IsInitialized()) {
		return ensurePythonTypesReady();
	}
	Py_Initialize();
	if (!Py_IsInitialized()) {
		errorMessage = "Could not initialize embedded Python interpreter.";
		return false;
	}
	return ensurePythonTypesReady();
}

std::string buildHookModuleSource(const std::string& initCode,
                                  const std::string& onDispatchCode) {
	auto indentBlock = [](const std::string& code) {
		if (code.empty()) {
			return std::string("    pass\n");
		}
		std::ostringstream indented;
		std::istringstream input(code);
		std::string line;
		bool any = false;
		while (std::getline(input, line)) {
			indented << "    " << line << "\n";
			any = true;
		}
		if (!any) {
			indented << "    pass\n";
		}
		return indented.str();
	};

	std::ostringstream source;
	source << "def __genesys_init(context, model, component=None):\n";
	source << indentBlock(initCode);
	source << "\n";
	source << "def __genesys_on_dispatch(context, model, component, entity):\n";
	source << indentBlock(onDispatchCode);
	return source.str();
}

bool beginCapture(bool enabled,
                  PyObject** previousStdout,
                  PyObject** previousStderr,
                  PyObject** captureStdout,
                  PyObject** captureStderr,
                  std::string& errorMessage) {
	*previousStdout = nullptr;
	*previousStderr = nullptr;
	*captureStdout = nullptr;
	*captureStderr = nullptr;
	if (!enabled) {
		return true;
	}

	PyObject* ioModule = PyImport_ImportModule("io");
	if (ioModule == nullptr) {
		errorMessage = formatPythonException();
		return false;
	}
	PyObject* stringIoClass = PyObject_GetAttrString(ioModule, "StringIO");
	Py_DECREF(ioModule);
	if (stringIoClass == nullptr) {
		errorMessage = formatPythonException();
		return false;
	}
	*captureStdout = PyObject_CallObject(stringIoClass, nullptr);
	*captureStderr = PyObject_CallObject(stringIoClass, nullptr);
	Py_DECREF(stringIoClass);
	if (*captureStdout == nullptr || *captureStderr == nullptr) {
		Py_XDECREF(*captureStdout);
		Py_XDECREF(*captureStderr);
		errorMessage = formatPythonException();
		return false;
	}

	*previousStdout = PySys_GetObject("stdout");
	*previousStderr = PySys_GetObject("stderr");
	Py_XINCREF(*previousStdout);
	Py_XINCREF(*previousStderr);
	if (PySys_SetObject("stdout", *captureStdout) != 0 || PySys_SetObject("stderr", *captureStderr) != 0) {
		errorMessage = formatPythonException();
		return false;
	}
	return true;
}

std::string endCapture(PyObject* captureObject) {
	if (captureObject == nullptr) {
		return "";
	}
	PyObject* text = PyObject_CallMethod(captureObject, "getvalue", nullptr);
	if (text == nullptr) {
		return formatPythonException();
	}
	const char* utf8 = PyUnicode_AsUTF8(text);
	std::string result = utf8 != nullptr ? utf8 : "";
	Py_DECREF(text);
	return result;
}

void restoreCapture(PyObject* previousStdout,
                    PyObject* previousStderr,
                    PyObject* captureStdout,
                    PyObject* captureStderr) {
	if (captureStdout != nullptr) {
		PySys_SetObject("stdout", previousStdout != nullptr ? previousStdout : Py_None);
	}
	if (captureStderr != nullptr) {
		PySys_SetObject("stderr", previousStderr != nullptr ? previousStderr : Py_None);
	}
	Py_XDECREF(previousStdout);
	Py_XDECREF(previousStderr);
	Py_XDECREF(captureStdout);
	Py_XDECREF(captureStderr);
}

} // namespace
#endif

ModelDataDefinition* PythonRuntime::NewInstance(Model* model, std::string name) {
	return new PythonRuntime(model, name);
}

PythonRuntime::PythonRuntime(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<PythonRuntime>(), name) {
	auto* propPythonExecutable = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getPythonExecutable, this), std::bind(&PythonRuntime::setPythonExecutable, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "PythonExecutable", "");
	auto* propFacadeObjectName = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getFacadeObjectName, this), std::bind(&PythonRuntime::setFacadeObjectName, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "FacadeObjectName", "");
	auto* propCaptureOutput = new SimulationControlGeneric<bool>(
			std::bind(&PythonRuntime::isCaptureOutput, this), std::bind(&PythonRuntime::setCaptureOutput, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "CaptureOutput", "");
	auto* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getLastStatus, this), std::bind(&PythonRuntime::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "LastStatus", "");
	auto* propLastErrorMessage = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getLastErrorMessage, this), std::bind(&PythonRuntime::setLastErrorMessage, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "LastErrorMessage", "");
	auto* propLastStdout = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getLastStdout, this), std::bind(&PythonRuntime::setLastStdout, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "LastStdout", "");
	auto* propLastStderr = new SimulationControlGeneric<std::string>(
			std::bind(&PythonRuntime::getLastStderr, this), std::bind(&PythonRuntime::setLastStderr, this, std::placeholders::_1),
			Util::TypeOf<PythonRuntime>(), getName(), "LastStderr", "");

	_parentModel->getControls()->insert(propPythonExecutable);
	_parentModel->getControls()->insert(propFacadeObjectName);
	_parentModel->getControls()->insert(propCaptureOutput);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastErrorMessage);
	_parentModel->getControls()->insert(propLastStdout);
	_parentModel->getControls()->insert(propLastStderr);

	_addSimulationControl(propPythonExecutable);
	_addSimulationControl(propFacadeObjectName);
	_addSimulationControl(propCaptureOutput);
	_addSimulationControl(propLastStatus);
	_addSimulationControl(propLastErrorMessage);
	_addSimulationControl(propLastStdout);
	_addSimulationControl(propLastStderr);
}

PythonRuntime::~PythonRuntime() {
	_invalidatePreparedHooks();
}

PluginInformation* PythonRuntime::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PythonRuntime>(), &PythonRuntime::LoadInstance, &PythonRuntime::NewInstance);
	info->setCategory("ExternalIntegration");
	info->setDescriptionHelp("Experimental embedded Python runtime for PythonForG. It validates hook syntax during model checking, executes init/on_dispatch hooks inside the simulator process, exposes the simulator facade as the global object 'simulator', and captures stdout/stderr when configured.");
	info->setObservation("This integration is experimental, executes trusted model code in-process, and is not a security sandbox.");
	return info;
}

ModelDataDefinition* PythonRuntime::LoadInstance(Model* model, PersistenceRecord* fields) {
	PythonRuntime* newElement = new PythonRuntime(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load PythonRuntime instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string PythonRuntime::show() {
	return ModelDataDefinition::show()
			+ ",pythonExecutable=\"" + _pythonExecutable + "\""
			+ ",facadeObjectName=\"" + _facadeObjectName + "\""
			+ ",captureOutput=" + std::string(_captureOutput ? "true" : "false")
			+ ",lastStatus=\"" + _lastStatus + "\""
			+ ",lastErrorMessageSize=" + std::to_string(_lastErrorMessage.size())
			+ ",lastStdoutSize=" + std::to_string(_lastStdout.size())
			+ ",lastStderrSize=" + std::to_string(_lastStderr.size());
}

void PythonRuntime::setPythonExecutable(std::string pythonExecutable) {
	_pythonExecutable = std::move(pythonExecutable);
}

std::string PythonRuntime::getPythonExecutable() const {
	return _pythonExecutable;
}

void PythonRuntime::setFacadeObjectName(std::string facadeObjectName) {
	_facadeObjectName = std::move(facadeObjectName);
	_invalidatePreparedHooks();
}

std::string PythonRuntime::getFacadeObjectName() const {
	return _facadeObjectName;
}

void PythonRuntime::setCaptureOutput(bool captureOutput) {
	_captureOutput = captureOutput;
}

bool PythonRuntime::isCaptureOutput() const {
	return _captureOutput;
}

void PythonRuntime::setLastStatus(std::string lastStatus) {
	_lastStatus = std::move(lastStatus);
}

std::string PythonRuntime::getLastStatus() const {
	return _lastStatus;
}

void PythonRuntime::setLastErrorMessage(std::string lastErrorMessage) {
	_lastErrorMessage = std::move(lastErrorMessage);
}

std::string PythonRuntime::getLastErrorMessage() const {
	return _lastErrorMessage;
}

void PythonRuntime::setLastStdout(std::string lastStdout) {
	_lastStdout = std::move(lastStdout);
}

std::string PythonRuntime::getLastStdout() const {
	return _lastStdout;
}

void PythonRuntime::setLastStderr(std::string lastStderr) {
	_lastStderr = std::move(lastStderr);
}

std::string PythonRuntime::getLastStderr() const {
	return _lastStderr;
}

bool PythonRuntime::_validateIdentifier(std::string& errorMessage) const {
	static const std::regex identifierPattern("^[A-Za-z_][A-Za-z0-9_]*$");
	if (_facadeObjectName.empty()) {
		errorMessage += "FacadeObjectName must not be empty. ";
		return false;
	}
	if (!std::regex_match(_facadeObjectName, identifierPattern)) {
		errorMessage += "FacadeObjectName must be a valid Python identifier. ";
		return false;
	}
	return true;
}

bool PythonRuntime::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_pythonExecutable.empty()) {
		errorMessage += "PythonExecutable must not be empty. ";
		resultAll = false;
	}
	resultAll &= _validateIdentifier(errorMessage);
#if !GENESYS_HAS_PYTHON_INTEGRATION
	errorMessage += "Experimental Python integration is not available in this build. Reconfigure with GENESYS_ENABLE_PYTHON_INTEGRATION=ON and Python3 Development.Embed available. ";
	return false;
#else
	return resultAll;
#endif
}

bool PythonRuntime::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_pythonExecutable = fields->loadField("pythonExecutable", DEFAULT.pythonExecutable);
		_facadeObjectName = fields->loadField("facadeObjectName", DEFAULT.facadeObjectName);
		_captureOutput = fields->loadField("captureOutput", DEFAULT.captureOutput);
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastErrorMessage = fields->loadField("lastErrorMessage", DEFAULT.lastErrorMessage);
		_lastStdout = fields->loadField("lastStdout", DEFAULT.lastStdout);
		_lastStderr = fields->loadField("lastStderr", DEFAULT.lastStderr);
		_invalidatePreparedHooks();
	}
	return res;
}

void PythonRuntime::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("pythonExecutable", _pythonExecutable, DEFAULT.pythonExecutable, saveDefaultValues);
	fields->saveField("facadeObjectName", _facadeObjectName, DEFAULT.facadeObjectName, saveDefaultValues);
	fields->saveField("captureOutput", _captureOutput, DEFAULT.captureOutput, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastErrorMessage", _lastErrorMessage, DEFAULT.lastErrorMessage, saveDefaultValues);
	fields->saveField("lastStdout", _lastStdout, DEFAULT.lastStdout, saveDefaultValues);
	fields->saveField("lastStderr", _lastStderr, DEFAULT.lastStderr, saveDefaultValues);
}

void PythonRuntime::resetExecutionState() {
	_lastStatus = DEFAULT.lastStatus;
	_lastErrorMessage.clear();
	_lastStdout.clear();
	_lastStderr.clear();
}

bool PythonRuntime::validateHooks(const std::string& initCode,
                                  const std::string& onDispatchCode,
                                  std::string& errorMessage) const {
	errorMessage.clear();
	std::string runtimeCheckMessage;
	if (!const_cast<PythonRuntime*>(this)->_check(runtimeCheckMessage)) {
		errorMessage = runtimeCheckMessage;
		return false;
	}
#if !GENESYS_HAS_PYTHON_INTEGRATION
	return false;
#else
	std::string initError = runtimeCheckMessage;
	if (!ensureInterpreterInitialized(initError)) {
		errorMessage = initError;
		return false;
	}
	PythonGilLock gil;
	const std::string source = buildHookModuleSource(initCode, onDispatchCode);
	PyObject* code = Py_CompileString(source.c_str(), (getName() + ".py").c_str(), Py_file_input);
	if (code == nullptr) {
		errorMessage = formatPythonException();
		return false;
	}
	Py_DECREF(code);
	return true;
#endif
}

void PythonRuntime::_invalidatePreparedHooks() {
#if GENESYS_HAS_PYTHON_INTEGRATION
	if (!Py_IsInitialized()) {
		_preparedGlobals = nullptr;
		_preparedInitFunction = nullptr;
		_preparedDispatchFunction = nullptr;
		_preparedFacadeObject = nullptr;
		_preparedSignature.clear();
		return;
	}
	PythonGilLock gil;
	Py_XDECREF(reinterpret_cast<PyObject*>(_preparedGlobals));
	Py_XDECREF(reinterpret_cast<PyObject*>(_preparedInitFunction));
	Py_XDECREF(reinterpret_cast<PyObject*>(_preparedDispatchFunction));
	Py_XDECREF(reinterpret_cast<PyObject*>(_preparedFacadeObject));
	_preparedGlobals = nullptr;
	_preparedInitFunction = nullptr;
	_preparedDispatchFunction = nullptr;
	_preparedFacadeObject = nullptr;
#endif
	_preparedSignature.clear();
}

bool PythonRuntime::_prepareHooks(ModelComponent* component,
                                  const std::string& initCode,
                                  const std::string& onDispatchCode,
                                  std::string& errorMessage) {
	errorMessage.clear();
#if !GENESYS_HAS_PYTHON_INTEGRATION
	errorMessage = "Python integration is not available in this build.";
	_lastStatus = "Unavailable";
	_lastErrorMessage = errorMessage;
	return false;
#else
	std::ostringstream signature;
	signature << _facadeObjectName << '\n' << initCode << "\n---\n" << onDispatchCode;
	if (_preparedGlobals != nullptr && _preparedSignature == signature.str()) {
		return true;
	}

	std::string initError;
	if (!ensureInterpreterInitialized(initError)) {
		errorMessage = initError;
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	PythonGilLock gil;
	_invalidatePreparedHooks();

	PyObject* globals = PyDict_New();
	if (globals == nullptr) {
		errorMessage = formatPythonException();
		return false;
	}
	if (PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins()) != 0) {
		Py_DECREF(globals);
		errorMessage = formatPythonException();
		return false;
	}

	PyObject* simulatorObject = createSimulatorObject(new SimulatorFacade(_parentModel->getParentSimulator()), true);
	if (simulatorObject == nullptr) {
		Py_DECREF(globals);
		errorMessage = formatPythonException();
		return false;
	}
	if (PyDict_SetItemString(globals, _facadeObjectName.c_str(), simulatorObject) != 0) {
		Py_DECREF(simulatorObject);
		Py_DECREF(globals);
		errorMessage = formatPythonException();
		return false;
	}

	const std::string source = buildHookModuleSource(initCode, onDispatchCode);
	const std::string filename = (component != nullptr ? component->getName() : getName()) + ".py";
	PyObject* code = Py_CompileString(source.c_str(), filename.c_str(), Py_file_input);
	if (code == nullptr) {
		Py_DECREF(simulatorObject);
		Py_DECREF(globals);
		errorMessage = formatPythonException();
		return false;
	}

	PyObject* evalResult = PyEval_EvalCode(code, globals, globals);
	Py_DECREF(code);
	Py_XDECREF(evalResult);
	if (PyErr_Occurred() != nullptr) {
		errorMessage = formatPythonException();
		Py_DECREF(simulatorObject);
		Py_DECREF(globals);
		return false;
	}

	PyObject* initFunction = PyDict_GetItemString(globals, "__genesys_init");
	PyObject* dispatchFunction = PyDict_GetItemString(globals, "__genesys_on_dispatch");
	if (initFunction == nullptr || dispatchFunction == nullptr) {
		errorMessage = "Generated Python hooks were not found after compiling the script.";
		Py_DECREF(simulatorObject);
		Py_DECREF(globals);
		return false;
	}

	Py_INCREF(initFunction);
	Py_INCREF(dispatchFunction);

	_preparedGlobals = globals;
	_preparedInitFunction = initFunction;
	_preparedDispatchFunction = dispatchFunction;
	_preparedFacadeObject = simulatorObject;
	_preparedSignature = signature.str();
	return true;
#endif
}

bool PythonRuntime::executeInitHook(ModelComponent* component,
                                    const std::string& initCode,
                                    const std::string& onDispatchCode,
                                    std::string& errorMessage) {
	errorMessage.clear();
	_lastStdout.clear();
	_lastStderr.clear();
	if (!_prepareHooks(component, initCode, onDispatchCode, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}
#if !GENESYS_HAS_PYTHON_INTEGRATION
	return false;
#else
	PythonGilLock gil;
	PyObject* previousStdout = nullptr;
	PyObject* previousStderr = nullptr;
	PyObject* captureStdout = nullptr;
	PyObject* captureStderr = nullptr;
	if (!beginCapture(_captureOutput, &previousStdout, &previousStderr, &captureStdout, &captureStderr, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	SimulatorFacade* facade = reinterpret_cast<PyGenesysSimulatorObject*>(_preparedFacadeObject)->facade;
	PyObject* contextObject = createContextObject(facade, _parentModel, component, nullptr);
	PyObject* modelObject = createModelObject(_parentModel);
	PyObject* componentObject = createComponentObject(component);
	PyObject* result = PyObject_CallFunctionObjArgs(reinterpret_cast<PyObject*>(_preparedInitFunction),
	                                               contextObject,
	                                               modelObject,
	                                               componentObject,
	                                               nullptr);
	Py_XDECREF(contextObject);
	Py_XDECREF(modelObject);
	Py_XDECREF(componentObject);

	const bool success = result != nullptr;
	Py_XDECREF(result);
	if (!success) {
		errorMessage = formatPythonException();
	}

	_lastStdout = endCapture(captureStdout);
	_lastStderr = endCapture(captureStderr);
	restoreCapture(previousStdout, previousStderr, captureStdout, captureStderr);

	_lastStatus = success ? "Succeeded" : "Failed";
	_lastErrorMessage = success ? std::string("") : errorMessage;
	return success;
#endif
}

bool PythonRuntime::executeOnDispatchHook(ModelComponent* component,
                                          Entity* entity,
                                          const std::string& initCode,
                                          const std::string& onDispatchCode,
                                          std::string& errorMessage) {
	errorMessage.clear();
	_lastStdout.clear();
	_lastStderr.clear();
	if (!_prepareHooks(component, initCode, onDispatchCode, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}
#if !GENESYS_HAS_PYTHON_INTEGRATION
	return false;
#else
	PythonGilLock gil;
	PyObject* previousStdout = nullptr;
	PyObject* previousStderr = nullptr;
	PyObject* captureStdout = nullptr;
	PyObject* captureStderr = nullptr;
	if (!beginCapture(_captureOutput, &previousStdout, &previousStderr, &captureStdout, &captureStderr, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	SimulatorFacade* facade = reinterpret_cast<PyGenesysSimulatorObject*>(_preparedFacadeObject)->facade;
	PyObject* contextObject = createContextObject(facade, _parentModel, component, entity);
	PyObject* modelObject = createModelObject(_parentModel);
	PyObject* componentObject = createComponentObject(component);
	PyObject* entityObject = createEntityObject(entity);
	PyObject* result = PyObject_CallFunctionObjArgs(reinterpret_cast<PyObject*>(_preparedDispatchFunction),
	                                               contextObject,
	                                               modelObject,
	                                               componentObject,
	                                               entityObject,
	                                               nullptr);
	Py_XDECREF(contextObject);
	Py_XDECREF(modelObject);
	Py_XDECREF(componentObject);
	Py_XDECREF(entityObject);

	const bool success = result != nullptr;
	Py_XDECREF(result);
	if (!success) {
		errorMessage = formatPythonException();
	}

	_lastStdout = endCapture(captureStdout);
	_lastStderr = endCapture(captureStderr);
	restoreCapture(previousStdout, previousStderr, captureStdout, captureStderr);

	_lastStatus = success ? "Succeeded" : "Failed";
	_lastErrorMessage = success ? std::string("") : errorMessage;
	return success;
#endif
}
