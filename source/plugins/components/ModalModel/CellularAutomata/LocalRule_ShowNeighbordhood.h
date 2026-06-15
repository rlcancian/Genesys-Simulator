#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

class LocalRule_ShowNeighbordhood : public LocalRule {
public:
    LocalRule_ShowNeighbordhood(CellularAutomataBase* parentCellularAutomata)
    : LocalRule(parentCellularAutomata) {  }
    LocalRule_ShowNeighbordhood(const LocalRule_ShowNeighbordhood& orig): LocalRule(orig) {}
    virtual ~LocalRule_ShowNeighbordhood() = default;
public:
    virtual void applyRule(Cell* cell) {
        auto* enumerableStateSet = dynamic_cast<StateSet_Enumerable*>(stateSet);
        if (cell == nullptr || enumerableStateSet == nullptr || enumerableStateSet->getStatesSize() < 3)
            return;
        if (cell->getCurrentState()==*enumerableStateSet->getState(1))
            cell->setNextState(cell->getCurrentState());
        else {
            for (Cell* n: cell->getNeighbors()) {
                if (n != nullptr && n->getCurrentState()==*enumerableStateSet->getState(1)) {
                    cell->setNextState(*enumerableStateSet->getState(2));
                    return;
                }
            }
            cell->setNextState(*enumerableStateSet->getState(0));
        }
    }
};
