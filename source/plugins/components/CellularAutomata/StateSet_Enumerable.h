#pragma once

#include <string>
#include <vector>
#include "State.h"
#include "StateSet.h"

class CellularAutomataBase;

class StateSet_Enumerable: public StateSet{
public:
	StateSet_Enumerable(CellularAutomataBase* parentCellularAutomata, std::vector<State*> states = {});
	StateSet_Enumerable(const StateSet_Enumerable& orig);
	virtual ~StateSet_Enumerable()=default;
public:
	virtual std::string show();
	unsigned int size();
	void addState(State* state);
	unsigned int getStatesSize();
	State* getState(unsigned int rank);
	State* getState(std::string name);
	void setStates(std::vector<State*> states);
	std::vector<State*> getStates() const;
protected:
	CellularAutomataBase* parentCellularAutomata;
	std::vector<State*> states;
private:
};
