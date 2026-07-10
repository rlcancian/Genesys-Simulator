#pragma once

#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

#include <cmath>
#include <limits>
#include <string>

class StateSet_Integer : public StateSet {
public:
	StateSet_Integer(CellularAutomataBase* parentCellularAutomata,
			long minValue = std::numeric_limits<long>::min(),
			long maxValue = std::numeric_limits<long>::max())
		: StateSet(parentCellularAutomata), minValue(minValue), maxValue(maxValue) {
	}

	virtual bool contains(const State& state) const override {
		const double value = state.getDoubleValue();
		return std::isfinite(value) &&
				std::floor(value) == value &&
				value >= static_cast<double>(minValue) &&
				value <= static_cast<double>(maxValue);
	}

	virtual std::string show() const override {
		return "[" + std::to_string(minValue) + "," + std::to_string(maxValue) + "]";
	}

	virtual std::string typeName() const override {
		return "integer";
	}

private:
	long minValue;
	long maxValue;
};
