#pragma once

#include "LocalRule.h"
#include "Cell.h"
#include "State.h"

class LocalRule_GameOfLife: public LocalRule {
public:
    LocalRule_GameOfLife(CellularAutomataBase* parentCellularAutomata)
    :LocalRule(parentCellularAutomata) { }
    LocalRule_GameOfLife(const LocalRule_GameOfLife& orig): LocalRule(orig) { }
    virtual ~LocalRule_GameOfLife()=default;
public:
    virtual void applyRule(Cell* cell) {
        unsigned int living = 0;
        for (Cell* neigh : cell->getNeighbors()) {
			living += neigh->getCurrentState().getValue();
        }
		if (cell->getCurrentState().getValue() == 1) {
            if (living < 2 || living > 3)
				cell->setNextState(State(0));
            else
                cell->setNextState(cell->getCurrentState());
        } else if (living == 3)
			cell->setNextState(State(1));
        else 
            cell->setNextState(cell->getCurrentState());
    }
};
