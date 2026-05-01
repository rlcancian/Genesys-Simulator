/*
 * File:   GroProgram.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"
#include "kernel/simulator/Model.h"

#include <fstream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GroProgram::GetPluginInformation;
}
#endif

ModelDataDefinition* GroProgram::NewInstance(Model* model, std::string name) {
	return new GroProgram(model, name);
}

GroProgram::GroProgram(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<GroProgram>(), name) {
	SimulationControlSourceCodeString* propSourceCode = new SimulationControlSourceCodeString(
			std::bind(&GroProgram::getSourceCodeProperty, this),
			std::bind(&GroProgram::setSourceCodeProperty, this, std::placeholders::_1),
			Util::TypeOf<GroProgram>(), getName(), "SourceCode",
			"Gro source code associated with this reusable program",
			false,
			false,
			false,
			std::bind(&GroProgram::_validateSourceCodeSyntax, this, std::placeholders::_1, std::placeholders::_2));
	_parentModel->getControls()->insert(propSourceCode);
	_addSimulationControl(propSourceCode);
}

std::string GroProgram::show() {
	return ModelDataDefinition::show() + ", sourceLength=" + std::to_string(_sourceCode.size());
}

PluginInformation* GroProgram::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GroProgram>(), &GroProgram::LoadInstance,
	                                                &GroProgram::NewInstance);
	info->setCategory("BiologicalModeling");
	info->setDescriptionHelp("Stores reusable Gro source code for biological simulation components. "
	                         "The default starter is now based on the morphogenesis example so GUI work can "
	                         "display growth, division, signals, and fluorescence with a richer colony layout.");
	return info;
}

ModelDataDefinition* GroProgram::LoadInstance(Model* model, PersistenceRecord* fields) {
	GroProgram* newElement = new GroProgram(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

void GroProgram::setSourceCode(std::string sourceCode) {
	_sourceCode = sourceCode;
}

void GroProgram::setSourceCodeProperty(SourceCodeString sourceCode) {
	_sourceCode = sourceCode.str();
}

std::string GroProgram::getSourceCode() const {
	return _sourceCode;
}

SourceCodeString GroProgram::getSourceCodeProperty() const {
	return SourceCodeString(_sourceCode);
}

bool GroProgram::createDefaultGroProgram(const std::string& filename) {
	// Keep the model-side source synchronized with the generated morphogenesis
	// starter even when the caller does not request an external file yet.
	_sourceCode = DEFAULT.sourceCode;

	if (filename.empty()) {
		return true;
	}

	std::ofstream output(filename, std::ios::out | std::ios::trunc);
	if (!output.is_open()) {
		return false;
	}

	output << _sourceCode;
	return output.good();
}

bool GroProgram::validateSyntax(std::string& errorMessage) const {
	// An empty source is a valid "not created yet" state for GUI-driven flows.
	return _validateSourceCodeSyntax(_sourceCode, errorMessage);
}

bool GroProgram::_validateSourceCodeSyntax(const std::string& sourceCode, std::string& errorMessage) const {
	// An empty source is a valid "not created yet" state for GUI-driven flows.
	if (sourceCode.empty()) {
		errorMessage.clear();
		return true;
	}

	errorMessage.clear();
	const GroProgramParser::Result result = GroProgramParser().parse(sourceCode);
	if (!result.accepted) {
		errorMessage = result.errorMessage;
	}
	return result.accepted;
}

bool GroProgram::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_sourceCode = fields->loadField("sourceCode", DEFAULT.sourceCode);
	}
	return res;
}

void GroProgram::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("sourceCode", _sourceCode, DEFAULT.sourceCode, saveDefaultValues);
}

bool GroProgram::_check(std::string& errorMessage) {
	return validateSyntax(errorMessage);
}

void GroProgram::_createReportStatisticsDataDefinitions() {
}

void GroProgram::_createEditableDataDefinitions() {
}

void GroProgram::_createOthersDataDefinitions() {
}
