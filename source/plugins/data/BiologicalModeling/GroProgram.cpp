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

namespace {

std::string buildDefaultGroProgramSource(const std::string& filename) {
	const std::string label = filename.empty() ? "inline Gro program" : filename;
	return "// Default starter created for " + label + "\n"
	       "program colony() {\n"
	       "    // Advance one biological tick and grow while the colony is still small.\n"
	       "    tick();\n"
	       "    if (population < 8) {\n"
	       "        grow(1);\n"
	       "    }\n"
	       "}\n";
}

} // namespace

ModelDataDefinition* GroProgram::NewInstance(Model* model, std::string name) {
	return new GroProgram(model, name);
}

GroProgram::GroProgram(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<GroProgram>(), name) {
	SimulationControlString* propSourceCode = new SimulationControlString(
			std::bind(&GroProgram::getSourceCode, this),
			std::bind(&GroProgram::setSourceCode, this, std::placeholders::_1),
			Util::TypeOf<GroProgram>(), getName(), "SourceCode",
			"Gro source code associated with this reusable program",
			false,
			false,
			false,
			std::bind(&GroProgram::_validateSourceCodeSyntax, this, std::placeholders::_1, std::placeholders::_2));
	// Large GRO source text is edited in a dedicated code dialog instead of a single-line string editor.
	propSourceCode->setPreferredEditorHint(SimulationControlEditorHint::CodeEditor);
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

bool GroProgram::createDefaultGroProgram(const std::string& filename) {
	// Keep the model-side source synchronized with the generated starter even
	// when the caller does not request an external file yet.
	_sourceCode = buildDefaultGroProgramSource(filename);

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
