/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   CppCode.h
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:24
 */

#ifndef CPPCODE_H
#define CPPCODE_H

#include "../../kernel/simulator/ModelDataDefinition.h"
//#include "../../kernel/simulator/ModelDataDefinition.h"

class CppCode : public ModelDataDefinition {
public:

	class CodeResult {
	public:
		CodeResult(bool success = true, std::string generalMessage = "", unsigned int lineNumber = 1) {
			this->success = success;
			this->generalMessage = generalMessage;
			this->lineNumber = lineNumber;
		}
		bool success;
		std::string generalMessage;
		unsigned int lineNumber;
	};

public:
	CppCode(Model* model, std::string name = "");
	virtual ~CppCode() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show();
public: // gets & sets
	void setCode(std::string _code);
	std::string getCode() const;
	bool IsLibraryLoaded() const;
public: // new methods
	CodeResult compile();
	CodeResult loadLibrary();
	CodeResult unloadLibrary();
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // could be overriden 
	virtual bool _check(std::string* errorMessage);
	// virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	virtual void _createInternalAndAttachedData();
private:

	const struct DEFAULT_VALUES {
		std::string code = "";
		std::string tempDir = ".temp/";
		std::string compiler = "g++";
		std::string flags = "";
	} DEFAULT;
	std::string _code = DEFAULT.code;
	std::string _tempDir = DEFAULT.tempDir;
	std::string _compiler = DEFAULT.compiler;
	std::string _flags = DEFAULT.flags;

	bool libraryLoaded = false;
};

#endif /* CPPCODE_H */

