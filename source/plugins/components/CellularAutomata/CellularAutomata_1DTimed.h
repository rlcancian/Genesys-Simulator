#pragma once

#include "Cell.h"
#include "CellularAutomataBase.h"
#include "Lattice.h"
#include "LocalRule.h"
#include "State.h"

//! Complete uniform CA specific for unidimensional lattices where 2nd dimension is time (usefull for elementary CA)
class CellularAutomata_1DTimed: public CellularAutomataBase {
public:
	CellularAutomata_1DTimed(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
	:CellularAutomataBase(lattice, stateSet, neighborhood, localRule) { }
    CellularAutomata_1DTimed(const CellularAutomata_1DTimed& orig)
	: CellularAutomataBase(orig){ }
    virtual ~CellularAutomata_1DTimed() = default;
protected:
    virtual void applyLocalRule() override {
        Cell* cell;
		State* state;
        int nextRow;
		int maxRows = std::min<int>(lattice->getDimension(1)-2, lattice->getDimension(1)-2); // simulatedTime);
        for (int row=maxRows; row>=0; row--) {
            nextRow = row + 1;
            // move cell states to the next row 
			for (int col=0; col<lattice->getDimension(0); col++) { // move to row bellow
				cell = lattice->getCell({col,row});
				*state = cell->getCurrentState();
				lattice->setCellState({col,nextRow}, state);
            }
            if (row==0) {
                // apply the same local rule for all unidimensional cells at the same time
                for (int col=0; col<lattice->getDimension(0); col++) {
                    cell = lattice->getCell({col,row});
                    localRule->applyRule(cell);
                }
            }
        }
        // alfter changing the nextState of all unidimensional cells, update them (currentState = nextState)
        for (int col=0; col<lattice->getDimension(0); col++) {
            cell = lattice->getCell({col,0});
            cell->updateState();
        }
    }
protected:
};
