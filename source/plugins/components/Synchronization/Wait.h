/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Wait.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#ifndef WAIT_H
#define WAIT_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "../../data/Synchronization/SignalData.h"
#include "kernel/simulator/OnEventManager.h"

/*!
 * \brief Holds an entity in a queue until a signal arrives, a condition
 * becomes true, or it is explicitly removed elsewhere.
 *
 * Arena correspondence: this is Arena's "Hold module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Advanced Process Panel", p. 54),
 * renamed `Wait` in GenESyS. `WaitType` (`WaitForSignal/InfiniteHold/
 * ScanForCondition`) matches Arena's Type exactly; `_condition` and
 * `_limitExpression` match Condition and Limit; the attached `SignalData`
 * matches Wait for Value; an entity held with `InfiniteHold` is later
 * released by Remove, exactly as in Arena.
 *
 * Known difference from Arena: `_queue` is a plain `Queue*` rather than a
 * full `QueueableItem` (Queue/Set/Attribute/Expression), a gap already
 * flagged with a `@TODO` in the source.
 */
class Wait : public ModelComponent {
public:

	enum class WaitType : int {
		WaitForSignal = 0, InfiniteHold = 1, ScanForCondition = 2, num_elements = 3
	};
public:
	static std::string convertEnumToStr(WaitType type);
public: // constructors
	Wait(Model* model, std::string name = "");
	virtual ~Wait() = default;
public: // virtual
	virtual std::string show() override;
public: //
	void setSignalData(SignalData* signal);
	void setWaitType(WaitType _watType);
	Wait::WaitType getWaitType() const;
	void setCondition(std::string _condition);
	std::string getCondition() const;
	Queue* getQueue() const;
	void setQueue(Queue* queue);
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	std::string getlimitExpression() const;
	void setLimitExpression(const std::string &newLimitExpression);

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden .
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	// virtual void _createInternalAndAttachedData() override;
	//virtual ParserChangesInformation* _getParserChangesInformation();

protected:
	//virtual void _createInternalStatisticReporters() override;
	virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	//virtual void _createAttachedAttributes() override;

private: // methods
	unsigned int _handlerForSignalDataEvent(SignalData* signalData);
	void _handlerForAfterProcessEventEvent(SimulationEvent* event);
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const WaitType waitType = Wait::WaitType::WaitForSignal;
		const std::string condition = "";
        const std::string limitExpression = "0";
	} DEFAULT;
	WaitType _waitType = DEFAULT.waitType;
	std::string _condition = DEFAULT.condition;
	std::string limitExpression = DEFAULT.limitExpression;
private: // internal
	Queue *_queue = nullptr; // @TODO: It should be a QueueableItem, (Queue or Set)
private: // attached
	SignalData* _signalData = nullptr;
	bool _isScanConditionHandlerRegistered = false; // local guard to avoid duplicate registration in repeated checks
private: // attributes 1:n
};


#endif /* WAIT_H */
