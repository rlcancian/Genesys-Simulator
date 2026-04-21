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
protected:
	CellularAutomataBase* parentCellularAutomata;
private:
};
