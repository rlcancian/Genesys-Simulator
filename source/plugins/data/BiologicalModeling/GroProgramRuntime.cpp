/*
 * File:   GroProgramRuntime.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"

#include <cctype>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

double resolveIdentifierValue(const std::string& identifier, const GroProgramRuntimeState& state) {
	if (identifier == "population") {
		return static_cast<double>(state.populationSize);
	}
	if (identifier == "colony_time" || identifier == "colonyTime") {
		return state.colonyTime;
	}
	if (identifier == "simulation_step" || identifier == "simulationStep") {
		return state.simulationStep;
	}
	if (identifier == "dt") {
		return state.simulationStep;
	}
	if (identifier == "tick_count" || identifier == "tickCount") {
		return static_cast<double>(state.tickCount);
	}

	const auto contextual = state.contextVariables.find(identifier);
	if (contextual != state.contextVariables.end()) {
		return contextual->second;
	}

	const auto found = state.variables.find(identifier);
	return found != state.variables.end() ? found->second : 0.0;
}

class NumericExpressionParser {
public:
	NumericExpressionParser(const std::string& expressionText, const GroProgramRuntimeState& state)
		: _expressionText(expressionText), _state(state) {
	}

	bool parse(double& value, std::string& errorMessage) {
		value = 0.0;
		errorMessage.clear();
		if (!parseLogicalOr(value, errorMessage)) {
			return false;
		}
		skipWhitespace();
		if (_position != _expressionText.size()) {
			errorMessage = "GroProgramRuntime expression has unexpected trailing token near \"" +
			              _expressionText.substr(_position) + "\". ";
			return false;
		}
		if (!std::isfinite(value)) {
			errorMessage = "GroProgramRuntime expression produced a non-finite value. ";
			return false;
		}
		return true;
	}

private:
	bool parseLogicalOr(double& value, std::string& errorMessage) {
		if (!parseLogicalAnd(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (!match("||") && !match("|")) {
				break;
			}
			double right = 0.0;
			if (!parseLogicalAnd(right, errorMessage)) {
				return false;
			}
			value = (value != 0.0 || right != 0.0) ? 1.0 : 0.0;
		}
		return true;
	}

	bool parseLogicalAnd(double& value, std::string& errorMessage) {
		if (!parseEquality(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (!match("&&") && !match("&")) {
				break;
			}
			double right = 0.0;
			if (!parseEquality(right, errorMessage)) {
				return false;
			}
			value = (value != 0.0 && right != 0.0) ? 1.0 : 0.0;
		}
		return true;
	}

	bool parseEquality(double& value, std::string& errorMessage) {
		if (!parseComparison(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (match("==")) {
				double right = 0.0;
				if (!parseComparison(right, errorMessage)) {
					return false;
				}
				value = value == right ? 1.0 : 0.0;
				continue;
			}
			if (match("!=")) {
				double right = 0.0;
				if (!parseComparison(right, errorMessage)) {
					return false;
				}
				value = value != right ? 1.0 : 0.0;
				continue;
			}
			if (match("=")) {
				double right = 0.0;
				if (!parseComparison(right, errorMessage)) {
					return false;
				}
				value = value == right ? 1.0 : 0.0;
				continue;
			}
			break;
		}
		return true;
	}

	bool parseComparison(double& value, std::string& errorMessage) {
		if (!parseTerm(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (match("<=")) {
				double right = 0.0;
				if (!parseTerm(right, errorMessage)) {
					return false;
				}
				value = value <= right ? 1.0 : 0.0;
				continue;
			}
			if (match(">=")) {
				double right = 0.0;
				if (!parseTerm(right, errorMessage)) {
					return false;
				}
				value = value >= right ? 1.0 : 0.0;
				continue;
			}
			if (match("<")) {
				double right = 0.0;
				if (!parseTerm(right, errorMessage)) {
					return false;
				}
				value = value < right ? 1.0 : 0.0;
				continue;
			}
			if (match(">")) {
				double right = 0.0;
				if (!parseTerm(right, errorMessage)) {
					return false;
				}
				value = value > right ? 1.0 : 0.0;
				continue;
			}
			break;
		}
		return true;
	}

	bool parseTerm(double& value, std::string& errorMessage) {
		if (!parseFactor(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (match("+")) {
				double right = 0.0;
				if (!parseFactor(right, errorMessage)) {
					return false;
				}
				value += right;
				continue;
			}
			if (match("-")) {
				double right = 0.0;
				if (!parseFactor(right, errorMessage)) {
					return false;
				}
				value -= right;
				continue;
			}
			break;
		}
		return true;
	}

	bool parseFactor(double& value, std::string& errorMessage) {
		if (!parseUnary(value, errorMessage)) {
			return false;
		}
		while (true) {
			skipWhitespace();
			if (match("*")) {
				double right = 0.0;
				if (!parseUnary(right, errorMessage)) {
					return false;
				}
				value *= right;
				continue;
			}
			if (match("/")) {
				double right = 0.0;
				if (!parseUnary(right, errorMessage)) {
					return false;
				}
				if (right == 0.0) {
					errorMessage = "GroProgramRuntime expression division by zero is not allowed. ";
					return false;
				}
				value /= right;
				continue;
			}
			break;
		}
		return true;
	}

	bool parseUnary(double& value, std::string& errorMessage) {
		skipWhitespace();
		if (match("-")) {
			if (!parseUnary(value, errorMessage)) {
				return false;
			}
			value = -value;
			return true;
		}
		if (match("!")) {
			if (!parseUnary(value, errorMessage)) {
				return false;
			}
			value = value == 0.0 ? 1.0 : 0.0;
			return true;
		}
		return parsePrimary(value, errorMessage);
	}

	bool parsePrimary(double& value, std::string& errorMessage) {
		skipWhitespace();
		if (_position >= _expressionText.size()) {
			errorMessage = "GroProgramRuntime expression ended unexpectedly. ";
			return false;
		}

		if (match("(")) {
			if (!parseLogicalOr(value, errorMessage)) {
				return false;
			}
			skipWhitespace();
			if (!match(")")) {
				errorMessage = "GroProgramRuntime expression is missing a closing parenthesis. ";
				return false;
			}
			return true;
		}

		if (std::isdigit(static_cast<unsigned char>(_expressionText[_position])) || _expressionText[_position] == '.') {
			std::size_t parsedCharacters = 0;
			try {
				value = std::stod(_expressionText.substr(_position), &parsedCharacters);
			} catch (const std::exception&) {
				errorMessage = "GroProgramRuntime could not parse numeric literal near \"" +
				              _expressionText.substr(_position) + "\". ";
				return false;
			}
			_position += parsedCharacters;
			return true;
		}

		const std::string identifier = parseIdentifier();
		if (!identifier.empty()) {
			if (identifier == "true") {
				value = 1.0;
				return true;
			}
			if (identifier == "false") {
				value = 0.0;
				return true;
			}

			const std::size_t functionScan = _position;
			skipWhitespace();
			if (_position < _expressionText.size() && _expressionText[_position] == '(') {
				if (!parseFunctionCall(identifier, value, errorMessage)) {
					return false;
				}
				return true;
			}
			_position = functionScan;
			value = resolveIdentifierValue(identifier, _state);
			return true;
		}

		errorMessage = "GroProgramRuntime expression has an unsupported token near \"" +
		              _expressionText.substr(_position) + "\". ";
		return false;
	}

	std::string parseIdentifier() {
		skipWhitespace();
		if (_position >= _expressionText.size()) {
			return "";
		}
		const unsigned char first = static_cast<unsigned char>(_expressionText[_position]);
		if (!std::isalpha(first) && _expressionText[_position] != '_') {
			return "";
		}

		const std::size_t start = _position;
		++_position;
		while (_position < _expressionText.size()) {
			const unsigned char current = static_cast<unsigned char>(_expressionText[_position]);
			if (std::isalnum(current) || _expressionText[_position] == '_') {
				++_position;
				continue;
			}
			if (_expressionText[_position] == '.') {
				const std::size_t dotPosition = _position++;
				if (_position >= _expressionText.size()) {
					_position = dotPosition;
					break;
				}
				const unsigned char next = static_cast<unsigned char>(_expressionText[_position]);
				if (!std::isalpha(next) && _expressionText[_position] != '_') {
					_position = dotPosition;
					break;
				}
				continue;
			}
			break;
		}
		return _expressionText.substr(start, _position - start);
	}

	bool parseFunctionCall(const std::string& functionName, double& value, std::string& errorMessage) {
		if (!match("(")) {
			errorMessage = "GroProgramRuntime expression function call is missing an opening parenthesis. ";
			return false;
		}

		std::vector<double> arguments;
		skipWhitespace();
		if (!match(")")) {
			while (true) {
				double argumentValue = 0.0;
				if (!parseLogicalOr(argumentValue, errorMessage)) {
					return false;
				}
				arguments.push_back(argumentValue);
				skipWhitespace();
				if (match(")")) {
					break;
				}
				if (!match(",")) {
					errorMessage = "GroProgramRuntime expression function call expects ',' or ')'. ";
					return false;
				}
			}
		}

		if (functionName == "get_signal") {
			if (arguments.size() > 1) {
				errorMessage = "GroProgramRuntime get_signal expression accepts zero or one argument in the current subset. ";
				return false;
			}
			value = resolveIdentifierValue("local_signal", _state);
			return true;
		}

		if (functionName == "signal") {
			if (arguments.empty()) {
				errorMessage = "GroProgramRuntime signal expression expects at least one numeric argument. ";
				return false;
			}
			value = arguments.front();
			return true;
		}

		errorMessage = "GroProgramRuntime expression does not support function \"" + functionName + "\". ";
		return false;
	}

	bool match(const std::string& token) {
		skipWhitespace();
		if (_expressionText.compare(_position, token.size(), token) != 0) {
			return false;
		}
		_position += token.size();
		return true;
	}

	void skipWhitespace() {
		while (_position < _expressionText.size() &&
		       std::isspace(static_cast<unsigned char>(_expressionText[_position]))) {
			++_position;
		}
	}

private:
	const std::string& _expressionText;
	const GroProgramRuntimeState& _state;
	std::size_t _position = 0;
};

bool evaluateExpression(const std::string& expressionText, const GroProgramRuntimeState& state,
	                    double& value, std::string& errorMessage) {
	return NumericExpressionParser(expressionText, state).parse(value, errorMessage);
}

bool evaluatePositiveIntegerExpression(const std::string& expressionText, const GroProgramRuntimeState& state,
	                                   unsigned int& value, std::string& errorMessage, bool allowZero = false) {
	double numericValue = 0.0;
	if (!evaluateExpression(expressionText, state, numericValue, errorMessage)) {
		return false;
	}
	if (!std::isfinite(numericValue)) {
		errorMessage = "GroProgramRuntime integer expression produced a non-finite value. ";
		return false;
	}
	if ((!allowZero && numericValue <= 0.0) || (allowZero && numericValue < 0.0)) {
		errorMessage = "GroProgramRuntime integer expression must be " +
		              std::string(allowZero ? "non-negative" : "positive") + ". ";
		return false;
	}
	const double rounded = std::round(numericValue);
	if (std::fabs(rounded - numericValue) > 1e-9 ||
	    rounded > static_cast<double>(std::numeric_limits<unsigned int>::max())) {
		errorMessage = "GroProgramRuntime integer expression must evaluate to a whole unsigned value. ";
		return false;
	}
	value = static_cast<unsigned int>(rounded);
	return true;
}

bool evaluateNonNegativeExpression(const std::string& expressionText, const GroProgramRuntimeState& state,
	                               double& value, std::string& errorMessage) {
	if (!evaluateExpression(expressionText, state, value, errorMessage)) {
		return false;
	}
	if (value < 0.0) {
		errorMessage = "GroProgramRuntime scalar expression must be non-negative. ";
		return false;
	}
	return true;
}

bool parseStringLiteral(const std::string& text, std::string& value) {
	if (text.size() < 2) {
		return false;
	}
	const char delimiter = text.front();
	if ((delimiter != '"' && delimiter != '\'') || text.back() != delimiter) {
		return false;
	}
	value = text.substr(1, text.size() - 2);
	return true;
}

bool parseOptionalPositiveAmount(const GroProgramIr::Command& command, const GroProgramRuntimeState& state,
	                             unsigned int& amount, std::string& errorMessage) {
	amount = 1;
	if (command.arguments.empty()) {
		return true;
	}
	if (command.arguments.size() != 1 ||
	    !evaluatePositiveIntegerExpression(command.arguments.front(), state, amount, errorMessage, false)) {
		errorMessage = "GroProgramRuntime command " + command.functionName +
		              " expects zero or one positive integer expression argument. ";
		return false;
	}
	return true;
}

bool addPopulation(unsigned int currentPopulation, unsigned int amount, unsigned int& populationSize) {
	if (amount > std::numeric_limits<unsigned int>::max() - currentPopulation) {
		return false;
	}
	populationSize = currentPopulation + amount;
	return true;
}

bool removePopulation(unsigned int currentPopulation, unsigned int amount, unsigned int& populationSize) {
	if (amount > currentPopulation) {
		return false;
	}
	populationSize = currentPopulation - amount;
	return true;
}

GroProgramRuntime::PopulationMutation makePopulationMutation(GroProgramRuntime::PopulationMutationType type,
                                                             unsigned int value,
                                                             unsigned int previousPopulationSize,
                                                             unsigned int resultingPopulationSize) {
	GroProgramRuntime::PopulationMutation mutation;
	mutation.type = type;
	mutation.value = value;
	mutation.previousPopulationSize = previousPopulationSize;
	mutation.resultingPopulationSize = resultingPopulationSize;
	return mutation;
}

bool executeCommands(const std::vector<GroProgramIr::Command>& commands, GroProgramRuntimeState& state,
	                 GroProgramRuntime::ExecutionResult& result) {
	for (const GroProgramIr::Command& command : commands) {
		if (command.isAssignment()) {
			if (command.assignmentOnlyIfUnset &&
			    state.variables.find(command.assignmentTarget) != state.variables.end()) {
				continue;
			}
			double assignedValue = 0.0;
			if (!evaluateExpression(command.expressionText, state, assignedValue, result.errorMessage)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime assignment to \"" + command.assignmentTarget + "\" failed: " +
				                     result.errorMessage;
				return false;
			}
			state.variables[command.assignmentTarget] = assignedValue;
			result.assignedVariables[command.assignmentTarget] = assignedValue;
			++result.executedCommands;
			continue;
		}

		if (command.isIfStatement()) {
			double conditionValue = 0.0;
			if (!evaluateExpression(command.expressionText, state, conditionValue, result.errorMessage)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime if condition failed: " + result.errorMessage;
				return false;
			}
			++result.executedCommands;
			const std::vector<GroProgramIr::Command>& selectedBranch =
					conditionValue != 0.0 ? command.thenCommands : command.elseCommands;
			if (!executeCommands(selectedBranch, state, result)) {
				return false;
			}
			continue;
		}

		if (!command.isFunctionCall()) {
			result.skippedRawStatements.push_back(command.sourceText);
			continue;
		}

		if (command.functionName == "tick") {
			if (!command.arguments.empty()) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime tick command does not accept arguments. ";
				return false;
			}
			state.colonyTime += state.simulationStep;
			++state.tickCount;
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "set") {
			std::string settingName;
			double settingValue = 0.0;
			if (command.arguments.size() != 2 ||
			    !parseStringLiteral(command.arguments[0], settingName) ||
			    !evaluateExpression(command.arguments[1], state, settingValue, result.errorMessage) ||
			    !std::isfinite(settingValue)) {
				result.succeeded = false;
				result.errorMessage =
						"GroProgramRuntime set command expects one quoted setting name and one numeric expression. ";
				return false;
			}

			if (settingName == "dt") {
				if (settingValue <= 0.0) {
					result.succeeded = false;
					result.errorMessage = "GroProgramRuntime dt setting must be greater than zero. ";
					return false;
				}
				state.simulationStep = settingValue;
				state.variables["dt"] = settingValue;
				result.assignedVariables["dt"] = settingValue;
			} else {
				// Preserve generic Gro set("name", value) state as a scalar variable when no dedicated binding exists yet.
				state.variables[settingName] = settingValue;
				result.assignedVariables[settingName] = settingValue;
			}
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "grow") {
			unsigned int amount = 1;
			if (!parseOptionalPositiveAmount(command, state, amount, result.errorMessage)) {
				result.succeeded = false;
				return false;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!addPopulation(state.populationSize, amount, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime grow command would overflow population size. ";
				return false;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Grow,
			                                                            amount,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "divide") {
			if (!command.arguments.empty()) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime divide command does not accept arguments. ";
				return false;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!addPopulation(state.populationSize, state.populationSize, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime divide command would overflow population size. ";
				return false;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Divide,
			                                                            previousPopulationSize,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "die") {
			unsigned int amount = 1;
			if (!parseOptionalPositiveAmount(command, state, amount, result.errorMessage)) {
				result.succeeded = false;
				return false;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!removePopulation(state.populationSize, amount, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime die command would remove more bacteria than available. ";
				return false;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Die,
			                                                            amount,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

			if (command.functionName == "set_population") {
				unsigned int populationSize = 0;
				if (command.arguments.size() != 1 ||
				    !evaluatePositiveIntegerExpression(command.arguments.front(), state, populationSize, result.errorMessage, false)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime set_population command expects one positive integer expression argument. ";
				return false;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			state.populationSize = populationSize;
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::SetPopulation,
			                                                            populationSize,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
				++result.executedCommands;
				continue;
			}

			if (command.functionName == "emit_signal" ||
			    command.functionName == "consume_signal" ||
			    command.functionName == "set_signal") {
				double signalValue = 0.0;
				if ((command.arguments.size() != 1 && command.arguments.size() != 2) ||
				    !evaluateNonNegativeExpression(command.arguments.back(), state, signalValue, result.errorMessage)) {
					result.succeeded = false;
					result.errorMessage = "GroProgramRuntime " + command.functionName +
					                      " command expects one value argument or one signal handle plus one value argument. ";
					return false;
				}

				GroProgramRuntime::SignalMutation mutation;
				mutation.type = command.functionName == "emit_signal" ? GroProgramRuntime::SignalMutationType::Emit :
				                command.functionName == "consume_signal" ? GroProgramRuntime::SignalMutationType::Consume :
				                                                          GroProgramRuntime::SignalMutationType::Set;
				mutation.value = signalValue;
				result.signalMutations.push_back(mutation);
				++result.executedCommands;
				continue;
			}

			result.unsupportedCommands.push_back(command.sourceText);
		}

	return true;
}

} // namespace

GroProgramRuntime::ExecutionResult GroProgramRuntime::execute(const GroProgramIr& ir, GroProgramRuntimeState& state) const {
	ExecutionResult result;

	if (state.simulationStep <= 0.0) {
		result.succeeded = false;
		result.errorMessage = "GroProgramRuntime simulation step must be greater than zero. ";
		return result;
	}

	executeCommands(ir.commands, state, result);
	return result;
}
