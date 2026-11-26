/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Buffer.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on
 */

#include "plugins/components/DiscreteProcessing/Synchronization/Buffer.h"
#include "../../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Buffer::GetPluginInformation;
}
#endif

ModelDataDefinition* Buffer::NewInstance(Model* model, std::string name) {
	return new Buffer(model, name);
}

std::string Buffer::convertEnumToStr(AdvanceOn advance) {
	switch (static_cast<int> (advance)) {
		case 0: return "NewArrivals";
		case 1: return "Signal";
	}
	return "Unknown";
}

std::string Buffer::convertEnumToStr(ArrivalOnFullBufferRule arrival) {
	switch (static_cast<int> (arrival)) {
		case 0: return "Dispose";
		case 1: return "SendToBulkPort";
		case 2: return "ReplaceLastPosition";
	}
	return "Unknown";
}

Buffer::Buffer(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Buffer>(), name) {
    SimulationControlGenericEnum<Buffer::ArrivalOnFullBufferRule, Buffer>* propArrivalRule = new SimulationControlGenericEnum<Buffer::ArrivalOnFullBufferRule, Buffer>(
                std::bind(&Buffer::getarrivalOnFullBufferRule, this),
                std::bind(&Buffer::setArrivalOnFullBufferRule, this, std::placeholders::_1),
                Util::TypeOf<Buffer>(), getName(), "ArrivalOnFullBufferRule", "");
    SimulationControlGenericEnum<Buffer::AdvanceOn, Buffer>* propAdvanceOn = new SimulationControlGenericEnum<Buffer::AdvanceOn, Buffer>(
                std::bind(&Buffer::getadvanceOn, this),
                std::bind(&Buffer::setAdvanceOn, this, std::placeholders::_1),
                Util::TypeOf<Buffer>(), getName(), "AdvanceOn", "");
	SimulationControlGeneric<unsigned int>* propCapacity = new SimulationControlGeneric<unsigned int>(
				std::bind(&Buffer::getcapacity, this),
				std::bind(&Buffer::setCapacity, this, std::placeholders::_1),
				Util::TypeOf<Buffer>(), getName(), "Capacity", "");
	SimulationControlGenericClass<Queue*, Model*, Queue>* propQueue = new SimulationControlGenericClass<Queue*, Model*, Queue>(
				_parentModel,
				std::bind(&Buffer::getQueue, this),
				std::bind(&Buffer::setQueue, this, std::placeholders::_1),
				Util::TypeOf<Buffer>(), getName(), "Queue", "", false, true, false, nullptr, false, false, nullptr, nullptr, nullptr, true);
	SimulationControlGenericClass<SignalData*, Model*, SignalData>* propSignal = new SimulationControlGenericClass<SignalData*, Model*, SignalData>(
				_parentModel,
				std::bind(&Buffer::getsignal, this),
				std::bind(&Buffer::setSignal, this, std::placeholders::_1),
				Util::TypeOf<Buffer>(), getName(), "Signal", "");

    _parentModel->getControls()->insert(propArrivalRule);
    _parentModel->getControls()->insert(propAdvanceOn);
	_parentModel->getControls()->insert(propCapacity);
	_parentModel->getControls()->insert(propQueue);
	_parentModel->getControls()->insert(propSignal);

	// setting properties
    _addSimulationControl(propArrivalRule);
    _addSimulationControl(propAdvanceOn);
	_addSimulationControl(propCapacity);
	_addSimulationControl(propQueue);
	_addSimulationControl(propSignal);

	_constructionComplete = true;
	_createEditableDataDefinitions();
}

Buffer::~Buffer() {
}

std::string Buffer::show() {
	return ModelComponent::show() + "";
}

// public static

ModelComponent* Buffer::LoadInstance(Model* model, PersistenceRecord *fields) {
	Buffer* newComponent = new Buffer(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Buffer instance: " + std::string(e.what()));
	}
	return newComponent;
}

PluginInformation* Buffer::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Buffer>(), &Buffer::LoadInstance, &Buffer::NewInstance);
	info->setDescriptionHelp("//@TODO");
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("signaldata.so");
	info->setCategory("Arena/Synchronization");
	return info;
}

// protected virtual -- must be overriden

void Buffer::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_capacity == 0) {
		traceError("Buffer \"" + getName() + "\" received entity with invalid Capacity=0");
		return;
	}
	if (_buffer == nullptr) {
		traceError("Buffer \"" + getName() + "\" received entity without an associated Queue");
		return;
	}
	if (_advanceOn == AdvanceOn::NewArrivals) {
		// just move on
		Entity* first = _advance(entity);
		if (first != nullptr) {
			traceSimulation(this, "Sending entity to the next component");
			_parentModel->sendEntityToComponent(first, _connections->getFrontConnection());
		}
	} else { // advance on signal. Do not move. Only check if buffer is full
		if (_buffer->size() >= _capacity) { // full buffer
			traceSimulation(this, "Entity arrived on a full buffer");
			switch (_arrivalOnFullBufferRule) {
				case ArrivalOnFullBufferRule::Dispose:
					traceSimulation(this, "Disposing arriving entity "+entity->getName());
					break;
				case ArrivalOnFullBufferRule::SendToBulkPort:
					traceSimulation(this, "Sending entity to the bulk port");
					_parentModel->sendEntityToComponent(entity, _connections->getConnectionAtPort(1));
					break;
				case ArrivalOnFullBufferRule::ReplaceLastPosition:
				{
					Waiting* waiting = _buffer->getAtRank(_buffer->size() - 1);
					Entity* replaced = waiting != nullptr ? waiting->getEntity() : nullptr;
					if (replaced == nullptr) {
						traceError("Buffer \"" + getName() + "\" found an invalid entity in its Queue");
						break;
					}
					traceSimulation(this, "Entity "+entity->getName()+" will replace entity "+replaced->getName()+" on the buffer");
					traceSimulation(this, "Disposing replaced entity "+replaced->getName());
					_buffer->removeElement(waiting);
					_parentModel->removeEntity(replaced);
					_buffer->insertElement(new Waiting(entity, _parentModel->getSimulation() != nullptr ? _parentModel->getSimulation()->getSimulatedTime() : 0.0, this));
					break;
				}
				case ArrivalOnFullBufferRule::num_elements:
					traceError("Invalid ArrivalOnFullBufferRule enum value: num_elements");
					break;
			}
		} else { // insert
			_buffer->insertElement(new Waiting(entity, _parentModel->getSimulation() != nullptr ? _parentModel->getSimulation()->getSimulatedTime() : 0.0, this));
		}
	}
}

bool Buffer::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_arrivalOnFullBufferRule = static_cast<ArrivalOnFullBufferRule>(fields->loadField("arrivalOnFullBufferRule", static_cast<int>(DEFAULT.arrivalOnFullBufferRule)));
		_advanceOn = static_cast<AdvanceOn>(fields->loadField("advanceOn", static_cast<int>(DEFAULT.advanceOn)));
		_capacity = fields->loadField("capacity", DEFAULT.capacity);
		_bufferReferenceName = fields->loadField("queue", "");
		if (!_bufferReferenceName.empty()) {
			Queue* loadedQueue = dynamic_cast<Queue*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), _bufferReferenceName));
			_discardDefaultQueue();
			_buffer = loadedQueue;
			if (_buffer != nullptr) _bufferReferenceName.clear();
		}
		std::string signalName = fields->loadField("signalData", "");
		if (signalName != "") {
			_attachedSignal = dynamic_cast<SignalData*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<SignalData>(), signalName));
		}
	}
	return res;
}

void Buffer::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("arrivalOnFullBufferRule", static_cast<int>(_arrivalOnFullBufferRule), static_cast<int>(DEFAULT.arrivalOnFullBufferRule), saveDefaultValues);
	fields->saveField("advanceOn", static_cast<int>(_advanceOn), static_cast<int>(DEFAULT.advanceOn), saveDefaultValues);
	fields->saveField("capacity", _capacity, DEFAULT.capacity, saveDefaultValues);
	if (_buffer != nullptr) {
		fields->saveField("queue", _buffer->getName(), "", saveDefaultValues);
	}
	if (_attachedSignal != nullptr) {
		fields->saveField("signalData", _attachedSignal->getName(), "", saveDefaultValues);
	}
}


// protected virtual -- could be overriden

//ParserChangesInformation* TemplateDataDefinition::_getParserChangesInformation() {}

bool Buffer::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_capacity == 0) {
		errorMessage = "Buffer \"" + getName() + "\" must have Capacity greater than zero";
		traceError(errorMessage);
		resultAll = false;
	}
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Queue>(), _buffer, "Queue", errorMessage);
	if (_advanceOn == AdvanceOn::Signal && _attachedSignal == nullptr) {
		errorMessage = "Buffer \"" + getName() + "\" configured with AdvanceOn=Signal requires a valid SignalData";
		traceError(errorMessage);
		resultAll = false;
	}
	return resultAll;
}

ParserChangesInformation* Buffer::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void Buffer::_initBetweenReplications() {
}

unsigned int Buffer::_handlerForSignalDataEvent(SignalData* signalData) {
	// got a signal. Buffer will advance
	traceSimulation(this, "Buffer "+this->getName()+" received signal "+signalData->getName());
	Entity* first = _advance(nullptr);
	traceSimulation(this, "Buffer entities moved forward");
	if (first != nullptr) {
		traceSimulation(this, "Entity "+first->getName()+" was in the first position of the buffer");
		_parentModel->sendEntityToComponent(first, this->getConnectionManager()->getFrontConnection());
	} else {
		traceSimulation(this, "First position of the buffer was empty");
	}
	return 1;
}


// void Buffer::_createAttachedAttributes() { }

void Buffer::_addSimulationControl(SimulationControl* property) {
	ModelDataDefinition::_addSimulationControl(property);
}

Queue* Buffer::getQueue() const {
	return _buffer;
}

void Buffer::setQueue(Queue* newQueue) {
	if (_buffer == newQueue) {
		return;
	}
	if (_defaultBufferQueue != nullptr && _defaultBufferQueue != newQueue) {
		_defaultBufferQueue = nullptr;
	}
	_buffer = newQueue;
	_bufferReferenceName.clear();
	if (_buffer != nullptr) {
		_optionalEditableDataDefinitionInsert("Queue", _buffer);
	} else {
		_optionalEditableDataDefinitionRemove("Queue");
	}
}


SignalData *Buffer::getsignal() const {
	return _attachedSignal;
}

void Buffer::setSignal(SignalData *newSignal) {
	_attachedSignal = newSignal;
}

unsigned int Buffer::getcapacity() const {
	return _capacity;
}

void Buffer::setCapacity(unsigned int newCapacity) {
	_capacity = newCapacity;
}

Buffer::AdvanceOn Buffer::getadvanceOn() const {
	return _advanceOn;
}

void Buffer::setAdvanceOn(Buffer::AdvanceOn newAdvanceOn) {
	_advanceOn = newAdvanceOn;
}

Buffer::ArrivalOnFullBufferRule Buffer::getarrivalOnFullBufferRule() const {
	return _arrivalOnFullBufferRule;
}

void Buffer::setArrivalOnFullBufferRule(Buffer::ArrivalOnFullBufferRule newArrivalOnFullBufferRule){
	_arrivalOnFullBufferRule = newArrivalOnFullBufferRule;
}

Entity* Buffer::_advance(Entity* enteringEntity) {
	if (_buffer == nullptr || _buffer->size() == 0) {
		if (_buffer != nullptr && enteringEntity != nullptr) {
			_buffer->insertElement(new Waiting(enteringEntity, _parentModel->getSimulation() != nullptr ? _parentModel->getSimulation()->getSimulatedTime() : 0.0, this));
		}
		return nullptr;
	}
	Waiting* waiting = _buffer->first();
	Entity* result = waiting != nullptr ? waiting->getEntity() : nullptr;
	_buffer->removeElement(waiting);
	if (enteringEntity != nullptr) {
		_buffer->insertElement(new Waiting(enteringEntity, _parentModel->getSimulation() != nullptr ? _parentModel->getSimulation()->getSimulatedTime() : 0.0, this));
	}
	return result;
}

// void Buffer::_createInternalStatisticReporters() { }

void Buffer::_createEditableDataDefinitions() {
	PluginManager* pm = _parentModel->getParentSimulator()->getPluginManager();
	if (!_bufferReferenceName.empty() && _buffer == nullptr) {
		_buffer = dynamic_cast<Queue*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), _bufferReferenceName));
		if (_buffer != nullptr) _bufferReferenceName.clear();
	}
	if (_constructionComplete && _buffer == nullptr && _bufferReferenceName.empty()) {
		_buffer = new Queue(_parentModel, getName() + ".Queue");
		_defaultBufferQueue = _buffer;
	}
	if (_buffer != nullptr) {
		_optionalEditableDataDefinitionInsert("Queue", _buffer);
	} else {
		_optionalEditableDataDefinitionRemove("Queue");
	}
	//attached
	if (_advanceOn == AdvanceOn::Signal) {
		if (_signalWithRegisteredHandler != nullptr && _signalWithRegisteredHandler != _attachedSignal) {
			_signalWithRegisteredHandler->removeSignalDataEventHandler(this);
			_signalWithRegisteredHandler = nullptr;
		}
		if (_attachedSignal  == nullptr) {
			_attachedSignal = pm->newInstance<SignalData>(_parentModel, getName() + "." + "SignalData");
			if (_attachedSignal == nullptr) {
				traceError("Buffer \"" + getName() + "\" failed to create SignalData while configured with AdvanceOn=Signal");
				_optionalEditableDataDefinitionRemove("SignalData");
				return;
			}
		}
		if (_attachedSignal == nullptr) {
			_optionalEditableDataDefinitionRemove("SignalData");
			return;
		}
		SignalData::SignalDataEventHandler handler = SignalData::SetSignalDataEventHandler<Buffer>(&Buffer::_handlerForSignalDataEvent, this);
		if (!_attachedSignal->hasSignalDataEventHandler(this)) {
			_attachedSignal->addSignalDataEventHandler(handler, this);
		}
		_signalWithRegisteredHandler = _attachedSignal;
		_optionalEditableDataDefinitionInsert("SignalData", _attachedSignal);
	} else {
		if (_signalWithRegisteredHandler != nullptr) {
			_signalWithRegisteredHandler->removeSignalDataEventHandler(this);
			_signalWithRegisteredHandler = nullptr;
		}
		_optionalEditableDataDefinitionRemove("SignalData");
	}
}

void Buffer::_discardDefaultQueue() {
	if (_defaultBufferQueue == nullptr) {
		return;
	}
	Queue* defaultQueue = _defaultBufferQueue;
	_defaultBufferQueue = nullptr;
	if (defaultQueue->size() == 0) {
		delete defaultQueue;
	}
}
