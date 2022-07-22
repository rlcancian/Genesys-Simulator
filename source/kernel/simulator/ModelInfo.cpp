/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelInfo.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 7 de Novembro de 2018, 18:13
 */

#include "ModelInfo.h"
#include "Model.h"

//using namespace GenesysKernel;

ModelInfo::ModelInfo() {
	_name = "Model " + std::to_string(Util::GenerateNewIdOfType<Model>());
}

std::string ModelInfo::show() {
	return "analystName=\"" + this->_analystName + "\"" +
			",description=\"" + this->_description + "\"" +
			",name=\"" + this->_name + "\"" +
			",version=" + this->_version;
}

void ModelInfo::setName(std::string _name) {
	this->_name = _name;
	_hasChanged = true;
}

std::string ModelInfo::getName() const {
	return _name;
}

void ModelInfo::setAnalystName(std::string _analystName) {
	this->_analystName = _analystName;
	_hasChanged = true;
}

std::string ModelInfo::getAnalystName() const {
	return _analystName;
}

void ModelInfo::setDescription(std::string _description) {
	this->_description = _description;
	_hasChanged = true;
}

std::string ModelInfo::getDescription() const {
	return _description;
}

void ModelInfo::setProjectTitle(std::string _projectTitle) {
	this->_projectTitle = _projectTitle;
	_hasChanged = true;
}

std::string ModelInfo::getProjectTitle() const {
	return _projectTitle;
}

void ModelInfo::setVersion(std::string _version) {
	this->_version = _version;
	_hasChanged = true;
}

std::string ModelInfo::getVersion() const {
	return _version;
}

void ModelInfo::loadInstance(PersistenceRecord *fields) {
	this->_analystName = fields->loadField("analystName");
	this->_description = fields->loadField("description");
	this->_name = fields->loadField("name");
	this->_projectTitle = fields->loadField("projectTitle");
	this->_version = fields->loadField("version");
	_hasChanged = false;
}

// @TODO:!: implement check method (to check things like terminating condition)

void ModelInfo::saveInstance(PersistenceRecord *fields) {
	fields->saveField("typename", "ModelInfo");
	fields->saveField("analystName", _analystName);
	fields->saveField("description", _description);
	fields->saveField("name", this->getName());
	fields->saveField("projectTitle", _projectTitle);
	fields->saveField("version", _version);
	_hasChanged = false;
}

bool ModelInfo::hasChanged() const {
	return _hasChanged;
}
