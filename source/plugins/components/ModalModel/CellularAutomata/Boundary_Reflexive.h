/*
 * File:   Boundary_Reflexive.h
 *
 * Boundary condition that mirrors out-of-bounds coordinates back into the
 * lattice.
 */

#pragma once

#include "plugins/components/ModalModel/CellularAutomata/BoundaryCondition.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

class Boundary_Reflexive : public BoundaryCondition {
public:
	Boundary_Reflexive(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr)
		: BoundaryCondition(lattice, neighborhood) {
	}

	Boundary_Reflexive(const Boundary_Reflexive& orig)
		: BoundaryCondition(orig) {
	}

	virtual ~Boundary_Reflexive() = default;

public:
	virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition, std::vector<int> neighborNDimPosition) override {
		long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
		if (neighborCellNumber < 0) {
			for (unsigned short dim = 0; dim < neighborNDimPosition.size(); ++dim) {
				const int dimensionSize = static_cast<int>(lattice->getDimension(dim));
				neighborNDimPosition.at(dim) = _reflectPosition(neighborNDimPosition.at(dim), dimensionSize);
			}
			neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
		}
		return lattice->getCell(neighborCellNumber);
	}

private:
	int _reflectPosition(int position, int dimensionSize) const {
		if (dimensionSize <= 1)
			return 0;

		const int period = 2 * (dimensionSize - 1);
		position %= period;
		if (position < 0)
			position += period;
		if (position >= dimensionSize)
			position = period - position;
		return position;
	}
};
