#pragma once

#include <algorithm>
#include "plugins/components/ModalModel/CellularAutomata/BoundaryCondition.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

//! Adiabatic (absorbing / no-flux) boundary condition.
//!
//! When a neighbor position falls outside the lattice, the out-of-bounds
//! coordinate is clamped to the nearest valid index.  This makes border cells
//! "see themselves" when looking outward, so no state flows across the edge —
//! analogous to an insulated wall in thermodynamics.
//!
//! Example for a 1-D domain [0, 4]:
//!   cell 0, neighbor at -1  →  clamped to 0  (cell 0 itself)
//!   cell 4, neighbor at  5  →  clamped to 4  (cell 4 itself)
class Boundary_Adiabatic : public BoundaryCondition {
public:
    Boundary_Adiabatic(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr)
        : BoundaryCondition(lattice, neighborhood) {}

    Boundary_Adiabatic(const Boundary_Adiabatic& orig)
        : BoundaryCondition(orig) {}

    virtual ~Boundary_Adiabatic() = default;

public:
    virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition,
                                   std::vector<int> neighborNDimPosition) override {
        long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        if (neighborCellNumber >= 0)
            return lattice->getCell(neighborCellNumber);

        // Clamp each out-of-bounds coordinate to [0, maxPos-1]
        for (unsigned short dim = 0; dim < static_cast<unsigned short>(neighborNDimPosition.size()); ++dim) {
            int maxPos = static_cast<int>(lattice->getDimension(dim));
            neighborNDimPosition[dim] = std::clamp(neighborNDimPosition[dim], 0, maxPos - 1);
        }

        neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        return lattice->getCell(neighborCellNumber);
    }
};
