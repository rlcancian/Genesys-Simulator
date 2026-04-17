/*
 * File:   GroProgramIr.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMIR_H
#define GROPROGRAMIR_H

#include "plugins/data/BiologicalModeling/GroProgramAst.h"

#include <string>
#include <vector>

/*!
 * \brief Initial semantic representation compiled from GroProgramAst.
 *
 * The IR is deliberately non-executable for this phase. It only classifies
 * top-level statements into generic function calls or raw statements so later
 * runtime work can bind selected calls to bacteria-colony behavior.
 */
struct GroProgramIr {
	struct Command {
		enum class Kind {
			RawStatement,
			FunctionCall
		};

		Kind kind = Kind::RawStatement;
		std::string sourceText = "";
		std::string functionName = "";
		std::vector<std::string> arguments;

		bool isFunctionCall() const {
			return kind == Kind::FunctionCall;
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
