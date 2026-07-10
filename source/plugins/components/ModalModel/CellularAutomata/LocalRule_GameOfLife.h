#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"

class LocalRule_GameOfLife: public LocalRule {
public:
    LocalRule_GameOfLife(CellularAutomataBase* parentCellularAutomata)
    :LocalRule(parentCellularAutomata) { }
    LocalRule_GameOfLife(const LocalRule_GameOfLife& orig): LocalRule(orig) { }
    virtual ~LocalRule_GameOfLife()=default;
public:
    virtual void applyRule(Cell* cell) override {
        unsigned int living = 0;
        for (Cell* neigh : cell->getNeighbors()) {
			living += stateValue(neigh);
        }
		if (stateValue(cell) == 1) {
            if (living < 2 || living > 3)
				setNextStateFromValue(cell, 0);
            else
                cell->setNextState(cell->getCurrentState());
        } else if (living == 3)
			setNextStateFromValue(cell, 1);
        else 
            cell->setNextState(cell->getCurrentState());
    }
};
