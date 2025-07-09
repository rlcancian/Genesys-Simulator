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
#include "../../kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Write::GetPluginInformation;
}
#endif

ModelDataDefinition* Write::NewInstance(Model* model, std::string name) {
	return new Write(model, name);
}

std::string Write::convertEnumToStr(WriteToType type) {
	switch (static_cast<int> (type)) {
		case 0: return "SCREEN";
		case 1: return "FILE";
	}
	return "Unknown";
}

Write::Write(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Write>(), name) {
	SimulationControlGeneric<std::string>* propFilename = new SimulationControlGeneric<std::string>(
									std::bind(&Write::filename, this), std::bind(&Write::setFilename, this, std::placeholders::_1),
									Util::TypeOf<Write>(), getName(), "Filename", "");
    SimulationControlGenericEnum<Write::WriteToType, Write>* propWriteToType = new SimulationControlGenericEnum<Write::WriteToType, Write>(
                                    std::bind(&Write::writeToType, this), std::bind(&Write::setWriteToType, this, std::placeholders::_1),
                                    Util::TypeOf<Write>(), getName(), "WriteToType", "");

	// _parentModel->getControls()->insert();
	_parentModel->getControls()->insert(propFilename);
    _parentModel->getControls()->insert(propWriteToType);

	// setting properties
	// _addProperty();
	_addProperty(propFilename);
    _addProperty(propWriteToType);
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
	if (this->_writeToType == Write::WriteToType::FILE) { // file is kept open during replication
		_savefile.open(_filename, std::ofstream::app);
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
				if (this->_writeToType == Write::WriteToType::SCREEN) {
					traceSimulation(this, TraceManager::Level::L2_results, message);
				} else if (this->_writeToType == Write::WriteToType::FILE) {
					_savefile << message << std::endl;
				}
				message = "";
			}
		}
	}
	if (!lastWasShown) {
		if (this->_writeToType == Write::WriteToType::SCREEN) {
			traceSimulation(this, TraceManager::Level::L2_results, message);
		} else if (this->_writeToType == Write::WriteToType::FILE) {
			_savefile << message << std::endl;
		}
	}
	if (this->_writeToType == Write::WriteToType::FILE) { // file is kept open during replication (//@TODO: whould need to intercept end of simulation event
		_savefile.close();
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

void Write::_initBetweenReplications() {
	if (this->_writeToType == Write::WriteToType::FILE) {
		try {
			if (!_savefile.is_open()) {
				_savefile.open(_filename, std::ofstream::app);
			} else {
				_savefile.flush(); // flush the content of previous replication
			}
			_savefile << "#ReplicationNumber=" << _parentModel->getSimulation()->getCurrentReplicationNumber() << std::endl; //"/" << _parentModel->getSimulation()->getNumberOfReplications() << std::endl;
			_savefile.close();
		} catch (...) {

		}
	}
}

bool Write::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_writeToType = static_cast<WriteToType> (fields->loadField("writeToType", static_cast<int> (DEFAULT.writeToType)));
		_filename = fields->loadField("filename", DEFAULT.filename);
		unsigned short writesSize = fields->loadField("writes", 0u);
		for (unsigned short i = 0; i < writesSize; i++) {
			std::string text = fields->loadField("write" + Util::StrIndex(i), "");
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
		fields->saveField("write" + Util::StrIndex(i), text, "", saveDefaultValues);
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
			resultAll &= _parentModel->checkExpression(msgElem->text, "writeExpression" + Util::strIndex(i), errorMessage);
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
