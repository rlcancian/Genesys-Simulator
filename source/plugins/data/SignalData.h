/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   SignalData.h
 * Author: rlcancian
 *
 * Created on 25 de maio de 2022, 14:37
 */

#ifndef SIGNALDATA_H
#define SIGNALDATA_H

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/util/List.h"

class SignalData : public ModelDataDefinition {
public:
	typedef std::function<unsigned int(SignalData*) > SignalDataEventHandler; //< Returns the total number of freed entities.
	typedef std::pair<SignalDataEventHandler, ModelComponent*> PairSignalDataEventHandler;
	template<typename Class>
	static SignalDataEventHandler SetSignalDataEventHandler(unsigned int (Class::*function)(SignalData*), Class * object) {
		return std::bind(function, object, std::placeholders::_1);
	}
public:
	SignalData(Model* model, std::string name = "");
	virtual ~SignalData() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: //virtual
	virtual std::string show();
public:
	unsigned int generateSignal(double signalValue, unsigned int limit);
	void addSignalDataEventHandler(SignalDataEventHandler eventHandler, ModelComponent* component);
	unsigned int remainsToLimit() const;
	void decreaseRemainLimit();

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // could be overriden .
	virtual bool _check(std::string* errorMessage);
	virtual void _initBetweenReplications();
	//virtual void _createInternalAndAttachedData();
	//virtual ParserChangesInformation* _getParserChangesInformation();
private: // methods
	unsigned int  _notifySignalDataEventHandlers(); ///< Notify observer classes that some of the resource capacity has been released. It is useful for allocation components (such as Seize) to know when an entity waiting into a queue can try to seize the resource again
private: //1::1
	unsigned int _remainsToLimit;
private: //1::n
	List<PairSignalDataEventHandler*>* _signalDataEventHandlers = new List<PairSignalDataEventHandler*>();
};

#endif /* SIGNALDATA_H */

