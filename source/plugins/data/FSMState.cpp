/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   FSMState.cpp
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:26
 */

#include "FSMState.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation()
{
    return &FSMState::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition *FSMState::NewInstance(Model *model, std::string name)
{
    return new FSMState(model, name);
}

FSMState::FSMState(Model *model, std::string name) : ModelDataDefinition(model, Util::TypeOf<FSMState>(), name)
{
}

// public

std::string FSMState::show()
{
    return ModelDataDefinition::show();
}

// public static

ModelDataDefinition *FSMState::LoadInstance(Model *model, std::map<std::string, std::string> *fields)
{
    FSMState *newElement = new FSMState(model);
    try
    {
        newElement->_loadInstance(fields);
    }
    catch (const std::exception &e)
    {
    }
    return newElement;
}

PluginInformation *FSMState::GetPluginInformation()
{
    PluginInformation *info = new PluginInformation(Util::TypeOf<FSMState>(), &FSMState::LoadInstance, &FSMState::NewInstance);
    info->setDescriptionHelp("//@TODO");
    // info->setDescriptionHelp("");
    // info->setObservation("");
    // info->setMinimumOutputs();
    // info->setDynamicLibFilenameDependencies();
    // info->setFields();
    //  ...
    return info;
}

// protected virtual -- must be overriden

bool FSMState::_loadInstance(std::map<std::string, std::string> *fields)
{
    bool res = ModelDataDefinition::_loadInstance(fields);
    if (res)
    {
        try
        {
        }
        catch (...)
        {
        }
    }
    return res;
}

std::map<std::string, std::string> *FSMState::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelDataDefinition::_saveInstance(saveDefaultValues); // Util::TypeOf<Queue>());
    return fields;
}

// protected virtual -- could be overriden

// ParserChangesInformation* FSMState::_getParserChangesInformation() {}

bool FSMState::_check(std::string *errorMessage)
{
    bool resultAll = true;
    return resultAll;
}

void FSMState::_initBetweenReplications()
{
}

// private
