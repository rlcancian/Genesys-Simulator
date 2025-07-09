/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*
 * File:   ModelComponent.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 15:56
 */

#include "ModelComponent.h"
#include "Model.h"

//using namespace GenesysKernel;


//
// public: //! constructors
//

ModelComponent::ModelComponent(Model* model, std::string componentTypename, std::string name) : ModelDataDefinition(model, componentTypename, name, false) {
	model->getComponentManager()->insert(this);
	// ADD_PROPERTY(std::string, ModelComponent, "Description", getDescription, setDescription)
}

ModelComponent::~ModelComponent() {
    _parentModel->getComponentManager()->remove(this);
}


//
// public: //! new public user methods for this component
//

ConnectionManager* ModelComponent::getConnectionManager() const {
	return _connections; // @TODO How to know if it changes?
}

bool ModelComponent::hasBreakpointAt() {
	return _parentModel->getSimulation()->getBreakpointsOnComponent()->find(this) != _parentModel->getSimulation()->getBreakpointsOnComponent()->list()->end();
}

void ModelComponent::setDescription(std::string description) {
	if (_description != description) {
		this->_description = description;
		_hasChanged = true;
	}
}

std::string ModelComponent::getDescription() const {
	return _description;
}


// ...


//
// public: //! virtual methods
//


std::string ModelComponent::show() {
	return ModelDataDefinition::show(); // "{id=" + std::to_string(this->_id) + ",name=\""+this->_name + "\"}"; // , nextComponents[]=(" + _nextComponents->show() + ")}";
}


//
// public: //! static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

void ModelComponent::SaveInstance(PersistenceRecord *fields, ModelComponent* component) {
	component->trace("Writing component \"" + component->getName() + "\"", TraceManager::Level::L9_mostDetailed);
	try {
		component->_saveInstance(fields, component->_getSaveDefaultsOption());
	} catch (const std::exception& e) {
		component->traceError("Error executing component " + component->show(), e);
	}
}

void ModelComponent::CreateInternalData(ModelComponent* component) {
	//component->_model->getTraceManager()->trace(TraceManager::Level::blockArrival, "Writing component \"" + component->_name + "\""); //std::to_string(component->_id));
	try {
		component->_createInternalAndAttachedData();
	} catch (const std::exception e) {
		component->traceError("Error creating elements of component " + component->show(), e);
	};
}

bool ModelComponent::Check(ModelComponent* component) {
	component->trace("Checking " + component->_typename + ": \"" + component->getName() + "\""); //std::to_string(component->_id));
	bool res = false;
	std::string* errorMessage = new std::string();
	Util::IncIndent();
	{
		try {
			res = component->_check(errorMessage);
			if (!res) {
				component->traceError("Error: Checking has failed with message '" + *errorMessage + "'");
			}
		} catch (const std::exception& e) {
			component->traceError("Error verifying component " + component->show(), e);
		}
	}
	Util::DecIndent();
	return res;
}

void ModelComponent::DispatchEvent(Event* event) {
	Entity* entity = event->getEntity();
	ModelComponent* component = event->getComponent();
	unsigned int inputPortNumber = event->getComponentinputPortNumber();
    /* @TODO: Show entity based on a configuration
	//"Entity " +std::to_string(entity->entityNumber())
	std::string msg =  entity->getName() + " has arrived at component \"" + component->getName() + "\"";
	if (component->getDescription() != "")
		msg += ": " + component->getDescription();
	if (inputPortNumber > 0)
		msg += " by input " + std::to_string(inputPortNumber);
	component->traceSimulation(component, TraceManager::Level::L7_internal, msg); //:L6_arrival
	*/
	Util::IncIndent();
	try {
		component->_onDispatchEvent(entity, inputPortNumber);
	} catch (const std::exception& e) {
		component->traceError("Error executing component " + component->show(), e);
	}
	Util::DecIndent();
}


//
// protected: //! virtual method that must be overriden
//

bool ModelComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_description = fields->loadField("caption", DEFAULT.description);
		// Now it should load nextComponents. The problem is that the nextComponent may not be loaded yet.
	}
	return res;
}

void ModelComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("caption", _description, DEFAULT.description, saveDefaultValues);
	if (true) {//(_connections->size() != 1) { // save nextSize only if it is != 1
		fields->saveField("nexts", _connections->size(), DEFAULT.nextSize, saveDefaultValues);
	}
	unsigned short i;
	for (std::pair<unsigned int, Connection*> connectionPair : *_connections->connections()) {
		i = connectionPair.first; // output port
		if (_connections->connections()->size() == 1 && i == 0) {
			fields->saveField("nextId", connectionPair.second->component->_id, 0, saveDefaultValues);
		} else {
			fields->saveField("nextId" + Util::StrIndex(i), connectionPair.second->component->_id, 0, saveDefaultValues);
		}
		if (connectionPair.second->channel.portNumber != 0) {//((*it)->second != 0) { // save nextinputPortNumber only if it is != 0
			fields->saveField("nextinputPortNumber" + Util::StrIndex(i), connectionPair.second->channel.portNumber, DEFAULT.nextinputPortNumber, saveDefaultValues);
		}
	}
}


//
// protected: //! virtual methods that could be overriden by derived classes, if needed
//

/*
bool ModelComponent::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* ModelComponent::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	changes->getassignments().append("");
	changes->getexpressionProductions().append("");
	changes->getexpressions().append("");
	changes->getfunctionProdutions().append("");
	changes->getassignments().append("");
	changes->getincludes().append("");
	changes->gettokens().append("");
	changes->gettypeObjs().append("");
	return changes;
}
*/

/*
void ModelComponent::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void ModelComponent::_createInternalAndAttachedData() {
	if (_internalDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_internalDataDefinition = pm->newInstance<DummyElement>(_parentModel, getName() + "." + "JustaDummy");
		_internalDataInsert("JustaDummy", _internalDataDefinition);
	}
	if (_attachedDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_attachedDataDefinition = pm->newInstance<DummyElement>(_parentModel);
		_attachedDataInsert("JustaDummy", _attachedDataDefinition);
	}
}
*/

/*
void ModelComponent::_addProperty(PropertyBase* property) {
}
*/
