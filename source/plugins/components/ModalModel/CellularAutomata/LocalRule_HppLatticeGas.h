#pragma once

#include <iostream>
#include <vector>

#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

class LocalRule_HppLatticeGas : public LocalRule {
public:
	// Simplified HPP lattice gas: particles stream on the four von Neumann
	// directions and head-on two-particle collisions rotate by 90 degrees.
	// Conceptual reference: local PDF "2018_Book_Adamatzky A. Cellular Automata...Systems Science 2ed 2018.pdf",
	// physical systems / lattice-gas cellular automata discussion.
	explicit LocalRule_HppLatticeGas(CellularAutomataBase* parentCellularAutomata)
		: LocalRule(parentCellularAutomata) {
	}

	void applyRule(Cell* cell) override {
		if (cell == nullptr || parentCellularAutomata == nullptr || parentCellularAutomata->getLattice() == nullptr) {
			return;
		}
		if (stateSet != nullptr && dynamic_cast<HppLatticeGasStateSet*>(stateSet) == nullptr) {
			std::cerr << "LocalRule_HppLatticeGas requires HppLatticeGasStateSet." << std::endl;
			cell->setNextState(cell->getCurrentState());
			return;
		}

		Lattice* lattice = parentCellularAutomata->getLattice();
		const std::vector<int> position = cell->getPosition();
		if (position.size() < 2 || lattice->getNumDimensions() < 2) {
			cell->setNextState(cell->getCurrentState());
			return;
		}
		// sem tempo para acessar por Von Neuman
		Cell* northCell = lattice->getCell(wrappedPosition({position[0], position[1] - 1}, lattice));
		Cell* eastCell = lattice->getCell(wrappedPosition({position[0] + 1, position[1]}, lattice));
		Cell* southCell = lattice->getCell(wrappedPosition({position[0], position[1] + 1}, lattice));
		Cell* westCell = lattice->getCell(wrappedPosition({position[0] - 1, position[1]}, lattice));

		HppLatticeGasState incoming;
		incoming.set(HppLatticeGasState::North, hasDirection(southCell, HppLatticeGasState::North));
		incoming.set(HppLatticeGasState::East, hasDirection(westCell, HppLatticeGasState::East));
		incoming.set(HppLatticeGasState::South, hasDirection(northCell, HppLatticeGasState::South));
		incoming.set(HppLatticeGasState::West, hasDirection(eastCell, HppLatticeGasState::West));

		const bool verticalCollision = incoming.has(HppLatticeGasState::North) &&
		                               incoming.has(HppLatticeGasState::South) &&
		                               !incoming.has(HppLatticeGasState::East) &&
		                               !incoming.has(HppLatticeGasState::West);
		const bool horizontalCollision = incoming.has(HppLatticeGasState::East) &&
		                                 incoming.has(HppLatticeGasState::West) &&
		                                 !incoming.has(HppLatticeGasState::North) &&
		                                 !incoming.has(HppLatticeGasState::South);

		if (verticalCollision) {
			cell->setNextState(HppLatticeGasState({{false, true, false, true}}));
		} else if (horizontalCollision) {
			cell->setNextState(HppLatticeGasState({{true, false, true, false}}));
		} else {
			cell->setNextState(incoming);
		}
	}

private:
	static std::vector<int> wrappedPosition(std::vector<int> position, Lattice* lattice) {
		for (unsigned short dimension = 0; dimension < position.size(); ++dimension) {
			const int dimensionSize = static_cast<int>(lattice->getDimension(dimension));
			position[dimension] = ((position[dimension] % dimensionSize) + dimensionSize) % dimensionSize;
		}
		return position;
	}

	static bool hasDirection(Cell* cell, HppLatticeGasState::Direction direction) {
		if (cell == nullptr) {
			return false;
		}
		const auto* hpp = cell->getCurrentState().as<HppLatticeGasState>();
		if (hpp != nullptr) {
			return hpp->has(direction);
		}
		return (cell->getCurrentState().getValue() & (1L << direction)) != 0;
	}
};
