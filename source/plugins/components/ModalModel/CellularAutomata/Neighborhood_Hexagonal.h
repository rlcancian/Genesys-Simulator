#pragma once

#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"

#include <vector>

class Neighborhood_Hexagonal : public Neighborhood {
public:
	Neighborhood_Hexagonal(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
		: Neighborhood(parentCellularAutomata, radius, boundary) {
		this->name = "Hexagonal";
	}
	Neighborhood_Hexagonal(const Neighborhood_Hexagonal& orig) : Neighborhood(orig) {}
	virtual ~Neighborhood_Hexagonal() = default;

public:
	virtual std::string show() override {
		return "hexagonal";
	}

	virtual std::vector<Cell*> getNeighbors(Cell* cell) override {
		std::vector<Cell*> neighbors;
		const std::vector<int> position = cell->getPosition();
		const bool oddColumn = (position.at(0) % 2) != 0;
		const std::vector<std::pair<unsigned short, int>> leftUp = oddColumn
			? std::vector<std::pair<unsigned short, int>>{{0, -1}, {1, 1}}
			: std::vector<std::pair<unsigned short, int>>{{0, -1}};
		const std::vector<std::pair<unsigned short, int>> rightUp = oddColumn
			? std::vector<std::pair<unsigned short, int>>{{0, 1}, {1, 1}}
			: std::vector<std::pair<unsigned short, int>>{{0, 1}};
		const std::vector<std::pair<unsigned short, int>> leftDown = oddColumn
			? std::vector<std::pair<unsigned short, int>>{{0, -1}}
			: std::vector<std::pair<unsigned short, int>>{{0, -1}, {1, -1}};
		const std::vector<std::pair<unsigned short, int>> rightDown = oddColumn
			? std::vector<std::pair<unsigned short, int>>{{0, 1}}
			: std::vector<std::pair<unsigned short, int>>{{0, 1}, {1, -1}};

		neighbors.emplace_back(getNeighborCell(position, leftUp));
		neighbors.emplace_back(getNeighborCell(position, {{1, 1}}));
		neighbors.emplace_back(getNeighborCell(position, rightUp));
		neighbors.emplace_back(getNeighborCell(position, rightDown));
		neighbors.emplace_back(getNeighborCell(position, {{1, -1}}));
		neighbors.emplace_back(getNeighborCell(position, leftDown));
		if (includeCellItself)
			neighbors.emplace_back(cell);
		return neighbors;
	}
};
