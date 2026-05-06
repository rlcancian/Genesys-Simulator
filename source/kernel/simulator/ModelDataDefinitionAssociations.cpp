/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinitionAssociations.cpp
 * Author: Codex
 *
 * Created on 6 de Maio de 2026
 */

#include "ModelDataDefinitionAssociations.h"

#include "Attribute.h"
#include "Model.h"
#include "ModelDataDefinition.h"
#include "ModelDataManager.h"

#include <cassert>
#include <exception>
#include <list>
#include <utility>

namespace {

template <typename MapType>
void clearMap(MapType& map) {
	map.clear();
}

template <typename MapType>
void insertIntoMap(MapType& map, const std::string& key, ModelDataDefinition* data) {
	if (data == nullptr) {
		map.erase(key);
		return;
	}
	map[key] = data;
}

template <typename MapType>
void removeFromMap(MapType& map, const std::string& key) {
	map.erase(key);
}

template <typename MapType>
void replaceBucketEntry(MapType& bucket, const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(bucket, key, data);
}

} // namespace

ModelDataDefinitionAssociations::ModelDataDefinitionAssociations(ModelDataDefinition* owner) : _owner(owner) {
}

ModelDataDefinitionAssociations::~ModelDataDefinitionAssociations() {
	clearAll();
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinitionAssociations::getInternalData() const {
	return const_cast<std::map<std::string, ModelDataDefinition*>*>(&_internalData);
}

std::map<std::string, ModelDataDefinition*>* ModelDataDefinitionAssociations::getAttachedData() const {
	return const_cast<std::map<std::string, ModelDataDefinition*>*>(&_attachedData);
}

ModelDataDefinition* ModelDataDefinitionAssociations::getInternalData(const std::string& key) const {
	const auto it = _internalData.find(key);
	return it != _internalData.end() ? it->second : nullptr;
}

void ModelDataDefinitionAssociations::clearAll() {
	clearInternalData();
	clearAttachedData();
	clearStatisticReporters();
	clearMandatoryAttachedAttributes();
	clearMandatoryEditableDataDefinitions();
	clearOptionalEditableDataDefinitions();
	clearMandatoryNonEditableDataDefinitions();
	clearOptionalNonEditableDataDefinitions();
}

void ModelDataDefinitionAssociations::clearInternalData() {
	std::vector<std::string> internalKeys;
	internalKeys.reserve(_internalData.size());
	for (const auto& pair : _internalData) {
		internalKeys.push_back(pair.first);
	}
	for (const std::string& key : internalKeys) {
		internalDataRemove(key);
	}
	_statisticReporters.clear();
	_mandatoryNonEditableDataDefinitions.clear();
	_optionalNonEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::clearAttachedData() {
	std::vector<std::string> attachedKeys;
	attachedKeys.reserve(_attachedData.size());
	for (const auto& pair : _attachedData) {
		attachedKeys.push_back(pair.first);
	}
	for (const std::string& key : attachedKeys) {
		attachedDataRemove(key);
	}
	_mandatoryAttachedAttributes.clear();
	_mandatoryEditableDataDefinitions.clear();
	_optionalEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::internalDataInsert(const std::string& key, ModelDataDefinition* data) {
	if (data == nullptr) {
		internalDataRemove(key);
		return;
	}
	auto it = _internalData.find(key);
	if (it == _internalData.end()) {
		_internalData.insert({key, data});
	} else if (it->second != data) {
		_deleteOwnedInternalData(it->second);
		it->second = data;
	}
	if (_owner != nullptr && _owner->_parentModel != nullptr) {
		ModelDataManager* dataManager = _owner->_parentModel->getDataManager();
		if (dataManager->getDataDefinition(data->getClassname(), data->getId()) == nullptr) {
			dataManager->insert(data);
		}
	}
}

void ModelDataDefinitionAssociations::internalDataRemove(const std::string& key) {
	auto it = _internalData.find(key);
	if (it == _internalData.end()) {
		return;
	}
	_deleteOwnedInternalData(it->second);
	_internalData.erase(it);
}

void ModelDataDefinitionAssociations::attachedDataInsert(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_attachedData, key, data);
}

void ModelDataDefinitionAssociations::attachedDataRemove(const std::string& key) {
	removeFromMap(_attachedData, key);
}

void ModelDataDefinitionAssociations::attachedAttributesInsert(const std::vector<std::string>& neededNames) {
	if (_owner == nullptr || _owner->_parentModel == nullptr) {
		return;
	}
	ModelDataManager* elements = _owner->_parentModel->getDataManager();
	for (const std::string& neededName : neededNames) {
		ModelDataDefinition* attr = elements->getDataDefinition(Util::TypeOf<Attribute>(), neededName);
		if (attr == nullptr) {
			attr = new Attribute(_owner->_parentModel, neededName);
		}
		insertMandatoryAttachedAttribute(neededName, attr);
	}
}

void ModelDataDefinitionAssociations::checkCreateAttachedReferencedDataDefinition(const std::string& expression) {
	if (_owner == nullptr || _owner->_parentModel == nullptr || expression.empty()) {
		return;
	}
	std::map<std::string, std::list<std::string>*> referencedDataDefinitions;
	_owner->_parentModel->checkReferencesToDataDefinitions(expression, &referencedDataDefinitions);
	if (!referencedDataDefinitions.empty()) {
		Util::IncIndent();
		ModelDataManager* elemMan = _owner->_parentModel->getDataManager();
		for (const auto& pair : referencedDataDefinitions) {
			for (const std::string& referedName : *pair.second) {
				ModelDataDefinition* referedElem = elemMan->getDataDefinition(pair.first, referedName);
				assert(referedElem != nullptr);
				attachedDataInsert("Refered_" + pair.first, referedElem);
				_owner->trace(_owner->getName() + " has an expression that refers to " + pair.first + " " + referedName +
				              ", and therefore was attached.");
			}
		}
		Util::DecIndent();
	}
	for (auto& pair : referencedDataDefinitions) {
		delete pair.second;
		pair.second = nullptr;
	}
	referencedDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::createInternalAndAttachedData() {
	createInternalStatisticReporters();
	createAttachedAttributes();
	createNonEditableDataDefinitions();
	createEditableDataDefinitions();
}

void ModelDataDefinitionAssociations::createInternalStatisticReporters() {
	if (_owner == nullptr) {
		return;
	}
	try {
		_owner->_createInternalStatisticReporters();
	} catch (const std::exception& e) {
		_owner->traceError("Error creating report-statistics data definitions for " + _owner->getClassname() + " " + _owner->getName(), e);
	} catch (...) {
		_owner->traceError("Unknown error creating report-statistics data definitions for " + _owner->getClassname() + " " + _owner->getName());
	}
}

void ModelDataDefinitionAssociations::createAttachedAttributes() {
	if (_owner == nullptr) {
		return;
	}
	try {
		_owner->_createAttachedAttributes();
	} catch (const std::exception& e) {
		_owner->traceError("Error creating attributes for " + _owner->getClassname() + " " + _owner->getName(), e);
	} catch (...) {
		_owner->traceError("Unknown error creating attribu for " + _owner->getClassname() + " " + _owner->getName());
	}
}

void ModelDataDefinitionAssociations::createNonEditableDataDefinitions() {
	if (_owner == nullptr) {
		return;
	}
	try {
		_owner->_createNonEditableDataDefinitions();
	} catch (const std::exception& e) {
		_owner->traceError("Error creating non editable data definitions for " + _owner->getClassname() + " " + _owner->getName(), e);
	} catch (...) {
		_owner->traceError("Unknown error creating non editable data definitions for " + _owner->getClassname() + " " + _owner->getName());
	}
}

void ModelDataDefinitionAssociations::createEditableDataDefinitions() {
	if (_owner == nullptr) {
		return;
	}
	try {
		_owner->_createEditableDataDefinitions();
	} catch (const std::exception& e) {
		_owner->traceError("Error creating editable data definitions for " + _owner->getClassname() + " " + _owner->getName(), e);
	} catch (...) {
		_owner->traceError("Unknown error creating editable data definitions for " + _owner->getClassname() + " " + _owner->getName());
	}
}

void ModelDataDefinitionAssociations::insertStatisticReporter(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_statisticReporters, key, data);
	internalDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeStatisticReporter(const std::string& key) {
	removeFromMap(_statisticReporters, key);
	internalDataRemove(key);
}

void ModelDataDefinitionAssociations::clearStatisticReporters() {
	for (const auto& pair : _statisticReporters) {
		internalDataRemove(pair.first);
	}
	_statisticReporters.clear();
}

void ModelDataDefinitionAssociations::insertMandatoryAttachedAttribute(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_mandatoryAttachedAttributes, key, data);
	attachedDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeMandatoryAttachedAttribute(const std::string& key) {
	removeFromMap(_mandatoryAttachedAttributes, key);
	attachedDataRemove(key);
}

void ModelDataDefinitionAssociations::clearMandatoryAttachedAttributes() {
	std::vector<std::string> keys;
	keys.reserve(_mandatoryAttachedAttributes.size());
	for (const auto& pair : _mandatoryAttachedAttributes) {
		keys.push_back(pair.first);
	}
	for (const std::string& key : keys) {
		attachedDataRemove(key);
	}
	_mandatoryAttachedAttributes.clear();
}

void ModelDataDefinitionAssociations::insertMandatoryEditableDataDefinition(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_mandatoryEditableDataDefinitions, key, data);
	attachedDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeMandatoryEditableDataDefinition(const std::string& key) {
	removeFromMap(_mandatoryEditableDataDefinitions, key);
	attachedDataRemove(key);
}

void ModelDataDefinitionAssociations::clearMandatoryEditableDataDefinitions() {
	std::vector<std::string> keys;
	keys.reserve(_mandatoryEditableDataDefinitions.size());
	for (const auto& pair : _mandatoryEditableDataDefinitions) {
		keys.push_back(pair.first);
	}
	for (const std::string& key : keys) {
		attachedDataRemove(key);
	}
	_mandatoryEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::insertOptionalEditableDataDefinition(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_optionalEditableDataDefinitions, key, data);
	attachedDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeOptionalEditableDataDefinition(const std::string& key) {
	removeFromMap(_optionalEditableDataDefinitions, key);
	attachedDataRemove(key);
}

void ModelDataDefinitionAssociations::clearOptionalEditableDataDefinitions() {
	std::vector<std::string> keys;
	keys.reserve(_optionalEditableDataDefinitions.size());
	for (const auto& pair : _optionalEditableDataDefinitions) {
		keys.push_back(pair.first);
	}
	for (const std::string& key : keys) {
		attachedDataRemove(key);
	}
	_optionalEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::insertMandatoryNonEditableDataDefinition(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_mandatoryNonEditableDataDefinitions, key, data);
	internalDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeMandatoryNonEditableDataDefinition(const std::string& key) {
	removeFromMap(_mandatoryNonEditableDataDefinitions, key);
	internalDataRemove(key);
}

void ModelDataDefinitionAssociations::clearMandatoryNonEditableDataDefinitions() {
	std::vector<std::string> keys;
	keys.reserve(_mandatoryNonEditableDataDefinitions.size());
	for (const auto& pair : _mandatoryNonEditableDataDefinitions) {
		keys.push_back(pair.first);
	}
	for (const std::string& key : keys) {
		internalDataRemove(key);
	}
	_mandatoryNonEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::insertOptionalNonEditableDataDefinition(const std::string& key, ModelDataDefinition* data) {
	insertIntoMap(_optionalNonEditableDataDefinitions, key, data);
	internalDataInsert(key, data);
}

void ModelDataDefinitionAssociations::removeOptionalNonEditableDataDefinition(const std::string& key) {
	removeFromMap(_optionalNonEditableDataDefinitions, key);
	internalDataRemove(key);
}

void ModelDataDefinitionAssociations::clearOptionalNonEditableDataDefinitions() {
	std::vector<std::string> keys;
	keys.reserve(_optionalNonEditableDataDefinitions.size());
	for (const auto& pair : _optionalNonEditableDataDefinitions) {
		keys.push_back(pair.first);
	}
	for (const std::string& key : keys) {
		internalDataRemove(key);
	}
	_optionalNonEditableDataDefinitions.clear();
}

void ModelDataDefinitionAssociations::_deleteOwnedInternalData(ModelDataDefinition* data) {
	if (data == nullptr) {
		return;
	}
	if (_owner != nullptr && _owner->_parentModel != nullptr) {
		_owner->_parentModel->getDataManager()->remove(data);
	}
	delete data;
}
