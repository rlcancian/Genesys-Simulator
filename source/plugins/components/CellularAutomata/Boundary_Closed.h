/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   Boundary_Closed.h
 * Author: cancian
 *
 * Created on 10 de abril de 2023, 16:22
 */
#pragma once

#include "BoundaryCondition.h"
#include "Cell.h"
#include "Lattice.h"
#include "State.h"

class Boundary_Closed : public BoundaryCondition {
public:

    Boundary_Closed(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr) 
    : BoundaryCondition(lattice, neighborhood) {
        fixedCell = new Cell();
    }

    Boundary_Closed(const Boundary_Closed& orig)
    : BoundaryCondition(orig) {
    }
    virtual ~Boundary_Closed() = default;
public:
    virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition, std::vector<int> neighborNDimPosition) override {
        long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        if (neighborCellNumber < 0) { // out of bounds
            int pos, maxPos;
            for (unsigned short dim = 0; dim < neighborNDimPosition.size(); dim++) {
                pos = neighborNDimPosition.at(dim);
                maxPos = (int) (lattice->getDimension(dim));
                if (pos < 0 || pos >= maxPos) { // out of bound
                    pos = (2 * maxPos + pos) % maxPos;
                    // position corrected
                    neighborNDimPosition.at(dim) = pos;
                }
            }
            //std::cout << " bond fixed to " <<Util::showNDimPosition(&neighborNDimPosition);
            neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        }
        return lattice->getCell(neighborCellNumber);
    }
public:

	void setFixedState(State fixedState) {
		fixedCell->setCurrentState(fixedState);
    }

	State getFixedState() const {
		return fixedCell->getCurrentState();
    }
private:
    Cell* fixedCell;
};
