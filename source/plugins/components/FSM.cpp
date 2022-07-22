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
// TODO - Set final states
// TODO - Pass entity along when reaching final states
// TODO - Implement _loadInstance, _saveInstance and _check
// TODO - Update docs

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

void FSM::_transitionAll()
{
    for (const auto &entityState : _entityStates)
    {
        Entity *entity = entityState.first;
        FSMState *from = entityState.second;
        List<FSM::TransitionData> *transitionDatum = _transitionMap[from];

        FSM::TransitionData *transitionToFollow = _findTransition(transitionDatum);

        if (transitionToFollow == nullptr)
        {
            _parentModel->getTracer()->trace("Entity " + entity->getName() + " had no valid transition");
            continue;
        }

        _parentModel->getTracer()->trace("Transitioning " + entity->getName() + " from " + from->getName() + " to " + transitionToFollow->to->getName() + " through " + transitionToFollow->transition->getName());

        transitionToFollow->transition->perform();

        // TODO - Does this affect the loop we're in?? Should we create a new map and merge it after the loop?
        _entityStates[entity] = transitionToFollow->to;
    }
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
    return info;
}

// protected virtual -- must be overriden

void FSM::_onDispatchEvent(Entity *entity, unsigned int inputPortNumber)
{
    // _parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
    // this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
    _entityStates[entity] = _initialState;
    _transitionAll();
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
    return resultAll;
}
