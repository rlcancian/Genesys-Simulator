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
    _parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
    this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
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

void FSM::_initBetweenReplications()
{
}
