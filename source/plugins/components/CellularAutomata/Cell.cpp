#include "Cell.h"
#include "State.h"

/* **************
 *  PUBLIC
 * **************/

Cell::Cell(/*StateSet* stateSet, */long cellNum, std::vector<int> position) {
    //this->stateSet = stateSet;
    this->cellNum = cellNum;
    this->position = position;
	updatePending = false;
}

Cell::Cell(const Cell& orig) {
	position.reserve(orig.position.size());
	for (unsigned int pos : orig.position) {
		this->position.emplace_back(pos);
	}
	//this->stateSet = orig.stateSet; // TODO Singleton
	this->currentState = orig.currentState; // TODO: Singleton
}

/* **************
 *  PUBLIC
 * **************/

std::string Cell::show() {
    std::string m = "num=" + std::to_string(cellNum) 
			//+",pos=" +Util::showNDimPosition(position)
            //+",state=(" + currentState->show()+ ")"
            ;
    return m;
}

bool Cell::updateState() {
	if (!updatePending)
		return false;
    previousState = currentState;
    currentState = nextState;
	updatePending = false;
	// TODO Notify
    return true;
}

bool Cell::isUpdatePending() {
	return updatePending;
}

void Cell::draw() {
     
}

//void Cell::applyLocalRule() {    }

/* **************
 *  PUBLIC
 * **************/

/*
void Cell::setStateSet(StateSet* stateSet) {
    this->stateSet = stateSet;
}

StateSet* Cell::getStateSet() const {
    return stateSet;
}
*/ 

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

void Cell::setCurrentState(State currentState) {
    this->previousState = this->currentState;
	this->currentState = currentState;
	updatePending = false;
}

State Cell::getCurrentState() const {
	return currentState;
}

State Cell::getPreviousState() const {
	return previousState;
}

void Cell::setNextState(State nextState) {
	this->nextState = nextState;
	updatePending = true;
}

State Cell::getNextState() const {
	return nextState;
}

void Cell::setNeighbors(std::vector<Cell*> neighbors) {
    this->neighbors.reserve(neighbors.size());
    this->neighbors = neighbors;
}

std::vector<Cell*> Cell::getNeighbors() const {
    return neighbors;
}
