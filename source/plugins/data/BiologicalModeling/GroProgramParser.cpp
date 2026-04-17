/*
 * File:   GroProgramParser.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramParser.h"

#include <cctype>
#include <string>
#include <vector>

namespace {

std::size_t skipWhitespace(const std::string& sourceCode, std::size_t position) {
	while (position < sourceCode.size() && std::isspace(static_cast<unsigned char>(sourceCode[position]))) {
		++position;
	}
	return position;
}

std::string trim(const std::string& sourceCode) {
	const std::size_t first = skipWhitespace(sourceCode, 0);
	if (first == sourceCode.size()) {
		return "";
	}

	std::size_t last = sourceCode.size() - 1;
	while (last > first && std::isspace(static_cast<unsigned char>(sourceCode[last]))) {
		--last;
	}
	return sourceCode.substr(first, last - first + 1);
}

std::string readIdentifier(const std::string& sourceCode, std::size_t& position) {
	if (position >= sourceCode.size()) {
		return "";
	}
	const unsigned char first = static_cast<unsigned char>(sourceCode[position]);
	if (!std::isalpha(first) && sourceCode[position] != '_') {
		return "";
	}

	const std::size_t start = position;
	++position;
	while (position < sourceCode.size()) {
		const unsigned char current = static_cast<unsigned char>(sourceCode[position]);
		if (!std::isalnum(current) && sourceCode[position] != '_') {
			break;
		}
		++position;
	}
	return sourceCode.substr(start, position - start);
}

std::size_t findMatchingDelimiter(const std::string& sourceCode, std::size_t openingIndex) {
	if (openingIndex >= sourceCode.size()) {
		return std::string::npos;
	}

	const char openingDelimiter = sourceCode[openingIndex];
	char closingDelimiter = '\0';
	if (openingDelimiter == '(') {
		closingDelimiter = ')';
	} else if (openingDelimiter == '[') {
		closingDelimiter = ']';
	} else if (openingDelimiter == '{') {
		closingDelimiter = '}';
	} else {
		return std::string::npos;
	}

	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	bool inLineComment = false;
	bool inBlockComment = false;

	for (std::size_t i = openingIndex; i < sourceCode.size(); ++i) {
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
				return std::string::npos;
			}
			const char stackOpening = delimiterStack.back();
			const bool matches = (stackOpening == '(' && current == ')') ||
			                     (stackOpening == '[' && current == ']') ||
			                     (stackOpening == '{' && current == '}');
			if (!matches) {
				return std::string::npos;
			}
			delimiterStack.pop_back();
			if (stackOpening == openingDelimiter && current == closingDelimiter && delimiterStack.empty()) {
				return i;
			}
		}
	}

	return std::string::npos;
}

std::vector<GroProgramAst::Statement> splitTopLevelStatements(const std::string& sourceCode) {
	std::vector<GroProgramAst::Statement> statements;
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	bool inLineComment = false;
	bool inBlockComment = false;
	std::size_t statementStart = 0;

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
			if (!delimiterStack.empty()) {
				delimiterStack.pop_back();
			}
			continue;
		}

		if (current == ';' && delimiterStack.empty()) {
			GroProgramAst::Statement statement;
			statement.sourceText = trim(sourceCode.substr(statementStart, i - statementStart));
			if (!statement.sourceText.empty()) {
				statements.push_back(statement);
			}
			statementStart = i + 1;
		}
	}

	GroProgramAst::Statement trailingStatement;
	trailingStatement.sourceText = trim(sourceCode.substr(statementStart));
	if (!trailingStatement.sourceText.empty()) {
		statements.push_back(trailingStatement);
	}
	return statements;
}

GroProgramAst buildAst(const std::string& sourceCode) {
	GroProgramAst ast;
	ast.bodySource = trim(sourceCode);
	ast.statements = splitTopLevelStatements(ast.bodySource);

	std::size_t position = skipWhitespace(sourceCode, 0);
	const std::string programKeyword = "program";
	if (sourceCode.compare(position, programKeyword.size(), programKeyword) != 0) {
		return ast;
	}

	position += programKeyword.size();
	if (position >= sourceCode.size() || !std::isspace(static_cast<unsigned char>(sourceCode[position]))) {
		return ast;
	}

	position = skipWhitespace(sourceCode, position);
	const std::string programName = readIdentifier(sourceCode, position);
	if (programName.empty()) {
		return ast;
	}

	position = skipWhitespace(sourceCode, position);
	if (position >= sourceCode.size() || sourceCode[position] != '(') {
		return ast;
	}
	const std::size_t closingParenthesis = findMatchingDelimiter(sourceCode, position);
	if (closingParenthesis == std::string::npos) {
		return ast;
	}

	position = skipWhitespace(sourceCode, closingParenthesis + 1);
	if (position >= sourceCode.size() || sourceCode[position] != '{') {
		return ast;
	}
	const std::size_t closingBrace = findMatchingDelimiter(sourceCode, position);
	if (closingBrace == std::string::npos) {
		return ast;
	}

	const std::size_t trailing = skipWhitespace(sourceCode, closingBrace + 1);
	if (trailing != sourceCode.size()) {
		return ast;
	}

	ast.sourceForm = GroProgramAst::SourceForm::ProgramBlock;
	ast.programName = programName;
	ast.bodySource = trim(sourceCode.substr(position + 1, closingBrace - position - 1));
	ast.statements = splitTopLevelStatements(ast.bodySource);
	return ast;
}

}

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
	result.ast = buildAst(sourceCode);
	return result;
}
