#include "UpdatePolicy.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"

#include <algorithm>
#include <random>
#include <vector>

// ============================================================================
// SYNCHRONOUS UPDATE
// ============================================================================

void SynchronousUpdate::execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) {
    if (lattice == nullptr || rule == nullptr) {
        return;
    }

    unsigned long totalCells = lattice->getCellsSize();

    // Fase 1: Calcular próximos estados e armazenar em buffer
    std::vector<State> nextStates;
    nextStates.reserve(totalCells);

    for (unsigned long i = 0; i < totalCells; ++i) {
        Cell* cell = lattice->getCell(static_cast<long>(i));
        if (cell == nullptr) {
            nextStates.push_back(State(0));
            continue;
        }

        // Aplica regra: calcula nextState
        rule->applyRule(cell);
        nextStates.push_back(cell->getNextState());
    }

    // Fase 2: Aplicar todos os próximos estados simultaneamente
    for (unsigned long i = 0; i < totalCells; ++i) {
        Cell* cell = lattice->getCell(static_cast<long>(i));
        if (cell != nullptr) {
            cell->setCurrentState(nextStates[i]);
        }
    }
}

// ============================================================================
// RANDOM ASYNCHRONOUS UPDATE
// ============================================================================

void RandomAsynchronousUpdate::execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) {
    if (lattice == nullptr || rule == nullptr) {
        return;
    }

    unsigned long totalCells = lattice->getCellsSize();

    // Cria lista de índices e embaralha
    std::vector<unsigned long> indices(totalCells);
    for (unsigned long i = 0; i < totalCells; ++i) {
        indices[i] = i;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // Atualiza células em ordem aleatória (in-place)
    for (unsigned long idx : indices) {
        Cell* cell = lattice->getCell(static_cast<long>(idx));
        if (cell == nullptr) {
            continue;
        }

        // Aplica regra e atualiza imediatamente
        rule->applyRule(cell);
        cell->setCurrentState(cell->getNextState());
    }
}

// ============================================================================
// SEQUENTIAL UPDATE
// ============================================================================

void SequentialUpdate::execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) {
    if (lattice == nullptr || rule == nullptr) {
        return;
    }

    unsigned long totalCells = lattice->getCellsSize();

    // Atualiza células em ordem sequencial (in-place)
    for (unsigned long i = 0; i < totalCells; ++i) {
        Cell* cell = lattice->getCell(static_cast<long>(i));
        if (cell == nullptr) {
            continue;
        }

        // Aplica regra e atualiza imediatamente
        rule->applyRule(cell);
        cell->setCurrentState(cell->getNextState());
    }
}
