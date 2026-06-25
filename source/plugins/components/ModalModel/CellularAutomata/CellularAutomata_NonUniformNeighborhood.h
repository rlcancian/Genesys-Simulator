#pragma once

#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include <unordered_map>
#include <vector>

//! Non-uniform CA where each cell may have its own neighborhood.
//! Cells without a specific neighborhood use the CA's global neighborhood (set at init time).
//! Per-cell neighborhoods must be created with registerWithCA=false to avoid
//! overwriting the CA's global neighborhood pointer.
class CellularAutomata_NonUniformNeighborhood : public CellularAutomataBase {
public:
    CellularAutomata_NonUniformNeighborhood(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
        : CellularAutomataBase(lattice, stateSet, neighborhood, localRule) {}
    CellularAutomata_NonUniformNeighborhood(const CellularAutomata_NonUniformNeighborhood& orig)
        : CellularAutomataBase(orig) {}
    virtual ~CellularAutomata_NonUniformNeighborhood() = default;

public:
    void setCellNeighborhood(long cellNumber, Neighborhood* hood) {
        _cellNeighborhoods[cellNumber] = hood;
    }

    void setCellNeighborhood(std::vector<int> position, Neighborhood* hood) {
        if (lattice != nullptr) {
            long cellNumber = lattice->cellNDimPosition2Number(position);
            if (cellNumber >= 0)
                _cellNeighborhoods[cellNumber] = hood;
        }
    }

    const std::unordered_map<long, Neighborhood*>& getCellNeighborhoods() const {
        return _cellNeighborhoods;
    }

    virtual bool init() override {
        // Step 1: normal init — creates all cells and computes neighbors using the global neighborhood
        bool res = CellularAutomataBase::init();
        // Step 2: override neighbors for cells that have a per-cell neighborhood
        for (auto& entry : _cellNeighborhoods) {
            long cellNumber = entry.first;
            Neighborhood* hood = entry.second;
            if (hood == nullptr)
                continue;
            if (hood->getBoundary() != nullptr)
                hood->getBoundary()->setLattice(lattice);
            Cell* cell = lattice->getCell(cellNumber);
            if (cell != nullptr)
                cell->setNeighbors(hood->getNeighbors(cell));
        }
        return res;
    }

protected:
    // The non-uniformity is already baked into cell->getNeighbors() at init time,
    // so applyLocalRule uses the same uniform rule on all cells.
    virtual void applyLocalRule() override {
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            Cell* cell = lattice->getCell(cellNumber);
            if (localRule != nullptr)
                localRule->applyRule(cell);
        }
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            lattice->getCell(cellNumber)->updateState();
        }
    }

private:
    std::unordered_map<long, Neighborhood*> _cellNeighborhoods;
};
