#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

StateSet::StateSet(CellularAutomataBase* parentCellularAutomata) {
	this->parentCellularAutomata = parentCellularAutomata;
	if (parentCellularAutomata != nullptr) {
		parentCellularAutomata->setStateSet(this);
	}
}

StateSet::StateSet(const StateSet& orig) {
	this->parentCellularAutomata = orig.parentCellularAutomata;
}

bool StateSet::contains(const State& state) const {
	(void)state;
	return true;
}

std::unique_ptr<State> StateSet::createDefaultState() const {
	return std::make_unique<State>(0);
}

std::unique_ptr<State> StateSet::parseState(const std::string& text) const {
	try {
		return std::make_unique<State>(std::stol(text));
	} catch (...) {
		return nullptr;
	}
}

bool StateSet::isFinite() const {
	return false;
}

bool StateSet::isDiscrete() const {
	return false;
}

bool StateSet::isContinuous() const {
	return false;
}

bool StateSet::isComposite() const {
	return false;
}

std::vector<std::unique_ptr<State>> StateSet::enumerateStates() const {
	return {};
}

std::string StateSet::name() const {
	return "StateSet";
}
