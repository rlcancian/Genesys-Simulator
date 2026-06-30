#pragma once

#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"

#include <vector>

class Neighborhood_Triangular : public Neighborhood {
public:
	Neighborhood_Triangular(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
		: Neighborhood(parentCellularAutomata, radius, boundary) {
		this->name = "Triangular";
	}
	Neighborhood_Triangular(const Neighborhood_Triangular& orig) : Neighborhood(orig) {}
	virtual ~Neighborhood_Triangular() = default;

public:
	virtual std::string show() override {
		return "triangular";
	}

	virtual std::vector<Cell*> getNeighbors(Cell* cell) override {
		std::vector<Cell*> neighbors;
		const std::vector<int> position = cell->getPosition();
		neighbors.emplace_back(getNeighborCell(position, {{0, -1}}));
		neighbors.emplace_back(getNeighborCell(position, {{0, 1}}));
		const int verticalDelta = ((position.at(0) + position.at(1)) % 2 == 0) ? -1 : 1;
		neighbors.emplace_back(getNeighborCell(position, {{1, verticalDelta}}));
		if (includeCellItself)
			neighbors.emplace_back(cell);
		return neighbors;
	}
};
