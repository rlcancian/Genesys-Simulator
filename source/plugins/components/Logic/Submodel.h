/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Submodel.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:04
 */

#ifndef SUBMODEL_H
#define SUBMODEL_H

#include "kernel/simulator/ModelComponent.h"

/*!
 This component ...
 */
class Submodel : public ModelComponent {
public: // constructors
	Submodel(Model* model, std::string name = "");
	virtual ~Submodel() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // virtual
	//virtual void _initBetweenReplications();
	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;
	virtual bool _check(std::string& errorMessage) override;

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

private: // methods
private: // attributes 1:1
private: // attributes 1:n
};


#endif /* SUBMODEL_H */

