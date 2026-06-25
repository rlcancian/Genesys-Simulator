#pragma once

#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include <unordered_map>
#include <vector>

//! Non-uniform CA where each cell may have its own local rule.
//! Cells without a specific rule fall back to the CA's global localRule.
class CellularAutomata_NonUniformRule : public CellularAutomataBase {
public:
    CellularAutomata_NonUniformRule(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr)
        : CellularAutomataBase(lattice, stateSet, neighborhood, localRule) {}
    CellularAutomata_NonUniformRule(const CellularAutomata_NonUniformRule& orig)
        : CellularAutomataBase(orig) {}
    virtual ~CellularAutomata_NonUniformRule() = default;

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
