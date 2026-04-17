/*
 * File:   GroProgramParser.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramParser.h"

#include <vector>

GroProgramParser::Result GroProgramParser::parse(const std::string& sourceCode) const {
	Result result;

	if (sourceCode.empty()) {
		result.errorMessage = "GroProgram source code is empty. ";
		return result;
	}

	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	bool inLineComment = false;
	bool inBlockComment = false;

	for (std::size_t i = 0; i < sourceCode.size(); ++i) {
		const char current = sourceCode[i];
		const char next = i + 1 < sourceCode.size() ? sourceCode[i + 1] : '\0';

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
				result.errorMessage = "GroProgram has an unmatched closing delimiter. ";
				return result;
			}
			const char opening = delimiterStack.back();
			const bool matches = (opening == '(' && current == ')') || (opening == '[' && current == ']') ||
			                     (opening == '{' && current == '}');
			if (!matches) {
				result.errorMessage = "GroProgram has mismatched delimiters. ";
				return result;
			}
			delimiterStack.pop_back();
		}
	}

	if (inString) {
		result.errorMessage = "GroProgram has an unterminated string literal. ";
		return result;
	}
	if (inBlockComment) {
		result.errorMessage = "GroProgram has an unterminated block comment. ";
		return result;
	}
	if (!delimiterStack.empty()) {
		result.errorMessage = "GroProgram has unmatched opening delimiters. ";
		return result;
	}

	result.accepted = true;
	return result;
}
