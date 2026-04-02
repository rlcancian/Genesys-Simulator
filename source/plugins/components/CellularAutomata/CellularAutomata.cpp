#include "CellularAutomata.h"

/* **************
 *  PUBLIC
 * **************/

CellularAutomata::CellularAutomata(std::string name, Lattice* lattice, StateSet* stateSet, Neighborhood* neighborhood, LocalRule* localRule) {
	this->name = name;
	this->lattice = lattice;
	this->stateSet = stateSet;
	this->neighborhood = neighborhood;
	this->localRule = localRule;
}

CellularAutomata::CellularAutomata(const CellularAutomata& orig) {

}

/* **************
 *  PUBLIC
 * **************/


bool CellularAutomata::init() {
	simulatedTime = 0;
	bool res = lattice->init();
	notifyObservers(CellularAutomataEvent(this));
	return res;
}

void CellularAutomata::step() {
	applyLocalRule();
	simulatedTime++;
	notifyObservers(CellularAutomataEvent(this));
}

void CellularAutomata::start() {
	running = true;
	while (simulatedTime <= simulationLength && running) {
		step();
		for (unsigned int i=0; i<1e3; i++)
			std::this_thread::yield();
	}
	running = false;
}

void CellularAutomata::pause() {
	running = false;
}

/* **************
 *  PUBLIC
 * **************/

std::string CellularAutomata::getName() const {
	return this->name;
}

void CellularAutomata::setName(const std::string name) {
	this->name = name;
}

void CellularAutomata::setLocalRule(LocalRule* localRule) {
	this->localRule = localRule;
}

LocalRule* CellularAutomata::getLocalRule() const {
	return localRule;
}

void CellularAutomata::setNeighborhood(Neighborhood* neighborhood) {
	this->neighborhood = neighborhood;
}

Neighborhood* CellularAutomata::getNeighborhood() const {
	return neighborhood;
}

void CellularAutomata::setStateSet(StateSet* stateSet) {
	this->stateSet = stateSet;
}

StateSet* CellularAutomata::getStateSet() const {
	return stateSet;
}

void CellularAutomata::setLattice(Lattice* lattice) {
	this->lattice = lattice;
}

Lattice* CellularAutomata::getLattice() const {
	return lattice;
}

void CellularAutomata::setSimulationLength(unsigned int simulationLength) {
	this->simulationLength = simulationLength;
}

unsigned int CellularAutomata::getSimulationLength() const {
	return simulationLength;
}

unsigned int CellularAutomata::getSimulatedTime() const {
	return simulatedTime;
}

bool CellularAutomata::isRunning() const {
	return running;
}

/* **************
 *  PROTECTED
 * **************/

void CellularAutomata::notifyObservers(CellularAutomataEvent event) {
	for (CellularAutomataEventObserver observer : *observers) {
		observer(event);
	}
}

void CellularAutomata::applyLocalRule() {
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
