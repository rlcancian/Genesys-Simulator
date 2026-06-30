#ifndef BIOKINETICLAWEXPRESSION_H
#define BIOKINETICLAWEXPRESSION_H

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

/**
 * Small expression evaluator for biochemical kinetic laws.
 *
 * The evaluator intentionally avoids the global GenESyS parser because the
 * generated parser does not currently expose BioSpecies and BioParameter as
 * kinetic-law symbols. Supported syntax is numeric literals, identifiers,
 * parentheses, unary +/- operators, +, -, *, /, ^, and a small set of math
 * functions.
 */
class BioKineticLawExpression {
public:
	using SymbolResolver = std::function<bool(const std::string& name, double& value)>;

	bool evaluate(const std::string& expression, SymbolResolver resolver, double& value, std::string& errorMessage) const {
		Parser parser(expression, resolver, errorMessage);
		value = parser.parseExpression();
		parser.skipWhitespace();
		if (!errorMessage.empty()) {
			return false;
		}
		if (!parser.atEnd()) {
			errorMessage = "Unexpected token at position " + std::to_string(parser.position()) + ".";
			return false;
		}
		if (!std::isfinite(value)) {
			errorMessage = "Expression evaluated to a non-finite value.";
			return false;
		}
		return true;
	}

private:
	class Parser {
	public:
		Parser(const std::string& expression, SymbolResolver resolver, std::string& errorMessage)
				: _expression(expression), _resolver(resolver), _errorMessage(errorMessage) {
		}

		double parseExpression() {
			double value = parseTerm();
			while (_errorMessage.empty()) {
				skipWhitespace();
				if (consume('+')) {
					value += parseTerm();
				} else if (consume('-')) {
					value -= parseTerm();
				} else {
					break;
				}
			}
			return value;
		}

		void skipWhitespace() {
			while (_position < _expression.size() && std::isspace(static_cast<unsigned char>(_expression[_position])) != 0) {
				++_position;
			}
		}

		bool atEnd() const {
			return _position >= _expression.size();
		}

		unsigned int position() const {
			return static_cast<unsigned int>(_position);
		}

	private:
		double parseTerm() {
			double value = parsePower();
			while (_errorMessage.empty()) {
				skipWhitespace();
				if (consume('*')) {
					value *= parsePower();
				} else if (consume('/')) {
					const double denominator = parsePower();
					if (denominator == 0.0) {
						_errorMessage = "Division by zero.";
						return 0.0;
					}
					value /= denominator;
				} else {
					break;
				}
			}
			return value;
		}

		double parsePower() {
			double value = parseUnary();
			skipWhitespace();
			if (consume('^')) {
				value = std::pow(value, parsePower());
			}
			return value;
		}

		double parseUnary() {
			skipWhitespace();
			if (consume('+')) {
				return parseUnary();
			}
			if (consume('-')) {
				return -parseUnary();
			}
			return parsePrimary();
		}

		double parsePrimary() {
			skipWhitespace();
			if (_position >= _expression.size()) {
				_errorMessage = "Unexpected end of expression.";
				return 0.0;
			}
			if (consume('(')) {
				double value = parseExpression();
				skipWhitespace();
				if (!consume(')')) {
					_errorMessage = "Missing closing parenthesis.";
					return 0.0;
				}
				return value;
			}
			if (std::isdigit(static_cast<unsigned char>(_expression[_position])) != 0 || _expression[_position] == '.') {
				return parseNumber();
			}
			if (isIdentifierStart(_expression[_position])) {
				const std::string identifier = parseIdentifier();
				skipWhitespace();
				if (consume('(')) {
					return parseFunction(identifier);
				}
				double value = 0.0;
				if (_resolver == nullptr || !_resolver(identifier, value)) {
					_errorMessage = "Unknown kinetic-law symbol \"" + identifier + "\".";
					return 0.0;
				}
				return value;
			}
			_errorMessage = "Unexpected token at position " + std::to_string(_position) + ".";
			return 0.0;
		}

		double parseNumber() {
			const size_t start = _position;
			while (_position < _expression.size() &&
					(std::isdigit(static_cast<unsigned char>(_expression[_position])) != 0 || _expression[_position] == '.')) {
				++_position;
			}
			if (_position < _expression.size() && (_expression[_position] == 'e' || _expression[_position] == 'E')) {
				++_position;
				if (_position < _expression.size() && (_expression[_position] == '+' || _expression[_position] == '-')) {
					++_position;
				}
				while (_position < _expression.size() && std::isdigit(static_cast<unsigned char>(_expression[_position])) != 0) {
					++_position;
				}
			}
			try {
				return std::stod(_expression.substr(start, _position - start));
			} catch (...) {
				_errorMessage = "Invalid numeric literal at position " + std::to_string(start) + ".";
				return 0.0;
			}
		}

		std::string parseIdentifier() {
			const size_t start = _position;
			++_position;
			while (_position < _expression.size() && isIdentifierChar(_expression[_position])) {
				++_position;
			}
			return _expression.substr(start, _position - start);
		}

		double parseFunction(const std::string& name) {
			std::vector<double> args;
			skipWhitespace();
			if (!consume(')')) {
				while (_errorMessage.empty()) {
					args.push_back(parseExpression());
					skipWhitespace();
					if (consume(')')) {
						break;
					}
					if (!consume(',')) {
						_errorMessage = "Missing comma or closing parenthesis in function call.";
						return 0.0;
					}
				}
			}
			return applyFunction(name, args);
		}

		double applyFunction(const std::string& name, const std::vector<double>& args) {
			if (name == "abs" && args.size() == 1) {
				return std::fabs(args[0]);
			}
			if (name == "exp" && args.size() == 1) {
				return std::exp(args[0]);
			}
			if (name == "log" && args.size() == 1) {
				if (args[0] <= 0.0) {
					_errorMessage = "log() requires a positive argument.";
					return 0.0;
				}
				return std::log(args[0]);
			}
			if (name == "sqrt" && args.size() == 1) {
				if (args[0] < 0.0) {
					_errorMessage = "sqrt() requires a non-negative argument.";
					return 0.0;
				}
				return std::sqrt(args[0]);
			}
			if (name == "min" && args.size() == 2) {
				return std::min(args[0], args[1]);
			}
			if (name == "max" && args.size() == 2) {
				return std::max(args[0], args[1]);
			}
			if (name == "pow" && args.size() == 2) {
				return std::pow(args[0], args[1]);
			}
			_errorMessage = "Unsupported function \"" + name + "\" or invalid argument count.";
			return 0.0;
		}

		bool consume(char expected) {
			skipWhitespace();
			if (_position < _expression.size() && _expression[_position] == expected) {
				++_position;
				return true;
			}
			return false;
		}

		bool isIdentifierStart(char ch) const {
			return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
		}

		bool isIdentifierChar(char ch) const {
			return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '.';
		}

	private:
		const std::string& _expression;
		SymbolResolver _resolver;
		std::string& _errorMessage;
		size_t _position = 0;
	};
};

#endif /* BIOKINETICLAWEXPRESSION_H */
