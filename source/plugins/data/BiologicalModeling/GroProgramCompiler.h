/*
 * File:   GroProgramCompiler.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMCOMPILER_H
#define GROPROGRAMCOMPILER_H

#include "plugins/data/BiologicalModeling/GroProgramAst.h"
#include "plugins/data/BiologicalModeling/GroProgramIr.h"

/*!
 * \brief Converts parsed Gro AST into the first non-executable semantic IR.
 */
class GroProgramCompiler {
public:
	/*! \brief Builds a lightweight semantic IR from a parsed Gro AST. */
	GroProgramIr compile(const GroProgramAst& ast) const;
};

#endif /* GROPROGRAMCOMPILER_H */
