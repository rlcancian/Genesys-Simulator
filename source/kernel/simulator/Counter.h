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
 The Counter modeldatum is used to count events, and its internal count value is added by a configurable amount, usually incremented by one.
 */
class Counter : public ModelDataDefinition {
public:
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
	 */
	void clear();
	/*!
	 * \brief incCountValue
	 * \param value
	 */
	void incCountValue(/*int*/double value = 1.0);
	/*!
	 * \brief getCountValue
	 * \return
	 */
	double /*unsigned long*/ getCountValue() const;
	/*!
	 * \brief getParent
	 * \return
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

