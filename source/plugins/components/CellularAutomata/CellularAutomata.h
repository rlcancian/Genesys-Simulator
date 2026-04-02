#pragma once

#include "Lattice.h"
#include "StateSet.h"
#include "Neighborhood.h"
#include "LocalRule.h"
#include <string>
#include <functional>
#include <list>
#include <thread>

class CellularAutomata;

class CellularAutomataEvent { // event for observers
public:
	CellularAutomataEvent(CellularAutomata* ca) {
		this->ca = ca;
	}
	CellularAutomata* ca;
};

typedef std::function<void(CellularAutomataEvent) > CellularAutomataEventObserver; ///< A pointer to a member function that will be notified on an event

class CellularAutomata {
public:
	CellularAutomata(std::string name = "", Lattice* lattice = nullptr, StateSet* stateSet = nullptr, Neighborhood* neighborhood = nullptr, LocalRule* localRule = nullptr);
	CellularAutomata(const CellularAutomata& orig);
	virtual ~CellularAutomata() = default;
public:
	//virtual std::string show();
	virtual bool init(); ///< Init lattice and cells
	virtual void step(); ///< Create next configuration c(t+1) = G(c(t))
	virtual void start(); ///< Run (step) until end of simulation
	virtual void pause(); ///< Pase the "run"
	template<typename Class> void addObserver(Class * object, void (Class::*function)(CellularAutomataEvent));
public:
	std::string getName() const;
	void setName(const std::string name);
	void setLocalRule(LocalRule* localRule);
	LocalRule* getLocalRule() const;
	void setNeighborhood(Neighborhood* neighborhood);
	Neighborhood* getNeighborhood() const;
	void setStateSet(StateSet* stateSet);
	StateSet* getStateSet() const;
	void setLattice(Lattice* lattice);
	Lattice* getLattice() const;
	void setSimulationLength(unsigned int simulationLength);
	unsigned int getSimulationLength() const;
	unsigned int getSimulatedTime() const;
	bool isRunning() const;
protected:
	void notifyObservers(CellularAutomataEvent event); ///< Invoke observers then something (an event) happes
	virtual void applyLocalRule() = 0;  ///< Pure virtual method that has to be overriden by derived classes
protected:
	Lattice* lattice;
	StateSet* stateSet;
	Neighborhood* neighborhood;
	LocalRule* localRule;
	std::string name;
	unsigned int simulatedTime;
	unsigned int simulationLength;
	bool running;
private:
	std::list<CellularAutomataEventObserver>* observers = new std::list<CellularAutomataEventObserver>();
};

template<typename Class> void CellularAutomata::addObserver(Class * object, void (Class::*function)(CellularAutomataEvent)) {
	CellularAutomataEventObserver observer = std::bind(function, object, std::placeholders::_1);
	observers->emplace_back(observer);
}
