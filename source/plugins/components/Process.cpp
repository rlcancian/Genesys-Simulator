/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Process.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Maio de 2019, 18:41
 */

#include "Process.h"

#include <memory>

#include "../../kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Process::GetPluginInformation;
}
#endif


ModelDataDefinition* Process::NewInstance(Model* model, std::string name) {
	return new Process(model, name);
}

Process::Process(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Process>(), name) {
	_createInternalAndAttachedData(); // its's called by the constructor because internal components can be accessed by process' public methods, so they must exist ever since
}

std::string Process::show() {
	return ModelComponent::show() + "";
}

void Process::setPriority(unsigned short _priority) {
	_seize->setPriority(_priority);
}

unsigned short Process::getPriority() const {
	return _seize->getPriority();
}

void Process::setAllocationType(unsigned int _allocationType) {
	_seize->setAllocationType(_allocationType);
}

unsigned int Process::getAllocationType() const {
	return _seize->getAllocationType();
}

List<SeizableItem*>* Process::getSeizeRequests() const {
	return _seize->getSeizeRequests();
}

void Process::setQueueableItem(QueueableItem* _queueableItem) {
	_seize->setQueueableItem(_queueableItem);
}

QueueableItem* Process::getQueueableItem() const {
	return _seize->getQueueableItem();
}

void Process::setSaveAttribute(std::string _saveAttribute) {
	_seize->setSaveAttribute(_saveAttribute);
}

std::string Process::getSaveAttribute() const {
	return _seize->getSaveAttribute();
}

void Process::setDelayExpression(std::string _delayExpression) {
	_delay->setDelayExpression(_delayExpression);
}

std::string Process::delayExpression() const {
	return _delay->delayExpression();
}

void Process::setDelayTimeUnit(Util::TimeUnit _delayTimeUnit) {
	_delay->setDelayTimeUnit(_delayTimeUnit);
}

Util::TimeUnit Process::delayTimeUnit() const {
	return _delay->delayTimeUnit();
}

ModelComponent* Process::LoadInstance(Model* model, PersistenceRecord *fields) {
	Process* newComponent = new Process(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Process::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	this->_parentModel->sendEntityToComponent(entity, _seize, 0.0);
}

void Process::_adjustConnections() {
	if (getConnections()->size() > 0 && getConnections()->getFrontConnection()->component != _seize) {
		// chance connections so Process is connected to Seize, and Release to the one that Process was connected to
		Connection* connection = new Connection();
		connection->component = getConnections()->getFrontConnection()->component;
		connection->port = getConnections()->getFrontConnection()->port;
		getConnections()->getFrontConnection()->component = _seize;
		getConnections()->getFrontConnection()->port = 0;
		_release->getConnections()->connections()->clear();
		_release->getConnections()->insert(connection);
	}
}

bool Process::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_seize->setAllocationType(fields->loadField("allocationType", _seize->DEFAULT.allocationType));
		_seize->setPriority(fields->loadField("priority", _seize->DEFAULT.priority));
		_seize->setSaveAttribute(fields->loadField("saveAttribute", _seize->DEFAULT.saveAttribute));
		QueueableItem* queueableItem = new QueueableItem(nullptr);
		queueableItem->setElementManager(_parentModel->getDataManager());
		queueableItem->loadInstance(fields);
		_seize->setQueueableItem(queueableItem);
		unsigned short numRequests = fields->loadField("resquests", _seize->DEFAULT.seizeRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItem* item = new SeizableItem(nullptr);
			item->setElementManager(_parentModel->getDataManager());
			item->loadInstance(fields, i);
			_seize->getSeizeRequests()->insert(item);
		}
		_delay->setDelayExpression(fields->loadField("delayExpression", _delay->DEFAULT.delayExpression));
		_delay->setDelayTimeUnit(fields->loadField("delayExpressionTimeUnit", _delay->DEFAULT.delayTimeUnit));
		_seize->setModelLevel(_id);
		_delay->setModelLevel(_id);
		_release->setModelLevel(_id);
	}
	return res;
}

void Process::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	_adjustConnections();
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	auto seizefields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
	ModelComponent::SaveInstance(seizefields.get(), _seize);
	seizefields->erase("id");
	seizefields->erase("typename");
	seizefields->erase("name");
	seizefields->erase("nexts");
	seizefields->erase("nextId");
	seizefields->erase("caption");
	seizefields->erase("reportStatistics");
	fields->insert(seizefields->begin(), seizefields->end());
	auto delayfields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
	ModelComponent::SaveInstance(delayfields.get(), _delay);
	delayfields->erase("id");
	delayfields->erase("typename");
	delayfields->erase("name");
	delayfields->erase("nexts");
	delayfields->erase("nextId");
	delayfields->erase("caption");
	delayfields->erase("reportStatistics");
	fields->insert(delayfields->begin(), delayfields->end());
	auto releasefields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
	ModelComponent::SaveInstance(releasefields.get(), _release);
	Util::identification next = releasefields->loadField("nextId", -1);
	fields->saveField("nextId", next);
}

PluginInformation* Process::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Process>(), &Process::LoadInstance, &Process::NewInstance);
	info->insertDynamicLibFileDependence("seize.so");
	info->insertDynamicLibFileDependence("delay.so");
	info->insertDynamicLibFileDependence("release.so");
	return info;
}

void Process::_createInternalAndAttachedData() {
	if (_seize == nullptr) {
		PluginManager* plugins = _parentModel->getParentSimulator()->getPlugins();
		// the following components are created into the "_id" model level (a submodel) and therefore will not be saved
		_seize = plugins->newInstance<Seize>(_parentModel, getName() + ".Seize");
		_delay = plugins->newInstance<Delay>(_parentModel, getName() + ".Delay");
		_release = plugins->newInstance<Release>(_parentModel, getName() + ".Release");
		_seize->setModelLevel(_id); // set level as subcomponent
		_delay->setModelLevel(_id); // set level as subcomponent
		_release->setModelLevel(_id); // set level as subcomponent
		_seize->getConnections()->insert(_delay);
		_delay->getConnections()->insert(_release);
		_internalDataInsert("Seize", _seize);
		_internalDataInsert("Delay", _delay);
		_internalDataInsert("Release", _release);
	}
	_adjustConnections();
}

bool Process::_check(std::string* errorMessage) {
	bool resultAll = true;
	// garantee that release releases exactlly what seize seizes
	_release->getReleaseRequests()->clear();
	for (SeizableItem* item : *_seize->getSeizeRequests()->list()) {
		_release->getReleaseRequests()->insert(item);
	}
	resultAll &= ModelComponent::Check(_seize);
	resultAll &= ModelComponent::Check(_delay);
	resultAll &= ModelComponent::Check(_release);
	*errorMessage += "";
	return resultAll;
}
