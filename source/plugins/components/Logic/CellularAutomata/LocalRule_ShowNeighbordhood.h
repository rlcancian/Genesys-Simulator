#pragma once

#include "plugins/components/Logic/CellularAutomata/LocalRule.h"
#include "plugins/components/Logic/CellularAutomata/Cell.h"

class LocalRule_ShowNeighbordhood : public LocalRule {
public:
    LocalRule_ShowNeighbordhood(CellularAutomataBase* parentCellularAutomata)
    : LocalRule(parentCellularAutomata) {  }
    LocalRule_ShowNeighbordhood(const LocalRule_ShowNeighbordhood& orig): LocalRule(orig) {}
    virtual ~LocalRule_ShowNeighbordhood() = default;
public:
    virtual void applyRule(Cell* cell) {
        if (cell->getCurrentState()==stateSet->getState(1))
            cell->setNextState(cell->getCurrentState());
        else {
            for (Cell* n: cell->getNeighbors()) {
                if (n->getCurrentState()==stateSet->getState(1)) {
                    cell->setNextState(stateSet->getState(2));
                    return;
                }
            }
            cell->setNextState(stateSet->getState(0));
        }
    }
};
