/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Buffer.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "kernel/simulator/ModelComponent.h"
#include "plugins/data/DiscreteProcessing/SignalData.h"

/*!
 This component ...
 */
class Buffer : public ModelComponent {
public:
	enum class AdvanceOn : int {
		NewArrivals = 0, Signal = 1, num_elements = 2
	};
	enum class ArrivalOnFullBufferRule : int {
		Dispose = 0, SendToBulkPort = 1, ReplaceLastPosition = 2, num_elements = 3
	};
	static std::string convertEnumToStr(AdvanceOn advance);
	static std::string convertEnumToStr(ArrivalOnFullBufferRule arrival);
public: // constructors
	Buffer(Model* model, std::string name = "");
	virtual ~Buffer() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	Buffer::ArrivalOnFullBufferRule getarrivalOnFullBufferRule() const;
	void setArrivalOnFullBufferRule(Buffer::ArrivalOnFullBufferRule newArrivalOnFullBufferRule);
	Buffer::AdvanceOn getadvanceOn() const;
	void setAdvanceOn(Buffer::AdvanceOn newAdvanceOn);
	unsigned int getcapacity() const;
	void setCapacity(unsigned int newCapacity);
	SignalData *getsignal() const;
	void setSignal(SignalData *newSignal);

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	virtual ParserChangesInformation* _getParserChangesInformation() override;
	virtual void _initBetweenReplications() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	virtual void _addProperty(SimulationControl* property) override;
private: // methods
	unsigned int _handlerForSignalDataEvent(SignalData* signalData);
	Entity* _advance(Entity* enteringEntity);
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const AdvanceOn advanceOn = AdvanceOn::NewArrivals;
		const ArrivalOnFullBufferRule arrivalOnFullBufferRule = ArrivalOnFullBufferRule::Dispose;
		const unsigned int capacity = 1;
	} DEFAULT;
	ArrivalOnFullBufferRule _arrivalOnFullBufferRule = DEFAULT.arrivalOnFullBufferRule;
	AdvanceOn _advanceOn = DEFAULT.advanceOn;
	unsigned int _capacity = DEFAULT.capacity;
private:
	std::vector<Entity*>* _buffer = new std::vector<Entity*>;
private: // attached
	SignalData* _attachedSignal = nullptr;
	SignalData* _signalWithRegisteredHandler = nullptr;
};

#endif /* BUFFER_H */
