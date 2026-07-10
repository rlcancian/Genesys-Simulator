#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "plugins/components/ModalModel/CellularAutomata/State.h"

class CellularAutomataBase;

class StateSet {
public:
	StateSet(CellularAutomataBase* parentCellularAutomata);
	StateSet(const StateSet& orig);
	virtual ~StateSet()=default;
public:
	virtual bool contains(const State& state) const;
	virtual std::unique_ptr<State> createDefaultState() const;
	virtual std::unique_ptr<State> parseState(const std::string& text) const;
	virtual bool isFinite() const;
	virtual bool isDiscrete() const;
	virtual bool isContinuous() const;
	virtual bool isComposite() const;
	virtual std::vector<std::unique_ptr<State>> enumerateStates() const;
	virtual std::string name() const;
protected:
	CellularAutomataBase* parentCellularAutomata;
private:
};

class IntegerStateSet : public StateSet {
public:
	IntegerStateSet(CellularAutomataBase* parentCellularAutomata,
	                std::optional<long> minimum = std::nullopt,
	                std::optional<long> maximum = std::nullopt)
		: StateSet(parentCellularAutomata), _minimum(minimum), _maximum(maximum) {
	}

	bool contains(const State& state) const override {
		if (state.as<RealState>() != nullptr || state.as<CompositeState>() != nullptr || state.as<HppLatticeGasState>() != nullptr) {
			return false;
		}
		const long value = state.getValue();
		return (!_minimum.has_value() || value >= *_minimum) && (!_maximum.has_value() || value <= *_maximum);
	}
	std::unique_ptr<State> createDefaultState() const override {
		return std::make_unique<State>(_minimum.value_or(0));
	}
	std::unique_ptr<State> parseState(const std::string& text) const override {
		try {
			auto state = std::make_unique<State>(std::stol(text));
			return contains(*state) ? std::move(state) : nullptr;
		} catch (...) {
			return nullptr;
		}
	}
	bool isFinite() const override { return _minimum.has_value() && _maximum.has_value(); }
	bool isDiscrete() const override { return true; }
	std::vector<std::unique_ptr<State>> enumerateStates() const override {
		std::vector<std::unique_ptr<State>> states;
		if (!isFinite()) {
			return states;
		}
		for (long value = *_minimum; value <= *_maximum; ++value) {
			states.emplace_back(std::make_unique<State>(value));
			if (value == std::numeric_limits<long>::max()) {
				break;
			}
		}
		return states;
	}
	std::string name() const override { return "IntegerStateSet"; }
private:
	std::optional<long> _minimum;
	std::optional<long> _maximum;
};

class NaturalStateSet : public IntegerStateSet {
public:
	explicit NaturalStateSet(CellularAutomataBase* parentCellularAutomata, std::optional<long> maximum = std::nullopt)
		: IntegerStateSet(parentCellularAutomata, 0, maximum) {
	}
	std::string name() const override { return "NaturalStateSet"; }
};

class RealIntervalStateSet : public StateSet {
public:
	RealIntervalStateSet(CellularAutomataBase* parentCellularAutomata,
	                     std::optional<double> minimum = std::nullopt,
	                     std::optional<double> maximum = std::nullopt)
		: StateSet(parentCellularAutomata), _minimum(minimum), _maximum(maximum) {
	}

	bool contains(const State& state) const override {
		const auto* real = state.as<RealState>();
		const double value = real != nullptr ? real->realValue() : static_cast<double>(state.getValue());
		return (!_minimum.has_value() || value >= *_minimum) && (!_maximum.has_value() || value <= *_maximum);
	}
	std::unique_ptr<State> createDefaultState() const override {
		return std::make_unique<RealState>(_minimum.value_or(0.0));
	}
	std::unique_ptr<State> parseState(const std::string& text) const override {
		try {
			auto state = std::make_unique<RealState>(std::stod(text));
			return contains(*state) ? std::move(state) : nullptr;
		} catch (...) {
			return nullptr;
		}
	}
	bool isContinuous() const override { return true; }
	std::string name() const override { return "RealIntervalStateSet"; }
private:
	std::optional<double> _minimum;
	std::optional<double> _maximum;
};

class CompositeStateSet : public StateSet {
public:
	explicit CompositeStateSet(CellularAutomataBase* parentCellularAutomata)
		: StateSet(parentCellularAutomata) {
	}

	bool contains(const State& state) const override { return state.as<CompositeState>() != nullptr; }
	std::unique_ptr<State> createDefaultState() const override { return std::make_unique<CompositeState>(); }
	bool isComposite() const override { return true; }
	std::string name() const override { return "CompositeStateSet"; }
};

class HppLatticeGasStateSet : public StateSet {
public:
	explicit HppLatticeGasStateSet(CellularAutomataBase* parentCellularAutomata)
		: StateSet(parentCellularAutomata) {
	}

	bool contains(const State& state) const override { return state.as<HppLatticeGasState>() != nullptr; }
	std::unique_ptr<State> createDefaultState() const override { return std::make_unique<HppLatticeGasState>(); }
	std::unique_ptr<State> parseState(const std::string& text) const override {
		try {
			auto state = std::make_unique<HppLatticeGasState>();
			state->setValue(std::stol(text));
			return state;
		} catch (...) {
			return nullptr;
		}
	}
	bool isFinite() const override { return true; }
	bool isDiscrete() const override { return true; }
	bool isComposite() const override { return true; }
	std::vector<std::unique_ptr<State>> enumerateStates() const override {
		std::vector<std::unique_ptr<State>> states;
		for (long mask = 0; mask < 16; ++mask) {
			auto state = std::make_unique<HppLatticeGasState>();
			state->setValue(mask);
			states.emplace_back(std::move(state));
		}
		return states;
	}
	std::string name() const override { return "HppLatticeGasStateSet"; }
};
