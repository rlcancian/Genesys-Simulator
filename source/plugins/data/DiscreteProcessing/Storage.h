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


#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
//#include "kernel/simulator/ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"

/*
Storage module
DESCRIPTION
The Storage module defines the name of a storage. Storages are automatically created
by any module that references the storage so that this module is seldom needed. The
only time this module is needed is when a storage is defined as a member of a storage
set or specified using an attribute or expression.
TYPICAL USES
* Defining an animate storage for a set of storages
PROMPTS
Prompt Description
Name The name of the storage set being defined. This name must be
unique.
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

