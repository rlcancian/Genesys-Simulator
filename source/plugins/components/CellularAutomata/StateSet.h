#pragma once

#include <string>
#include <vector>
#include "State.h"

class CellularAutomataBase;

class StateSet {
public:
	StateSet(CellularAutomataBase* parentCellularAutomata);
	StateSet(const StateSet& orig);
	virtual ~StateSet()=default;
public:
protected:
	CellularAutomataBase* parentCellularAutomata;
private:
};
