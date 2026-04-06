#pragma once

#include "LocalRule.h"
#include "Cell.h"

class LocalRule_Growty : public LocalRule {
public:
    LocalRule_Growty(CellularAutomataBase* parentCellularAutomata)
    : LocalRule(parentCellularAutomata) {  }
    LocalRule_Growty(const LocalRule_Growty& orig): LocalRule(orig) {}
    virtual ~LocalRule_Growty() = default;
public:
    virtual void applyRule(Cell* cell) {
        unsigned int sum = 0;
        for (Cell* neigh : cell->getNeighbors()) {
			sum += neigh->getCurrentState().getValue();
        }
        if (sum <= 3 || sum == 5)
			cell->setNextState(State(0));
        else
			cell->setNextState(State(1));
    }
};
