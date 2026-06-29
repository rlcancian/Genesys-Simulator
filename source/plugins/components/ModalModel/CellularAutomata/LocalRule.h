#pragma once
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

class PersistenceRecord;

class LocalRule {
public:
	LocalRule(CellularAutomataBase* parentCellularAutomata,  StateSet* stateSet=nullptr){
		this->parentCellularAutomata = parentCellularAutomata;
		parentCellularAutomata->setLocalRule(this);
		if (stateSet != nullptr)
			this->stateSet = stateSet;
		else
			this->stateSet = parentCellularAutomata->getStateSet();
	}
	LocalRule(const LocalRule& orig){}
    virtual ~LocalRule()=default;
public:
    virtual void applyRule(Cell* cell) = 0; ///< Pure virtual method that has to be overiden by derived class, responsable for applying the local rule to a cell
	virtual std::string getName() const { return "LocalRule"; }
	virtual std::string getRuleType() const { return "LocalRule"; }

	// Persistência (declaradas como virtuais para permitir override nas derivadas)
	virtual bool _loadInstance(PersistenceRecord* fields);
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues);

public:
	void setStateSet(StateSet* stateSet){
		this->stateSet = stateSet;
	}
protected:
    CellularAutomataBase* parentCellularAutomata;
    StateSet* stateSet;
};
