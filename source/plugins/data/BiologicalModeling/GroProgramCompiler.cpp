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

std::size_t skipWhitespaceAndComments(const std::string& text, std::size_t position) {
	while (position < text.size()) {
		if (std::isspace(static_cast<unsigned char>(text[position]))) {
			++position;
			continue;
		}
		if (position + 1 < text.size() && text[position] == '/' && text[position + 1] == '/') {
			position += 2;
			while (position < text.size() && text[position] != '\n') {
				++position;
			}
			continue;
		}
		if (position + 1 < text.size() && text[position] == '/' && text[position + 1] == '*') {
			position += 2;
			while (position + 1 < text.size() && !(text[position] == '*' && text[position + 1] == '/')) {
				++position;
			}
			if (position + 1 < text.size()) {
				position += 2;
			}
			continue;
		}
		break;
	}
	return position;
}

std::string trim(const std::string& text) {
	const std::size_t first = skipWhitespaceAndComments(text, 0);
	if (first == text.size()) {
		return "";
	}

	std::size_t last = text.size();
	while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
		--last;
	}
	return text.substr(first, last - first);
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

std::string readQualifiedIdentifier(const std::string& text, std::size_t& position) {
	const std::size_t start = position;
	std::string identifier = readIdentifier(text, position);
	if (identifier.empty()) {
		return "";
	}

	while (position < text.size() && text[position] == '.') {
		const std::size_t dotPosition = position++;
		std::string member = readIdentifier(text, position);
		if (member.empty()) {
			position = dotPosition;
			break;
		}
		identifier += "." + member;
	}
	return identifier;
}

bool isQualifiedIdentifier(const std::string& text) {
	if (text.empty()) {
		return false;
	}
	std::size_t position = 0;
	const std::string identifier = readQualifiedIdentifier(text, position);
	return !identifier.empty() && position == text.size();
}

bool startsWithKeyword(const std::string& text, std::size_t position, const std::string& keyword) {
	if (position + keyword.size() > text.size() || text.compare(position, keyword.size(), keyword) != 0) {
		return false;
	}
	if (position > 0) {
		const unsigned char previous = static_cast<unsigned char>(text[position - 1]);
		if (std::isalnum(previous) || text[position - 1] == '_') {
			return false;
		}
	}
	const std::size_t nextPosition = position + keyword.size();
	if (nextPosition < text.size()) {
		const unsigned char next = static_cast<unsigned char>(text[nextPosition]);
		if (std::isalnum(next) || text[nextPosition] == '_') {
			return false;
		}
	}
	return true;
}

std::size_t findMatchingDelimiter(const std::string& text, std::size_t openingIndex) {
	if (openingIndex >= text.size()) {
		return std::string::npos;
	}

	const char openingDelimiter = text[openingIndex];
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

	for (std::size_t i = openingIndex; i < text.size(); ++i) {
		const char current = text[i];
		const char next = i + 1 < text.size() ? text[i + 1] : '\0';

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
			const char opening = delimiterStack.back();
			const bool matches = (opening == '(' && current == ')') ||
			                     (opening == '[' && current == ']') ||
			                     (opening == '{' && current == '}');
			if (!matches) {
				return std::string::npos;
			}
			delimiterStack.pop_back();
			if (opening == openingDelimiter && current == closingDelimiter && delimiterStack.empty()) {
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

bool findTopLevelAssignment(const std::string& statementText, std::size_t& assignmentIndex, std::size_t& assignmentLength) {
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';

	for (std::size_t i = 0; i < statementText.size(); ++i) {
		const char current = statementText[i];
		const char previous = i > 0 ? statementText[i - 1] : '\0';
		const char next = i + 1 < statementText.size() ? statementText[i + 1] : '\0';

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

		if (delimiterStack.empty() && current == ':' && next == '=') {
			assignmentIndex = i;
			assignmentLength = 2;
			return true;
		}

		if (current == '=' && delimiterStack.empty()) {
			const bool isComparison = previous == ':' || previous == '=' || previous == '!' ||
			                          previous == '<' || previous == '>' || next == '=';
			if (!isComparison) {
				assignmentIndex = i;
				assignmentLength = 1;
				return true;
			}
		}
	}

	return false;
}

std::size_t consumeSimpleStatement(const std::string& text, std::size_t position, std::string& statementText) {
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	bool inLineComment = false;
	bool inBlockComment = false;

	const std::size_t start = position;
	for (std::size_t i = position; i < text.size(); ++i) {
		const char current = text[i];
		const char next = i + 1 < text.size() ? text[i + 1] : '\0';

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

		if ((current == ';' || current == ',') && delimiterStack.empty()) {
			statementText = trim(text.substr(start, i - start));
			return i + 1;
		}
	}

	statementText = trim(text.substr(start));
	return text.size();
}

std::vector<GroProgramIr::Command> compileSimpleStatement(const std::string& statementText) {
	std::vector<GroProgramIr::Command> commands;
	GroProgramIr::Command command;
	command.sourceText = trim(statementText);
	if (command.sourceText.empty()) {
		return commands;
	}

	std::size_t assignmentIndex = std::string::npos;
	std::size_t assignmentLength = 0;
	if (findTopLevelAssignment(command.sourceText, assignmentIndex, assignmentLength)) {
		const std::string left = trim(command.sourceText.substr(0, assignmentIndex));
		const std::string right = trim(command.sourceText.substr(assignmentIndex + assignmentLength));
		if (isQualifiedIdentifier(left) && !right.empty()) {
			if (right.front() == '[' && right.back() == ']') {
				const std::string recordBody = right.substr(1, right.size() - 2);
				for (const std::string& fieldEntry : splitTopLevelArguments(recordBody)) {
					std::size_t fieldAssignmentIndex = std::string::npos;
					std::size_t fieldAssignmentLength = 0;
					if (!findTopLevelAssignment(fieldEntry, fieldAssignmentIndex, fieldAssignmentLength)) {
						continue;
					}
					const std::string fieldName = trim(fieldEntry.substr(0, fieldAssignmentIndex));
					const std::string fieldValue = trim(fieldEntry.substr(fieldAssignmentIndex + fieldAssignmentLength));
					if (!isQualifiedIdentifier(fieldName) || fieldValue.empty()) {
						continue;
					}

					GroProgramIr::Command fieldAssignment;
					fieldAssignment.kind = GroProgramIr::Command::Kind::Assignment;
					fieldAssignment.sourceText = trim(fieldEntry);
					fieldAssignment.assignmentTarget = left + "." + fieldName;
					fieldAssignment.expressionText = fieldValue;
					// GRO record literals behave like persistent per-bacterium defaults in the current subset.
					fieldAssignment.assignmentOnlyIfUnset = true;
					commands.push_back(fieldAssignment);
				}
				return commands;
			}

			command.kind = GroProgramIr::Command::Kind::Assignment;
			command.assignmentTarget = left;
			command.expressionText = right;
			commands.push_back(command);
			return commands;
		}
	}

	std::size_t position = skipWhitespaceAndComments(command.sourceText, 0);
	const std::string functionName = readIdentifier(command.sourceText, position);
	if (functionName.empty()) {
		commands.push_back(command);
		return commands;
	}

	position = skipWhitespaceAndComments(command.sourceText, position);
	if (position >= command.sourceText.size() || command.sourceText[position] != '(') {
		commands.push_back(command);
		return commands;
	}

	const std::size_t closingParenthesis = findMatchingDelimiter(command.sourceText, position);
	if (closingParenthesis == std::string::npos) {
		commands.push_back(command);
		return commands;
	}

	const std::size_t trailing = skipWhitespaceAndComments(command.sourceText, closingParenthesis + 1);
	if (trailing != command.sourceText.size()) {
		commands.push_back(command);
		return commands;
	}

	command.kind = GroProgramIr::Command::Kind::FunctionCall;
	command.functionName = functionName;
	command.arguments = splitTopLevelArguments(command.sourceText.substr(position + 1, closingParenthesis - position - 1));
	commands.push_back(command);
	return commands;
}

std::vector<GroProgramIr::Command> compileStatements(const std::string& text);

bool tryCompileIfStatement(const std::string& text, std::size_t& position, GroProgramIr::Command& command) {
	const std::size_t start = position;
	if (!startsWithKeyword(text, position, "if")) {
		return false;
	}

	position += 2;
	position = skipWhitespaceAndComments(text, position);
	if (position >= text.size() || text[position] != '(') {
		position = start;
		return false;
	}

	const std::size_t closingParenthesis = findMatchingDelimiter(text, position);
	if (closingParenthesis == std::string::npos) {
		position = start;
		return false;
	}
	const std::string conditionExpression = trim(text.substr(position + 1, closingParenthesis - position - 1));

	position = skipWhitespaceAndComments(text, closingParenthesis + 1);
	if (position >= text.size() || text[position] != '{') {
		position = start;
		return false;
	}

	const std::size_t thenClosingBrace = findMatchingDelimiter(text, position);
	if (thenClosingBrace == std::string::npos) {
		position = start;
		return false;
	}
	const std::string thenBody = text.substr(position + 1, thenClosingBrace - position - 1);
	position = skipWhitespaceAndComments(text, thenClosingBrace + 1);

	std::vector<GroProgramIr::Command> elseCommands;
	if (startsWithKeyword(text, position, "else")) {
		position += 4;
		position = skipWhitespaceAndComments(text, position);
		if (position >= text.size() || text[position] != '{') {
			position = start;
			return false;
		}
		const std::size_t elseClosingBrace = findMatchingDelimiter(text, position);
		if (elseClosingBrace == std::string::npos) {
			position = start;
			return false;
		}
		const std::string elseBody = text.substr(position + 1, elseClosingBrace - position - 1);
		elseCommands = compileStatements(elseBody);
		position = elseClosingBrace + 1;
	}

	command.kind = GroProgramIr::Command::Kind::IfStatement;
	command.expressionText = conditionExpression;
	command.thenCommands = compileStatements(thenBody);
	command.elseCommands = elseCommands;
	command.sourceText = trim(text.substr(start, position - start));
	return true;
}

bool tryCompileRuleStatement(const std::string& text, std::size_t& position, GroProgramIr::Command& command) {
	const std::size_t start = position;
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	std::size_t colonIndex = std::string::npos;

	for (std::size_t i = position; i < text.size(); ++i) {
		const char current = text[i];
		const char next = i + 1 < text.size() ? text[i + 1] : '\0';

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

		if (current == ':' && next == '=' && delimiterStack.empty()) {
			return false;
		}
		if (current == ':' && delimiterStack.empty()) {
			colonIndex = i;
			break;
		}
		if ((current == ';' || current == ',') && delimiterStack.empty()) {
			return false;
		}
	}

	if (colonIndex == std::string::npos) {
		return false;
	}

	const std::string conditionExpression = trim(text.substr(start, colonIndex - start));
	if (conditionExpression.empty()) {
		return false;
	}

	position = skipWhitespaceAndComments(text, colonIndex + 1);
	if (position >= text.size() || text[position] != '{') {
		position = start;
		return false;
	}

	const std::size_t closingBrace = findMatchingDelimiter(text, position);
	if (closingBrace == std::string::npos) {
		position = start;
		return false;
	}

	command.kind = GroProgramIr::Command::Kind::IfStatement;
	command.expressionText = conditionExpression;
	command.thenCommands = compileStatements(text.substr(position + 1, closingBrace - position - 1));
	command.sourceText = trim(text.substr(start, closingBrace - start + 1));
	command.elseCommands.clear();
	position = closingBrace + 1;
	return true;
}

std::vector<GroProgramIr::Command> compileStatements(const std::string& text) {
	std::vector<GroProgramIr::Command> commands;
	std::size_t position = 0;

	while (true) {
		position = skipWhitespaceAndComments(text, position);
		if (position >= text.size()) {
			break;
		}

		GroProgramIr::Command command;
		std::size_t nextPosition = position;

		// Control-flow statements do not end with semicolons, so they need their
		// own structural parsing path before the generic statement fallback.
		if (tryCompileIfStatement(text, nextPosition, command)) {
			position = nextPosition;
			if (!command.sourceText.empty()) {
				commands.push_back(command);
			}
			continue;
		}
		if (tryCompileRuleStatement(text, nextPosition, command)) {
			position = nextPosition;
			if (!command.sourceText.empty()) {
				commands.push_back(command);
			}
			continue;
		}

		std::string statementText;
		position = consumeSimpleStatement(text, position, statementText);
		for (GroProgramIr::Command compiledCommand : compileSimpleStatement(statementText)) {
			if (!compiledCommand.sourceText.empty()) {
				commands.push_back(compiledCommand);
			}
		}
	}

	return commands;
}

} // namespace

GroProgramIr GroProgramCompiler::compile(const GroProgramAst& ast) const {
	GroProgramIr ir;
	ir.sourceForm = ast.sourceForm;
	ir.programName = ast.programName;
	if (ast.isProgramBlock()) {
		ir.commands = compileStatements(ast.bodySource);
		return ir;
	}

	for (const GroProgramAst::Statement& statement : ast.statements) {
		const std::vector<GroProgramIr::Command> statementCommands = compileStatements(statement.sourceText);
		ir.commands.insert(ir.commands.end(), statementCommands.begin(), statementCommands.end());
	}
	for (const GroProgramAst::NamedProgram& namedProgram : ast.namedPrograms) {
		ir.namedPrograms[namedProgram.name] = compileStatements(namedProgram.bodySource);
	}
	return ir;
}
