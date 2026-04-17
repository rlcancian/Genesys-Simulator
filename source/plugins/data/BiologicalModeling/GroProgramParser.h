/*
 * File:   GroProgramParser.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMPARSER_H
#define GROPROGRAMPARSER_H

#include "plugins/data/BiologicalModeling/GroProgramAst.h"

#include <string>

/*!
 * \brief Plugin-side parser boundary for reusable Gro source programs.
 *
 * This helper currently preserves the permissive lexical validation that used
 * to live inside GroProgram. It is intentionally small so later phases can grow
 * a real grammar, AST, and semantic checks without changing GroProgram storage.
 */
class GroProgramParser {
public:
	struct Result {
		bool accepted = false;
		std::string errorMessage = "";
		GroProgramAst ast;
	};

public:
	/*! \brief Parses a Gro source string using the current lexical boundary. */
	Result parse(const std::string& sourceCode) const;
};

#endif /* GROPROGRAMPARSER_H */
