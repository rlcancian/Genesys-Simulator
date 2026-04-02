#pragma once

#include "CellularAutomataBase.h"
#include "LocalRule.h"

//! Complete uniform CA
class CellularAutomata_Classic: public CellularAutomataBase {
public:
	CellularAutomata_Classic(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
	:CellularAutomataBase(lattice, stateSet, neighborhood, localRule) { }
    CellularAutomata_Classic(const CellularAutomata_Classic& orig): CellularAutomataBase(orig){ }
    virtual ~CellularAutomata_Classic() = default;
protected:
    virtual void applyLocalRule() override { ///< overrides pure virtual method
        Cell* cell;
        // apply the same local rule to all cells at the same time
        for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
            cell = lattice->getCell(cellNumber);
            localRule->applyRule(cell);
        }
        // alfter changing the nextState of all cells, update them (currentState = nextState)
        for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
            cell = lattice->getCell(cellNumber);
            cell->updateState();
        }
    }
};
