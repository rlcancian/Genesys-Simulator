#pragma once

#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include <functional>
#include <unordered_map>
#include <vector>

//! Non-uniform CA: each cell may have its own local rule, its own neighborhood, or both.
//!
//! - Cells without a specific rule fall back to the CA's global localRule.
//! - Cells without a specific neighborhood use the global neighborhood (computed at init time).
//!
//! Per-cell neighborhoods must be created with registerWithCA=false to avoid
//! overwriting the CA's global neighborhood pointer.
class CellularAutomata_NonUniform : public CellularAutomataBase {
public:
    CellularAutomata_NonUniform(Lattice* lattice = nullptr, StateSet* stateSet = nullptr,
                                Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
        : CellularAutomataBase(lattice, stateSet, neighborhood, localRule) {}
    CellularAutomata_NonUniform(const CellularAutomata_NonUniform& orig)
        : CellularAutomataBase(orig) {}
    virtual ~CellularAutomata_NonUniform() = default;

public:
    // --- Per-cell rule API ---

    void setCellRule(long cellNumber, LocalRule* rule) {
        _cellLocalRules[cellNumber] = rule;
    }

    void setCellRule(std::vector<int> position, LocalRule* rule) {
        if (lattice != nullptr) {
            long cellNumber = lattice->cellNDimPosition2Number(position);
            if (cellNumber >= 0)
                _cellLocalRules[cellNumber] = rule;
        }
    }

    const std::unordered_map<long, LocalRule*>& getCellRules() const {
        return _cellLocalRules;
    }

    // --- Per-cell neighborhood API ---

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

    // --- Region API ---
    //! Assign the same rule to every cell inside the rectangular region
    //! [posMin, posMax] (inclusive on all dimensions).
    //! Requires the lattice to be attached before calling.
    void setRegionRule(std::vector<int> posMin, std::vector<int> posMax, LocalRule* rule) {
        if (lattice == nullptr) return;
        std::vector<int> current(posMin.size());
        _iterateRegion(posMin, posMax, current, 0, [this, rule](std::vector<int>& pos) {
            long cellNumber = lattice->cellNDimPosition2Number(pos);
            if (cellNumber >= 0)
                _cellLocalRules[cellNumber] = rule;
        });
    }

    //! Assign the same neighborhood to every cell inside the rectangular region
    //! [posMin, posMax] (inclusive on all dimensions).
    //! Requires the lattice to be attached before calling.
    void setRegionNeighborhood(std::vector<int> posMin, std::vector<int> posMax, Neighborhood* hood) {
        if (lattice == nullptr) return;
        std::vector<int> current(posMin.size());
        _iterateRegion(posMin, posMax, current, 0, [this, hood](std::vector<int>& pos) {
            long cellNumber = lattice->cellNDimPosition2Number(pos);
            if (cellNumber >= 0)
                _cellNeighborhoods[cellNumber] = hood;
        });
    }

    // --- Lifecycle ---

    virtual bool init() override {
        // Step 1: standard init — creates all cells and assigns neighbors using the global neighborhood
        bool res = CellularAutomataBase::init();
        // Step 2: override neighbors for cells that have a per-cell neighborhood
        for (auto& [cellNumber, hood] : _cellNeighborhoods) {
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
    virtual void applyLocalRule() override {
        // Phase 1: compute next state for each cell using its specific rule or the global fallback
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            Cell* cell = lattice->getCell(cellNumber);
            auto it = _cellLocalRules.find(cellNumber);
            LocalRule* rule = (it != _cellLocalRules.end()) ? it->second : localRule;
            if (rule != nullptr)
                rule->applyRule(cell);
        }
        // Phase 2: commit next state to current state (synchronous update)
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            lattice->getCell(cellNumber)->updateState();
        }
    }

private:
    std::unordered_map<long, LocalRule*>    _cellLocalRules;
    std::unordered_map<long, Neighborhood*> _cellNeighborhoods;

    // Recursively visits every integer position in [posMin, posMax] (inclusive).
    void _iterateRegion(const std::vector<int>& posMin, const std::vector<int>& posMax,
                        std::vector<int>& current, int dim,
                        const std::function<void(std::vector<int>&)>& callback) {
        if (dim == static_cast<int>(posMin.size())) {
            callback(current);
            return;
        }
        for (int coord = posMin[dim]; coord <= posMax[dim]; ++coord) {
            current[dim] = coord;
            _iterateRegion(posMin, posMax, current, dim + 1, callback);
        }
    }
};
