/* 
 * File:   Neighborhood_VonNeumann.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 14:27
 */

#pragma once

#include <string>
#include <vector>

#include "Neighborhood.h"
#include "Lattice.h"

class Neighborhood_VonNeumann : public Neighborhood {
public:

    Neighborhood_VonNeumann(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
    :Neighborhood(parentCellularAutomata, radius, boundary) {
         this->name = "Von Neumann";
    }

    Neighborhood_VonNeumann(const Neighborhood_VonNeumann& orig)
    :Neighborhood(orig) {
    }
    virtual ~Neighborhood_VonNeumann() = default;
public:

    virtual std::string show() override {
        return "-";
    }

    virtual std::vector<Cell*> getNeighbors(Cell* cell)override {
        std::vector<Cell*> neighbors;
        unsigned short numDimensions = parentCellularAutomata->getLattice()->getNumDimensions();
		std::vector<int> cellPosition = cell->getPosition();
        // TODO: Redo for n-dims, not switch for every case
        switch (numDimensions) {
            case 1:
                for (int r = 1; r <= radius; r++) {
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{0, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{0, -r}}));
                }
                break;
            case 2:
                for (int r = 1; r <= radius; r++) {
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{0, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{0, r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{1, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition,{{1, r}}));
                    // TODO Só funcina para r=1
                }
                break;
            case 3:
                break;
            default:
                break;
        }
        if (includeCellItself)
            neighbors.emplace_back(cell);
        return neighbors;
    }
};
