/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PluginInformation.cpp
 * Author: rlcancian
 * 
 * Created on 9 de Setembro de 2019, 20:02
 */

#include "PluginInformation.h"

//using namespace GenesysKernel;

PluginInformation::PluginInformation(std::string pluginTypename, StaticLoaderComponentInstance componentloader, StaticConstructorDataDefinitionInstance elementConstructor) {
	this->_componentloader = componentloader;
	this->_elementloader = nullptr;
	this->_elementConstructor = elementConstructor;
	this->_isComponent = true;
	this->_pluginTypename = pluginTypename;
}

PluginInformation::PluginInformation(std::string pluginTypename, StaticLoaderDataDefinitionInstance elementloader, StaticConstructorDataDefinitionInstance elementConstructor) {
	this->_componentloader = nullptr;
	this->_elementloader = elementloader;
	this->_elementConstructor = elementConstructor;
	this->_isComponent = false;
	this->_pluginTypename = pluginTypename;
}

// Free dependency and field containers owned by plugin metadata instances.
PluginInformation::~PluginInformation() {
	delete _dynamicLibFilenameDependencies;
	delete _fields;
}

StaticConstructorDataDefinitionInstance PluginInformation::getDataDefinitionConstructor() const {
	return _elementConstructor;
}

StaticLoaderDataDefinitionInstance PluginInformation::getDataDefinitionLoader() const {
	return _elementloader;
}

StaticLoaderComponentInstance PluginInformation::GetComponentLoader() const {
	return _componentloader;
}

bool PluginInformation::isGenerateReport() const {
	return _generateReport;
}

bool PluginInformation::isComponent() const {
	return _isComponent;
}

bool PluginInformation::isSendTransfer() const {
	return _sendTransfer;
}

bool PluginInformation::isReceiveTransfer() const {
	return _receiveTransfer;
}

bool PluginInformation::isSink() const {
	return _isSink;
}

bool PluginInformation::isSource() const {
	return _isSource;
}

std::string PluginInformation::getObservation() const {
	return _observation;
}

std::string PluginInformation::getVersion() const {
	return _version;
}

std::string PluginInformation::getDate() const {
	return _date;
}

std::string PluginInformation::getAuthor() const {
	return _author;
}

std::string PluginInformation::getPluginTypename() const {
	return _pluginTypename;
}

void PluginInformation::insertDynamicLibFileDependence(std::string filename) {
	_dynamicLibFilenameDependencies->insert(_dynamicLibFilenameDependencies->end(), filename);
}

void PluginInformation::setDynamicLibFilenameDependencies(std::list<std::string>* dynamicLibFilenameDependencies) {
	this->_dynamicLibFilenameDependencies = dynamicLibFilenameDependencies;
}

std::list<std::string>* PluginInformation::getDynamicLibFilenameDependencies() const {
	return _dynamicLibFilenameDependencies;
}

void PluginInformation::setGenerateReport(bool generateReport) {
	this->_generateReport = generateReport;
}

void PluginInformation::setSendTransfer(bool sendTransfer) {
	this->_sendTransfer = sendTransfer;
}

void PluginInformation::setReceiveTransfer(bool receiveTransfer) {
	this->_receiveTransfer = receiveTransfer;
}

void PluginInformation::setSink(bool Sink) {
	_isSink = Sink;
}

void PluginInformation::setSource(bool Source) {
	_isSource = Source;
}

void PluginInformation::setObservation(std::string observation) {
	this->_observation = observation;
}

void PluginInformation::setVersion(std::string version) {
	this->_version = version;
}

void PluginInformation::setDate(std::string date) {
	this->_date = date;
}

void PluginInformation::setAuthor(std::string author) {
	this->_author = author;
}

void PluginInformation::setMaximumOutputs(unsigned short _maximumOutputs) {
	this->_maximumOutputs = _maximumOutputs;
}

unsigned short PluginInformation::getMaximumOutputs() const {
	return _maximumOutputs;
}

void PluginInformation::setMinimumOutputs(unsigned short _minimumOutputs) {
	this->_minimumOutputs = _minimumOutputs;
}

unsigned short PluginInformation::getMinimumOutputs() const {
	return _minimumOutputs;
}

void PluginInformation::setMaximumInputs(unsigned short _maximumInputs) {
	this->_maximumInputs = _maximumInputs;
}

unsigned short PluginInformation::getMaximumInputs() const {
	return _maximumInputs;
}

void PluginInformation::setMinimumInputs(unsigned short _minimumInputs) {
	this->_minimumInputs = _minimumInputs;
}

unsigned short PluginInformation::getMinimumInputs() const {
	return _minimumInputs;
}

void PluginInformation::setDescriptionHelp(std::string _descriptionHelp) {
	this->_descriptionHelp = _descriptionHelp;
}

std::string PluginInformation::getDescriptionHelp() const {
	return _descriptionHelp;
}

void PluginInformation::setFields(std::map<std::string, std::string>* _fields) {
	this->_fields = _fields;
}

std::map<std::string, std::string>* PluginInformation::getFields() const {
	return _fields;
}

void PluginInformation::setLanguageTemplate(std::string _languageTemplate) {
	this->_languageTemplate = _languageTemplate;
}

std::string PluginInformation::getLanguageTemplate() const {
	return _languageTemplate;
}

void PluginInformation::setCategory(std::string _category) {
	this->_category = _category;
}

std::string PluginInformation::getCategory() const {
	return _category;
}

