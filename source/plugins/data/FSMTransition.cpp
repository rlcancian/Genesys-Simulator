/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   FSMTransition.cpp
 * Author: Henrique da Cunha Buss
 *
 * Created on 28 de maio de 2022, 11:20
 */

#include "FSMTransition.h"
#include "../../kernel/simulator/Model.h"

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
    PropertyT<std::string> *guardExpressionProperty = new PropertyT<std::string>(
        Util::TypeOf<FSMTransition>(),
        "Guard Expression",
        DefineGetter<FSMTransition, std::string>(this, &FSMTransition::guardExpression),
        DefineSetter<FSMTransition, std::string>(this, &FSMTransition::setGuardExpression));
    model->getControls()->insert(guardExpressionProperty);
    _addProperty(guardExpressionProperty);

    PropertyT<std::string> *delayExpressionProperty = new PropertyT<std::string>(
        Util::TypeOf<FSMTransition>(),
        "Delay Expression",
        DefineGetter<FSMTransition, std::string>(this, &FSMTransition::delayExpression),
        DefineSetter<FSMTransition, std::string>(this, &FSMTransition::setDelayExpression));
    model->getControls()->insert(delayExpressionProperty);
    _addProperty(delayExpressionProperty);

    PropertyT<Util::TimeUnit> *timeUnitProperty = new PropertyT<Util::TimeUnit>(
        Util::TypeOf<FSMTransition>(),
        "Delay Time Unit",
        DefineGetter<FSMTransition, Util::TimeUnit>(this, &FSMTransition::delayTimeUnit),
        DefineSetter<FSMTransition, Util::TimeUnit>(this, &FSMTransition::setDelayTimeUnit));
    model->getControls()->insert(timeUnitProperty);
    _addProperty(timeUnitProperty);
}

// public

std::string FSMTransition::show()
{
    return ModelDataDefinition::show() +
           ",guardExpression=" + this->_guardExpression +
           ",delayExpression=" + this->_delayExpression +
           ",timeUnit=" + std::to_string(static_cast<int>(this->_delayTimeUnit));
}

void FSMTransition::setGuardExpression(std::string expression)
{
    _guardExpression = expression;
}

std::string FSMTransition::guardExpression()
{
    return _guardExpression;
}

void FSMTransition::setDelayExpression(std::string expression)
{
    _delayExpression = expression;
}

std::string FSMTransition::delayExpression()
{
    return _delayExpression;
}

void FSMTransition::setDelayTimeUnit(Util::TimeUnit timeUnit)
{
    _delayTimeUnit = timeUnit;
}

Util::TimeUnit FSMTransition::delayTimeUnit() const
{
    return _delayTimeUnit;
}

double FSMTransition::delay() const
{
    double waitTime = _parentModel->parseExpression(_delayExpression);
    Util::TimeUnit standardTimeUnit = _parentModel->getSimulation()->getReplicationBaseTimeUnit();
    waitTime *= Util::TimeUnitConvert(_delayTimeUnit, standardTimeUnit);

    return waitTime;
}

void FSMTransition::onTransition(std::function<void(Model *, Entity *)> handler)
{
    _onTransition = handler;
}

void FSMTransition::perform(Entity *entity)
{
    if (_onTransition == nullptr)
    {
        return;
    }

    _onTransition(_parentModel, entity);
}

bool FSMTransition::canPerform()
{
    return _parentModel->parseExpression(_guardExpression);
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
    std::string text = "The FSMTransition class is used to define a transition in a FSM. ";
    text += "A transition can have a guard expression and a delay expression. ";
    text += "The guard expression is used to define the condition to perform the transition. ";
    text += "The delay expression is used to define the time to wait before performing the transition. ";
    text += "The time unit is used to define the time unit of the delay expression.";
    info->setDescriptionHelp(text);
    return info;
}

// protected virtual -- must be overriden

bool FSMTransition::_loadInstance(std::map<std::string, std::string> *fields)
{
    bool res = ModelDataDefinition::_loadInstance(fields);
    if (res)
    {
        _guardExpression = LoadField(fields, "guardExpression", DEFAULT.guardExpression);
        _delayExpression = LoadField(fields, "delayExpression", DEFAULT.delayExpression);
        _delayTimeUnit = LoadField(fields, "delayTimeUnit", DEFAULT.delayTimeUnit);
    }
    return res;
}

std::map<std::string, std::string> *FSMTransition::_saveInstance(bool saveDefaultValues)
{
    std::map<std::string, std::string> *fields = ModelDataDefinition::_saveInstance(saveDefaultValues); // Util::TypeOf<Queue>());
    SaveField(fields, "guardExpression", _guardExpression, DEFAULT.guardExpression, saveDefaultValues);
    SaveField(fields, "delayExpression", _delayExpression, DEFAULT.delayExpression, saveDefaultValues);
    SaveField(fields, "delayTimeUnit", _delayTimeUnit, DEFAULT.delayTimeUnit, saveDefaultValues);
    return fields;
}

// protected virtual -- could be overriden

bool FSMTransition::_check(std::string *errorMessage)
{
    bool validGuardExpression = _parentModel->checkExpression(_guardExpression, "Guard expression", errorMessage);
    bool validDelayExpression = _parentModel->checkExpression(_delayExpression, "Delay expression", errorMessage);
    return validGuardExpression && validDelayExpression;
}

// private
