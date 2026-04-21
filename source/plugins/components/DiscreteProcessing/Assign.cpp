/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Assign.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Agosto de 2018, 10:10
 */

#include "plugins/components/DiscreteProcessing/Assign.h"
#include <string>
#include <algorithm>
#include <cctype>
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/DiscreteProcessing/Variable.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Assign::GetPluginInformation;
}
#endif

ModelDataDefinition* Assign::NewInstance(Model* model, std::string name) {
	return new Assign(model, name);
}

Assign::Assign(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Assign>(), name) {
	// This block enables explicit typed creation for Assignment list elements.
	SimulationControlGenericListPointer<Assignment*, Model*, Assignment>* propAssignments = new SimulationControlGenericListPointer<Assignment*, Model*, Assignment> (
									_parentModel,
                                    std::bind(&Assign::getAssignments, this), std::bind(&Assign::addAssignment, this, std::placeholders::_1), std::bind(&Assign::removeAssignment, this, std::placeholders::_1),
									Util::TypeOf<Assign>(), getName(), "Assignments", "", true, true, false,
                                    [](Model* model, const std::string& name) {
                                        (void)model;
                                        return new Assignment(name, "");
                                    },
                                    [](Model* model) {
                                        (void)model;
                                        return new Assignment("", "");
                                    });

	_parentModel->getControls()->insert(propAssignments);

	_addSimulationControl(propAssignments);
}

std::string Assign::show() {
	std::string txt = ModelComponent::show() + ",assignments=[";
	for (std::list<Assignment*>::iterator it = _assignments->list()->begin(); it != _assignments->list()->end(); it++) {
		txt += (*it)->getDestination() + "=" + (*it)->getExpression() + ",";
	}
	txt = txt.substr(0, txt.length() - 1) + "]";
	return txt;
}

List<Assignment*>* Assign::getAssignments() const {
	return _assignments;
}

void Assign::addAssignment(Assignment* newAssignment) {
	_assignments->insert(newAssignment);
}

void Assign::removeAssignment(Assignment* assignment) {
	_assignments->remove(assignment);
}

PluginInformation* Assign::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Assign>(), &Assign::LoadInstance, &Assign::NewInstance);
	//info->insertDynamicLibFileDependence("attribute.so");
	info->insertDynamicLibFileDependence("variable.so");
	info->setCategory("DiscreteProcessing");
	std::string text = "";
	text += "This module is used for assigning new values to variables, entity attributes, entity types, entity pictures, or other system variables.";
	text += " Multiple assignments can be made with a single Assign module.";
	text += " TYPICAL USES: (1) Accumulate the number of subassemblies added to a part;";
	text += " (2) Change an entity’s type to represent the customer copy of a multi - page form;";
	text += " (3) Establish a customer’s priority";
	info->setDescriptionHelp(text);

	return info;
}

ModelComponent* Assign::LoadInstance(Model* model, PersistenceRecord *fields) {
	Assign* newComponent = new Assign(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}

	return newComponent;
}

void Assign::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	Assignment* let;
    std::list<Assignment*>* assignments = this->_assignments->list();
    for (std::list<Assignment*>::iterator it = assignments->begin(); it != assignments->end(); it++) {

		let = (*it);
		double value = _parentModel->parseExpression(let->getExpression());
		_parentModel->parseExpression(let->getDestination() + "=" + std::to_string(value));
        //traceSimulation(this, "Let \"" + let->getDestination() + "\" = " + Util::StrTruncIfInt(std::to_string(value)) + "  // " + let->getExpression());
        traceSimulation(this, "Let " + let->getDestination() + " = \"" + let->getExpression() + "\" = " + Util::StrTruncIfInt(std::to_string(value)));
    }

	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

//void Assign::_initBetweenReplications() {}

bool Assign::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		unsigned int nv = fields->loadField("assignments", DEFAULT.assignmentsSize);
		for (unsigned short i = 0; i < nv; i++) {
			Assignment* item = new Assignment("", "");
			item->loadInstance(fields, i);
			this->_assignments->insert(item);
		}
	}
	return res;
}

void Assign::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	Assignment* let;
	fields->saveField("assignments", _assignments->size(), DEFAULT.assignmentsSize, saveDefaultValues);
	unsigned short i = 0;
	for (std::list<Assignment*>::iterator it = _assignments->list()->begin(); it != _assignments->list()->end(); it++, i++) {
		let = (*it);
		let->saveInstance(fields, i, saveDefaultValues);
	}
}

bool Assign::_check(std::string& errorMessage) {
	bool resultAll = true;
	_attachedDataClear();
	int i = 0;
	for (Assignment* let : *_assignments->list()) {
		if (let->getDestination().empty()) {
			errorMessage += "Assignment destination cannot be empty. ";
			resultAll = false;
			continue;
		}

		const std::string baseDestination = _destinationBaseName(let->getDestination());
		const std::string destinationType = let->isAttributeNotVariable() ? Util::TypeOf<Attribute>() : Util::TypeOf<Variable>();
		ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(destinationType, baseDestination);
		if (data == nullptr) {
			errorMessage += destinationType + " \"" + baseDestination + "\" for assignment destination is not in the model. ";
			resultAll = false;
		} else {
			_attachedDataInsert("Assignment" + Util::StrIndex(i), data);
		}

		resultAll &= _parentModel->checkExpression(let->getExpression(), "assignment", errorMessage);
		i++;
	}
	return resultAll;
}

void Assign::_createInternalAndAttachedData() {
	_attachedDataClear();
	ModelDataManager* elems = _parentModel->getDataManager();
	for (Assignment* ass : *_assignments->list()) {
		ModelDataDefinition* elem = nullptr;
		std::string name;
		const std::string baseDestination = _destinationBaseName(ass->getDestination());
		if (ass->isAttributeNotVariable()) {
			name = "Attribute";
			elem = elems->getDataDefinition(Util::TypeOf<Attribute>(), baseDestination);
		} else {
			name = "Variable";
			elem = elems->getDataDefinition(Util::TypeOf<Variable>(), baseDestination);
		}
		//assert elem != nullptr
		if (elem != nullptr) {
			this->_attachedDataInsert(name + "_" + baseDestination, elem);
		}
	}
}

std::string Assign::_destinationBaseName(const std::string& destination) {
	const std::string::size_type bracket = destination.find('[');
	if (bracket == std::string::npos) {
		return destination;
	}
	std::string base = destination.substr(0, bracket);
	base.erase(std::find_if(base.rbegin(), base.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), base.end());
	return base;
}
