#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include <map>
#include <algorithm>

//! User-defined local rule example: majority vote.
//! Each cell takes the most frequent state among its neighbors.
//! If there is a tie, the smallest state value wins.
//! This rule is designed to be used with non-uniform CA to demonstrate
//! how a user can define arbitrary custom rules by inheriting from LocalRule.
class LocalRule_Custom : public LocalRule {
public:
    LocalRule_Custom(CellularAutomataBase* parentCellularAutomata)
        : LocalRule(parentCellularAutomata) {}
    LocalRule_Custom(const LocalRule_Custom& orig) : LocalRule(orig) {}
    virtual ~LocalRule_Custom() = default;

public:
    virtual void applyRule(Cell* cell) override {
        const std::vector<Cell*>& neighbors = cell->getNeighbors();
        if (neighbors.empty()) {
            cell->setNextState(cell->getCurrentState());
            return;
        }
        std::map<long, int> counts;
        for (Cell* neighbor : neighbors) {
            counts[neighbor->getCurrentState().getValue()]++;
        }
        long majority = std::max_element(
            counts.begin(), counts.end(),
            [](const std::pair<long,int>& a, const std::pair<long,int>& b) {
                return a.second < b.second;
            }
        )->first;
        cell->setNextState(State(majority));
    }
};
