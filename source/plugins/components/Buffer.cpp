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

#include "Buffer.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

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
	SimulationControlGenericClass<SignalData*, Model*, SignalData>* propSignal = new SimulationControlGenericClass<SignalData*, Model*, SignalData>(
				_parentModel,
				std::bind(&Buffer::getsignal, this),
				std::bind(&Buffer::setSignal, this, std::placeholders::_1),
				Util::TypeOf<Buffer>(), getName(), "Signal", "");

    _parentModel->getControls()->insert(propArrivalRule);
    _parentModel->getControls()->insert(propAdvanceOn);
	_parentModel->getControls()->insert(propCapacity);
	_parentModel->getControls()->insert(propSignal);

	// setting properties
    _addProperty(propArrivalRule);
    _addProperty(propAdvanceOn);
	_addProperty(propCapacity);
	_addProperty(propSignal);
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

	}
	return newComponent;
}

PluginInformation* Buffer::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Buffer>(), &Buffer::LoadInstance, &Buffer::NewInstance);
	info->setDescriptionHelp("//@TODO");
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("signaldata.so");
	return info;
}

// protected virtual -- must be overriden

void Buffer::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_capacity == 0) {
		traceError("Buffer \"" + getName() + "\" received entity with invalid Capacity=0");
		return;
	}
	if (_buffer->size() != _capacity) {
		_buffer->resize(_capacity, nullptr);
	}
	if (_advanceOn == AdvanceOn::NewArrivals) {
		// just move on
		Entity* first = _advance(entity);
		if (first != nullptr) {
			_parentModel->sendEntityToComponent(first, _connections->getFrontConnection());
		}
	} else { // advance on signal. Do not move. Only check if buffer is full
		if (_buffer->at(_capacity-1) != nullptr) { // full buffer
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
					Entity* replaced = _buffer->at(_capacity-1);
					traceSimulation(this, "Entity "+entity->getName()+" will replace entity "+replaced->getName()+" on the buffer");
					traceSimulation(this, "Disposing replaced entity "+replaced->getName());
					_parentModel->removeEntity(replaced);
					_buffer->at(_capacity-1) = entity;
					break;
				}
				case ArrivalOnFullBufferRule::num_elements:
					traceError("Invalid ArrivalOnFullBufferRule enum value: num_elements");
					break;
			}
		} else { // insert
			// Keep insertion coherent by placing the arriving entity in the first free slot.
			for (unsigned int i = 0; i < _capacity; i++) {
				if (_buffer->at(i) == nullptr) {
					_buffer->at(i) = entity;
					break;
				}
			}
		}
	}
}

bool Buffer::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_arrivalOnFullBufferRule = static_cast<ArrivalOnFullBufferRule>(fields->loadField("arrivalOnFullBufferRule", static_cast<int>(DEFAULT.arrivalOnFullBufferRule)));
		_advanceOn = static_cast<AdvanceOn>(fields->loadField("advanceOn", static_cast<int>(DEFAULT.advanceOn)));
		_capacity = fields->loadField("capacity", DEFAULT.capacity);
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
	if (_attachedSignal != nullptr) {
		fields->saveField("signalData", _attachedSignal->getName(), "", saveDefaultValues);
	}
}


// protected virtual -- could be overriden

//ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool Buffer::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_capacity == 0) {
		errorMessage = "Buffer \"" + getName() + "\" must have Capacity greater than zero";
		traceError(errorMessage);
		resultAll = false;
	}
	if (_advanceOn == AdvanceOn::Signal && _attachedSignal == nullptr) {
		errorMessage = "Buffer \"" + getName() + "\" configured with AdvanceOn=Signal requires a valid SignalData";
		traceError(errorMessage);
		resultAll = false;
	}
	if (_buffer != nullptr && _capacity > 0 && _buffer->size() != _capacity) {
		_buffer->resize(_capacity, nullptr); // keep check idempotent for rechecks
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
	_buffer->clear();
	_buffer->resize(_capacity);
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


void Buffer::_createInternalAndAttachedData() {
	PluginManager* pm = _parentModel->getParentSimulator()->getPluginManager();
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
				_attachedDataRemove("SignalData");
				return;
			}
		}
		SignalData::SignalDataEventHandler handler = SignalData::SetSignalDataEventHandler<Buffer>(&Buffer::_handlerForSignalDataEvent, this);
		if (!_attachedSignal->hasSignalDataEventHandler(this)) {
			_attachedSignal->addSignalDataEventHandler(handler, this);
		}
		_signalWithRegisteredHandler = _attachedSignal;
		_attachedDataInsert("SignalData", _attachedSignal);
	} else {
		if (_signalWithRegisteredHandler != nullptr) {
			_signalWithRegisteredHandler->removeSignalDataEventHandler(this);
			_signalWithRegisteredHandler = nullptr;
		}
		_attachedDataRemove("SignalData");
	}
}

void Buffer::_addProperty(SimulationControl* property) {
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
	if (_buffer == nullptr || _buffer->empty()) {
		// Safety guard for inconsistent runtime states (for example, Capacity misconfiguration).
		return nullptr;
	}
	Entity *result = _buffer->front();
	_buffer->erase(_buffer->begin());
	_buffer->push_back(enteringEntity);
	return result;
}
