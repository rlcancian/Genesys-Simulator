/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   Boundary_Fixed.h
 * Author: cancian
 *
 * Created on 6 de abril de 2023, 13:48
 */
#pragma once

#include "BoundaryCondition.h"
#include "Cell.h"
#include "Lattice.h"
#include "State.h"

class Boundary_Fixed : public BoundaryCondition {
public:

    Boundary_Fixed(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr)
    : BoundaryCondition(lattice, neighborhood) {
        fixedCell = new Cell();
        fixedCell->setCellNumber(-99);
    }

    Boundary_Fixed(const Boundary_Fixed& orig)
    : BoundaryCondition(orig) {
    }
    virtual ~Boundary_Fixed() = default;
public:

    virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition, std::vector<int> neighborNDimPosition) override {
        long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        if (neighborCellNumber < 0) { // out of bounds           
            //std::cout << " bond fixed to fixedCell";
            return fixedCell;
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
