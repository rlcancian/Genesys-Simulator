#pragma once

#include <string>


class Cell;
#include "BoundaryCondition.h"
#include "CellularAutomataBase.h"

//! Defines the complete neighborhood of a cell
class Neighborhood {
public:
	Neighborhood(CellularAutomataBase* parentCellularAutomata, unsigned short radius=1, BoundaryCondition* boundary= nullptr, bool includeCellItself=false){
		this->parentCellularAutomata = parentCellularAutomata;
		parentCellularAutomata->setNeighborhood(this);
		this->radius = radius;
		this->boundary = boundary;
		this->includeCellItself = includeCellItself;
	}
	Neighborhood(const Neighborhood& orig){}
    virtual ~Neighborhood()=default;
public:
	virtual std::string show(){
		return "-";
	}
    virtual std::vector<Cell*> getNeighbors(Cell* cell) = 0; ///< pure virtual method that has to be overriden by derived class, responsible for defining the neighborhood itself.
public:
	void setRadius(unsigned short radius) {
		this->radius = radius;
	}
	unsigned short getRadius() const{
		return radius;
	}
	void setBoundary(BoundaryCondition* boundary) {
		this->boundary = boundary;
		boundary->setNeighborhood(this);
	}
	BoundaryCondition* getBoundary() const{
		return boundary;
	}
	void setIncludeCellItself(bool includeCellItself){
		this->includeCellItself = includeCellItself;
	}
	bool getIncludeCellItself() const{
		return includeCellItself;
	}
public:
protected:
	Cell* getNeighborCell(std::vector<int> cellPosition, std::vector<std::pair<unsigned short, int>> dimensionChanges){
		std::vector<int> neighborPosition;
		neighborPosition = cellPosition;
		for (std::pair<unsigned short, int> change : dimensionChanges) {
			neighborPosition.at(change.first) += change.second;
		}
		//std::cout << "Neig pos from "<<Util::showNDimPosition(cellPosition)<< " is " <<Util::showNDimPosition(&neighborPosition);
		Cell* neighCell = boundary->getNeighborCell(cellPosition, neighborPosition);
		//std::cout << std::endl;
		return neighCell;
	}
protected:
    std::string name;
    CellularAutomataBase* parentCellularAutomata;
    BoundaryCondition* boundary;
    unsigned short radius;
    bool includeCellItself;
private:
};
