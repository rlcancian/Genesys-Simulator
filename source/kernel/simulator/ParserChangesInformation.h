/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ParserChangesInformation.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 20:42
 */

#ifndef PARSERCHANGESINFORMATION_H
#define PARSERCHANGESINFORMATION_H

#include <string>
#include <list>

class ParserChangesInformation {
public:
	ParserChangesInformation();
	virtual ~ParserChangesInformation() = default;

public: // gets and sets
	// TODO(genesys|parser-api|naming): Review the consistency of this class public API names.
	// There are mixed naming patterns such as getincludes() and getfunctionProdutions().
	// Any rename may break existing callers and should be planned carefully.
	std::string getincludes() const;
	void setIncludes(const std::string &newIncludes);

	std::string gettokens() const;
	void setTokens(const std::string &newTokens);

	std::string gettypeObjs() const;
	void setTypeObjs(const std::string &newTypeObjs);

	std::string getexpressions() const;
	void setExpressions(const std::string &newExpressions);

	std::string getexpressionProductions() const;
	void setExpressionProductions(const std::string &newExpressionProductions);

	std::string getassignments() const;
	void setAssignments(const std::string &newAssignments);

	// TODO(genesys|parser-api|typo): Confirm whether "Produtions" should actually be "Productions".
	// Do not rename it directly without mapping impacts on existing callers.
	std::string getfunctionProdutions() const;
	void setFunctionProdutions(const std::string &newFunctionProdutions);

private:
	std::string _includes = "";
	std::string _tokens = "";
	std::string _typeObjs = "";
	std::string _expressions = "";
	std::string _expressionProductions = "";
	std::string _assignments = "";
	std::string _functionProdutions = "";

};

#endif /* PARSERCHANGESINFORMATION_H */

