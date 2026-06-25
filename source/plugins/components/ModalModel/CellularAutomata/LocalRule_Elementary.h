#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"
#include <cmath>

class LocalRule_Elementary: public LocalRule {
public:
    LocalRule_Elementary(CellularAutomataBase* parentCellularAutomata, uint8_t ruleNumber=30, StateSet* stateSet=nullptr)
	:LocalRule(parentCellularAutomata, stateSet) {
		this->ruleNumber = ruleNumber;
	}
    LocalRule_Elementary(const LocalRule_Elementary& orig): LocalRule(orig) { }
    virtual ~LocalRule_Elementary()=default;
public:
    virtual void applyRule(Cell* cell) override {
        // Wolfram index for a 1D radius-1 elementary CA = 4*left + 2*center + 1*right (left is the MSB).
        // Computed with integer shifts; the previous version used std::pow on doubles, which is fragile.
        // Neighbors of a centered radius-1 1D cell arrive in canonical order {left, right}.
        const std::vector<Cell*> neighbors = cell->getNeighbors();
        long index = static_cast<long>(stateValue(cell)) << 1; // center, weight 2
        if (neighbors.size() >= 1)
            index += static_cast<long>(stateValue(neighbors[0])) << 2; // left, weight 4
        if (neighbors.size() >= 2)
            index += static_cast<long>(stateValue(neighbors[1]));      // right, weight 1
        const int bit = (ruleNumber >> index) & 1;
        setNextStateFromValue(cell, bit);
    }
public:
    uint8_t getRuleNumber() const { return ruleNumber; }
    void setRuleNumber(const uint8_t ruleNumber) {this->ruleNumber = ruleNumber;}
private:
    uint8_t ruleNumber;
};
