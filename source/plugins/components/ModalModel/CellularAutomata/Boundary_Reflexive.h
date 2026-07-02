#pragma once

#include "plugins/components/ModalModel/CellularAutomata/BoundaryCondition.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"

//! Mirror (reflective) boundary condition.
//!
//! When a neighbor position falls outside the lattice the coordinate is
//! reflected back as if there were a mirror at each edge.  For a 1-D domain
//! [0, D-1]:
//!
//!   p < 0      →  reflected to -p
//!   p >= D     →  reflected to 2*(D-1) - p
//!
//! The reflection is applied repeatedly until the coordinate lands inside the
//! valid range, so it is correct for any neighborhood radius.
class Boundary_Reflexive : public BoundaryCondition {
public:
    Boundary_Reflexive(Lattice* lattice = nullptr, Neighborhood* neighborhood = nullptr)
        : BoundaryCondition(lattice, neighborhood) {}

    Boundary_Reflexive(const Boundary_Reflexive& orig)
        : BoundaryCondition(orig) {}

    virtual ~Boundary_Reflexive() = default;

public:
    virtual Cell* getNeighborCell(std::vector<int> cellNDimPosition,
                                   std::vector<int> neighborNDimPosition) override {
        long neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        if (neighborCellNumber >= 0)
            return lattice->getCell(neighborCellNumber);

        // Reflect each out-of-bounds coordinate back into [0, maxPos-1]
        for (unsigned short dim = 0; dim < static_cast<unsigned short>(neighborNDimPosition.size()); ++dim) {
            int pos    = neighborNDimPosition[dim];
            int maxPos = static_cast<int>(lattice->getDimension(dim));
            // Iterate reflections until the coordinate is in range
            while (pos < 0 || pos >= maxPos) {
                if (pos < 0)
                    pos = -pos;
                if (pos >= maxPos)
                    pos = 2 * (maxPos - 1) - pos;
            }
            neighborNDimPosition[dim] = pos;
        }

        neighborCellNumber = lattice->cellNDimPosition2Number(neighborNDimPosition);
        return lattice->getCell(neighborCellNumber);
    }
};
