#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

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

