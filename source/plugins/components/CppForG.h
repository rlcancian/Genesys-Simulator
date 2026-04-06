/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   CppForG.h
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:37
 */

#pragma once

#include "../../kernel/simulator/ModelComponent.h"
#include "../../kernel/simulator/Simulator.h"
#include "../data/CppCompiler.h"

extern "C" typedef void (*initBetweenReplications_t)(Model* model);
extern "C" typedef void (*onDispatchEvent_t)(Simulator* simulator, Model* model, Entity* entity);

/*!
 This component ...
 */
class CppForG : public ModelComponent {
public: // constructors
	CppForG(Model* model, std::string name = "");
	virtual ~CppForG() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setCppCompiler(CppCompiler* _cppCompiler);
	CppCompiler* getCppCompiler() const;
	void setInitBetweenReplicationCode(std::string _initBetweenReplicationCode);
	std::string getInitBetweenReplicationCode() const;
	void setOnDispatchEventCode(std::string _onDispatchEventCode);
	std::string getOnDispatchEventCode() const;
	void setIncludesCode(std::string _includesCode);
	std::string getIncludesCode() const;
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	virtual void _initBetweenReplications() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	//virtual void _addProperty(PropertyBase* property);

private: // methods

private: // attributes 1:1

	const struct DEFAULT_VALUES {
		std::string onDispatchEventCode = "";
		std::string initBetweenReplicationCode = "";
		std::string includesCode = "";
	} DEFAULT;
	std::string _onDispatchEventCode = DEFAULT.onDispatchEventCode;
	std::string _initBetweenReplicationCode = DEFAULT.initBetweenReplicationCode;
	std::string _includesCode = DEFAULT.includesCode;
	//
	std::string _sourceFilename;
	std::string _outputFilename;
	// pointers to functions on a shared libraty
	onDispatchEvent_t dispatchEvent_SharedLibHandler;
	initBetweenReplications_t initBetweenReplications_SharedLibHandler;
private: // internal (TODO: should be Attached??)
	CppCompiler* _cppCompiler = nullptr;
private: // attributes 1:n
};
