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
	/*!
	 * \brief setFirstCreation
	 * \param _firstCreation
	 */
	void setFirstCreation(double _firstCreation);
	/*!
	 * \brief getFirstCreation
	 * \return
	 */
	double getFirstCreation() const;
	/*!
	 * \brief setEntityType
	 * \param _entityType
	 */
	void setEntityType(EntityType* _entityType);
	/*!
	 * \brief setEntityTypeName
	 * \param entityTypeName
	 */
	void setEntityTypeName(std::string entityTypeName);
	/*!
	 * \brief getEntityType
	 * \return
	 */
	EntityType* getEntityType() const;
	/*!
	 * \brief setTimeUnit
	 * \param _timeUnit
	 */
	void setTimeUnit(Util::TimeUnit _timeUnit);
	/*!
	 * \brief getTimeUnit
	 * \return
	 */
	Util::TimeUnit getTimeUnit() const;
	/*!
	 * \brief setTimeBetweenCreationsExpression
	 * \param _timeBetweenCreations
	 * \param _timeUnit
	 */
	void setTimeBetweenCreationsExpression(std::string _timeBetweenCreations, Util::TimeUnit _timeUnit=Util::TimeUnit::unknown);
	/*!
	 * \brief getTimeBetweenCreationsExpression
	 * \return
	 */
	std::string getTimeBetweenCreationsExpression() const;
	/*!
	 * \brief setMaxCreations
	 * \param _maxCreations
	 */
	void setMaxCreations(unsigned long _maxCreations);
	/*!
	 * \brief setMaxCreations
	 * \param _maxCreationsExpression
	 */
	void setMaxCreations(std::string _maxCreationsExpression);
	/*!
	 * \brief getMaxCreations
	 * \return
	 */
	std::string getMaxCreations() const;
	/*!
	 * \brief getEntitiesCreated
	 * \return
	 */
	unsigned int getEntitiesCreated() const;
	/*!
	 * \brief setEntitiesCreated
	 * \param _entitiesCreated
	 */
	void setEntitiesCreated(unsigned int _entitiesCreated);
	/*!
	 * \brief setEntitiesPerCreation
	 * \param _entitiesPerCreation
	 */
	void setEntitiesPerCreation(unsigned int _entitiesPerCreation);
	/*!
	 * \brief getEntitiesPerCreation
	 * \return
	 */
	unsigned int getEntitiesPerCreation() const;
public:
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _initBetweenReplications() override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
    virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalAndAttachedData() override;
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

