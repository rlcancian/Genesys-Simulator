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
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

ModelDataDefinition::ModelDataDefinition(Model* model, std::string thistypename, std::string name,
                                         bool insertIntoModel) {
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
	_hasChanged = false;
	if (insertIntoModel) {
		model->insert(this);
	}
	// make "name" a property of a component
	SimulationControlGeneric<std::string>* propName = new SimulationControlGeneric<std::string>(
		std::bind(&ModelDataDefinition::getName, this),
		std::bind(&ModelDataDefinition::setName, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Name", "");
	SimulationControlGeneric<bool>* propReportStatistics = new SimulationControlGeneric<bool>(
		std::bind(&ModelDataDefinition::isReportStatistics, this),
		std::bind(&ModelDataDefinition::setReportStatistics, this, std::placeholders::_1),
		Util::TypeOf<ModelDataDefinition>(), getName(), "Report Statistics", "");

	_parentModel->getControls()->insert(propName);

	// setting properties
	_addSimulationControl(propName);
	_addSimulationControl(propReportStatistics);
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

//ModelDataDefinition::ModelDataDefinition(const ModelDataDefinition &orig) {
//this->_parentModel = orig->_parentModel;
//this->_name = "copy_of_" + orig->_name;
//this->_typename = orig->_typename;
//}

ModelDataDefinition::~ModelDataDefinition() {
	// Release all owned internal modeldata definitions registered by this model element.
	_internalDataClear();
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
	// Destroy the internal-data registry container after its owned contents are cleared.
	delete _internalData;
	_internalData = nullptr;
	// Clear and destroy the attached-data registry without deleting referenced external objects.
	if (_attachedData != nullptr) {
		_attachedData->clear();
		delete _attachedData;
		_attachedData = nullptr;
	}
}

void ModelDataDefinition::_internalDataClear() {
	for (std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin(); it != _internalData->end();
	     it++) {
		this->_parentModel->getDataManager()->remove((*it).second);
		delete ((*it).second); //->~ModelDataDefinition();
	}
	_internalData->clear();
}

void ModelDataDefinition::_internalDataInsert(std::string key, ModelDataDefinition* data) {
	if (data == nullptr)
		return;
	std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->find(key);
	if (it == _internalData->end()) {
		_internalData->insert({key, data});
	} else if ((*it).second != data) {
		this->_parentModel->getDataManager()->remove((*it).second);
		delete ((*it).second);
		_internalData->erase(it);
		_internalData->insert({key, data});
	}

	// Internal data definitions are owned children of this object and must also exist in the
	// model-wide registry immediately, otherwise GUI features that list or synchronize all data
	// definitions observe an incomplete model until a later full model check rebuilds the state.
	ModelDataManager* dataManager = this->_parentModel->getDataManager();
	if (dataManager->getDataDefinition(data->getClassname(), data->getId()) == nullptr) {
		dataManager->insert(data);
	}
}

void ModelDataDefinition::_internalDataRemove(std::string key) {
	std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin();
	while (it != _internalData->end()) {
		if ((*it).first == key) {
			this->_parentModel->getDataManager()->remove((*it).second);
			delete ((*it).second); //->~ModelDataDefinition();
			_internalData->erase(it);
			it = _internalData->begin();
		}
	}
}

void ModelDataDefinition::_attachedAttributesInsert(std::vector<std::string> neededNames) {
	/* include attributes needed */
	ModelDataManager* elements = _parentModel->getDataManager();
	std::string neededName;
	for (unsigned int i = 0; i < neededNames.size(); i++) {
		neededName = neededNames[i];
		ModelDataDefinition* attr1 = elements->getDataDefinition(Util::TypeOf<Attribute>(), neededName);
		if (attr1 == nullptr) {
			attr1 = new Attribute(_parentModel, neededName);
		}
		_attachedDataInsert(neededName, attr1);
	}
}

void ModelDataDefinition::_attachedDataInsert(std::string key, ModelDataDefinition* data) {
	if (data == nullptr)
		return;
	std::map<std::string, ModelDataDefinition*>::iterator it = _attachedData->find(key);
	if (data == nullptr) {
		if (it != _attachedData->end()) {
			_attachedData->erase(it);
		}
	}
	else {
		if (it == _attachedData->end()) {
			_attachedData->insert({key, data});
		}
		else {
			if ((*it).second != data) {
				_attachedData->erase(it);
				_attachedData->insert({key, data});
			}
		}
	}
}

void ModelDataDefinition::_attachedDataRemove(std::string key) {
	// Remove only the non-owning attachment registry entry and keep the referenced object lifetime untouched.
	std::map<std::string, ModelDataDefinition*>::iterator it = _attachedData->begin();
	while (it != _attachedData->end()) {
		if ((*it).first == key) {
			_attachedData->erase(it);
			it = _attachedData->begin();
		}
		else {
			it++;
		}
	}
}

void ModelDataDefinition::_attachedDataClear() {
	_attachedData->clear();
}

void ModelDataDefinition::_checkCreateAttachedReferencedDataDefinition(std::string expression) {
	//(std::map<std::string, std::list<std::string>*>* referencedDataDefinitions) {
	std::map<std::string, std::list<std::string>*> referencedDataDefinitions;
	// = nullptr; // = new std::map<std::string, std::list<std::string>*>();
	_parentModel->checkReferencesToDataDefinitions(expression, &referencedDataDefinitions);
	if (referencedDataDefinitions.size() > 0) {
		Util::IncIndent();
		ModelDataDefinition* referedElem;
		ModelDataManager* elemMan = _parentModel->getDataManager();
		for (auto pair : referencedDataDefinitions) {
			for (std::string referedName : *pair.second) {
				referedElem = elemMan->getDataDefinition(pair.first, referedName);
				assert(referedElem != nullptr);
				_attachedDataInsert("Refered_" + pair.first, referedElem);
				trace(this->getName() + " has an expression that refers to " + pair.first + " " + referedName +
					", and therefore was attached.");
			}
		}
		Util::DecIndent();
	}
	// Release temporary lists allocated during reference extraction to avoid local ownership leaks.
	for (auto& pair : referencedDataDefinitions) {
		delete pair.second;
		pair.second = nullptr;
	}
	referencedDataDefinitions.clear();
}

bool ModelDataDefinition::_getSaveDefaultsOption() {
	return _parentModel->getPersistence()->getOption(ModelPersistence_if::Options::SAVEDEFAULTS);
}

bool ModelDataDefinition::_loadInstance(PersistenceRecord* fields) {
	int id = fields->loadField("id", -1);
	if (id > 0) this->_id = id;
	else return false;

	setName(fields->loadField("name", ""));
	this->_reportStatistics = fields->
		loadField("reportStatistics", TraitsKernel<ModelDataDefinition>::reportStatistics);

	return true;
}

void ModelDataDefinition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	fields->saveField("typename", _typename);
	fields->saveField("id", _id);
	fields->saveField("name", _name);
	fields->saveField("reportStatistics", _reportStatistics, TraitsKernel<ModelDataDefinition>::reportStatistics,
	                  saveDefaultValues);
}

bool ModelDataDefinition::_check(std::string& errorMessage) {
	errorMessage += "";
	return true; // if there is no ovveride, return true
}

ParserChangesInformation* ModelDataDefinition::_getParserChangesInformation() {
	return new ParserChangesInformation(); // if there is no override, return no changes
}

void ModelDataDefinition::_initBetweenReplications() {
	for (std::pair<std::string, ModelDataDefinition*> pair : *_internalData) {
		pair.second->_initBetweenReplications();
	}
}

std::string ModelDataDefinition::show() {
	std::string internal = "";
	if (_internalData->size() > 0) {
		internal = ", internal=[";
		for (std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin(); it != _internalData->
		     end(); it++) {
			internal += (*it).second->getName() + ",";
		}
		internal = internal.substr(0, internal.length() - 1) + "]";
	}
	return _name + internal; //"id=" + std::to_string(_id) + ",name=\"" + _name + "\"" + internal;
}

ModelDataDefinition* ModelDataDefinition::getInternalData(std::string name) const {
	std::map<std::string, ModelDataDefinition*>::iterator pairIt = _internalData->find(name);
	if (pairIt == _internalData->end()) {
		return nullptr;
	}
	else {
		return (*pairIt).second;
	}
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinition::getInternalData() const {
	return _internalData;
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinition::getAttachedData() const {
	return _attachedData;
}

//ModelDataDefinition* ModelDataDefinition::getInternalData(std::string key) const {
//	std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->find(key);
//	if (it != _internalData->end()) {
//		return (*it).second;
//	} else {
//		return nullptr;
//	}
//}

Util::identification ModelDataDefinition::getId() const {
	return _id;
}

void ModelDataDefinition::setName(std::string name) {
	name = Util::StrReplace(name, " ", "_");
	// rename every "stuff" related to this modeldatum (controls, responses and internelElements)
	if (name != _name) {
		std::string stuffName;
		unsigned int pos;
		for (std::pair<std::string, ModelDataDefinition*> child : *_internalData) {
			stuffName = child.second->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) {
				// != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				child.second->setName(stuffName);
			}
		}

		for (SimulationControl* control : *_parentModel->getControls()->list()) {
			stuffName = control->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) {
				// != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				control->setName(stuffName);
			}
		}

		for (SimulationResponse* response : *_parentModel->getResponses()->list()) {
			stuffName = response->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) {
				// != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				response->setName(stuffName);
			}
		}
		this->_name = name;
		_hasChanged = true;
	}
}

std::string ModelDataDefinition::getName() const {
	return _name;
}

std::string ModelDataDefinition::getClassname() const {
	return _typename;
}

void ModelDataDefinition::InitBetweenReplications(ModelDataDefinition* modeldatum) {
	modeldatum->trace("Initing " + modeldatum->getClassname() + " \"" + modeldatum->getName() + "\"",
	                  TraceManager::Level::L9_mostDetailed); //std::to_string(component->_id));
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
		//modeldatum->_model->getTrace()->traceError(e, "Error saving anElement " + modeldatum->show());
	}
}

bool ModelDataDefinition::Check(ModelDataDefinition* modeldatum, std::string& errorMessage) {
	//    modeldatum->_model->getTraceManager()->trace(TraceManager::Level::L9_mostDetailed, "Checking " + modeldatum->_typename + ": " + modeldatum->_name); //std::to_string(modeldatum->_id));
	bool res = false;
	Util::IncIndent();
	{
		try {
			res = modeldatum->_check(errorMessage);
			if (!res) {
				//                modeldatum->_model->getTraceManager()->traceError(TraceManager::Level::errors, "Error: Checking has failed with message '" + *errorMessage + "'");
			}
		}
		catch (const std::exception& e) {
			//            modeldatum->_model->getTraceManager()->traceError(e, "Error verifying modeldatum " + modeldatum->show());
		}
	}
	Util::DecIndent();
	return res;
}

void ModelDataDefinition::CreateInternalData(ModelDataDefinition* modeldatum) {
	try {
		Util::IncIndent();
		modeldatum->_createInternalAndAttachedData();
		Util::DecIndent();
	}
	catch (const std::exception& e) {
		//modeldatum->...->_model->getTraceManager()->traceError(e, "Error creating elements of modeldatum " + modeldatum->show());
	};
}

void ModelDataDefinition::_createInternalAndAttachedData() {
}

void ModelDataDefinition::_addSimulationControl(SimulationControl* control) {
	_simulationControls->insert(control);
}

/*
void ModelDataDefinition::_addSimulationResponse(SimulationControl* response) {
	_simulationResponses->insert(response); // @ToDo: (importante): Check if exists before insert?
}
*/

List<SimulationControl*>* ModelDataDefinition::getSimulationControls() const {
	return _simulationControls;
}

TraceManager::Level ModelDataDefinition::getTraceLevelSpecific() const {
	return _traceLevelSpecific;
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

void ModelDataDefinition::trace(std::string text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceReport(text, level);
}

void ModelDataDefinition::traceError(std::string text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceError(text, level);
}

void ModelDataDefinition::traceError(std::string text, std::exception e) {
	_parentModel->getTracer()->traceError(text, e);
}

void ModelDataDefinition::traceReport(std::string text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceReport(text, level);
}

void ModelDataDefinition::traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component,
                                          std::string text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, time, entity, component, text, level, true);
}

void ModelDataDefinition::traceSimulation(void* thisobject, std::string text, TraceManager::Level level) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, text, level, true);
}

void ModelDataDefinition::traceSimulation(void* thisobject, TraceManager::Level level, std::string text) {
	if (_checkSpecificTraceLevel(level))
		_parentModel->getTracer()->traceSimulation(thisobject, text, level, true);
}
