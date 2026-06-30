#pragma once

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
	virtual bool tryMakeState(long value, State* state) const;
	virtual bool tryMakeState(double value, State* state) const;
	virtual std::string show() const;
	virtual std::string typeName() const;
protected:
	CellularAutomataBase* parentCellularAutomata;
private:
};
