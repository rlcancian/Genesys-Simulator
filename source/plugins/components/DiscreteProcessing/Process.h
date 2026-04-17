/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Process.h
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Maio de 2019, 18:41
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "kernel/simulator/ModelComponent.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Release.h"

/*!
 This component ...
 */
class Process : public ModelComponent {
public: // constructors
	Process(Model* model, std::string name = "");
	virtual ~Process() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setPriority(unsigned short _priority);
	unsigned short getPriority() const;
	void setPriorityExpression(std::string priority);
	std::string getPriorityExpression() const;
	void setAllocationType(Util::AllocationType _allocationType);
	Util::AllocationType getAllocationType() const;
	List<SeizableItem*>* getSeizeRequests() const;
	void addSeizeRequest(SeizableItem* newRequest);
	void removeSeizeRequest(SeizableItem* request);
	void setQueueableItem(QueueableItem* _queueableItem);
	QueueableItem* getQueueableItem() const;
	void setDelayExpression(std::string _delayExpression);
	void setDelayExpression(std::string _delayExpression, Util::TimeUnit _delayTimeUnit);
	std::string delayExpression() const;
	void setDelayTimeUnit(Util::TimeUnit _delayTimeUnit);
	Util::TimeUnit delayTimeUnit() const;
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	// virtual void _initBetweenReplications();
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalAndAttachedData() override;
private: // methods
	void _adjustConnections();
	void _ensureInternalComponents();
	void _reconcileInternalComponents();
	bool _hasConsistentInternalChain(std::string& errorMessage) const;
private: // attributes 1:1
	Seize* _seize = nullptr;
	Delay* _delay = nullptr;
	Release* _release = nullptr;
	bool _flagConstructing;
	//ModelComponent* _nextComponent = nullptr;
	//unsigned int _nextInput;
private: // attributes 1:n
};

#endif /* PROCESS_H */
