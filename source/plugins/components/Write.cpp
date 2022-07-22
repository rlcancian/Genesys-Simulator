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
#include "../../kernel/simulator/Model.h"

#include <fstream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Write::GetPluginInformation;
}
#endif

ModelDataDefinition* Write::NewInstance(Model* model, std::string name) {
	return new Write(model, name);
}

Write::Write(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Write>(), name) {
}

std::string Write::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Write::LoadInstance(Model* model, PersistenceRecord *fields) {
	Write* newComponent = new Write(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

//List<WriteText*>* Write::getWrites() const {
//	return _writeElements;
//}

void Write::insertText(std::list<std::string> texts) {
	for (std::string text : texts) {
		_writeElements->list()->push_back(text);
	}

	if (texts.size() > 0 && texts.back().substr(texts.back().length() - 1, 1) != "\n") {
		_writeElements->list()->push_back("\n");
	}
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

void Write::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	std::ofstream savefile;
	if (this->_writeToType == Write::WriteToType::FILE) {
		savefile.open(_filename, std::ofstream::app);
	}
	std::string message = "";
	bool lastWasShown = true;
	for (std::string msgElem : *_writeElements->list()) {
		if (msgElem.substr(0, 1) == "@") { // to start with '@' is the signal that the following text is an expression to the parser
			message += std::to_string(_parentModel->parseExpression(msgElem.substr(1, msgElem.length() - 1)));
		} else {
			message += msgElem;
		}
		lastWasShown = msgElem.substr(msgElem.length() - 1, 1) == "\n";
		if (lastWasShown) {
			message = message.substr(0, message.length() - 1);
			if (message != "") {
				if (this->_writeToType == Write::WriteToType::SCREEN) { //@TODO: Write To FILE not implemented
					_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L2_results, message);
				} else if (this->_writeToType == Write::WriteToType::FILE) {
					savefile << message << std::endl;
				}
				message = "";
			}
		}
	}
	if (!lastWasShown) {
		if (this->_writeToType == Write::WriteToType::SCREEN) { //@TODO: Write To FILE not implemented
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L2_results, message);
		} else if (this->_writeToType == Write::WriteToType::FILE) {
			savefile << message << std::endl;
		}
	}
	if (this->_writeToType == Write::WriteToType::FILE) {
		savefile.close();
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
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

bool Write::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_writeToType = static_cast<WriteToType> (fields->loadField("writeToType", static_cast<int> (DEFAULT.writeToType)));
		_filename = fields->loadField("filename", DEFAULT.filename);
		unsigned short writesSize = fields->loadField("writes", 0u);
		for (unsigned short i = 0; i < writesSize; i++) {
			std::string text = fields->loadField("write" + strIndex(i), "");
			_writeElements->insert(text);
			i++;
		}
	}
	return res;
}

void Write::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("writeToType", static_cast<int> (_writeToType), static_cast<int> (DEFAULT.writeToType), saveDefaultValues);
	fields->saveField("filename", _filename, DEFAULT.filename, saveDefaultValues);
	fields->saveField("writes", _writeElements->size(), 0u, saveDefaultValues);
	unsigned short i = 0;
	for (std::string text : *_writeElements->list()) {
		//@ TODO: NEED TO AVOID \N TO BE SAVE AS A REAL NEW LINE. SHOULD SAVE "\n"
		fields->saveField("write" + strIndex(i), text, "", saveDefaultValues);
		i++;
	}
}

bool Write::_check(std::string* errorMessage) {
	bool resultAll = true;
	/*
	WriteText* msgElem;
	unsigned short i = 0;
	std::list<WriteText*>* msgs = this->_writeElements->list();
	for (std::list<WriteText*>::iterator it = msgs->begin(); it != msgs->end(); it++) {
		msgElem = (*it);
		i++;
		if (msgElem->isExpression) {
			resultAll &= _parentModel->checkExpression(msgElem->text, "writeExpression" + strIndex(i), errorMessage);
		}
	}
	 */
	// when cheking the model (before simulating it), remove the file if exists
	std::remove(_filename.c_str());

	return resultAll;
}

PluginInformation* Write::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Write>(), &Write::LoadInstance, &Write::NewInstance);
	info->setCategory("Input Output");
	// ...
	return info;
}


