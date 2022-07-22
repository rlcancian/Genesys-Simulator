/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SourceModelCOmponent.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:50
 */

#ifndef SOURCEMODELCOMPONENT_H
#define SOURCEMODELCOMPONENT_H

#include "ModelComponent.h"
#include <string>
#include <limits>
//namespace GenesysKernel {

/*!
 * A source component implements the base for inserting entities into the model when its simulation is initialized.
 * During the initialization, the new and empty future events list is populated by events of creating entities and
 * sending them to the source components existing in the model
 */
class SourceModelComponent : public ModelComponent {
public:
	SourceModelComponent(Model* model, std::string componentTypename, std::string name = "");
	virtual ~SourceModelComponent() = default;
public: // get & set
	void setFirstCreation(double _firstCreation);
	double getFirstCreation() const;
	void setEntityType(EntityType* _entityType);
	void setEntityTypeName(std::string entityTypeName);
	EntityType* getEntityType() const;
	void setTimeUnit(Util::TimeUnit _timeUnit);
	Util::TimeUnit getTimeUnit() const;
	void setTimeBetweenCreationsExpression(std::string _timeBetweenCreations);
	std::string getTimeBetweenCreationsExpression() const;
	void setMaxCreations(unsigned long _maxCreations);
	void setMaxCreations(std::string _maxCreationsExpression);
	std::string getMaxCreations() const;
	unsigned int getEntitiesCreated() const;
	void setEntitiesCreated(unsigned int _entitiesCreated);
	void setEntitiesPerCreation(unsigned int _entitiesPerCreation);
	unsigned int getEntitiesPerCreation() const;
public:
	virtual std::string show();
protected:
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _initBetweenReplications();
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
protected: // get & set
	EntityType* _entityType = nullptr;

	const struct DEFAULT_VALUES {
		const double firstCreation = 0.0;
		const unsigned int entitiesPerCreation = 1;
		const std::string maxCreationsExpression = std::to_string(std::numeric_limits<unsigned int>::max());
		const std::string timeBetweenCreationsExpression = "EXPO(1.0)";
		const Util::TimeUnit timeBetweenCreationsTimeUnit = Util::TimeUnit::second;
		//
		const std::string entityTypename = "entitytype";
	} DEFAULT;
	double _firstCreation = DEFAULT.firstCreation;
	unsigned int _entitiesPerCreation = DEFAULT.entitiesPerCreation;
	std::string _maxCreationsExpression = DEFAULT.maxCreationsExpression;
	std::string _timeBetweenCreationsExpression = DEFAULT.timeBetweenCreationsExpression;
	Util::TimeUnit _timeBetweenCreationsTimeUnit = DEFAULT.timeBetweenCreationsTimeUnit;
	unsigned int _entitiesCreatedSoFar = 0;
};
//namespace\\}
#endif /* SOURCEMODELCOMPONENT_H */

