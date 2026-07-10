#pragma once



#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"
#include <memory>
#include <vector>
#include <string>

class State;

class Cell {
public:  // constructors
	Cell(StateSet* stateSet = nullptr, long cellNum=0, std::vector<int> position={});
	Cell(long cellNum, std::vector<int> position);
    Cell(const Cell& orig);
	Cell& operator=(const Cell& orig);
    virtual ~Cell()=default;
public: // methods
    virtual std::string show();
    bool updateState(); ///< update cell state (currentState = nextState)
    bool isUpdatePending(); ///< true if nextState differs from currentState
    virtual void draw(); ///< virtual method that can be override by derived classes
public: // getters
    void setCellNumber(long cellNum); ///< cell rank position in the lattice vector
    long getCellNumber() const;
	void setPosition(std::vector<int> position);
	std::vector<int> getPosition() const; ///< n-dimensional position
	void setStateSet(StateSet* stateSet);
	StateSet* getStateSet() const;
	const State& getPreviousState() const;
	bool setCurrentState(const State& currentState);
	const State& getCurrentState() const;
	bool setNextState(const State& nextState);
	const State& getNextState() const;
    void setNeighbors(std::vector<Cell*> neighbors);
    std::vector<Cell*> getNeighbors() const;
protected:
	bool acceptsState(const State& state) const;
	std::unique_ptr<State> previousState;
	std::unique_ptr<State> currentState;
	std::unique_ptr<State> nextState;
	StateSet* stateSet = nullptr;
	bool updatePending;
	std::vector<int> position;
    long cellNum;
    std::vector<Cell*> neighbors;
protected: // for other CA types
private:
};
