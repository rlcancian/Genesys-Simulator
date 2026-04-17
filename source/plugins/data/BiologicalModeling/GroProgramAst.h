/*
 * File:   GroProgramAst.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMAST_H
#define GROPROGRAMAST_H

#include <string>
#include <vector>

/*!
 * \brief Minimal parsed representation of a Gro source program.
 *
 * This AST intentionally models only the first integration target: either a
 * recognized `program name() { ... }` block or a permissive raw statement list.
 * Execution semantics and typed Gro constructs belong to later phases.
 */
struct GroProgramAst {
	enum class SourceForm {
		RawStatements,
		ProgramBlock
	};

	struct Statement {
		std::string sourceText = "";
	};

	SourceForm sourceForm = SourceForm::RawStatements;
	std::string programName = "";
	std::string bodySource = "";
	std::vector<Statement> statements;

	bool isProgramBlock() const {
		return sourceForm == SourceForm::ProgramBlock;
	}
};

#endif /* GROPROGRAMAST_H */
