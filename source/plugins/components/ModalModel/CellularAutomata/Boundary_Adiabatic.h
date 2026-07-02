/*
 * File:   Boundary_Adiabatic.h
 *
 * Boundary condition that clamps out-of-bounds coordinates to the nearest
 * lattice border cell.
 */

#pragma once

#include "plugins/components/ModalModel/CellularAutomata/BoundaryCondition.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

class Boundary_Adiabatic : public BoundaryCondition {
public:
	Boundary_Adiabatic(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr)
		: BoundaryCondition(lattice, neighborhood) {
	}

	Boundary_Adiabatic(const Boundary_Adiabatic& orig)
		: BoundaryCondition(orig) {
	}

	virtual ~Boundary_Adiabatic() = default;

public:
	virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition, std::vector<int> neighborNDimPosition) override {
		long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
		if (neighborCellNumber < 0) {
			for (unsigned short dim = 0; dim < neighborNDimPosition.size(); ++dim) {
				int maxPos = static_cast<int>(lattice->getDimension(dim)) - 1;
				if (neighborNDimPosition.at(dim) < 0)
					neighborNDimPosition.at(dim) = 0;
				else if (neighborNDimPosition.at(dim) > maxPos)
					neighborNDimPosition.at(dim) = maxPos;
			}
			neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
		}
		return lattice->getCell(neighborCellNumber);
	}
};
