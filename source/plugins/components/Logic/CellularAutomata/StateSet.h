#pragma once

#include <string>
#include <vector>
#include "plugins/components/Logic/CellularAutomata/State.h"

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
