#include "Lattice.h"
#include "CellularAutomataBase.h"
#include "State.h"
#include "StateSet.h"
#include "Neighborhood.h"

#include <random>
#include <cassert>
#include <iostream>

/* **************
 *  PUBLIC
 * **************/

Lattice::Lattice(CellularAutomataBase* parentCellularAutomata, Cell *progenitorCell, std::vector<unsigned short> dimensions, LatticeType latticeType) {
	this->parentCellularAutomata = parentCellularAutomata;
	parentCellularAutomata->setLattice(this);
	this->progenitorCell = progenitorCell;
	this->dimensions = dimensions;
	this->latticeType = latticeType;
	hasBeenInit = false;
}

Lattice::Lattice(const Lattice& orig) {

}

/* **************
 *  PUBLIC
 * **************/

std::string Lattice::show() {
	std::string m = "";
	short lastDim = 0;
	Cell *cell;
	for (long cellNumber = 0; cellNumber < cells.size(); cellNumber++) {//(unsigned int dimension: *dimensions) {
		//m += "[" + cells.at(cellNumber)->show() + "], ";
		cell = cells.at(cellNumber);
		if (cell->getPosition().at(dimensions.size() - 1) != lastDim) {
			lastDim = cell->getPosition().at(dimensions.size() - 1);
			m += "\n";
		}
		m += (std::to_string(cell->getCurrentState().getValue())[0] );
	}
	return m + "\n";
}

bool Lattice::init() {
	totalCells = 1;
	for (unsigned int dimension : dimensions) {
		totalCells *= dimension;
	}
	if (cells.size() < totalCells) {
		cells.resize(totalCells);
	}
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> unif(0, 1);
	unsigned short numDimension = 0;
	Cell* newCell;
	for (unsigned long cellNumber = 0; cellNumber < totalCells; cellNumber++) {
		if (cells.at(cellNumber) == nullptr) {
			if (progenitorCell != nullptr) {
				newCell = new Cell(*progenitorCell);
			} else {
				newCell = new Cell();
			}
			newCell->setCellNumber(cellNumber);
			newCell->setPosition(cellNumber2NDimPosition(cellNumber));
			//if (newCell->getCurrentState() == nullptr) {
			//	unsigned int numPossibleStates = parentCellularAutomata->getStateSet()->size(); //newCell->getStateSet()->getStates()->size();
			//	unsigned int choosenState = /*std::trunc*/(unsigned int) (unif(rng) * numPossibleStates);
			//	newCell->setCurrentState(parentCellularAutomata->getStateSet()->getState(choosenState)); //(newCell->getStateSet()->getStates()->at(choosenState));
			//}
			cells.at(cellNumber) = newCell;
		} else {
			newCell = cells.at(cellNumber);
			//std::cout << "Ja existia" << std::endl;
		}
		//        std::cout << "Cellnum:" << newCell->getCellNumber()
		//                << " Pos:" << Util::showNDimPosition(newCell->getPosition())
		//                << " State:" << newCell->getCurrentState()->show()
		//                << std::endl;
	}
	Neighborhood* neighb = parentCellularAutomata->getNeighborhood();
	neighb->getBoundary()->setLattice(this);
	for (Cell* cell : cells) {
		std::vector<Cell*> neighbors = neighb->getNeighbors(cell);
		cell->setNeighbors(neighbors);

		//        std::cout << "Cellnum:" << newCell->getCellNumber()
		//                << " Pos:" << Util::showNDimPosition(newCell->getPosition())
		//                << " State:" << newCell->getCurrentState()->show()
		//                << std::endl;
	}
	hasBeenInit = true;
	return true;
}

void Lattice::setCell(const std::vector<int> position, Cell* cell) {
	long cellNumber = cellNDimPosition2Number(position);
	if (cellNumber >= 0) {
		Cell* oldCell = cells.at(cellNumber);
		cells.at(cellNumber) = cell;
		if (oldCell != nullptr)
			oldCell->~Cell();
	}
}

bool Lattice::setCellState(long cellNumber, State* state, Cell* cell) {
	if (cells.size() <= cellNumber)
		cells.resize(cellNumber + 1);
	Cell* newCell = cell;
	if (newCell == nullptr)
		newCell = cells.at(cellNumber);
	if (newCell == nullptr) {
		if (progenitorCell == nullptr)
			return false;
		else
			newCell = new Cell(*progenitorCell);
	}
	newCell->setCellNumber(cellNumber);
	std::vector<int> position = cellNumber2NDimPosition(cellNumber);
	newCell->setPosition(position);
	if (state != nullptr)
		newCell->setCurrentState(*state);
	cells.at(cellNumber) = newCell;
	return true;
}

bool Lattice::setCellState(std::vector<int> position, State* state, Cell* cell) {
	long cellNumber = cellNDimPosition2Number(position);
	return setCellState(cellNumber, state, cell);
}

long Lattice::cellNDimPosition2Number(const std::vector<int> position) {
	long cellNum = 0;
	unsigned int dim = 0;
	unsigned int mult = 1;
	int pos;
	assert(position.size() == dimensions.size());
	//std::cout << "<";
	for (unsigned int dimension : dimensions) {
		pos = position.at(dim);
		if (pos < 0 || pos >= dimension) {
			//std::cout << pos << "> invalid" << std::endl;
			return -1; // invalid
		}
		//std::cout << pos << ",";
		cellNum += pos * mult;
		mult *= dimension;
		dim++;
	}
	//std::cout << "> maps to " << cellNum << " maps back to " << Util::showNDimPosition(cellNumber2NDimPosition(cellNum)) << std::endl;
	return cellNum;
}

std::vector<int> Lattice::cellNumber2NDimPosition(const long cellNumber) {
	std::vector<int> cellPosition;
	cellPosition.resize(dimensions.size());
	unsigned int dim = 0;
	unsigned int mult = 1;
	int pos;
	//std::cout << "CellNumber " << cellNumber << " maps to <";
	for (unsigned int dimension : dimensions) {
		pos = (cellNumber / mult) % dimension;
		cellPosition.at(dim++) = pos;
		mult *= dimension;
		//std::cout << pos << ",";
	}
	//std::cout << ">" << " and maps back to " << cellNDimPosition2Number(*cellPosition) << std::endl;
	return cellPosition;
}

Cell* Lattice::getCell(const long cellNumber) {
	return cells.at(cellNumber);
}

Cell* Lattice::getCell(const std::vector<int> position) {
	unsigned int cellNumber = this->cellNDimPosition2Number(position);
	return getCell(cellNumber);
}

/* **************
 *  PUBLIC
 * **************/


void Lattice::setProgenitorCell(Cell* progenitorCell) {
	this->progenitorCell = progenitorCell;
}

Cell* Lattice::getProgenitorCell() const {
	return progenitorCell;
}

unsigned short Lattice::getNumDimensions() {
	return dimensions.size();
}

unsigned int Lattice::getNumCell(unsigned short dimension) {
	return dimensions.at(dimension);
}

//void Lattice::setLatticeType(LatticeType latticeType) { this->latticeType = latticeType; }

LatticeType Lattice::getLatticeType() const {
	return latticeType;
}

unsigned long Lattice::getCellsSize() const {
	return cells.size();
}

unsigned long Lattice::calculateTotalCells() {
	totalCells = 1;
	for (unsigned int dimension : dimensions) {
		totalCells *= dimension;
	}
	if (cells.size() < totalCells) {
		cells.resize(totalCells);
	}
	return totalCells;
}

unsigned long Lattice::getTotalCells() const {
	return totalCells;
}

bool Lattice::isInit() const {
	return hasBeenInit;
}

void Lattice::setDimensions(std::vector<unsigned short> dimensions) {
	this->dimensions = dimensions;
}

std::vector<unsigned short> Lattice::getDimensions() const {
	return dimensions;
}

unsigned short Lattice::getDimension(unsigned short dimensionNumber) const {
	return dimensions.at(dimensionNumber);
}

std::vector<Cell*> Lattice::getCells() const {
	return cells;
}
//std::vector<Cell*>* Lattice::getCells() const {
//    return cells;
//}

bool Lattice::setNumCells(unsigned short dimension, unsigned int numCells, bool createDimensions) {
	if (dimensions.size() <= dimension) {
		if (createDimensions) {
			dimensions.resize(dimension + 1);
			// TODO: Complete all dimensions with 0 elements
		} else {
			return false;
		}
	}
	dimensions.at(dimension) = numCells;
	return true;
}
