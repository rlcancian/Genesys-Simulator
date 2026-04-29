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
	       "include gro\n"
	       "set ( \"dt\", 0.1 );\n"
	       "ahl := signal ( 1, 1 );\n"
	       "\n"
	       "program leader() := {\n"
	       "  p := [ t := 0, division_t := 0 ];\n"
	       "  true : { p.t := p.t + dt, p.division_t := p.division_t + dt }\n"
	       "  p.t > 0.2 : { emit_signal ( ahl, 40 ), p.t := 0 }\n"
	       "  p.division_t > 0.9 : { divide(), p.division_t := 0 }\n"
	       "};\n"
	       "\n"
	       "program follower() := {\n"
	       "  p := [ mode := 0, t := 0, growth_t := 0 ];\n"
	       "  true : { p.growth_t := p.growth_t + dt }\n"
	       "  p.mode = 0 & get_signal ( ahl ) > 0.01 : { p.mode := 1, p.t := 0 }\n"
	       "  p.mode = 1 : { p.t := p.t + dt }\n"
	       "  p.growth_t > 1.2 : { grow(), p.growth_t := 0 }\n"
	       "};\n"
	       "\n"
	       "ecoli ( [ x:= 0, y:= 0 ], program leader() );\n"
	       "ecoli ( [ x:= 0, y:= 10 ], program follower() );\n";
}

} // namespace

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
