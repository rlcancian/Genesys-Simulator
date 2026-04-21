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

#include "plugins/data/Template/DummyElement.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

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
	SimulationControlString* propSomeString = new SimulationControlString(
					 std::bind(&DummyElement::getSomeString, this),
					 std::bind(&DummyElement::setSomeString, this, std::placeholders::_1),
					 Util::TypeOf<DummyElement>(), getName(), "SomeString");
	SimulationControlDouble* propSomeUint = new SimulationControlDouble(
					 [this]() { return static_cast<double>(this->getSomeUint()); },
					 [this](double value) { this->setSomeUint(value < 0.0 ? 0u : static_cast<unsigned int>(value)); },
					 Util::TypeOf<DummyElement>(), getName(), "SomeUint");
	_parentModel->getControls()->insert(propSomeString);
	_parentModel->getControls()->insert(propSomeUint);
	_addProperty(propSomeString);
	_addProperty(propSomeUint);
}


//
// public: /// new public user methods for this component
//

void DummyElement::setSomeString(const std::string& someString) {
	_someString = someString;
}

std::string DummyElement::getSomeString() const {
	return _someString;
}

void DummyElement::setSomeUint(unsigned int someUint) {
	_someUint = someUint;
}

unsigned int DummyElement::getSomeUint() const {
	return _someUint;
}


//
// public: /// virtual methods
//

std::string DummyElement::show() {
	return ModelDataDefinition::show() +
			", someString=\"" + _someString + "\"" +
			", someUint=" + std::to_string(_someUint);
}



//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//


PluginInformation* DummyElement::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DummyElement>(), &DummyElement::LoadInstance, &DummyElement::NewInstance);
	info->setCategory("Template");
	info->setDescriptionHelp("Template/example ModelDataDefinition plugin used as a base for creating new data definitions.");
	info->setObservation("This plugin is a skeleton/template and not a final domain-specific data element.");
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

bool DummyElement::_check(std::string& errorMessage) {
	if (_someString.empty()) {
		errorMessage += "SomeString must not be empty. ";
	}
	if (_someUint == 0u) {
		errorMessage += "SomeUint must be greater than zero. ";
	}
	return errorMessage.empty();
}

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
