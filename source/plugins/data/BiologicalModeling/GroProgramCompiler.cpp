/*
 * File:   GroProgramCompiler.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramCompiler.h"

#include <cctype>
#include <string>
#include <vector>

namespace {

std::size_t skipWhitespace(const std::string& text, std::size_t position) {
	while (position < text.size() && std::isspace(static_cast<unsigned char>(text[position]))) {
		++position;
	}
	return position;
}

std::string trim(const std::string& text) {
	const std::size_t first = skipWhitespace(text, 0);
	if (first == text.size()) {
		return "";
	}

	std::size_t last = text.size() - 1;
	while (last > first && std::isspace(static_cast<unsigned char>(text[last]))) {
		--last;
	}
	return text.substr(first, last - first + 1);
}

std::string readIdentifier(const std::string& text, std::size_t& position) {
	if (position >= text.size()) {
		return "";
	}
	const unsigned char first = static_cast<unsigned char>(text[position]);
	if (!std::isalpha(first) && text[position] != '_') {
		return "";
	}

	const std::size_t start = position;
	++position;
	while (position < text.size()) {
		const unsigned char current = static_cast<unsigned char>(text[position]);
		if (!std::isalnum(current) && text[position] != '_') {
			break;
		}
		++position;
	}
	return text.substr(start, position - start);
}

std::size_t findMatchingParenthesis(const std::string& text, std::size_t openingIndex) {
	if (openingIndex >= text.size() || text[openingIndex] != '(') {
		return std::string::npos;
	}

	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';

	for (std::size_t i = openingIndex; i < text.size(); ++i) {
		const char current = text[i];

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
				return std::string::npos;
			}
			const char opening = delimiterStack.back();
			const bool matches = (opening == '(' && current == ')') ||
			                     (opening == '[' && current == ']') ||
			                     (opening == '{' && current == '}');
			if (!matches) {
				return std::string::npos;
			}
			delimiterStack.pop_back();
			if (opening == '(' && current == ')' && delimiterStack.empty()) {
				return i;
			}
		}
	}

	return std::string::npos;
}

std::vector<std::string> splitTopLevelArguments(const std::string& argumentsText) {
	std::vector<std::string> arguments;
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	std::size_t argumentStart = 0;

	for (std::size_t i = 0; i < argumentsText.size(); ++i) {
		const char current = argumentsText[i];

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
			if (!delimiterStack.empty()) {
				delimiterStack.pop_back();
			}
			continue;
		}

		if (current == ',' && delimiterStack.empty()) {
			arguments.push_back(trim(argumentsText.substr(argumentStart, i - argumentStart)));
			argumentStart = i + 1;
		}
	}

	const std::string trailingArgument = trim(argumentsText.substr(argumentStart));
	if (!trailingArgument.empty()) {
		arguments.push_back(trailingArgument);
	}
	return arguments;
}

GroProgramIr::Command compileStatement(const GroProgramAst::Statement& statement) {
	GroProgramIr::Command command;
	command.sourceText = trim(statement.sourceText);
	if (command.sourceText.empty()) {
		return command;
	}

	std::size_t position = skipWhitespace(command.sourceText, 0);
	const std::string functionName = readIdentifier(command.sourceText, position);
	if (functionName.empty()) {
		return command;
	}

	position = skipWhitespace(command.sourceText, position);
	if (position >= command.sourceText.size() || command.sourceText[position] != '(') {
		return command;
	}

	const std::size_t closingParenthesis = findMatchingParenthesis(command.sourceText, position);
	if (closingParenthesis == std::string::npos) {
		return command;
	}

	const std::size_t trailing = skipWhitespace(command.sourceText, closingParenthesis + 1);
	if (trailing != command.sourceText.size()) {
		return command;
	}

	command.kind = GroProgramIr::Command::Kind::FunctionCall;
	command.functionName = functionName;
	const std::string argumentsText = command.sourceText.substr(position + 1, closingParenthesis - position - 1);
	command.arguments = splitTopLevelArguments(argumentsText);
	return command;
}

}

GroProgramIr GroProgramCompiler::compile(const GroProgramAst& ast) const {
	GroProgramIr ir;
	ir.sourceForm = ast.sourceForm;
	ir.programName = ast.programName;

	for (const GroProgramAst::Statement& statement : ast.statements) {
		GroProgramIr::Command command = compileStatement(statement);
		if (!command.sourceText.empty()) {
			ir.commands.push_back(command);
		}
	}

	return ir;
}
