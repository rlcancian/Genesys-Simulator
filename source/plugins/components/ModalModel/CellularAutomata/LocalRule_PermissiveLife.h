#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"

//! Configurable survival/birth rule for binary-state CAs.
//! A live cell survives if its live neighbor count is in [surviveMin, surviveMax].
//! A dead cell is born if its live neighbor count is in [birthMin, birthMax].
//! Defaults (surviveMin=1, surviveMax=4, birthMin=2, birthMax=3) produce a permissive
//! "growth" dynamic when paired with VonNeumann neighborhoods: structures spread and
//! stabilize more easily than under Game of Life, making the zone boundary between
//! GoL and PermissiveLife visibly distinct in non-uniform simulations.
class LocalRule_PermissiveLife : public LocalRule {
    int _surviveMin, _surviveMax, _birthMin, _birthMax;
public:
    LocalRule_PermissiveLife(CellularAutomataBase* parentCellularAutomata,
                              int surviveMin = 1, int surviveMax = 4,
                              int birthMin = 2, int birthMax = 3)
        : LocalRule(parentCellularAutomata),
          _surviveMin(surviveMin), _surviveMax(surviveMax),
          _birthMin(birthMin), _birthMax(birthMax) {}
    LocalRule_PermissiveLife(const LocalRule_PermissiveLife& orig) : LocalRule(orig),
        _surviveMin(orig._surviveMin), _surviveMax(orig._surviveMax),
        _birthMin(orig._birthMin), _birthMax(orig._birthMax) {}
    virtual ~LocalRule_PermissiveLife() = default;

public:
    virtual void applyRule(Cell* cell) override {
        int liveCount = 0;
        for (Cell* neighbor : cell->getNeighbors())
            if (neighbor->getCurrentState().getValue() != 0)
                ++liveCount;
        long current = cell->getCurrentState().getValue();
        long next;
        if (current != 0)
            next = (liveCount >= _surviveMin && liveCount <= _surviveMax) ? current : 0;
        else
            next = (liveCount >= _birthMin && liveCount <= _birthMax) ? 1 : 0;
        cell->setNextState(State(next));
    }

    int getSurviveMin() const { return _surviveMin; }
    int getSurviveMax() const { return _surviveMax; }
    int getBirthMin()   const { return _birthMin; }
    int getBirthMax()   const { return _birthMax; }
};
