/*
 * File:   GroProgram.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GroProgram::GetPluginInformation;
}
#endif

ModelDataDefinition* GroProgram::NewInstance(Model* model, std::string name) {
	return new GroProgram(model, name);
}

GroProgram::GroProgram(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<GroProgram>(), name) {
	SimulationControlString* propSourceCode = new SimulationControlString(
			std::bind(&GroProgram::getSourceCode, this),
			std::bind(&GroProgram::setSourceCode, this, std::placeholders::_1),
			Util::TypeOf<GroProgram>(), getName(), "SourceCode",
			"Gro source code associated with this reusable program");
	_parentModel->getControls()->insert(propSourceCode);
	_addProperty(propSourceCode);
}

std::string GroProgram::show() {
	return ModelDataDefinition::show() + ", sourceLength=" + std::to_string(_sourceCode.size());
}

PluginInformation* GroProgram::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GroProgram>(), &GroProgram::LoadInstance,
	                                                &GroProgram::NewInstance);
	info->setCategory("BiologicalModeling");
	info->setDescriptionHelp("Stores reusable Gro source code for biological simulation components. "
	                         "The initial implementation stores source text and performs permissive lexical checks; "
	                         "complete Gro parsing and semantics are plugin-side future work.");
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

std::string GroProgram::getSourceCode() const {
	return _sourceCode;
}

bool GroProgram::validateSyntax(std::string& errorMessage) const {
	const GroProgramParser::Result result = GroProgramParser().parse(_sourceCode);
	if (!result.accepted) {
		errorMessage += result.errorMessage;
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
