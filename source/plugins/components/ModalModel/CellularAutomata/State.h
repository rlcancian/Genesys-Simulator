#pragma once

#include <array>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

/*
template<class T>
class State {
public:
	State<T>() {}
	State<T>(T value) {_value=value;}
	State<T>(const State& orig){}
	virtual ~State<T>() = default;
public:
	T getValue(){return _value;}
	void setValue(T value) {_value=value;}
protected:
	T _value;
private:
};
*/

class State {
public:
	State() : _value(0) {}
	State(long value) : _value(value) {}
	State(const State& orig) : _value(orig._value) {}
	virtual ~State() = default;
public:
	virtual std::unique_ptr<State> clone() const { return std::make_unique<State>(*this); }
	virtual bool equals(const State& other) const { return typeName() == other.typeName() && getValue() == other.getValue(); }
	virtual std::string toString() const { return std::to_string(_value); }
	virtual std::string show() const { return toString(); }
	virtual std::string typeName() const { return "State"; }
	virtual long getValue() const { return _value; }
	virtual void setValue(long value) { _value=value; }

	template <typename T>
	const T* as() const noexcept { return dynamic_cast<const T*>(this); }

	template <typename T>
	T* as() noexcept { return dynamic_cast<T*>(this); }
protected:
	long _value;
private:
};

inline bool operator==(const State& left, const State& right) {
	return left.equals(right);
}

inline bool operator!=(const State& left, const State& right) {
	return !(left == right);
}

class RealState : public State {
public:
	explicit RealState(double value = 0.0)
		: State(static_cast<long>(value)), _realValue(value) {
	}

	std::unique_ptr<State> clone() const override { return std::make_unique<RealState>(*this); }
	bool equals(const State& other) const override {
		const auto* real = other.as<RealState>();
		return real != nullptr && _realValue == real->_realValue;
	}
	std::string toString() const override {
		std::ostringstream stream;
		stream << _realValue;
		return stream.str();
	}
	std::string typeName() const override { return "RealState"; }
	long getValue() const override { return static_cast<long>(_realValue); }
	void setValue(long value) override { _realValue = static_cast<double>(value); _value = value; }
	double realValue() const { return _realValue; }
	void setRealValue(double value) { _realValue = value; _value = static_cast<long>(value); }
private:
	double _realValue = 0.0;
};

class CompositeState : public State {
public:
	using Field = std::pair<std::string, std::unique_ptr<State>>;

	CompositeState() = default;
	CompositeState(const CompositeState& orig) : State(orig) {
		for (const auto& field : orig._fields) {
			_fields.emplace_back(field.first, field.second != nullptr ? field.second->clone() : nullptr);
		}
	}
	CompositeState& operator=(const CompositeState& orig) {
		if (this == &orig) {
			return *this;
		}
		_value = orig._value;
		_fields.clear();
		for (const auto& field : orig._fields) {
			_fields.emplace_back(field.first, field.second != nullptr ? field.second->clone() : nullptr);
		}
		return *this;
	}

	std::unique_ptr<State> clone() const override { return std::make_unique<CompositeState>(*this); }
	bool equals(const State& other) const override {
		const auto* composite = other.as<CompositeState>();
		if (composite == nullptr || _fields.size() != composite->_fields.size()) {
			return false;
		}
		for (size_t index = 0; index < _fields.size(); ++index) {
			const auto& left = _fields.at(index);
			const auto& right = composite->_fields.at(index);
			if (left.first != right.first || left.second == nullptr || right.second == nullptr || *left.second != *right.second) {
				return false;
			}
		}
		return true;
	}
	std::string toString() const override {
		std::string text = "{";
		for (size_t index = 0; index < _fields.size(); ++index) {
			if (index > 0) {
				text += ",";
			}
			text += _fields.at(index).first + ":" + (_fields.at(index).second != nullptr ? _fields.at(index).second->toString() : "null");
		}
		return text + "}";
	}
	std::string typeName() const override { return "CompositeState"; }
	bool addField(std::string name, const State& state) {
		_fields.emplace_back(std::move(name), state.clone());
		return true;
	}
	const std::vector<Field>& fields() const { return _fields; }
private:
	std::vector<Field> _fields;
};

class HppLatticeGasState : public State {
public:
	enum Direction : size_t { North = 0, East = 1, South = 2, West = 3 };

	HppLatticeGasState() = default;
	explicit HppLatticeGasState(std::array<bool, 4> occupations)
		: State(maskFromOccupations(occupations)), _occupations(occupations) {
	}

	std::unique_ptr<State> clone() const override { return std::make_unique<HppLatticeGasState>(*this); }
	bool equals(const State& other) const override {
		const auto* hpp = other.as<HppLatticeGasState>();
		return hpp != nullptr && _occupations == hpp->_occupations;
	}
	std::string toString() const override {
		return std::string("{N:") + (_occupations[North] ? "1" : "0") +
		       ",E:" + (_occupations[East] ? "1" : "0") +
		       ",S:" + (_occupations[South] ? "1" : "0") +
		       ",W:" + (_occupations[West] ? "1" : "0") + "}";
	}
	std::string typeName() const override { return "HppLatticeGasState"; }
	long getValue() const override { return maskFromOccupations(_occupations); }
	void setValue(long value) override {
		_occupations = {{
			(value & (1L << North)) != 0,
			(value & (1L << East)) != 0,
			(value & (1L << South)) != 0,
			(value & (1L << West)) != 0,
		}};
		_value = getValue();
	}
	bool has(Direction direction) const { return _occupations[direction]; }
	void set(Direction direction, bool occupied) { _occupations[direction] = occupied; _value = getValue(); }
	const std::array<bool, 4>& occupations() const { return _occupations; }
	static long maskFromOccupations(const std::array<bool, 4>& occupations) {
		long mask = 0;
		for (size_t index = 0; index < occupations.size(); ++index) {
			if (occupations[index]) {
				mask |= (1L << index);
			}
		}
		return mask;
	}
private:
	std::array<bool, 4> _occupations{{false, false, false, false}};
};
