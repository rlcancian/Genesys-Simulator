/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Storage.h
 * Author: rlcancian
 *
 * Created on 20 de Storageembro de 2019, 20:06
 */

#ifndef STORAGE_H
#define STORAGE_H


#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
//#include "kernel/simulator/ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"

/*!
 * \brief Data definition representing a named storage location with
 * explicit area/capacity/density accounting.
 *
 * Arena correspondence: Arena's "Storage module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Advanced Process Panel", p. 78), which
 * is a near-empty placeholder object (only a Name) auto-created by the
 * Store/Unstore modules and only needed explicitly as a Storage-Set member.
 *
 * Known difference from Arena (GenESyS extends the concept rather than
 * narrowing it): this class carries real capacity semantics — \c
 * _totalArea, \c _capacity and \c _unitsPerArea — that Arena's Storage
 * module does not model at all; Arena tracks storage occupancy purely
 * through animation, not through a declared capacity/area contract.
 */
class Storage : public ModelDataDefinition {
public:
	Storage(Model* model, std::string name = "");
	virtual ~Storage() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
	void setTotalArea(double _totalArea);
	double getTotalArea() const;
    void setCapacity(unsigned int _capacity);
    unsigned int getCapacity() const;
    void setUnitsPerArea(double _unitsPerArea);
    double getUnitsPerArea() const;

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden 
	virtual bool _check(std::string& errorMessage) override;
	virtual ParserChangesInformation* _getParserChangesInformation() override;

protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:

	const struct DEFAULT_VALUES {
		const double totalArea = 1;
		const unsigned int capacity = 10;
		const double unitsPerArea = 1;
	} DEFAULT;
	double _totalArea = DEFAULT.totalArea;
	unsigned int _capacity = DEFAULT.capacity;
	double _unitsPerArea = DEFAULT.unitsPerArea;

private:
	//@TODO: Add statisticCollector for ProportionOfStorageUsage
};
#endif /* STORAGE_H */
