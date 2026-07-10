#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

/* **************
 *  PUBLIC
 * **************/

StateSet::StateSet(CellularAutomataBase* parentCellularAutomata) {
	this->parentCellularAutomata = parentCellularAutomata;
	parentCellularAutomata->setStateSet(this);
}

StateSet::StateSet(const StateSet& orig) {
	this->parentCellularAutomata = orig.parentCellularAutomata;
}

/* **************
 *  PUBLIC
 * **************/

bool StateSet::contains(const State& state) const {
	return true;
}

bool StateSet::tryMakeState(long value, State* state) const {
	return tryMakeState(static_cast<double>(value), state);
}

bool StateSet::tryMakeState(double value, State* state) const {
	State candidate(value);
	if (!contains(candidate))
		return false;
	if (state != nullptr)
		*state = candidate;
	return true;
}

std::string StateSet::show() const {
	return typeName();
}

std::string StateSet::typeName() const {
	return "generic";
}
