#pragma once

#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformNeighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include <unordered_map>
#include <vector>

//! Fully non-uniform CA: each cell may have both its own local rule and its own neighborhood.
//! Inherits per-cell neighborhood handling (init override) from CellularAutomata_NonUniformNeighborhood
//! and adds per-cell rule dispatch in applyLocalRule().
class CellularAutomata_NonUniform : public CellularAutomata_NonUniformNeighborhood {
public:
    CellularAutomata_NonUniform(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
        : CellularAutomata_NonUniformNeighborhood(lattice, stateSet, neighborhood, localRule) {}
    CellularAutomata_NonUniform(const CellularAutomata_NonUniform& orig)
        : CellularAutomata_NonUniformNeighborhood(orig) {}
    virtual ~CellularAutomata_NonUniform() = default;

public:
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

protected:
    // init() is inherited from CellularAutomata_NonUniformNeighborhood:
    // it sets per-cell neighborhoods after the default lattice init.

    virtual void applyLocalRule() override {
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            Cell* cell = lattice->getCell(cellNumber);
            auto it = _cellLocalRules.find(cellNumber);
            LocalRule* rule = (it != _cellLocalRules.end()) ? it->second : localRule;
            if (rule != nullptr)
                rule->applyRule(cell);
        }
        for (long cellNumber = 0; cellNumber < static_cast<long>(lattice->getCellsSize()); cellNumber++) {
            lattice->getCell(cellNumber)->updateState();
        }
    }

private:
    std::unordered_map<long, LocalRule*> _cellLocalRules;
};
