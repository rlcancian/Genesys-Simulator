#pragma once



#include "State.h"
#include <vector>
#include <string>

class State;

class Cell {
public:  // constructors
	Cell(/*StateSet* stateSet = nullptr, */long cellNum=0, std::vector<int> position={});
    Cell(const Cell& orig);
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
	State getPreviousState() const;
	void setCurrentState(State currentState);
	State getCurrentState() const;
	void setNextState(State nextState);
	State getNextState() const;
    void setNeighbors(std::vector<Cell*> neighbors);
    std::vector<Cell*> getNeighbors() const;
protected:
	State previousState;
	State currentState;
	State nextState;
	bool updatePending;
	std::vector<int> position;
    long cellNum;
    std::vector<Cell*> neighbors;
protected: // for other CA types
private:
};
