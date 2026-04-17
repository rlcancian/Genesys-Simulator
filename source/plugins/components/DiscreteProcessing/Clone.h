/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clone.h
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Novembro de 2021, 18:50
 */

#ifndef CLONE_H
#define CLONE_H

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Counter.h"

/*!
 This component ...
 */
class Clone : public ModelComponent {
public: // constructors
	Clone(Model* model, std::string name = "");
	virtual ~Clone() = default;
public: // virtual
	virtual std::string show() override;
public: // 
	void setNumClonesExpression(std::string numClones);
	std::string getNumClonesExpression() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden .
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalAndAttachedData() override;
	//virtual ParserChangesInformation* _getParserChangesInformation();
private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const std::string numClonesExpression = "1";
	} DEFAULT;
	std::string _numClonesExpression = DEFAULT.numClonesExpression;
	Counter* _counter = nullptr;
private: // attributes 1:n
};

#endif /* CLONE_H */

