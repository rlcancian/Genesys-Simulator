/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   Label.cpp
 * Author: rlcancian
 *
 * Created on 15 de janeiro de 2022, 10:13
 */

#include "plugins/data/DiscreteProcessing/Label.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Label::GetPluginInformation;
}
#endif

ModelDataDefinition* Label::NewInstance(Model* model, std::string name) {
	return new Label(model, name);
}

Label::Label(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Label>(), name) {
//	_addProperty(new PropertyT<std::string>(Util::TypeOf<Label>(), "Label",
//			DefineGetter<Label,std::string>(this, &Label::getLabel),
//			DefineSetter<Label,std::string>(this, &Label::setLabel)));

	SimulationControlGeneric<std::string>* propLabel = new SimulationControlGeneric<std::string>(
									std::bind(&Label::getLabel, this), std::bind(&Label::setLabel, this, std::placeholders::_1),
									Util::TypeOf<Label>(), getName(), "Label", "");

	_parentModel->getControls()->insert(propLabel);

	// setting properties
	_addProperty(propLabel);													
}

// static

ModelDataDefinition* Label::LoadInstance(Model* model, PersistenceRecord *fields) {
	Label* newElement = new Label(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

PluginInformation* Label::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Label>(), &Label::LoadInstance, &Label::NewInstance);
	std::string help = "Label modeldatum module represents a name to where entities can be sent to.";
	help += "Any receiving transfer module such as 'Enter' can be assigned to a label.";
	help += "Any other sending transfer module such as 'Route' can send an entity to a label, which corresponds to send it to the receiving module";
	help += "TIPICAL USES include sending an entity to somewhere else without a direct connection.";
	info->setDescriptionHelp(help);
	//info->setDescriptionHelp("");
	//info->setObservation("");
	//info->setMinimumOutputs();
	//info->setDynamicLibFilenameDependencies();
	//info->setFields();
	// ...
	return info;
}

//

std::string Label::show() {
	std::string msg = ModelDataDefinition::show();
	msg += ",label=\"" + _label + "\"";
	msg += ",enteringComponent=";
	msg += (_enteringLabelComponent != nullptr ? _enteringLabelComponent->getName() : "NULL");
	return msg;
}

void Label::setLabel(std::string _label) {
	this->_label = _label;
}

std::string Label::getLabel() const {
	return _label;
}

void Label::setEnterIntoLabelComponent(ModelComponent* enteringLabelComponent) {
	_enteringLabelComponent = enteringLabelComponent;
}

ModelComponent* Label::getEnterIntoLabelComponent() const {
	return _enteringLabelComponent;
}

void Label::sendEntityToLabelComponent(Entity* entity, double timeDelay) {
	//_parentModel->sendEntityToComponent(entity, _enteringLabelComponent->getConnections()->getFrontConnection(), timeDelay);
	if (_enteringLabelComponent == nullptr) {
		traceError("Label \"" + getName() + "\" has no entering component defined. Entity was not sent.", TraceManager::Level::L2_results);
		return;
	}
	_parentModel->sendEntityToComponent(entity, _enteringLabelComponent, timeDelay);
}

// must be overriden

bool Label::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			this->_label = fields->loadField("label", "");
			this->_enteringLabelComponent = nullptr;
			std::string componentName = fields->loadField("enteringComponentName", "");
			if (!componentName.empty()) {
				ModelComponent* comp = _parentModel->getComponentManager()->find(componentName);
				if (comp == nullptr) {
					traceError("Label \"" + getName() + "\" could not resolve entering component \"" + componentName + "\" while loading.", TraceManager::Level::L2_results);
				} else {
					this->_enteringLabelComponent = comp;
				}
			}
		} catch (...) {
		}
	}
	return res;
}

void Label::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("label", this->_label, "", saveDefaultValues);
	if (_enteringLabelComponent != nullptr) {
		fields->saveField("enteringComponentName", _enteringLabelComponent->getName(), "", saveDefaultValues);
	}
}

// could be overriden

bool Label::_check(std::string& errorMessage) {
	bool resultAll = true;
	_attachedDataRemove("EnteringLabelComponent");
	if (_enteringLabelComponent == nullptr) {
		errorMessage += "Label \"" + getName() + "\" entering component was not defined. ";
		resultAll = false;
	} else {
		ModelComponent* modelComponentByName = _parentModel->getComponentManager()->find(_enteringLabelComponent->getName());
		if (modelComponentByName == nullptr || modelComponentByName != _enteringLabelComponent) {
			errorMessage += "Label \"" + getName() + "\" entering component \"" + _enteringLabelComponent->getName() + "\" is stale or not owned by this model. ";
			resultAll = false;
		} else {
			_attachedDataInsert("EnteringLabelComponent", _enteringLabelComponent);
		}
	}
	return resultAll;
}

//ParserChangesInformation* Label::_getParserChangesInformation() {}

//void Label::_initBetweenReplications() {}

//void Label::_createInternalAndAttachedData() {}
