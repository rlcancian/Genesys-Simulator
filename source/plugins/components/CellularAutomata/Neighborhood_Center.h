#pragma once

#include <string>
#include <vector>

#include "Neighborhood.h"
#include "Cell.h"
#include "CellularAutomataBase.h"

class Neighborhood_Center: public Neighborhood {
public:
    Neighborhood_Center(CellularAutomataBase* parentCellularAutomata, unsigned short radius=1, BoundaryCondition* boundary= nullptr, bool includeCellItself=false)
    :Neighborhood(parentCellularAutomata, radius, boundary, includeCellItself) {
        this->name = "Center";
    }
   
    Neighborhood_Center(const Neighborhood_Center& orig)
    : Neighborhood(orig) {   }
    virtual ~Neighborhood_Center()=default;
public:
    virtual std::vector<Cell*> getNeighbors(Cell* cell) override {
        std::vector<Cell*> neighbors;
		//std::string m = "Center::getNeighbors(CellPos=" + Util::showNDimPosition(cell->getPosition())+")=";
        //unsigned short numDimensions = parentCellularAutomata->getLattice()->getNumDimensions();
		//std::vector<int>* cellPosition = cell->getPosition();
        for (int r = 1; r <= radius; r++) {
            int d = 0;
            //for (int d = 0; r < numDimensions; d++) {
				neighbors.emplace_back(getNeighborCell(cell->getPosition(), {{d, -r}}));
				neighbors.emplace_back(getNeighborCell(cell->getPosition(), {{d, r}}));
            //}
        }
        if (includeCellItself)
            neighbors.emplace_back(cell);
        return neighbors;        
    }
protected:
private:
};
