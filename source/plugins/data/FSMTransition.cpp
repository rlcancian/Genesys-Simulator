/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   FSMTransition.cpp
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:26
 */

#include "FSMTransition.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation()
{
    return &FSMTransition::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition *FSMTransition::NewInstance(Model *model, std::string name)
{
    return new FSMTransition(model, name);
}

FSMTransition::FSMTransition(Model *model, std::string name) : ModelDataDefinition(model, Util::TypeOf<FSMTransition>(), name)
{
}

// public

std::string FSMTransition::show()
{
    return ModelDataDefinition::show();
}

// public static

ModelDataDefinition *FSMTransition::LoadInstance(Model *model, std::map<std::string, std::string> *fields)
{
    FSMTransition *newElement = new FSMTransition(model);
    try
    {
        newElement->_loadInstance(fields);
    }
    catch (const std::exception &e)
    {
    }
    return newElement;
}

PluginInformation *FSMTransition::GetPluginInformation()
{
    PluginInformation *info = new PluginInformation(Util::TypeOf<FSMTransition>(), &FSMTransition::LoadInstance, &FSMTransition::NewInstance);
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

bool FSMTransition::_loadInstance(std::map<std::string, std::string> *fields)
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

std::map<std::string, std::string> *FSMTransition::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelDataDefinition::_saveInstance(saveDefaultValues); // Util::TypeOf<Queue>());
    return fields;
}

// protected virtual -- could be overriden

// ParserChangesInformation* FSMTransition::_getParserChangesInformation() {}

bool FSMTransition::_check(std::string *errorMessage)
{
    bool resultAll = true;
    return resultAll;
}

void FSMTransition::_initBetweenReplications()
{
}

// private
