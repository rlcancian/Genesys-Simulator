/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinition.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 21 de Junho de 2018, 19:40
 */

//#include <typeinfo>
#include "ModelDataDefinition.h"
#include <iostream>
#include <cassert>
#include <string>
#include "Model.h"
#include "../../TraitsKernel.h"

//using namespace GenesysKernel;

ModelDataDefinition::ModelDataDefinition(Model* model, std::string thistypename, std::string name,
                                         bool insertIntoModel) : _dataAssociations(this) {
	_id = Util::GenerateNewId(); //GenerateNewIdOfType(thistypename);
	_typename = thistypename;
	_parentModel = model;
	_reportStatistics = TraitsKernel<ModelDataDefinition>::reportStatistics;
	// @ToDo: (importante): shoould be a parameter before insertIntoModel
	name = Util::StrReplace(name, " ", "_");
	if (name == "")
		_name = thistypename + "_" + std::to_string(Util::GenerateNewIdOfType(thistypename));
	else if (name.substr(name.length() - 1, 1) == "%")
		// The "%" as suffix (last char means it will be replaced by a new ID
		_name = name.substr(0, name.length() - 1) + std::to_string(Util::GenerateNewIdOfType(thistypename));
	else
		_name = name;
	_label = "";
	_hasChanged = false;
	if (insertIntoModel) {
		model->insert(this);
	}
	// make "name" a property of a component
	SimulationControlGeneric<std::string>* propName = new SimulationControlGeneric<std::string>(
		std::bind(&ModelDataDefinition::getName, this),
		std::bind(&ModelDataDefinition::setName, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Name", "");
	SimulationControlGeneric<std::string>* propLabel = new SimulationControlGeneric<std::string>(
		std::bind(&ModelDataDefinition::getLabel, this),
		std::bind(&ModelDataDefinition::setLabel, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Label", "");
	SimulationControlGeneric<bool>* propReportStatistics = new SimulationControlGeneric<bool>(
		std::bind(&ModelDataDefinition::isReportStatistics, this),
		std::bind(&ModelDataDefinition::setReportStatistics, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Report Statistics", "");
	auto* propTraceLevel = new SimulationControlGenericEnum<TraceManager::Level, TraceManager>(
		std::bind(&ModelDataDefinition::getTraceLevelSpecific, this),
		std::bind(&ModelDataDefinition::setTraceLevelSpecific, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Trace Level", "",
		false, false, true);

	_parentModel->getControls()->insert(propName);
	_parentModel->getControls()->insert(propLabel);
	_parentModel->getControls()->insert(propReportStatistics);
	_parentModel->getControls()->insert(propTraceLevel);
	// setting properties
	_addSimulationControl(propName);
	_addSimulationControl(propLabel);
	_addSimulationControl(propReportStatistics);
	_addSimulationControl(propTraceLevel);
}

bool ModelDataDefinition::hasChanged() const {
	return _hasChanged;
}

void ModelDataDefinition::setHasChanged(bool hasChanged) {
	_hasChanged = hasChanged;
}

unsigned int ModelDataDefinition::getLevel() const {
	return _modelLevel;
}

void ModelDataDefinition::setModelLevel(unsigned int _modelLevel) {
	this->_modelLevel = _modelLevel;
}

ModelDataDefinition::~ModelDataDefinition() {
	// Release all owned internal modeldata definitions registered by this model element.
	_dataAssociations.clearAll();
	// Keep model registry consistent by removing this modeldata from the manager first.
	_parentModel->getDataManager()->remove(this);
	// Detach and destroy owned SimulationControl entries tracked by this model element.
	if (_simulationControls != nullptr) {
		for (SimulationControl* control : *_simulationControls->list()) {
			if (control == nullptr) {
				continue;
			}
			_parentModel->getControls()->remove(control);
			SimulationResponse* response = dynamic_cast<SimulationResponse*>(control);
			if (response != nullptr) {
				_parentModel->getResponses()->remove(response);
			}
			delete control;
		}
		delete _simulationControls;
		_simulationControls = nullptr;
	}
}

void ModelDataDefinition::_internaStatisticReportersClear() {
	_statisticReportersClear();
}

void ModelDataDefinition::_internalDataClear() {
	_dataAssociations.clearInternalData();
}

void ModelDataDefinition::_internalDataInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.internalDataInsert(key, data);
}

void ModelDataDefinition::_internalDataRemove(const std::string& key) {
	_dataAssociations.internalDataRemove(key);
}

void ModelDataDefinition::_attachedAttributesInsert(const std::vector<std::string>& neededNames) {
	_dataAssociations.attachedAttributesInsert(neededNames);
}

void ModelDataDefinition::_statisticReporterInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertStatisticReporter(key, data);
}

void ModelDataDefinition::_statisticReporterRemove(const std::string& key) {
	_dataAssociations.removeStatisticReporter(key);
}

void ModelDataDefinition::_statisticReportersClear() {
	_dataAssociations.clearStatisticReporters();
}

void ModelDataDefinition::_mandatoryAttachedAttributeInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertMandatoryAttachedAttribute(key, data);
}

void ModelDataDefinition::_mandatoryAttachedAttributeRemove(const std::string& key) {
	_dataAssociations.removeMandatoryAttachedAttribute(key);
}

void ModelDataDefinition::_mandatoryAttachedAttributesClear() {
	_dataAssociations.clearMandatoryAttachedAttributes();
}

void ModelDataDefinition::_mandatoryEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertMandatoryEditableDataDefinition(key, data);
}

void ModelDataDefinition::_mandatoryEditableDataDefinitionRemove(const std::string& key) {
	_dataAssociations.removeMandatoryEditableDataDefinition(key);
}

void ModelDataDefinition::_mandatoryEditableDataDefinitionsClear() {
	_dataAssociations.clearMandatoryEditableDataDefinitions();
}

void ModelDataDefinition::_optionalEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertOptionalEditableDataDefinition(key, data);
}

void ModelDataDefinition::_optionalEditableDataDefinitionRemove(const std::string& key) {
	_dataAssociations.removeOptionalEditableDataDefinition(key);
}

void ModelDataDefinition::_optionalEditableDataDefinitionsClear() {
	_dataAssociations.clearOptionalEditableDataDefinitions();
}

void ModelDataDefinition::_mandatoryNonEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertMandatoryNonEditableDataDefinition(key, data);
}

void ModelDataDefinition::_mandatoryNonEditableDataDefinitionRemove(const std::string& key) {
	_dataAssociations.removeMandatoryNonEditableDataDefinition(key);
}

void ModelDataDefinition::_mandatoryNonEditableDataDefinitionsClear() {
	_dataAssociations.clearMandatoryNonEditableDataDefinitions();
}

void ModelDataDefinition::_optionalNonEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.insertOptionalNonEditableDataDefinition(key, data);
}

void ModelDataDefinition::_optionalNonEditableDataDefinitionRemove(const std::string& key) {
	_dataAssociations.removeOptionalNonEditableDataDefinition(key);
}

void ModelDataDefinition::_optionalNonEditableDataDefinitionsClear() {
	_dataAssociations.clearOptionalNonEditableDataDefinitions();
}

void ModelDataDefinition::_attachedDataInsert(const std::string& key, ModelDataDefinition* data) {
	_dataAssociations.attachedDataInsert(key, data);
}

void ModelDataDefinition::_attachedDataRemove(const std::string& key) {
	_dataAssociations.attachedDataRemove(key);
}

void ModelDataDefinition::_attachedDataClear() {
	_dataAssociations.clearAttachedData();
}

void ModelDataDefinition::_checkCreateAttachedReferencedDataDefinition(const std::string& expression) {
	_dataAssociations.checkCreateAttachedReferencedDataDefinition(expression);
}

bool ModelDataDefinition::_getSaveDefaultsOption() {
	return _parentModel->getPersistence()->getOption(ModelPersistence_if::Options::SAVEDEFAULTS);
}

bool ModelDataDefinition::_loadInstance(PersistenceRecord* fields) {
	int id = fields->loadField("id", -1);
	if (id > 0) this->_id = id;
	else return false;

	setName(fields->loadField("name", ""));
	_label = fields->loadField("label", "");
	this->_reportStatistics = fields->
		loadField("reportStatistics", TraitsKernel<ModelDataDefinition>::reportStatistics);

	return true;
}

void ModelDataDefinition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	fields->saveField("typename", _typename);
	fields->saveField("id", _id);
	fields->saveField("name", _name);
	fields->saveField("label", _label, "", saveDefaultValues);
	fields->saveField("reportStatistics", _reportStatistics, TraitsKernel<ModelDataDefinition>::reportStatistics,
	                  saveDefaultValues);
}

void ModelDataDefinition::_createInternalStatisticReporters() { }

void ModelDataDefinition::_createEditableDataDefinitions() { }

void ModelDataDefinition::_createNonEditableDataDefinitions() { }

void ModelDataDefinition::_createAttachedAttributes() { }

void ModelDataDefinition::_templateCreateInternalStatisticReporters() {
	try {
		_createInternalStatisticReporters();
	}
	catch (const std::exception& e) {
		traceError("Error creating report-statistics data definitions for " + getClassname() + " " + getName(), e);
	}
	catch (...) {
		traceError("Unknown error creating report-statistics data definitions for " + getClassname() + " " +
			getName());
	}
}

void ModelDataDefinition::_templateCreateNonEditableDataDefinitions() {
	try {
		_createNonEditableDataDefinitions();
	}
	catch (const std::exception& e) {
		traceError("Error creating non editable data definitions for " + getClassname() + " " + getName(), e);
	}
	catch (...) {
		traceError("Unknown error creating non editable data definitions for " + getClassname() + " " + getName());
	}
}
void ModelDataDefinition::_templateCreateEditableDataDefinitions() {
	try {
		_createEditableDataDefinitions();
	}
	catch (const std::exception& e) {
		traceError("Error creating editable data definitions for " + getClassname() + " " + getName(), e);
	}
	catch (...) {
		traceError("Unknown error creating editable data definitions for " + getClassname() + " " + getName());
	}
}

void ModelDataDefinition::_templateCreateAttachedAttributes() {
	try {
		_createAttachedAttributes();
	}
	catch (const std::exception& e) {
		traceError("Error creating attributes for " + getClassname() + " " + getName(), e);
	}
	catch (...) {
		traceError("Unknown error creating attribu for " + getClassname() + " " + getName());
	}
}

bool ModelDataDefinition::_check(std::string& errorMessage) {
	errorMessage += "";
	return true; // if there is no ovveride, return true
}

ParserChangesInformation* ModelDataDefinition::_getParserChangesInformation() {
	return new ParserChangesInformation(); // if there is no override, return no changes
}

void ModelDataDefinition::_initBetweenReplications() {
	if (_dataAssociations.getInternalData() == nullptr) {
		return;
	}
	for (const auto& pair : *_dataAssociations.getInternalData()) {
		if (pair.second != nullptr) {
			pair.second->_initBetweenReplications();
		}
	}
}

std::string ModelDataDefinition::show() {
	std::string internal = "";
	if (_dataAssociations.getInternalData()->size() > 0) {
		internal = ", internal=[";
		for (const auto& pair : *_dataAssociations.getInternalData()) {
			internal += pair.second->getName() + ",";
		}
		internal = internal.substr(0, internal.length() - 1) + "]";
	}
	return _name + internal;
}

ModelDataDefinition* ModelDataDefinition::getInternalData(const std::string& name) const {
	return _dataAssociations.getInternalData(name);
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinition::getInternalData() const {
	return _dataAssociations.getInternalData();
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinition::getAttachedData() const {
	return _dataAssociations.getAttachedData();
}

Util::identification ModelDataDefinition::getId() const {
	return _id;
}

void ModelDataDefinition::setName(const std::string& name) {
	if (_parentModel == nullptr) {
		return;
	}
	std::string newName = Util::StrReplace(name, " ", "_");
	// Validate that new name is not empty
	if (newName.empty()) {
		return;
	}
	// rename every "stuff" related to this modeldatum (controls, responses and internelElements)
	if (newName != _name) {
		if (_label == _name) { // label was equal to the current name, therefore, changes it to the new name (spaces allowed)
			_label = name;
		}
		std::string stuffName;
		size_t pos;
		for (const auto& child : *_dataAssociations.getInternalData()) {
			stuffName = child.second->getName();
			pos = stuffName.find(_name);
			if (pos != std::string::npos) {
				stuffName.replace(pos, _name.length(), newName);
				child.second->setName(stuffName);
			}
		}

		for (SimulationControl* control : *_parentModel->getControls()->list()) {
			stuffName = control->getName();
			pos = stuffName.find(_name);
			if (pos != std::string::npos) {
				stuffName.replace(pos, _name.length(), newName);
				control->setName(stuffName);
			}
		}

		for (SimulationResponse* response : *_parentModel->getResponses()->list()) {
			stuffName = response->getName();
			pos = stuffName.find(_name);
			if (pos != std::string::npos) {
				stuffName.replace(pos, _name.length(), newName);
				response->setName(stuffName);
			}
		}
		this->_name = newName;
		_hasChanged = true;
	}
}

const std::string& ModelDataDefinition::getName() const {
	return _name;
}

void ModelDataDefinition::setLabel(const std::string& label) {
	_label = label;
}

const std::string& ModelDataDefinition::getLabel() const {
	return _label;
}

const std::string& ModelDataDefinition::getClassname() const {
	return _typename;
}

Model* ModelDataDefinition::getParentModel() const {
	return _parentModel;
}

void ModelDataDefinition::InitBetweenReplications(ModelDataDefinition* modeldatum) {
	modeldatum->trace("Initing " + modeldatum->getClassname() + " \"" + modeldatum->getName() + "\"",
	                  TraceManager::Level::L9_mostDetailed);
	try {
		modeldatum->_initBetweenReplications();
	}
	catch (const std::exception& e) {
		modeldatum->traceError("Error initing modeldatum " + modeldatum->show(), e);
	};
}

ModelDataDefinition* ModelDataDefinition::LoadInstance(Model* model, PersistenceRecord* fields, bool insertIntoModel) {
	std::string name = "";
	if (insertIntoModel) {
		// extracts the name from the fields even before "_laodInstance" and even before construct a new ModelDataDefinition in such way when constructing the ModelDataDefinition, it's done with the correct name and that correct name is show in trace
		name = fields->loadField("name", name);
	}
	ModelDataDefinition* newElement = new ModelDataDefinition(model, "ModelDataDefinition", name, insertIntoModel);
	try {
		newElement->_loadInstance(fields);
	}
	catch (const std::exception& e) {
	}
	return newElement;
}

ModelDataDefinition* ModelDataDefinition::NewInstance(Model* model, std::string name) {
	//nobody will never call this static method. Only its subclasses
	assert(false);
	return nullptr;
}

void ModelDataDefinition::SaveInstance(PersistenceRecord* fields, ModelDataDefinition* modeldatum) {
	try {
		modeldatum->_saveInstance(fields, modeldatum->_getSaveDefaultsOption());
	}
	catch (const std::exception& e) {
	}
}

bool ModelDataDefinition::Check(ModelDataDefinition* modeldatum, std::string& errorMessage) {
	bool res = false;
	Util::IncIndent();
	{
		try {
			res = modeldatum->_check(errorMessage);
		}
		catch (const std::exception& e) {
		}
	}
	Util::DecIndent();
	return res;
}

void ModelDataDefinition::CreateInternalData(ModelDataDefinition* modeldatum) {
	try {
		Util::IncIndent();
		modeldatum->_templateCreateInternalStatisticReporters();
		modeldatum->_templateCreateNonEditableDataDefinitions();
		modeldatum->_templateCreateEditableDataDefinitions();
		modeldatum->_templateCreateAttachedAttributes();
		Util::DecIndent();
	}
	catch (const std::exception& e) {
	};
}

bool ModelDataDefinition::CreateRelatedDataElements(ModelDataDefinition* modeldatum, std::string& errorMessage) {
	if (modeldatum == nullptr) {
		errorMessage.append("ModelDataDefinition is null.");
		return false;
	}

	CreateInternalData(modeldatum);
	return Check(modeldatum, errorMessage);
}

void ModelDataDefinition::_createInternalAndAttachedData() {
	_dataAssociations.createInternalAndAttachedData();
}

void ModelDataDefinition::_addSimulationControl(SimulationControl* control) {
	_simulationControls->insert(control);
}

List<SimulationControl*>* ModelDataDefinition::getSimulationControls() const {
	return _simulationControls;
}

TraceManager::Level ModelDataDefinition::getTraceLevelSpecific() const {
	return _traceLevelSpecific;
}

void ModelDataDefinition::setTraceLevelSpecific(TraceManager::Level level) {
	defineTraceLevelSpecific(level, true);
}

void ModelDataDefinition::defineTraceLevelSpecific(TraceManager::Level traceLevelSpecific,
                                                   bool traceLevelSpecificEnabled) {
	_traceLevelSpecific = traceLevelSpecific;
	_traceLevelSpecificEnabled = traceLevelSpecificEnabled;
}

bool ModelDataDefinition::isTraceLevelSpecificEnabled() const {
	return _traceLevelSpecificEnabled;
}

void ModelDataDefinition::setTraceLevelSpecificEnabled(bool traceLevelSpecificEnabled) {
	_traceLevelSpecificEnabled = traceLevelSpecificEnabled;
}

void ModelDataDefinition::setReportStatistics(bool reportStatistics) {
	if (_reportStatistics != reportStatistics) {
		this->_reportStatistics = reportStatistics;
		_hasChanged = true;
	}
}

bool ModelDataDefinition::isReportStatistics() const {
	return _reportStatistics;
}

// NOT just an easy access to trace manager, but a wrapper to check if specificTraceLevel applies

bool ModelDataDefinition::_checkSpecificTraceLevel(TraceManager::Level level) {
	if (_traceLevelSpecificEnabled && level > _traceLevelSpecific) {
		return false;
	}
	return true;
}

void ModelDataDefinition::trace(const std::string& text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceReport(text, level);
}

void ModelDataDefinition::traceError(const std::string& text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceError(text, level);
}

void ModelDataDefinition::traceError(const std::string& text, const std::exception& e) {
	_parentModel->getTracer()->traceError(text, e);
}

void ModelDataDefinition::traceReport(const std::string& text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceReport(text, level);
}

void ModelDataDefinition::traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component,
                                          const std::string& text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, time, entity, component, text, level, true);
}

void ModelDataDefinition::traceSimulation(void* thisobject, const std::string& text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, text, level, true);
}

void ModelDataDefinition::traceSimulation(void* thisobject, TraceManager::Level level, const std::string& text) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, text, level, true);
}
