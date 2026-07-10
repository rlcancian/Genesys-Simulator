/*
 * File:   GroProgramIr.h
 * Author: rlcancian
 *
 * Created on 17 de Abril de 2022
 */

#ifndef GROPROGRAMIR_H
#define GROPROGRAMIR_H

#include "plugins/data/BiochemicalSimulation/GroProgramAst.h"

#include <map>
#include <string>
#include <vector>

/*!
 * \brief Initial semantic representation compiled from GroProgramAst.
 *
 * The IR keeps the parsed program in a runtime-oriented shape. It can represent
 * function calls, assignments, and basic conditional blocks without pushing any
 * knowledge into the simulator kernel itself.
 */
struct GroProgramIr {
	struct Command {
		enum class Kind {
			RawStatement,
			FunctionCall,
			Assignment,
			IfStatement
		};

		Kind kind = Kind::RawStatement;
		std::string sourceText = "";
		std::string functionName = "";
		std::vector<std::string> arguments;
		std::string assignmentTarget = "";
		std::string expressionText = "";
		bool assignmentOnlyIfUnset = false;
		std::vector<Command> thenCommands;
		std::vector<Command> elseCommands;

		bool isFunctionCall() const {
			return kind == Kind::FunctionCall;
		}

		bool isAssignment() const {
			return kind == Kind::Assignment;
		}

		bool isIfStatement() const {
			return kind == Kind::IfStatement;
		}
	};

	struct NamedProgramDefinition {
		std::vector<std::string> parameters;
		std::string sourceText = "";
		std::vector<Command> commands;
	};

	GroProgramAst::SourceForm sourceForm = GroProgramAst::SourceForm::RawStatements;
	std::string programName = "";
	std::vector<Command> commands;
	std::map<std::string, NamedProgramDefinition> namedPrograms;

	bool isProgramBlock() const {
		return sourceForm == GroProgramAst::SourceForm::ProgramBlock;
	}

	bool hasNamedProgram(const std::string& name) const {
		return namedPrograms.find(name) != namedPrograms.end();
	}
};

#endif /* GROPROGRAMIR_H */
