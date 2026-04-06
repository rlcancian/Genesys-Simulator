/* 
 * File:   Neighborhood_Moore.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 14:27
 */

#pragma once

#include "Lattice.h"
#include "Neighborhood.h"
#include "Cell.h"

class Neighborhood_Moore : public Neighborhood {
public:
    Neighborhood_Moore(CellularAutomataBase* parentCellularAutomata, unsigned short radius = 1, BoundaryCondition* boundary = nullptr)
        :Neighborhood(parentCellularAutomata, radius, boundary) { this->name = "Moore"; }
    Neighborhood_Moore(const Neighborhood_Moore& orig):Neighborhood(orig) {    }
    virtual ~Neighborhood_Moore() = default;
public:
    virtual std::string show() override {
        return "todo";
    }
    virtual std::vector<Cell*> getNeighbors(Cell* cell)override {
        std::vector<Cell*> neighbors;
        unsigned short numDimensions = parentCellularAutomata->getLattice()->getNumDimensions();
		std::vector<int> cellPosition = cell->getPosition();
        // TODO:  Redo for n-dims.
        switch (numDimensions) {
          case 1:
                for (int r = 1; r <= radius; r++) {
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0, -r}}));
                }
                break;
            case 2:
                for (int r = 1; r <= radius; r++) {
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0, r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{1, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{1, r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0,-r},{1, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0,-r},{1, r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0,r},{1, -r}}));
                    neighbors.emplace_back(getNeighborCell(cellPosition, {{0,r},{1, r}}));
                    // Moore with radius > 1 is not working (extra loops nedded)
                }
                break;
            case 3:
                break;
            default:
                break;
        }
        if (includeCellItself)
            neighbors.emplace_back(cell);
        std::cout << "Neighs from " << cell->show() <<" are: ";
        for (Cell* n: neighbors) {
            std::cout << n->show() << ", ";
        }
        std::cout << std::endl;
        return neighbors;
    }
};
