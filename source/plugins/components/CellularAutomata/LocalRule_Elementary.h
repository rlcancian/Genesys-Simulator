#pragma once

#include "LocalRule.h"
#include "Cell.h"
#include "StateSet_Enumerable.h"
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
    virtual void applyRule(Cell* cell) {
		long number = 0;
        int bit, power = 2;
        for (Cell* neigh : cell->getNeighbors()) {
			number += neigh->getCurrentState().getValue() * std::pow(2, power--);
            if (power==1)
				number += std::pow(2, power--)*cell->getCurrentState().getValue();
        }
        bit = (ruleNumber & (uint8_t)std::pow(2,number)) >> number;
		cell->setNextState(State(bit));
    }
public:
    uint8_t getRuleNumber() const { return ruleNumber; }
    void setRuleNumber(const uint8_t ruleNumber) {this->ruleNumber = ruleNumber;}
private:
    uint8_t ruleNumber;
};
