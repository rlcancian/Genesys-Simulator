#pragma once
#include "plugins/components/Logic/CellularAutomata/Cell.h"
#include "plugins/components/Logic/CellularAutomata/CellularAutomataBase.h"


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
public:
	void setStateSet(StateSet* stateSet){
		this->stateSet = stateSet;
	}
protected:
    CellularAutomataBase* parentCellularAutomata;
    StateSet* stateSet;
};
