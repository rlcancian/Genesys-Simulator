/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   FSM.cpp
 * Author: Henrique da Cunha Buss
 *
 * Created on 22 de Maio de 2019, 18:41
 */

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
    _parentModel->getTracer()->trace("State " + state->getName() + " inserted into FSM " + this->getName());
}

void FSM::insertTransition(FSMTransition *transition, FSMState *from, FSMState *to)
{
    TransitionData transitionData = {transition, to};

    _transitionMap[from]->insert(transitionData);
    _parentModel->getTracer()->trace("Transition " + transition->getName() + " inserted into FSM " + this->getName());
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
    for (unsigned int i = 0; i < _entitiesWaitingForCondition->size(); i++)
    {
        Entity *entity = _entitiesWaitingForCondition->getAtRank(i);
        if (entity == event->getCurrentEvent()->getEntity())
        {
            continue;
        }

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
    std::string text = "The FSM module is used to model Finite State Machines. ";
    text += "Each FSM has a set of FSMStates and a set of FSMTransitions, which connect the states. ";
    text += "Each state may have a refinement (another FSM) inside of it. ";
    text += "When an entity enters a state, it will be sent to the refinement of that state, if any. ";
    info->setDescriptionHelp(text);

    info->setSendTransfer(true);

    return info;
}

// protected virtual -- must be overriden

void FSM::_onDispatchEvent(Entity *entity, unsigned int inputPortNumber)
{
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
    }
    return res;
}

std::map<std::string, std::string> *FSM::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelComponent::_saveInstance(saveDefaultValues);
    return fields;
}

// protected virtual -- could be overriden

bool FSM::_check(std::string *errorMessage)
{
    if (_states->size() == 0)
    {
        std::string msg = "FSM " + this->getName() + " has no states. ";
        errorMessage->append(msg);
        return false;
    }

    if (_initialState == nullptr)
    {
        std::string msg = "FSM " + this->getName() + " has no initial state. ";
        errorMessage->append(msg);
        return false;
    }

    if (!_reachesFinalState(_initialState, new List<FSMState *>()))
    {
        std::string msg = "FSM " + this->getName() + " can't reach any final state. ";
        errorMessage->append(msg);
        return false;
    }

    _parentModel->getOnEvents()->addOnAfterProcessEventHandler(this, &FSM::_handlerForAfterProcessEventEvent);

    return true;
}

bool FSM::_reachesFinalState(FSMState *state, List<FSMState *> *visited)
{
    if (state->isFinal())
    {
        return true;
    }

    if (visited->find(state) != visited->list()->end())
    {
        return false;
    }

    visited->insert(state);
    for (TransitionData &transitionData : *_transitionMap[state]->list())
    {
        if (_reachesFinalState(transitionData.to, visited))
        {
            return true;
        }
    }

    return false;
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
