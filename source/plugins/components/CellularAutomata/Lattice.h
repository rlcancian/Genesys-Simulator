#pragma once

#include "Cell.h"
#include <vector>
#include <string>

class CellularAutomataBase;

enum class LatticeType : int {
	RETICULAR = 1, TRIANGULAR = 2, HEXAGONAL = 3, NETWORK = 4, USERDEFINED = 5
};

//! n-dimensional lattice
class Lattice {

public:
	Lattice(CellularAutomataBase* parentCellularAutomata, Cell *progenitorCell = nullptr, std::vector<unsigned short> dimensions={}, LatticeType latticeType = LatticeType::RETICULAR);
	Lattice(const Lattice& orig);
	virtual ~Lattice() = default;
public:
	virtual std::string show();
	bool init(); ///< init all cells
	virtual long cellNDimPosition2Number(const std::vector<int> position); ///< Maps n-dimensional to cell number
	virtual std::vector<int> cellNumber2NDimPosition(const long cellNumber); ///< Maps cell number to n-dimensional
	Cell* getCell(const long cellNumber);
	Cell* getCell(const std::vector<int> position);
	void setCell(const std::vector<int> position, Cell* cell);
	bool setCellState(std::vector<int> position, State* state = nullptr, Cell* cell = nullptr);
	bool setCellState(long cellNumber, State* state = nullptr, Cell* cell = nullptr);
public:
	void setProgenitorCell(Cell *progenitorCell); ///< set cell used to fill the lattice
	Cell* getProgenitorCell() const;
	bool setNumCells(unsigned short dimension, unsigned int numCells, bool createDimensions = true);
	unsigned short getNumDimensions();
	unsigned int getNumCell(unsigned short dimension);
	LatticeType getLatticeType() const;
	unsigned long getCellsSize() const;
	unsigned long getTotalCells() const;
	unsigned long calculateTotalCells();
	bool isInit() const;
	void setDimensions(std::vector<unsigned short> dimensions);
	std::vector<unsigned short> getDimensions() const;
	unsigned short getDimension(unsigned short dimensionNumber) const;
	std::vector<Cell*> getCells() const;
protected:
	CellularAutomataBase* parentCellularAutomata;
	std::vector<Cell*> cells;
	std::vector<unsigned short> dimensions;
	Cell *progenitorCell;
	LatticeType latticeType;
	bool hasBeenInit;
	unsigned long totalCells;
};
