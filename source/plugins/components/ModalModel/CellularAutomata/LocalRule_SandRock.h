#pragma once

#include <vector>
#include <cmath>

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

class LocalRule_SandRock : public LocalRule {
public:
    LocalRule_SandRock(CellularAutomataBase* parentCellularAutomata)
        : LocalRule(parentCellularAutomata) {}

    LocalRule_SandRock(const LocalRule_SandRock& orig)
        : LocalRule(orig) {}

    virtual ~LocalRule_SandRock() = default;

    virtual void applyRule(Cell* cell) override {
        if (!cell || !stateSet) return;

        auto* enumerableStateSet = dynamic_cast<StateSet_Enumerable*>(stateSet);
        if (!enumerableStateSet || enumerableStateSet->getStatesSize() < 3) {
            cell->setNextState(cell->getCurrentState());
            return;
        }

        State* stVazio = enumerableStateSet->getState(0);
        State* stAreia = enumerableStateSet->getState(1);
        State* stRocha = enumerableStateSet->getState(2);

        long current = cell->getCurrentState().getValue();

        auto get = [&](int dx, int dy) -> long {
            return getRelativeState(cell, dx, dy);
        };

        if (current == 2) {
            cell->setNextState(*stRocha);
            return;
        }

        if (current == 1) {
            bool can_S  = (get(0, 1) == 0);
            bool can_SW = (get(-1, 1) == 0) && (get(-1, 0) != 1);
            bool e2_takes_SE = (get(2, 0) == 1) && (get(2, 1) != 0);
            bool can_SE = (get(1, 1) == 0) && (get(1, 0) != 1) && !e2_takes_SE;

            if (can_S || can_SW || can_SE) {
                cell->setNextState(*stVazio);
            } else {
                cell->setNextState(*stAreia);
            }
            return;
        }


        if (current == 0) {
            bool rec_N = (get(0, -1) == 1);
            bool rec_NE = (get(1, -1) == 1) && (get(1, 0) != 0) && (get(0, -1) != 1);
            bool nw_can_SW = (get(-2, 0) == 0) && (get(-2, -1) != 1);
            bool nw_e2_takes_SE = (get(1, -1) == 1) && (get(1, 0) != 0);
            bool rec_NW = (get(-1, -1) == 1) &&
                          (get(-1, 0) != 0) &&
                          !nw_can_SW &&
                          (get(0, -1) != 1) &&
                          !nw_e2_takes_SE;

            if (rec_N || rec_NE || rec_NW) {
                cell->setNextState(*stAreia);
            } else {
                cell->setNextState(*stVazio);
            }
            return;
        }
    }

private:
    /**
     * Busca robusta e dinâmica na malha (suporta saltos de até raio-2).
     * Resolve o problema de células "fantasmas" eliminando o uso de índices fixos no vetor.
     */
    long getRelativeState(Cell* cell, int dx, int dy) {
        if (dx == 0 && dy == 0) return cell->getCurrentState().getValue();

        std::vector<int> myPos = cell->getPosition();
        if (myPos.size() < 2) return 2;

        int tx = myPos[0] + dx;
        int ty = myPos[1] + dy;

        for (Cell* n1 : cell->getNeighbors()) {
            if (!n1) continue;
            std::vector<int> p1 = n1->getPosition();
            if (p1.size() >= 2 && p1[0] == tx && p1[1] == ty) {
                return n1->getCurrentState().getValue();
            }
        }

        if (std::abs(dx) <= 2 && std::abs(dy) <= 2) {
            for (Cell* n1 : cell->getNeighbors()) {
                if (!n1) continue;
                for (Cell* n2 : n1->getNeighbors()) {
                    if (!n2) continue;
                    std::vector<int> p2 = n2->getPosition();
                    if (p2.size() >= 2 && p2[0] == tx && p2[1] == ty) {
                        return n2->getCurrentState().getValue();
                    }
                }
            }
        }

        return 2;
    }
};
