/* 
 * File:   Neighborhood_VonNeumann.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 14:27
 */

#pragma once

#include <string>
#include <vector>
#include <cstdlib>

#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

class Neighborhood_VonNeumann : public Neighborhood {
public:

    Neighborhood_VonNeumann(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr, bool includeCellItself = false, bool registerWithCA = true)
    :Neighborhood(parentCellularAutomata, radius, boundary, includeCellItself, registerWithCA) {
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

        std::vector<int> offset(numDimensions, 0);
        collectNeighbors(cellPosition, offset, 0, neighbors);

        if (includeCellItself)
            neighbors.emplace_back(cell);
        return neighbors;
    }
private:
    void collectNeighbors(const std::vector<int>& cellPosition, std::vector<int>& offset, unsigned short dimension, std::vector<Cell*>& neighbors) {
        if (dimension == offset.size()) {
            bool isZeroOffset = true;
            unsigned int manhattanDistance = 0;
            std::vector<std::pair<unsigned short, int>> dimensionChanges;
            for (unsigned short dim = 0; dim < offset.size(); ++dim) {
                int delta = offset.at(dim);
                manhattanDistance += static_cast<unsigned int>(std::abs(delta));
                if (delta != 0) {
                    isZeroOffset = false;
                    dimensionChanges.emplace_back(dim, delta);
                }
            }
            if (!isZeroOffset && manhattanDistance <= radius) {
                // Offsets are visited from dimension 0 to n-1.
                neighbors.emplace_back(getNeighborCell(cellPosition, dimensionChanges));
            }
            return;
        }

        for (int delta = -static_cast<int>(radius); delta <= static_cast<int>(radius); ++delta) {
            offset.at(dimension) = delta;
            collectNeighbors(cellPosition, offset, dimension + 1, neighbors);
        }
    }
};
