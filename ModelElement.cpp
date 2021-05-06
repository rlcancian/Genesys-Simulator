/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelElement.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Junho de 2018, 19:40
 */

//#include <typeinfo>
#include <iostream>   
#include "ModelElement.h"
#include "Model.h"
#include "Traits.h"

//using namespace GenesysKernel;

ModelElement::ModelElement(Model* model, std::string thistypename, std::string name, bool insertIntoModel) {
	_id = Util::GenerateNewId(); //GenerateNewIdOfType(thistypename);
	_typename = thistypename;
	_parentModel = model;
	_reportStatistics = Traits<ModelElement>::reportStatistics; // \todo: shoould be a parameter before insertIntoModel
	if (name == "")
		_name = thistypename + "_" + std::to_string(Util::GenerateNewIdOfType(thistypename));
	else
		_name = name;
	_hasChanged = false;
	if (insertIntoModel) {
		model->insert(this);
	}
}

bool ModelElement::hasChanged() const {
	return _hasChanged;
}

//ModelElement::ModelElement(const ModelElement &orig) {
//this->_parentModel = orig->_parentModel;
//this->_name = "copy_of_" + orig->_name;
//this->_typename = orig->_typename;
//}

ModelElement::~ModelElement() {
	_parentModel->getTracer()->trace(Util::TraceLevel::L8_mostDetailed, "Removing Element \"" + this->_name + "\" from the model");
	_removeChildrenElements();
	_parentModel->getElements()->remove(this);
}

void ModelElement::_removeChildrenElements() {
	Util::IncIndent();
	{
		for (std::map<std::string, ModelElement*>::iterator it = _childrenElements->begin(); it != _childrenElements->end(); it++) {
			this->_parentModel->getElements()->remove((*it).second);
			(*it).second->~ModelElement();
		}
		_childrenElements->clear();
	}
	Util::DecIndent();

}

void ModelElement::_removeChildElement(std::string key) {
	Util::IncIndent();
	{
		//for (std::map<std::string, ModelElement*>::iterator it = _childrenElements->begin(); it != _childrenElements->end(); it++) {
		std::map<std::string, ModelElement*>::iterator it = _childrenElements->begin();
		while (it != _childrenElements->end()) {
			if ((*it).first == key) {
				this->_parentModel->getElements()->remove((*it).second);
				(*it).second->~ModelElement();
				_childrenElements->erase(it);
				it = _childrenElements->begin();
			}
		}
	}
	Util::DecIndent();
}

bool ModelElement::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = true;
	std::map<std::string, std::string>::iterator it;
	it = fields->find("id");
	it != fields->end() ? this->_id = std::stoi((*it).second) : res = false;
	it = fields->find("name");
	it != fields->end() ? this->_name = (*it).second : this->_name = "";
	//it != fields->end() ? this->_name = (*it).second : res = false;
	it = fields->find("reportStatistics");
	it != fields->end() ? this->_reportStatistics = std::stoi((*it).second) : this->_reportStatistics = Traits<ModelElement>::reportStatistics;
	return res;
}

std::map<std::string, std::string>* ModelElement::_saveInstance() {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	SaveField(fields, "typename", _typename);
	SaveField(fields, "id", this->_id);
	SaveField(fields, "name", _name);
	SaveField(fields, "reportStatistics", _reportStatistics, Traits<ModelElement>::reportStatistics);
	return fields;
}

bool ModelElement::_check(std::string* errorMessage) {
	return true; // if there is no ovveride, return true
}

ParserChangesInformation* ModelElement::_getParserChangesInformation() {
	return new ParserChangesInformation(); // if there is no override, return no changes
}

void ModelElement::_initBetweenReplications() {

}

/*
std::list<std::map<std::string,std::string>*>* ModelElement::_saveInstance(std::string type) {
	std::list<std::map<std::string,std::string>*>* fields = ModelElement::_saveInstance();
	fields->push_back(type);
	return fields;
}
 */

std::string ModelElement::show() {
	std::string children = "";
	if (_childrenElements->size() > 0) {
		children = ", children=[";
		for (std::map<std::string, ModelElement*>::iterator it = _childrenElements->begin(); it != _childrenElements->end(); it++) {
			children += (*it).second->getName() + ",";
		}
		children = children.substr(0, children.length() - 1) + "]";
	}
	return "id=" + std::to_string(_id) + ",name=\"" + _name + "\"" + children;
}

std::list<std::string>* ModelElement::getChildrenElementKeys() const {
	std::list<std::string>* result = new std::list<std::string>();
	return result;
}

Util::identification ModelElement::getId() const {
	return _id;
}

void ModelElement::setName(std::string name) {
	// rename every "stuff" related to this element (controls, responses and childrenElements
	if (name != _name) {
		std::string stuffName;
		unsigned int pos;
		for (std::pair<std::string, ModelElement*> child : *_childrenElements) {
			stuffName = child.second->getName();
			pos = stuffName.find(getName(), 0);
			if (pos != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				child.second->setName(stuffName);
			}
		}

		for (SimulationControl* control : *_parentModel->getControls()->list()) {
			stuffName = control->getName();
			pos = stuffName.find(getName(), 0);
			if (pos != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				control->setName(stuffName);
			}
		}

		for (SimulationResponse* response : *_parentModel->getResponses()->list()) {
			stuffName = response->getName();
			pos = stuffName.find(getName(), 0);
			if (pos != std::string::npos) {
				stuffName = stuffName.replace(pos, pos + getName().length(), name);
				response->setName(stuffName);
			}
		}
		this->_name = name;
		_hasChanged = true;
	}
}

std::string ModelElement::getName() const {
	return _name;
}

std::string ModelElement::getClassname() const {
	return _typename;
}

void ModelElement::InitBetweenReplications(ModelElement* element) {
	element->_parentModel->getTracer()->trace("Initing element \"" + element->getName() + "\"", Util::TraceLevel::L7_detailed); //std::to_string(component->_id));
	try {
		element->_initBetweenReplications();
	} catch (const std::exception& e) {
		element->_parentModel->getTracer()->traceError(e, "Error initing component " + element->show());
	};
}

ModelElement* ModelElement::LoadInstance(Model* model, std::map<std::string, std::string>* fields, bool insertIntoModel) {
	std::string name = "";
	if (insertIntoModel) {
		// extracts the name from the fields even before "_laodInstance" and even before construct a new ModelElement in such way when constructing the ModelElement, it's done with the correct name and that correct name is show in trace
		std::map<std::string, std::string>::iterator it = fields->find("name");
		if (it != fields->end())
			name = (*it).second;
	}
	ModelElement* newElement = new ModelElement(model, "ModelElement", name, insertIntoModel);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

std::map<std::string, std::string>* ModelElement::SaveInstance(ModelElement* element) {
	std::map<std::string, std::string>* fields; // = new std::list<std::string>();
	try {
		fields = element->_saveInstance();
	} catch (const std::exception& e) {
		//element->_model->getTrace()->traceError(e, "Error saving anElement " + element->show());
	}
	return fields;
}

bool ModelElement::Check(ModelElement* element, std::string* errorMessage) {
	//    element->_model->getTraceManager()->trace(Util::TraceLevel::L8_mostDetailed, "Checking " + element->_typename + ": " + element->_name); //std::to_string(element->_id));
	bool res = false;
	Util::IncIndent();
	{
		try {
			res = element->_check(errorMessage);
			if (!res) {
				//                element->_model->getTraceManager()->trace(Util::TraceLevel::errors, "Error: Checking has failed with message '" + *errorMessage + "'");
			}
		} catch (const std::exception& e) {
			//            element->_model->getTraceManager()->traceError(e, "Error verifying element " + element->show());
		}
	}
	Util::DecIndent();
	return res;
}

void ModelElement::CreateInternalElements(ModelElement* element) {
	try {
		Util::IncIndent();
		element->_createInternalElements();
		Util::DecIndent();
	} catch (const std::exception& e) {
		//element->...->_model->getTraceManager()->traceError(e, "Error creating elements of element " + element->show());
	};
}

void ModelElement::_createInternalElements() {

}

void ModelElement::setReportStatistics(bool reportStatistics) {
	if (_reportStatistics != reportStatistics) {
		this->_reportStatistics = reportStatistics;
		_hasChanged = true;
	}
}

bool ModelElement::isReportStatistics() const {
	return _reportStatistics;
}