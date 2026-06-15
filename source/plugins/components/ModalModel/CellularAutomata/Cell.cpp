#include "plugins/components/ModalModel/CellularAutomata/Cell.h"

Cell::Cell(StateSet* stateSet, long cellNum, std::vector<int> position) {
	this->stateSet = stateSet;
	this->cellNum = cellNum;
	this->position = position;
	previousState = std::make_unique<State>(0);
	currentState = stateSet != nullptr ? stateSet->createDefaultState() : std::make_unique<State>(0);
	nextState = currentState->clone();
	updatePending = false;
}

Cell::Cell(long cellNum, std::vector<int> position)
	: Cell(nullptr, cellNum, position) {
}

Cell::Cell(const Cell& orig) {
	*this = orig;
}

Cell& Cell::operator=(const Cell& orig) {
	if (this == &orig) {
		return *this;
	}
	position = orig.position;
	stateSet = orig.stateSet;
	previousState = orig.previousState != nullptr ? orig.previousState->clone() : std::make_unique<State>(0);
	currentState = orig.currentState != nullptr ? orig.currentState->clone() : std::make_unique<State>(0);
	nextState = orig.nextState != nullptr ? orig.nextState->clone() : currentState->clone();
	updatePending = orig.updatePending;
	cellNum = orig.cellNum;
	neighbors = orig.neighbors;
	return *this;
}

std::string Cell::show() {
	return "num=" + std::to_string(cellNum) + ",state=(" + currentState->show() + ")";
}

bool Cell::updateState() {
	if (!updatePending) {
		return false;
	}
	previousState = currentState != nullptr ? currentState->clone() : std::make_unique<State>(0);
	currentState = nextState != nullptr ? nextState->clone() : std::make_unique<State>(0);
	updatePending = false;
	return true;
}

bool Cell::isUpdatePending() {
	return updatePending;
}

void Cell::draw() {
}

void Cell::setStateSet(StateSet* stateSet) {
	this->stateSet = stateSet;
	if (this->stateSet == nullptr) {
		return;
	}
	if (currentState == nullptr || !this->stateSet->contains(*currentState)) {
		currentState = this->stateSet->createDefaultState();
		updatePending = false;
	}
	if (nextState == nullptr || !this->stateSet->contains(*nextState)) {
		nextState = currentState->clone();
		updatePending = false;
	}
	if (previousState == nullptr || !this->stateSet->contains(*previousState)) {
		previousState = currentState->clone();
	}
}

StateSet* Cell::getStateSet() const {
	return stateSet;
}

bool Cell::acceptsState(const State& state) const {
	return stateSet == nullptr || stateSet->contains(state);
}

void Cell::setCellNumber(long cellNum) {
	this->cellNum = cellNum;
}

long Cell::getCellNumber() const {
	return cellNum;
}

void Cell::setPosition(std::vector<int> position) {
	this->position = position;
}

std::vector<int> Cell::getPosition() const {
	return position;
}

bool Cell::setCurrentState(const State& currentState) {
	if (!acceptsState(currentState)) {
		return false;
	}
	previousState = this->currentState != nullptr ? this->currentState->clone() : std::make_unique<State>(0);
	this->currentState = currentState.clone();
	nextState = this->currentState->clone();
	updatePending = false;
	return true;
}

const State& Cell::getCurrentState() const {
	return *currentState;
}

const State& Cell::getPreviousState() const {
	return *previousState;
}

bool Cell::setNextState(const State& nextState) {
	if (!acceptsState(nextState)) {
		return false;
	}
	this->nextState = nextState.clone();
	updatePending = true;
	return true;
}

const State& Cell::getNextState() const {
	return *nextState;
}

void Cell::setNeighbors(std::vector<Cell*> neighbors) {
	this->neighbors.reserve(neighbors.size());
	this->neighbors = neighbors;
}

std::vector<Cell*> Cell::getNeighbors() const {
	return neighbors;
}
