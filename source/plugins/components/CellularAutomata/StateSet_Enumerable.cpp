#include "StateSet_Enumerable.h"
#include "CellularAutomataBase.h"

/* **************
 *  PUBLIC
 * **************/

StateSet_Enumerable::StateSet_Enumerable(CellularAutomataBase* parentCellularAutomata, std::vector<State*> states)
	:StateSet(parentCellularAutomata) {
	this->states = states;
}

StateSet_Enumerable::StateSet_Enumerable(const StateSet_Enumerable& orig)
	:StateSet(orig) {
	this->parentCellularAutomata = orig.parentCellularAutomata;
	this->states = orig.states; // Singleton

}

/* **************
 *  PUBLIC
 * **************/

std::string StateSet_Enumerable::show() {
	return "-";
}

unsigned int StateSet_Enumerable::size(){
	return states.size();
}

void StateSet_Enumerable::addState(State* state) {
	states.insert(states.end(), state);
}

void StateSet_Enumerable::setStates(std::vector<State*> states) {
	this->states = states;
}

std::vector<State*> StateSet_Enumerable::getStates() const {
	return states;
}

State* StateSet_Enumerable::getState(unsigned int rank) {
	return states.at(rank);
}

State* StateSet_Enumerable::getState(std::string name) {
	for (State* s: states) {
		if (std::to_string(s->getValue())==name) {
			return s;
		}
	}
	return nullptr;
}

unsigned int StateSet_Enumerable::getStatesSize() {
	return states.size();
}
