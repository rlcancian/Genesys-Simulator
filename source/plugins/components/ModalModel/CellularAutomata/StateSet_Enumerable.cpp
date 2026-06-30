#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

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

bool StateSet_Enumerable::contains(const State& state) const {
	for (State* allowedState : states) {
		if (allowedState != nullptr && allowedState->getDoubleValue() == state.getDoubleValue())
			return true;
	}
	return false;
}

bool StateSet_Enumerable::tryMakeState(long value, State* state) const {
	return tryMakeState(static_cast<double>(value), state);
}

bool StateSet_Enumerable::tryMakeState(double value, State* state) const {
	for (State* allowedState : states) {
		if (allowedState != nullptr && allowedState->getDoubleValue() == value) {
			if (state != nullptr)
				*state = *allowedState;
			return true;
		}
	}
	return false;
}

std::string StateSet_Enumerable::show() const {
	std::string output = "{";
	for (unsigned int i = 0; i < states.size(); ++i) {
		if (i > 0)
			output += ",";
		output += states.at(i) == nullptr ? "null" : std::to_string(states.at(i)->getDoubleValue());
	}
	return output + "}";
}

std::string StateSet_Enumerable::typeName() const {
	return "enumerated";
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
		if (std::to_string(s->getValue())==name || std::to_string(s->getDoubleValue())==name) {
			return s;
		}
	}
	return nullptr;
}

unsigned int StateSet_Enumerable::getStatesSize() {
	return states.size();
}
