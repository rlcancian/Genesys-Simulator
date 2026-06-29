#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Adiabatic.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Reflexive.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"

// ---------------------------------------------------------------------------
// Boundary_Reflexive tests
// ---------------------------------------------------------------------------

TEST(BoundaryReflexive, CornerCellSeesReflectedNeighbors) {
	// 1-D lattice [0..4] with Boundary_Reflexive and Center neighborhood (radius 1).
	// Cell 0 has a "left" neighbor at position -1 which should reflect to position 1.
	// Cell 4 has a "right" neighbor at position 5 which should reflect to position 3.
	CellularAutomata_Classic automaton;
	Boundary_Reflexive boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	Lattice lattice(&automaton, nullptr, {5});

	ASSERT_TRUE(automaton.init());

	std::vector<Cell*> neighbors0 = neighborhood.getNeighbors(lattice.getCell(0));
	ASSERT_EQ(neighbors0.size(), 2u);
	for (Cell* n : neighbors0)
		EXPECT_GE(n->getCellNumber(), 0);

	std::vector<Cell*> neighbors4 = neighborhood.getNeighbors(lattice.getCell(4));
	ASSERT_EQ(neighbors4.size(), 2u);
	for (Cell* n : neighbors4)
		EXPECT_LE(n->getCellNumber(), 4);
}

TEST(BoundaryReflexive, ReflectedNeighborHasSameStateAsInnerCell) {
	// 1-D lattice [0..4]. Cell 1 is alive.
	// Cell 0's left neighbor at -1 reflects to 1 → neighbors of cell 0 must include cell 1.
	CellularAutomata_Classic automaton;
	Boundary_Reflexive boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {5});

	ASSERT_TRUE(automaton.init());
	lattice.getCell(1)->setCurrentState(State(1));

	std::vector<Cell*> neighbors0 = neighborhood.getNeighbors(lattice.getCell(0));
	bool seesCell1 = false;
	for (Cell* n : neighbors0)
		if (n->getCellNumber() == 1) seesCell1 = true;
	EXPECT_TRUE(seesCell1);
}

// ---------------------------------------------------------------------------
// Boundary_Adiabatic tests
// ---------------------------------------------------------------------------

TEST(BoundaryAdiabatic, CornerCellSeesItselfForOutOfBoundsNeighbor) {
	// 1-D lattice [0..4]. Cell 0 with Adiabatic: left neighbor at -1 clamps to 0 (itself).
	CellularAutomata_Classic automaton;
	Boundary_Adiabatic boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	Lattice lattice(&automaton, nullptr, {5});

	ASSERT_TRUE(automaton.init());

	std::vector<Cell*> neighbors0 = neighborhood.getNeighbors(lattice.getCell(0));
	ASSERT_EQ(neighbors0.size(), 2u);
	bool seesItself = false;
	for (Cell* n : neighbors0)
		if (n->getCellNumber() == 0) seesItself = true;
	EXPECT_TRUE(seesItself);

	std::vector<Cell*> neighbors4 = neighborhood.getNeighbors(lattice.getCell(4));
	ASSERT_EQ(neighbors4.size(), 2u);
	bool seesItself4 = false;
	for (Cell* n : neighbors4)
		if (n->getCellNumber() == 4) seesItself4 = true;
	EXPECT_TRUE(seesItself4);
}

TEST(BoundaryAdiabatic, AllDeadGridRemainsDeadAfterStep) {
	// All-dead grid with Adiabatic boundary. No births should occur anywhere,
	// including at the boundary (the clamped self-neighbor contributes state 0).
	CellularAutomata_Classic automaton;
	Boundary_Adiabatic boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {5});

	ASSERT_TRUE(automaton.init());
	automaton.step();
	for (int i = 0; i < 5; ++i)
		EXPECT_EQ(lattice.getCell(i)->getCurrentState().getValue(), 0);
}
