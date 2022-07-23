/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Dummy.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Maio de 2019, 18:41
 */

// TODO - Set initial state ✅
// TODO - Check transition conditions when entity comes in ✅
// TODO - Perform transition (if appropriate) ✅
// TODO - Set final states ✅
// TODO - Pass entity along when reaching final states ✅
// TODO - Implement _loadInstance, _saveInstance and _check
// TODO - Update docs
// TODO - Listen to `onReplicationStep` ✅

// - Basic
// - Hierarchical
// - Transition side effects
// - Transition conditions
// - Transition delays
// - Waiting for condition

#include "FSM.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation()
{
    return &FSM::GetPluginInformation;
}
#endif

ModelDataDefinition *FSM::NewInstance(Model *model, std::string name)
{
    return new FSM(model, name);
}

FSM::FSM(Model *model, std::string name) : ModelComponent(model, Util::TypeOf<FSM>(), name)
{
}

std::string FSM::show()
{
    return ModelComponent::show() + "";
}

// public

List<FSMState *> *FSM::states() const
{
    return _states;
}

void FSM::insertState(FSMState *state)
{
    _states->insert(state);
    _transitionMap[state] = new List<TransitionData>();
}

void FSM::insertTransition(FSMTransition *transition, FSMState *from, FSMState *to)
{
    TransitionData transitionData = {transition, to};

    _transitionMap[from]->insert(transitionData);
}

void FSM::setInitialState(FSMState *state)
{
    _initialState = state;
}

// private

void FSM::_transition(Entity *entity)
{
    FSMState *currentState = _entityStates[entity];
    List<FSM::TransitionData> *transitionDatum = _transitionMap[currentState];

    FSM::TransitionData *transitionDataToFollow = _findTransition(transitionDatum);

    if (transitionDataToFollow == nullptr)
    {
        _parentModel->getTracer()->trace("Entity " + entity->getName() + " had no valid transition. Listening for events");

        // Only insert the entity into the list if it is not already there
        if (_entitiesWaitingForCondition->find(entity) == _entitiesWaitingForCondition->list()->end())
        {
            _entitiesWaitingForCondition->insert(entity);
        }
        return;
    }

    _entitiesWaitingForCondition->remove(entity);

    _parentModel->getTracer()->trace("Transitioning " + entity->getName() + " from " + currentState->getName() + " to " + transitionDataToFollow->to->getName() + " through " + transitionDataToFollow->transition->getName());

    transitionDataToFollow->transition->perform(entity);

    _entityStates[entity] = transitionDataToFollow->to;

    ModelComponent *nextComponent = this;
    if (transitionDataToFollow->to->hasRefinement())
    {
        nextComponent = transitionDataToFollow->to->refinement();
    }

    _parentModel->sendEntityToComponent(entity, nextComponent, transitionDataToFollow->transition->delay());
}

FSM::TransitionData *FSM::_findTransition(List<TransitionData> *transitions)
{
    for (TransitionData &transitionData : *transitions->list())
    {
        if (transitionData.transition->canPerform())
        {
            return &transitionData;
        }
    }

    return nullptr;
}

void FSM::_handlerForAfterProcessEventEvent(SimulationEvent *event)
{
    // After processing an event, check if there are any entities waiting for a condition, which may be true now
    for (Entity *entity : *_entitiesWaitingForCondition->list())
    {
        std::cout << "ENTITY WAITING FOR CONDITION :) " << entity->getName() << std::endl;
        _transition(entity);
    }
}

// public static

ModelComponent *FSM::LoadInstance(Model *model, std::map<std::string, std::string> *fields)
{
    FSM *newComponent = new FSM(model);
    try
    {
        newComponent->_loadInstance(fields);
    }
    catch (const std::exception &e)
    {
    }
    return newComponent;
}

PluginInformation *FSM::GetPluginInformation()
{
    PluginInformation *info = new PluginInformation(Util::TypeOf<FSM>(), &FSM::LoadInstance, &FSM::NewInstance);
    info->setDescriptionHelp("//@TODO");
    info->setSendTransfer(true);
    return info;
}

// protected virtual -- must be overriden

void FSM::_onDispatchEvent(Entity *entity, unsigned int inputPortNumber)
{
    // _parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
    // this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
    if (_entityStates.find(entity) == _entityStates.end())
    {
        // Entity is not yet in the FSM, so insert it into the initial state
        _entityStates[entity] = _initialState;

        if (_initialState->hasRefinement())
        {
            _parentModel->sendEntityToComponent(entity, _initialState->refinement());
            return;
        }
    }

    if (_entityStates[entity]->isFinal())
    {
        _parentModel->getTracer()->trace("Entity " + entity->getName() + " reached final state " + _entityStates[entity]->getName());
        _entityStates.erase(entity);
        _parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
        return;
    }

    _transition(entity);
}

bool FSM::_loadInstance(std::map<std::string, std::string> *fields)
{
    bool res = ModelComponent::_loadInstance(fields);
    if (res)
    {
        // @TODO: not implemented yet
    }
    return res;
}

std::map<std::string, std::string> *FSM::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelComponent::_saveInstance(saveDefaultValues);
    // @TODO: not implemented yet
    return fields;
}

// protected virtual -- could be overriden

// ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool FSM::_check(std::string *errorMessage)
{
    bool resultAll = true;

    if (resultAll)
    {
        _parentModel->getOnEvents()->addOnAfterProcessEventHandler(this, &FSM::_handlerForAfterProcessEventEvent);
    }

    return resultAll;
}

void FSM::_createInternalAndAttachedData()
{
    unsigned int connections = 1;
    for (FSMState *state : *_states->list())
    {
        if (state->hasRefinement())
        {
            state->refinement()->setModelLevel(_id);
            Connection *connection = new Connection();
            connection->component = state->refinement();
            connection->port = 0;
            this->getConnections()->insertAtPort(connections, connection);
            connections++;

            state->refinement()->getConnections()->connections()->clear();
            state->refinement()->getConnections()->insert(this);
        }
    }
}
