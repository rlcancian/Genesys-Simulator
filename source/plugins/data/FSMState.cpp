/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   FSMState.cpp
 * Author: Henrique da Cunha Buss
 *
 * Created on 11 de janeiro de 2022, 22:26
 */

#include "FSMState.h"
#include "../../kernel/simulator/Model.h"

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
    PropertyT<bool> *isFinalProperty = new PropertyT<bool>(
        Util::TypeOf<FSMState>(),
        "Is Final",
        DefineGetter<FSMState, bool>(this, &FSMState::isFinal),
        DefineSetter<FSMState, bool>(this, &FSMState::setIsFinal));
    model->getControls()->insert(isFinalProperty);
    _addProperty(isFinalProperty);
}

// public

std::string FSMState::show()
{
    return ModelDataDefinition::show();
}

bool FSMState::isFinal() const
{
    return _isFinal;
}

void FSMState::setIsFinal(bool isFinal)
{
    _isFinal = isFinal;
}

void FSMState::setRefinement(FSM *refinement)
{
    _refinement = refinement;
}

bool FSMState::hasRefinement() const
{
    return _refinement != nullptr;
}

FSM *FSMState::refinement() const
{
    return _refinement;
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
    std::string text = "The FSMState class is used to define a state in a FSM. ";
    text += "It can optionally have a refinement, meaning an entire FSM inside of it. ";
    text += "Entities will need to reach a final state of that FSM before going back to the parent FSM.";
    info->setDescriptionHelp(text);

    return info;
}

// protected virtual -- must be overriden

bool FSMState::_loadInstance(std::map<std::string, std::string> *fields)
{
    bool res = ModelDataDefinition::_loadInstance(fields);
    if (res)
    {
        _isFinal = LoadField(fields, "isFinal", false);
    }
    return res;
}

std::map<std::string, std::string> *FSMState::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelDataDefinition::_saveInstance(saveDefaultValues); // Util::TypeOf<Queue>());
    SaveField(fields, "isFinal", _isFinal, false, saveDefaultValues);
    return fields;
}

// protected virtual -- could be overriden

bool FSMState::_check(std::string *errorMessage)
{
    if (_refinement == nullptr)
    {
        return true;
    }

    return ModelComponent::Check(_refinement);
}

void FSMState::_initBetweenReplications()
{
}
