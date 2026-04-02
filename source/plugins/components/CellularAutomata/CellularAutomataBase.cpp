#include "CellularAutomataBase.h"
#include "Lattice.h"
#include "LocalRule.h"

/* **************
 *  PUBLIC
 * **************/

CellularAutomataBase::CellularAutomataBase(Lattice* lattice, StateSet* stateSet, Neighborhood* neighborhood, LocalRule* localRule) {
	this->lattice = lattice;
	this->stateSet = stateSet;
	this->neighborhood = neighborhood;
	this->localRule = localRule;
}

CellularAutomataBase::CellularAutomataBase(const CellularAutomataBase& orig) {

}

/* **************
 *  PUBLIC
 * **************/


bool CellularAutomataBase::init() {
	bool res = lattice->init();
	return res;
}

void CellularAutomataBase::step() {
	applyLocalRule();
}

/* **************
 *  PUBLIC
 * **************/

void CellularAutomataBase::setLocalRule(LocalRule* localRule) {
	this->localRule = localRule;
}

LocalRule* CellularAutomataBase::getLocalRule() const {
	return localRule;
}

void CellularAutomataBase::setNeighborhood(Neighborhood* neighborhood) {
	this->neighborhood = neighborhood;
}

Neighborhood* CellularAutomataBase::getNeighborhood() const {
	return neighborhood;
}

void CellularAutomataBase::setStateSet(StateSet* stateSet) {
	this->stateSet = stateSet;
}

StateSet* CellularAutomataBase::getStateSet() const {
	return stateSet;
}

void CellularAutomataBase::setLattice(Lattice* lattice) {
	this->lattice = lattice;
}

Lattice* CellularAutomataBase::getLattice() const {
	return lattice;
}

/* **************
 *  PROTECTED
 * **************/


void CellularAutomataBase::applyLocalRule() {
	Cell* cell;
	for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
		cell = lattice->getCell(cellNumber);
		//std::vector<Cell*> neighbors = cell->getNeighbors();
		localRule->applyRule(cell); //, neighbors);
	}
	for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
		cell = lattice->getCell(cellNumber);
		cell->updateState();
	}
}
