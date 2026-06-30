#pragma once

#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"

#include <vector>

class Neighborhood_Network : public Neighborhood {
public:
	Neighborhood_Network(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
		: Neighborhood(parentCellularAutomata, radius, boundary) {
		this->name = "Network";
	}
	Neighborhood_Network(const Neighborhood_Network& orig) : Neighborhood(orig) {}
	virtual ~Neighborhood_Network() = default;

public:
	virtual std::string show() override {
		return "network";
	}

	virtual std::vector<Cell*> getNeighbors(Cell* cell) override {
		std::vector<Cell*> neighbors = parentCellularAutomata->getLattice()->getNetworkNeighbors(static_cast<unsigned long>(cell->getCellNumber()));
		if (includeCellItself)
			neighbors.emplace_back(cell);
		return neighbors;
	}
};
