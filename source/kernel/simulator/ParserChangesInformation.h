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
	// TODO(genesys|parser-api|nomenclatura): Revisar a consistencia dos nomes publicos desta classe.
	// Existem metodos com padroes mistos, por exemplo getincludes() e getfunctionProdutions().
	// A mudanca pode quebrar consumidores existentes, entao deve ser feita de forma planejada.
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

	// TODO(genesys|parser-api|typo): Confirmar se "Produtions" deveria ser "Productions".
	// Nao renomear diretamente sem mapear impactos em chamadas existentes.
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

