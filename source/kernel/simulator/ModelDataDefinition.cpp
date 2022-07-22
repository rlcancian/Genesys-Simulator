/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinition.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:40
 */

//#include <typeinfo>
#include "ModelDataDefinition.h"
#include <iostream>
#include <cassert>
#include "Model.h"
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

ModelDataDefinition::ModelDataDefinition(Model* model, std::string thistypename, std::string name, bool insertIntoModel) {
	_id = Util::GenerateNewId(); //GenerateNewIdOfType(thistypename);
	_typename = thistypename;
	_parentModel = model;
	_reportStatistics = TraitsKernel<ModelDataDefinition>::reportStatistics; // @TODO: shoould be a parameter before insertIntoModel
	if (name == "")
		_name = thistypename + "_" + std::to_string(Util::GenerateNewIdOfType(thistypename));
	else if (name.substr(name.length() - 1, 1) == "%")
		_name = name.substr(0, name.length() - 1) + std::to_string(Util::GenerateNewIdOfType(thistypename));
	else
		_name = name;
	_hasChanged = false;
	if (insertIntoModel) {
		model->insert(this);
	}
	/*
	_addProperty(new PropertySetterString(Util::TypeOf<ModelDataDefinition>(), "name",
		DefineGetterString<ModelDataDefinition>(this, &ModelDataDefinition::getName),
		DefineSetterString<ModelDataDefinition>(this, &ModelDataDefinition::setName)));
	_addProperty(new PropertySetterBool(Util::TypeOf<ModelDataDefinition>(), "report statistics",
		DefineGetterBool<ModelDataDefinition>(this, &ModelDataDefinition::isReportStatistics),
		DefineSetterBool<ModelDataDefinition>(this, &ModelDataDefinition::setReportStatistics)));
	 */
	_addProperty(new PropertyT<std::string>(Util::TypeOf<ModelDataDefinition>(), "Name",
			DefineGetter<ModelDataDefinition, std::string>(this, &ModelDataDefinition::getName),
			DefineSetter<ModelDataDefinition, std::string>(this, &ModelDataDefinition::setName)));
	_addProperty(new PropertyT<bool>(Util::TypeOf<ModelDataDefinition>(), "Report Statistics",
			DefineGetter<ModelDataDefinition, bool>(this, &ModelDataDefinition::isReportStatistics),
			DefineSetter<ModelDataDefinition, bool>(this, &ModelDataDefinition::setReportStatistics)));
}

bool ModelDataDefinition::hasChanged() const {
	return _hasChanged;
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
	////_parentModel->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Removing Element \"" + this->_name + "\" from the model");
	_internalDataClear();
	_parentModel->getDataManager()->remove(this);
}

void ModelDataDefinition::_internalDataClear() {
	for (std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin(); it != _internalData->end(); it++) {
		this->_parentModel->getDataManager()->remove((*it).second);
		(*it).second->~ModelDataDefinition();
	}
	_internalData->clear();
}

void ModelDataDefinition::_internalDataInsert(std::string key, ModelDataDefinition* data) {
	if (data == nullptr)
		return;
	std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->find(key);
	if (data == nullptr) {
		if (it != _internalData->end()) {
			_internalData->erase(it);
		}
	} else {
		if (it == _internalData->end()) {
			_internalData->insert({key, data});
		} else {
			if ((*it).second != data) {
				_internalData->erase(it);
				_internalData->insert({key, data});
			}
		}
	}
}

void ModelDataDefinition::_internalDataRemove(std::string key) {
	std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin();
	while (it != _internalData->end()) {
		if ((*it).first == key) {
			this->_parentModel->getDataManager()->remove((*it).second);
			(*it).second->~ModelDataDefinition();
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
	} else {
		if (it == _attachedData->end()) {
			_attachedData->insert({key, data});
		} else {
			if ((*it).second != data) {
				_attachedData->erase(it);
				_attachedData->insert({key, data});
			}
		}
	}
}

void ModelDataDefinition::_attachedDataRemove(std::string key) {
	//for (std::map<std::string, ModelDataDefinition*>::iterator it = _internelElements->begin(); it != _internelElements->end(); it++) {
	std::map<std::string, ModelDataDefinition*>::iterator it = _attachedData->begin();
	while (it != _attachedData->end()) {
		if ((*it).first == key) {
			this->_parentModel->getDataManager()->remove((*it).second);
			(*it).second->~ModelDataDefinition();
			_attachedData->erase(it);
			it = _attachedData->begin();
		} else {
			it++;
		}
	}
}

bool ModelDataDefinition::_getSaveDefaultsOption() {
	return _parentModel->getPersistence()->getOption(ModelPersistence_if::Options::SAVEDEFAULTS);
}

bool ModelDataDefinition::_loadInstance(PersistenceRecord *fields) {
	bool res = true;

	int id = fields->loadField("id", -1);
	if (id > 0) {
		this->_id = id;
	} else {
		res = false;
	}
	
	this->_name = fields->loadField("name", "");

	int reportStats = fields->loadField("reportStatistics", -1);
	if (reportStats < 0) {
		this->_reportStatistics = TraitsKernel<ModelDataDefinition>::reportStatistics;
	} else {
		this->_reportStatistics = reportStats != 0;
	}

	return res;
}

void ModelDataDefinition::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	fields->saveField("typename", _typename);
	fields->saveField("id", _id);
	fields->saveField("name", _name);
	fields->saveField("reportStatistics", _reportStatistics, TraitsKernel<ModelDataDefinition>::reportStatistics, saveDefaultValues);
}

bool ModelDataDefinition::_check(std::string* errorMessage) {
	*errorMessage += "";
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
	std::string internel = "";
	if (_internalData->size() > 0) {
		internel = ", internel=[";
		for (std::map<std::string, ModelDataDefinition*>::iterator it = _internalData->begin(); it != _internalData->end(); it++) {
			internel += (*it).second->getName() + ",";
		}
		internel = internel.substr(0, internel.length() - 1) + "]";
	}
	return "id=" + std::to_string(_id) + ",name=\"" + _name + "\"" + internel;
}

ModelDataDefinition* ModelDataDefinition::getInternalData(std::string name) const {
	std::map<std::string, ModelDataDefinition*>::iterator pairIt = _internalData->find(name);
	if (pairIt == _internalData->end()) {
		return nullptr;
	} else {
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
	// rename every "stuff" related to this modeldatum (controls, responses and internelElements)
	if (name != _name) {
		std::string stuffName;
		unsigned int pos;
		for (std::pair<std::string, ModelDataDefinition*> child : *_internalData) {
			stuffName = child.second->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) {// != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				child.second->setName(stuffName);
			}
		}

		for (PropertyBase* control : *_parentModel->getControls()->list()) {
			stuffName = control->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) { // != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				control->setName(stuffName);
			}
		}

		for (PropertyBase* response : *_parentModel->getResponses()->list()) {
			stuffName = response->getName();
			pos = stuffName.find(getName(), 0);
			if (pos < stuffName.length()) {// != std::string::npos) {
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
	modeldatum->_parentModel->getTracer()->trace("Initing " + modeldatum->getClassname() + " \"" + modeldatum->getName() + "\"", TraceManager::Level::L9_mostDetailed); //std::to_string(component->_id));
	try {
		modeldatum->_initBetweenReplications();
	} catch (const std::exception& e) {
		modeldatum->_parentModel->getTracer()->traceError(e, "Error initing modeldatum " + modeldatum->show());
	};
}

ModelDataDefinition* ModelDataDefinition::LoadInstance(Model* model, PersistenceRecord *fields, bool insertIntoModel) {
	std::string name = "";
	if (insertIntoModel) {
		// extracts the name from the fields even before "_laodInstance" and even before construct a new ModelDataDefinition in such way when constructing the ModelDataDefinition, it's done with the correct name and that correct name is show in trace
		name = fields->loadField("name", name);
	}
	ModelDataDefinition* newElement = new ModelDataDefinition(model, "ModelDataDefinition", name, insertIntoModel);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

ModelDataDefinition* ModelDataDefinition::NewInstance(Model* model, std::string name) {
	//nobody will never call this static method. Only its subclasses
	assert(false);
	return nullptr;
}

void ModelDataDefinition::SaveInstance(PersistenceRecord *fields, ModelDataDefinition* modeldatum) {
	try {
		modeldatum->_saveInstance(fields, modeldatum->_getSaveDefaultsOption());
	} catch (const std::exception& e) {
		//modeldatum->_model->getTrace()->traceError(e, "Error saving anElement " + modeldatum->show());
	}
}

bool ModelDataDefinition::Check(ModelDataDefinition* modeldatum, std::string* errorMessage) {
	//    modeldatum->_model->getTraceManager()->trace(TraceManager::Level::L9_mostDetailed, "Checking " + modeldatum->_typename + ": " + modeldatum->_name); //std::to_string(modeldatum->_id));
	bool res = false;
	Util::IncIndent();
	{
		try {
			res = modeldatum->_check(errorMessage);
			if (!res) {
				//                modeldatum->_model->getTraceManager()->traceError(TraceManager::Level::errors, "Error: Checking has failed with message '" + *errorMessage + "'");
			}
		} catch (const std::exception& e) {
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
	} catch (const std::exception& e) {
		//modeldatum->...->_model->getTraceManager()->traceError(e, "Error creating elements of modeldatum " + modeldatum->show());
	};
}

void ModelDataDefinition::_createInternalAndAttachedData() {

}

void ModelDataDefinition::_addProperty(PropertyBase* property) {
	_properties->insert(_properties->end(), property);
}

std::list<PropertyBase *> *ModelDataDefinition::getProperties() const {
	return _properties;
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
