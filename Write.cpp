/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Write.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:06
 */

#include "Write.h"
#include "Model.h"

#include <fstream>

Write::Write(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Write>(), name) {
}

std::string Write::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Write::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Write* newComponent = new Write(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

List<WriteElement*>* Write::writeElements() const {
	return _writeElements;
}

void Write::setFilename(std::string _filename) {
	this->_filename = _filename;
}

std::string Write::filename() const {
	return _filename;
}

void Write::setWriteToType(WriteToType _writeToType) {
	this->_writeToType = _writeToType;
}

Write::WriteToType Write::writeToType() const {
	return _writeToType;
}

void Write::_execute(Entity* entity) {
	WriteElement* msgElem;
	std::list<WriteElement*>* msgs = this->_writeElements->list();
	std::string message = "";
	for (std::list<WriteElement*>::iterator it = msgs->begin(); it != msgs->end(); it++) {
		msgElem = (*it);
		if (msgElem->isExpression) {
			message += std::to_string(_parentModel->parseExpression(msgElem->text));
		} else {
			message += msgElem->text;
		}
		if (msgElem->newline) {
			if (this->_writeToType == Write::WriteToType::SCREEN) { //\todo: Write To FILE not implemented
				_parentModel->getTracer()->trace(Util::TraceLevel::L3_results, message);
			} else if (this->_writeToType == Write::WriteToType::FILE) {
				// open file
				std::ofstream savefile;
				savefile.open(_filename, std::ofstream::app);
				savefile << message << std::endl;
				savefile.close();
			}
			message = "";
		}
	}

	this->_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

void Write::_initBetweenReplications() {
	try {
		std::ofstream savefile;
		savefile.open(_filename, std::ofstream::app);
		savefile << "# Replication number " << _parentModel->getSimulation()->getCurrentReplicationNumber() << "/" << _parentModel->getSimulation()->getNumberOfReplications() << std::endl;
		savefile.close();
	} catch (...) {

	}
}

bool Write::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_writeToType = static_cast<WriteToType> (std::stoi((*fields->find("writeToType")).second));
		unsigned short writesSize = std::stoi((*fields->find("writesSize")).second);
		for (unsigned short i = 0; i < writesSize; i++) {
			std::string text = (*fields->find(text)).second;
			bool isExpression = static_cast<bool> (std::stoi((*fields->find("isExpression")).second));
			bool newline = static_cast<bool> (std::stoi((*fields->find("newline")).second));
			this->_writeElements->insert(new WriteElement(text, isExpression, newline));
		}
	}
	return res;
}

std::map<std::string, std::string>* Write::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	SaveField(fields, "writeToType", static_cast<int> (_writeToType));
	SaveField(fields, "writesSize", _writeElements->size(), 0u);
	unsigned short i = 0;
	WriteElement* writeElem;
	for (std::list<WriteElement*>::iterator it = _writeElements->list()->begin(); it != _writeElements->list()->end(); it++, i++) {
		writeElem = (*it);
		SaveField(fields, "isExpression" + std::to_string(i), writeElem->isExpression, writeElem->DEFAULT.isExpression);
		SaveField(fields, "newline" + std::to_string(i), writeElem->newline, writeElem->DEFAULT.newline);
		SaveField(fields, "text" + std::to_string(i), writeElem->text, writeElem->DEFAULT.text);
	}
	//this->_writeElements
	return fields;
}

bool Write::_check(std::string* errorMessage) {
	bool resultAll = true;
	WriteElement* msgElem;
	unsigned short i = 0;
	std::list<WriteElement*>* msgs = this->_writeElements->list();
	for (std::list<WriteElement*>::iterator it = msgs->begin(); it != msgs->end(); it++) {
		msgElem = (*it);
		i++;
		if (msgElem->isExpression) {
			resultAll &= _parentModel->checkExpression(msgElem->text, "writeExpression" + std::to_string(i), errorMessage);
		}
	}
	// when cheking the model (before simulating it), remove the file if exists
	std::remove(_filename.c_str());

	return resultAll;
}

PluginInformation* Write::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Write>(), &Write::LoadInstance);
	// ...
	return info;
}


