#include "StateSet.h"
#include "CellularAutomataBase.h"

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

