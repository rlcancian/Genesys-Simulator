/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EntityType.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 19:24
 */

#ifndef ENTITYTYPE_H
#define ENTITYTYPE_H

#include <string>
#include "ModelDataDefinition.h"
#include "StatisticsCollector.h"
#include "ModelDataManager.h"
#include "Plugin.h"

//#include "Model.h"
//namespace GenesysKernel {

/*!
 * 
 */
class EntityType : public ModelDataDefinition {
public:
	EntityType(Model* model, std::string name = "");
	virtual ~EntityType();
public:
	virtual std::string show() override;
public: //static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: //get & set
	// @ToDo: (importante): Change categories Waiting, Others, Transfer, etc to customizable ones
	void setInitialWaitingCost(double _initialWaitingCost);
	double initialWaitingCost() const;
	void setInitialOtherCost(double _initialOtherCost);
	double initialOtherCost() const;
	void setInitialNVACost(double _initialNVACost);
	double initialNVACost() const;
	void setInitialVACost(double _initialVACost);
	double initialVACost() const;
	void setInitialPicture(std::string _initialPicture);
	std::string initialPicture() const;
public: //get
	/*!
	 * \brief addGetStatisticsCollector
	 * \param name
	 * \return
	 */
	StatisticsCollector* addGetStatisticsCollector(std::string name);

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalAndAttachedData() override;
private:
	void _initCostsAndStatistics();
private:

	const struct DEFAULT_VALUES {
		const std::string initialPicture = "report";
		const double initialCost = 0.0;
	} DEFAULT;
	std::string _initialPicture = DEFAULT.initialPicture;
	double _initialVACost = DEFAULT.initialCost;
	double _initialNVACost = DEFAULT.initialCost;
	double _initialOtherCost = DEFAULT.initialCost;
	double _initialWaitingCost = DEFAULT.initialCost;
private: //1:n
	List<StatisticsCollector*>* _statisticsCollectors = new List<StatisticsCollector*>();
};
//namespace\\}
#endif /* ENTITYTYPE_H */
