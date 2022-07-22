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

#include "../../kernel/simulator/ModelComponent.h"
#include "Seize.h"
#include "Delay.h"
#include "Release.h"

/*!
 This component ...
 */
class Process : public ModelComponent {
public: // constructors
	Process(Model* model, std::string name = "");
	virtual ~Process() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setPriority(unsigned short _priority);
	unsigned short getPriority() const;
	void setAllocationType(unsigned int _allocationType);
	unsigned int getAllocationType() const;
	List<SeizableItem*>* getSeizeRequests() const;
	void setQueueableItem(QueueableItem* _queueableItem);
	QueueableItem* getQueueableItem() const;
	void setSaveAttribute(std::string _saveAttribute);
	std::string getSaveAttribute() const;
	void setDelayExpression(std::string _delayExpression);
	std::string delayExpression() const;
	void setDelayTimeUnit(Util::TimeUnit _delayTimeUnit);
	Util::TimeUnit delayTimeUnit() const;
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	// virtual void _initBetweenReplications();
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private: // methods
	void _adjustConnections();
private: // attributes 1:1
	Seize* _seize = nullptr;
	Delay* _delay = nullptr;
	Release* _release = nullptr;
	//ModelComponent* _nextComponent = nullptr;
	//unsigned int _nextInput;
private: // attributes 1:n
};

#endif /* PROCESS_H */

