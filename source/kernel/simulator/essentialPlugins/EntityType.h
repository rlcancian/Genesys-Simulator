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
#include "../model/ModelDataDefinition.h"
#include "StatisticsCollector.h"
#include "../model/ModelDataManager.h"
#include "../Plugin.h"

//#include "Model.h"
//namespace GenesysKernel {

/*!
 * \brief Data definition describing one class of entity that Create-family
 * components may instantiate at simulation time.
 *
 * Arena correspondence: the "Entity module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 44-45), which defines
 * an entity type's initial picture, holding cost/hour and initial
 * value-added/non-value-added/waiting/transfer/other cost attributes.
 *
 * Confirmed in this class: initial picture (\c initialPicture) and initial
 * value-added, non-value-added, waiting and other costs. \c
 * setInitialWaitingCost()/initialWaitingCost() and the sibling VA/NVA/Other
 * accessors are the closest correspondence to Arena's four cost categories.
 *
 * Known differences from Arena: there is no explicit "Holding Cost/Hour"
 * field, and Arena's fifth cost category ("Initial Transfer Cost") has no
 * dedicated accessor here; a maintainer should decide whether to add them or
 * keep the current four-category cost model. Runtime entities created from
 * an EntityType are instances of Entity, not of this class.
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
protected: // could be ovveriden
	//virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalStatisticReporters() override;
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
