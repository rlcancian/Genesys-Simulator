/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ParserChangesInformation.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 20:42
 */

#include "ParserChangesInformation.h"

ParserChangesInformation::ParserChangesInformation() {
}

std::string ParserChangesInformation::getincludes() const
{
	return _includes;
}

void ParserChangesInformation::setIncludes(const std::string &newIncludes)
{
	_includes = newIncludes;
}

std::string ParserChangesInformation::gettokens() const
{
	return _tokens;
}

void ParserChangesInformation::setTokens(const std::string &newTokens)
{
	_tokens = newTokens;
}

std::string ParserChangesInformation::gettypeObjs() const
{
	return _typeObjs;
}

void ParserChangesInformation::setTypeObjs(const std::string &newTypeObjs)
{
	_typeObjs = newTypeObjs;
}

std::string ParserChangesInformation::getexpressions() const
{
	return _expressions;
}

void ParserChangesInformation::setExpressions(const std::string &newExpressions)
{
	_expressions = newExpressions;
}

std::string ParserChangesInformation::getexpressionProductions() const
{
	return _expressionProductions;
}

void ParserChangesInformation::setExpressionProductions(const std::string &newExpressionProductions)
{
	_expressionProductions = newExpressionProductions;
}

std::string ParserChangesInformation::getassignments() const
{
	return _assignments;
}

void ParserChangesInformation::setAssignments(const std::string &newAssignments)
{
	_assignments = newAssignments;
}

std::string ParserChangesInformation::getfunctionProdutions() const
{
	return _functionProdutions;
}

void ParserChangesInformation::setFunctionProdutions(const std::string &newFunctionProdutions)
{
	_functionProdutions = newFunctionProdutions;
}

std::string ParserChangesInformation::getlexicalRules() const
{
	return _lexicalRules;
}

void ParserChangesInformation::setLexicalRules(const std::string &newLexicalRules)
{
	_lexicalRules = newLexicalRules;
}

std::string ParserChangesInformation::getlexicalLiterals() const
{
	return _lexicalLiterals;
}

void ParserChangesInformation::setLexicalLiterals(const std::string &newLexicalLiterals)
{
	_lexicalLiterals = newLexicalLiterals;
}
