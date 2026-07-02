#pragma once

#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

#include <cmath>
#include <string>

class StateSet_Bit : public StateSet {
public:
	StateSet_Bit(CellularAutomataBase* parentCellularAutomata)
		: StateSet(parentCellularAutomata) {
	}

	virtual bool contains(const State& state) const override {
		const double value = state.getDoubleValue();
		return std::isfinite(value) && (value == 0.0 || value == 1.0);
	}

	virtual std::string show() const override {
		return "{0,1}";
	}

	virtual std::string typeName() const override {
		return "bit";
	}
};
