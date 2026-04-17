/*
 * File:   GroProgram.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "kernel/simulator/Model.h"

#include <vector>

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
	info->setCategory("Biological Modeling");
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
	if (_sourceCode.empty()) {
		errorMessage += "GroProgram source code is empty. ";
		return false;
	}

	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	bool inLineComment = false;
	bool inBlockComment = false;

	for (std::size_t i = 0; i < _sourceCode.size(); ++i) {
		const char current = _sourceCode[i];
		const char next = i + 1 < _sourceCode.size() ? _sourceCode[i + 1] : '\0';

		if (inLineComment) {
			if (current == '\n') {
				inLineComment = false;
			}
			continue;
		}

		if (inBlockComment) {
			if (current == '*' && next == '/') {
				inBlockComment = false;
				++i;
			}
			continue;
		}

		if (inString) {
			if (current == '\\') {
				++i;
				continue;
			}
			if (current == stringDelimiter) {
				inString = false;
				stringDelimiter = '\0';
			}
			continue;
		}

		if (current == '/' && next == '/') {
			inLineComment = true;
			++i;
			continue;
		}
		if (current == '/' && next == '*') {
			inBlockComment = true;
			++i;
			continue;
		}
		if (current == '"' || current == '\'') {
			inString = true;
			stringDelimiter = current;
			continue;
		}

		if (current == '(' || current == '[' || current == '{') {
			delimiterStack.push_back(current);
			continue;
		}
		if (current == ')' || current == ']' || current == '}') {
			if (delimiterStack.empty()) {
				errorMessage += "GroProgram has an unmatched closing delimiter. ";
				return false;
			}
			const char opening = delimiterStack.back();
			const bool matches = (opening == '(' && current == ')') || (opening == '[' && current == ']') ||
			                     (opening == '{' && current == '}');
			if (!matches) {
				errorMessage += "GroProgram has mismatched delimiters. ";
				return false;
			}
			delimiterStack.pop_back();
		}
	}

	if (inString) {
		errorMessage += "GroProgram has an unterminated string literal. ";
		return false;
	}
	if (inBlockComment) {
		errorMessage += "GroProgram has an unterminated block comment. ";
		return false;
	}
	if (!delimiterStack.empty()) {
		errorMessage += "GroProgram has unmatched opening delimiters. ";
		return false;
	}

	return true;
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
