/*
 * File:   GroProgramIr.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMIR_H
#define GROPROGRAMIR_H

#include "plugins/data/BiologicalModeling/GroProgramAst.h"

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

	GroProgramAst::SourceForm sourceForm = GroProgramAst::SourceForm::RawStatements;
	std::string programName = "";
	std::vector<Command> commands;

	bool isProgramBlock() const {
		return sourceForm == GroProgramAst::SourceForm::ProgramBlock;
	}
};

#endif /* GROPROGRAMIR_H */
