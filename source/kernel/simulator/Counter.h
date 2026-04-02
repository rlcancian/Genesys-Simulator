/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   CounterDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 29 de Maio de 2019, 11:24
 */

#ifndef COUNTERDEFAULTIMPL1_H
#define COUNTERDEFAULTIMPL1_H

#include "ModelDataDefinition.h"
#include "ModelDataManager.h"
#include "Plugin.h"
//namespace GenesysKernel {

/*!
 * \brief Model datum used to accumulate event counts (or weighted counts).
 *
 * The counter value starts cleared for each replication (depending on model
 * lifecycle hooks) and can be incremented by arbitrary amounts.
 */
class Counter : public ModelDataDefinition {
public:
	/*! \brief Creates a counter model datum with optional parent relationship. */
	Counter(Model* model, std::string name = "", ModelDataDefinition* parent = nullptr);
	virtual ~Counter() = default;
public:
	virtual std::string show() override;
public: // public static methods
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	/*!
	 * \brief clear
	 * \details Resets the internal count value to zero.
	 */
	void clear();
	/*!
	 * \brief incCountValue
	 * \param value
	 * \details Adds \p value to the internal count (default increment is 1).
	 */
	void incCountValue(/*int*/double value = 1.0);
	/*!
	 * \brief getCountValue
	 * \return Current accumulated count value.
	 */
	double /*unsigned long*/ getCountValue() const;
	/*!
	 * \brief getParent
	 * \return Parent model datum associated with this counter (if any).
	 */
	ModelDataDefinition* getParent() const;
protected: //! must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: //! could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
private:
	ModelDataDefinition* _parent;
	double /*unsigned long*/ _count = 0;
};
//namespace\\}
#endif /* COUNTERDEFAULTIMPL1_H */
