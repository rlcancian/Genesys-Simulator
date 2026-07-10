#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"

class LocalRule_Growty : public LocalRule {
public:
    LocalRule_Growty(CellularAutomataBase* parentCellularAutomata)
    : LocalRule(parentCellularAutomata) {  }
    LocalRule_Growty(const LocalRule_Growty& orig): LocalRule(orig) {}
    virtual ~LocalRule_Growty() = default;
public:
    virtual void applyRule(Cell* cell) override {
        unsigned int sum = 0;
        for (Cell* neigh : cell->getNeighbors()) {
			sum += stateValue(neigh);
        }
        if (sum <= 3 || sum == 5)
			setNextStateFromValue(cell, 0);
        else
			setNextStateFromValue(cell, 1);
    }
};
