#pragma once


#include <string>
#include <functional>
#include <list>
#include <thread>

class Lattice;
class StateSet;
class Neighborhood;
class LocalRule;


class CellularAutomataBase {
public:
	CellularAutomataBase(Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr);
	CellularAutomataBase(const CellularAutomataBase& orig);
	virtual ~CellularAutomataBase() = default;
public:
	//virtual std::string show();
	virtual bool init(); ///< Init lattice and cells
	virtual void step(); ///< Create next configuration c(t+1) = G(c(t))
public:
	void setLocalRule(LocalRule* localRule);
	LocalRule* getLocalRule() const;
	void setNeighborhood(Neighborhood* neighborhood);
	Neighborhood* getNeighborhood() const;
	void setStateSet(StateSet* stateSet);
	StateSet* getStateSet() const;
	void setLattice(Lattice* lattice);
	Lattice* getLattice() const;
protected:
	virtual void applyLocalRule() = 0;  ///< Pure virtual method that has to be overriden by derived classes
protected:
	Lattice* lattice;
	StateSet* stateSet;
	Neighborhood* neighborhood;
	LocalRule* localRule;
};
