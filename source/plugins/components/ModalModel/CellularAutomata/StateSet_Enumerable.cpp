#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

StateSet_Enumerable::StateSet_Enumerable(CellularAutomataBase* parentCellularAutomata, std::vector<State*> states)
	:StateSet(parentCellularAutomata) {
	setStates(states);
}

StateSet_Enumerable::StateSet_Enumerable(const StateSet_Enumerable& orig)
	:StateSet(orig) {
	this->parentCellularAutomata = orig.parentCellularAutomata;
	for (const State* state : orig.states) {
		if (state != nullptr) {
			addState(*state);
		}
	}
}

std::string StateSet_Enumerable::show() {
	return "-";
}

bool StateSet_Enumerable::contains(const State& state) const {
	for (const State* availableState : states) {
		if (availableState != nullptr && *availableState == state) {
			return true;
		}
	}
	return false;
}

std::unique_ptr<State> StateSet_Enumerable::createDefaultState() const {
	if (!states.empty() && states.front() != nullptr) {
		return states.front()->clone();
	}
	return std::make_unique<State>(0);
}

std::unique_ptr<State> StateSet_Enumerable::parseState(const std::string& text) const {
	State* state = const_cast<StateSet_Enumerable*>(this)->getState(text);
	return state != nullptr ? state->clone() : nullptr;
}

bool StateSet_Enumerable::isFinite() const {
	return true;
}

bool StateSet_Enumerable::isDiscrete() const {
	return true;
}

std::vector<std::unique_ptr<State>> StateSet_Enumerable::enumerateStates() const {
	std::vector<std::unique_ptr<State>> result;
	result.reserve(states.size());
	for (const State* state : states) {
		if (state != nullptr) {
			result.emplace_back(state->clone());
		}
	}
	return result;
}

std::string StateSet_Enumerable::name() const {
	return "StateSet_Enumerable";
}

unsigned int StateSet_Enumerable::size(){
	return states.size();
}

void StateSet_Enumerable::addState(State* state) {
	if (state != nullptr) {
		addState(*state);
	}
}

void StateSet_Enumerable::addState(const State& state) {
	ownedStates.emplace_back(state.clone());
	states.emplace_back(ownedStates.back().get());
}

void StateSet_Enumerable::setStates(std::vector<State*> states) {
	this->states.clear();
	ownedStates.clear();
	for (State* state : states) {
		if (state != nullptr) {
			addState(*state);
		}
	}
}

std::vector<State*> StateSet_Enumerable::getStates() const {
	return states;
}

State* StateSet_Enumerable::getState(unsigned int rank) {
	return states.at(rank);
}

const State* StateSet_Enumerable::getState(unsigned int rank) const {
	return states.at(rank);
}

State* StateSet_Enumerable::getState(std::string name) {
	for (State* s: states) {
		if (s != nullptr && (s->toString()==name || std::to_string(s->getValue())==name)) {
			return s;
		}
	}
	return nullptr;
}

unsigned int StateSet_Enumerable::getStatesSize() {
	return states.size();
}
