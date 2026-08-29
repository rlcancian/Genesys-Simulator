/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Queue.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 17:12
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/util/List.h"
#include "../../../kernel/simulator/essentialPlugins/Entity.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "../../../kernel/simulator/essentialPlugins/StatisticsCollector.h"
#include "kernel/simulator/Plugin.h"
#include "../../../kernel/simulator/model/ModelComponent.h"

class Waiting {
public:
	Waiting(Entity* entity, double timeStartedWaiting, ModelComponent* thisComponent, unsigned int thisComponentOutputPort = 0) {
		_entity = entity;
		_thisComponent = thisComponent;
		_timeStartedWaiting = timeStartedWaiting;
		_thisComponentOutputPort = thisComponentOutputPort;
	}

	virtual ~Waiting() = default;
public:
	virtual std::string show() {
		return //ModelDataDefinition::show()+
		",entity=" + std::to_string(_entity->getId()) +
				",component=\"" + _thisComponent->getName() + "\"" +
				",inputPort=\"" + std::to_string(_thisComponentOutputPort) + "\"" +
				",timeStartedWaiting=" + std::to_string(_timeStartedWaiting);
	}
public:
	double getTimeStartedWaiting() const {
		return _timeStartedWaiting;
	}
	ModelComponent* geComponent() const {
		return _thisComponent;
	}
	Entity* getEntity() const {
		return _entity;
	}
	unsigned int geComponentOutputPort() const {
		return _thisComponentOutputPort;
	}
	unsigned long long getArrivalOrder() const {
		return _arrivalOrder;
	}
	void setArrivalOrder(unsigned long long arrivalOrder) {
		_arrivalOrder = arrivalOrder;
	}

protected:
private:
	Entity* _entity;
	ModelComponent* _thisComponent;
	double _timeStartedWaiting;
	unsigned int _thisComponentOutputPort;
	unsigned long long _arrivalOrder = 0;
};

/*!
 * \brief Data definition holding waiting entities for a component (a
 * Seize-family component, Process, Batch, etc.) and defining their ranking
 * rule.
 *
 * Arena correspondence: the "Queue module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 45-46). \c OrderRule
 * mirrors Arena's four ranking rules (First In First Out, Last In First Out,
 * lowest/highest attribute value first); \c _attributeName is the
 * evaluated attribute for the two attribute-based rules.
 *
 * Known difference from Arena: this class has no explicit "Shared" flag.
 * In GenESyS a Queue is simply a referenceable ModelDataDefinition, so any
 * number of components may already reference the same instance; Arena needs
 * an explicit opt-in because its queues are otherwise private to the module
 * that created them.
 *
 * \c isReportStatistics()/setReportStatistics() (inherited from
 * ModelDataDefinition) correspond to Arena's per-module "Report Statistics"
 * checkbox and gate the internal \c StatisticsCollector instances created by
 * \c _createInternalStatisticReporters().
 */
class Queue : public ModelDataDefinition {
public:

	enum class OrderRule : int {
		FIFO = 0, LIFO = 1, HIGHESTVALUE = 2, SMALLESTVALUE = 3, num_elements = 4
	};
public:
	static std::string convertEnumToStr(OrderRule rule);
public:
	Queue(Model* model, std::string name = "");
	virtual ~Queue();
public:
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void insertElement(Waiting* modeldatum);
	void removeElement(Waiting* modeldatum);
	unsigned int size();
	Waiting* first();
	Waiting* getAtRank(unsigned int rank);
	void setAttributeName(std::string _attributeName);
	std::string getAttributeName() const;
	void setOrderRule(OrderRule _orderRule);
	Queue::OrderRule getOrderRule() const;
	void setOrderRuleInt(int orderRule);
	int getOrderRuleInt() const;
public: // to implement SIMAN functions
	double sumAttributesFromWaiting(Util::identification attributeID); // use to implement SIMAN SAQUE function
	double getAttributeFromWaitingRank(unsigned int rank, Util::identification attributeID);
	//public:
	//	void initBetweenReplications();
protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual ParserChangesInformation* _getParserChangesInformation() override;
protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	void _initCStats();
	void _configureListComparator();
private:
	List<Waiting*>* _list = new List<Waiting*>();
	double _lastTimeNumberInQueueChanged = 0.0;
	unsigned long long _nextArrivalOrder = 0;
private: //1::1

	const struct DEFAULT_VALUES {
		const OrderRule orderRule = OrderRule::FIFO;
		const std::string attributeName = "";
	} DEFAULT;
	OrderRule _orderRule = DEFAULT.orderRule;
	std::string _attributeName = DEFAULT.attributeName;
private: // inner internal elements
	StatisticsCollector* _cstatNumberInQueue = nullptr;
	StatisticsCollector* _cstatTimeInQueue = nullptr;
};

#endif /* QUEUE_H */