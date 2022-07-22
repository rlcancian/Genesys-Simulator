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
#include "../data/CppCode.h"

/*!
 This component ...
 */
class CppForG : public ModelComponent {
public: // constructors
	CppForG(Model* model, std::string name = "");
	virtual ~CppForG() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setSourceCode(std::string _sourceCode);
	std::string getSourceCode() const;
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private: // methods
private: // attributes 1:1
	// internel
	CppCode* _cppcode = nullptr;
private: // attributes 1:n
};

