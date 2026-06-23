#pragma once
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"


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
	bool setNextStateFromValue(Cell* cell, long value) const {
		if (cell == nullptr)
			return false;
		State nextState(value);
		if (stateSet != nullptr && !stateSet->tryMakeState(value, &nextState))
			return false;
		cell->setNextState(nextState);
		return true;
	}
	long stateValue(const Cell* cell) const {
		return cell->getCurrentState().getValue();
	}
protected:
    CellularAutomataBase* parentCellularAutomata;
    StateSet* stateSet;
};
