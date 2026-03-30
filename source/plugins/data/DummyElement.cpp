/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   DummyElement.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 11 de janeiro de 2022, 22:26
 */

#include "DummyElement.h"

#ifdef PLUGINCONNECT_DYNAMIC

/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DummyElement::GetPluginInformation;
}
#endif

//
// public: /// constructors
//

DummyElement::DummyElement(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<DummyElement>(), name) {
}


//
// public: /// new public user methods for this component
//

// ...


//
// public: /// virtual methods
//

std::string DummyElement::show() {
	return ModelDataDefinition::show();
}



//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//


PluginInformation* DummyElement::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DummyElement>(), &DummyElement::LoadInstance, &DummyElement::NewInstance);
	//info->setCategory("Discrete Processing");
	//info->setMinimumInputs(1);
	//info->setMinimumOutputs(1);
	//info->setMaximumInputs(1);
	//info->setMaximumOutputs(1);
	//info->setSource(false);
	//info->setSink(false);
	//info->setSendTransfer(false);
	//info->setReceiveTransfer(false);
	//info->insertDynamicLibFileDependence("...");
	//info->setDescriptionHelp("//@TODO");
	//info->setAuthor("...");
	//info->setDate("...");
	//info->setObservation("...");
	return info;
}

ModelDataDefinition* DummyElement::LoadInstance(Model* model, PersistenceRecord *fields) {
	DummyElement* newElement = new DummyElement(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

ModelDataDefinition* DummyElement::NewInstance(Model* model, std::string name) {
	return new DummyElement(model, name);
//	_parentModel->getResponses()->insert(new SimulationControlDouble(
//					 std::bind(&DummyElement::getter, this),
//					 std::bind(&DummyElement::setter, this, std::placeholders::_1),
//					 this->getClassname(), getName(), "NameOfControl"));
}


//
// protected: /// virtual method that must be overriden
//

bool DummyElement::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			this->_someString = fields->loadField("someString", DEFAULT.someString);
			this->_someUint = fields->loadField("someUint", DEFAULT.someUint);
		} catch (...) {
		}
	}
	return res;
}

void DummyElement::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("someUint", _someUint, DEFAULT.someUint);
	fields->saveField("someString", _someString, DEFAULT.someString);
}

//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DummyElementt::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* DummyElementt::_getParserChangesInformation() {
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
void DummyElementt::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void DummyElementt::_createInternalAndAttachedData() {
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
void DummyElementt::_addProperty(SimulationControl* property) {
}
*/
