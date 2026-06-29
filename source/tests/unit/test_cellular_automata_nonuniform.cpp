#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Custom.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_PermissiveLife.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"

namespace {

// Minimal local rules used across non-uniform CA tests
class LocalRule_AlwaysDead : public LocalRule {
public:
	LocalRule_AlwaysDead(CellularAutomataBase* ca) : LocalRule(ca) {}
	virtual void applyRule(Cell* cell) override { cell->setNextState(State(0)); }
};

class LocalRule_AlwaysAlive : public LocalRule {
public:
	LocalRule_AlwaysAlive(CellularAutomataBase* ca) : LocalRule(ca) {}
	virtual void applyRule(Cell* cell) override { cell->setNextState(State(1)); }
};

} // namespace

// ---------------------------------------------------------------------------
// Per-cell rule tests
// ---------------------------------------------------------------------------

TEST(CellularAutomataNonUniformRule, AppliesPerCellRuleOverridingGlobalRule) {
	// 5-cell 1D lattice. Global rule: AlwaysAlive. Cells 1 and 3: AlwaysDead.
	// Expected after one step: [1, 0, 1, 0, 1]
	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	LocalRule_AlwaysDead deadRule(&automaton);   // registers first
	LocalRule_AlwaysAlive aliveRule(&automaton);  // registers last → global = aliveRule
	Lattice lattice(&automaton, nullptr, {5});

	automaton.setCellRule(1, &deadRule);
	automaton.setCellRule(3, &deadRule);

	ASSERT_TRUE(automaton.init());

	automaton.step();

	EXPECT_EQ(lattice.getCell(0)->getCurrentState().getValue(), 1); // aliveRule (global)
	EXPECT_EQ(lattice.getCell(1)->getCurrentState().getValue(), 0); // deadRule (specific)
	EXPECT_EQ(lattice.getCell(2)->getCurrentState().getValue(), 1); // aliveRule (global)
	EXPECT_EQ(lattice.getCell(3)->getCurrentState().getValue(), 0); // deadRule (specific)
	EXPECT_EQ(lattice.getCell(4)->getCurrentState().getValue(), 1); // aliveRule (global)
}

TEST(CellularAutomataNonUniformRule, FallsBackToGlobalRuleForUnassignedCells) {
	// 3-cell lattice. No per-cell overrides. All cells use global AlwaysDead rule.
	// Expected after one step: all 0.
	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	LocalRule_AlwaysDead deadRule(&automaton);
	Lattice lattice(&automaton, nullptr, {3});

	ASSERT_TRUE(automaton.init());
	lattice.getCell(0)->setCurrentState(State(1));
	lattice.getCell(1)->setCurrentState(State(1));
	lattice.getCell(2)->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell(0)->getCurrentState().getValue(), 0);
	EXPECT_EQ(lattice.getCell(1)->getCurrentState().getValue(), 0);
	EXPECT_EQ(lattice.getCell(2)->getCurrentState().getValue(), 0);
}

// ---------------------------------------------------------------------------
// Per-cell neighborhood tests
// ---------------------------------------------------------------------------

TEST(CellularAutomataNonUniformNeighborhood, AssignsCorrectNeighborCountPerCell) {
	// 5x5 lattice. Global: Moore r=1 (8 neighbors for interior cells).
	// Cell {2,2}: VonNeumann r=1 (4 neighbors for interior cells).
	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife rule(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	Neighborhood_VonNeumann vonNeumannForCenter(&automaton, 1, &boundary, false, false);
	automaton.setCellNeighborhood({2, 2}, &vonNeumannForCenter);

	ASSERT_TRUE(automaton.init());

	EXPECT_EQ(lattice.getCell({2, 2})->getNeighbors().size(), 4u); // VonNeumann
	EXPECT_EQ(lattice.getCell({1, 1})->getNeighbors().size(), 8u); // Moore
	EXPECT_EQ(lattice.getCell({3, 3})->getNeighbors().size(), 8u); // Moore
}

TEST(CellularAutomataNonUniformNeighborhood, EvolvesCorrectlyWithMixedNeighborhoods) {
	// 5x5 lattice, Game of Life rule.
	// Cell {2,2} uses VonNeumann: it sees only 4 neighbors instead of 8.
	// Horizontal blinker: {1,2}, {2,2}, {3,2} alive.
	// Under Moore: {2,2} has 2 live neighbors → survives (standard blinker).
	// Under VonNeumann for {2,2}: neighbors are {1,2},{3,2},{2,1},{2,3} → 2 live → survives.
	// After step, blinker rotates to vertical: {2,1},{2,2},{2,3} should be alive.
	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife rule(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	Neighborhood_VonNeumann vonNeumannForCenter(&automaton, 1, &boundary, false, false);
	automaton.setCellNeighborhood({2, 2}, &vonNeumannForCenter);

	ASSERT_TRUE(automaton.init());
	lattice.getCell({1, 2})->setCurrentState(State(1));
	lattice.getCell({2, 2})->setCurrentState(State(1));
	lattice.getCell({3, 2})->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell({2, 1})->getCurrentState().getValue(), 1);
	EXPECT_EQ(lattice.getCell({2, 2})->getCurrentState().getValue(), 1);
	EXPECT_EQ(lattice.getCell({2, 3})->getCurrentState().getValue(), 1);
	EXPECT_EQ(lattice.getCell({1, 2})->getCurrentState().getValue(), 0);
	EXPECT_EQ(lattice.getCell({3, 2})->getCurrentState().getValue(), 0);
}

// ---------------------------------------------------------------------------
// Combined rule + neighborhood per cell
// ---------------------------------------------------------------------------

TEST(CellularAutomataNonUniform, AppliesPerCellRuleAndNeighborhoodIndependently) {
	// 5x5 lattice.
	// Global: Moore r=1, AlwaysAlive rule.
	// Cell {2,2}: VonNeumann + AlwaysDead.
	// After one step: {2,2} = 0, all others = 1.
	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_AlwaysDead deadRule(&automaton);   // global = deadRule after this
	LocalRule_AlwaysAlive aliveRule(&automaton);  // global = aliveRule after this
	Lattice lattice(&automaton, nullptr, {5, 5});

	Neighborhood_VonNeumann vonNeumannForCenter(&automaton, 1, &boundary, false, false);
	automaton.setCellNeighborhood({2, 2}, &vonNeumannForCenter);
	automaton.setCellRule({2, 2}, &deadRule);

	ASSERT_TRUE(automaton.init());
	automaton.step();

	EXPECT_EQ(lattice.getCell({2, 2})->getCurrentState().getValue(), 0); // AlwaysDead
	EXPECT_EQ(lattice.getCell({2, 2})->getNeighbors().size(), 4u);        // VonNeumann
	EXPECT_EQ(lattice.getCell({0, 0})->getCurrentState().getValue(), 1); // AlwaysAlive
	EXPECT_EQ(lattice.getCell({4, 4})->getCurrentState().getValue(), 1); // AlwaysAlive
}

TEST(CellularAutomataNonUniform, SameGoLRuleDifferentNeighborhoodProducesDifferentOutcome) {
	// 7x7 lattice, Fixed boundary, global GoL + Moore.
	// Cell {4,4}: assigned VonNeumann neighborhood, keeps global GoL rule.
	// Cell {1,1}: uses default Moore neighborhood and global GoL rule.
	//
	// Live cells seeded:
	//   {3,3}, {5,3}, {3,5} — three diagonal neighbors of {4,4}
	//   {0,0}, {2,0}, {0,2} — three diagonal neighbors of {1,1}
	//
	// Under Moore, a dead cell with exactly 3 live neighbors is born (GoL birth rule).
	// Under VonNeumann, diagonal neighbors are invisible.
	//
	// Expected after one step:
	//   {1,1} = 1  (Moore sees {0,0},{2,0},{0,2} → 3 live diagonal → born)
	//   {4,4} = 0  (VonNeumann sees {3,4},{5,4},{4,3},{4,5} → 0 live → stays dead)
	CellularAutomata_NonUniform automaton;
	Boundary_Fixed boundary;
	Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {7, 7});

	Neighborhood_VonNeumann vnForCenter(&automaton, 1, &boundary, false, false);
	automaton.setCellNeighborhood({4, 4}, &vnForCenter);

	ASSERT_TRUE(automaton.init());

	lattice.getCell({3, 3})->setCurrentState(State(1));
	lattice.getCell({5, 3})->setCurrentState(State(1));
	lattice.getCell({3, 5})->setCurrentState(State(1));

	lattice.getCell({0, 0})->setCurrentState(State(1));
	lattice.getCell({2, 0})->setCurrentState(State(1));
	lattice.getCell({0, 2})->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell({1, 1})->getCurrentState().getValue(), 1); // Moore: 3 diagonal live → born
	EXPECT_EQ(lattice.getCell({4, 4})->getCurrentState().getValue(), 0); // VonNeumann: 0 orthogonal live → stays dead
}

// ---------------------------------------------------------------------------
// CellularAutomataComp dispatch contract
// ---------------------------------------------------------------------------

TEST(CellularAutomataCompDispatch, NonUniformAcceptsPerCellApiClassicDoesNot) {
	CellularAutomataBase* nonUniformBase = new CellularAutomata_NonUniform();
	CellularAutomataBase* classicBase    = new CellularAutomata_Classic();

	EXPECT_NE(dynamic_cast<CellularAutomata_NonUniform*>(nonUniformBase), nullptr);
	EXPECT_EQ(dynamic_cast<CellularAutomata_NonUniform*>(classicBase),    nullptr);

	delete nonUniformBase;
	delete classicBase;
}

// ---------------------------------------------------------------------------
// LocalRule_PermissiveLife
// ---------------------------------------------------------------------------

TEST(CellularAutomataPermissiveLife, DeadCellBornWhenNeighborCountInBirthRange) {
	// 5x5 lattice, VonNeumann r=1, Closed boundary.
	// PermissiveLife defaults: survives 1-4 live neighbors, born with 2-3.
	// Seed an L-shape: {2,2}, {3,2}, {2,3}.
	//
	// After one step:
	//   {2,2}: VonNeumann neighbors {1,2},{3,2}[a],{2,1},{2,3}[a] → 2 live → survives
	//   {3,2}: {2,2}[a],{4,2},{3,1},{3,3} → 1 live → survives (surviveMin=1)
	//   {2,3}: {1,3},{3,3},{2,2}[a],{2,4} → 1 live → survives
	//   {3,3}: {2,3}[a],{4,3},{3,2}[a],{3,4} → 2 live → BORN (birthMin=2)
	//   {1,2}: {0,2},{2,2}[a],{1,1},{1,3} → 1 live → NOT born (birthMin=2)
	CellularAutomata_Classic automaton;
	Boundary_Closed boundary;
	Neighborhood_VonNeumann neighborhood(&automaton, 1, &boundary);
	LocalRule_PermissiveLife permissiveRule(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	ASSERT_TRUE(automaton.init());
	lattice.getCell({2, 2})->setCurrentState(State(1));
	lattice.getCell({3, 2})->setCurrentState(State(1));
	lattice.getCell({2, 3})->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell({2, 2})->getCurrentState().getValue(), 1); // survives (2 live)
	EXPECT_EQ(lattice.getCell({3, 2})->getCurrentState().getValue(), 1); // survives (1 live)
	EXPECT_EQ(lattice.getCell({2, 3})->getCurrentState().getValue(), 1); // survives (1 live)
	EXPECT_EQ(lattice.getCell({3, 3})->getCurrentState().getValue(), 1); // born (2 live)
	EXPECT_EQ(lattice.getCell({1, 2})->getCurrentState().getValue(), 0); // not born (1 live < birthMin)
}

// ---------------------------------------------------------------------------
// LocalRule_Custom (majority vote)
// ---------------------------------------------------------------------------

TEST(CellularAutomataCustomRule, MajorityVoteConvergesUniformRegion) {
	// 3x1 lattice: [1, 0, 1]. Custom rule (majority).
	// Each cell sees its two neighbors.
	// Cell 0 (Closed): neighbors are cell 2 (1) and cell 1 (0) → tie → smaller state wins → 0
	// Cell 1: neighbors are cell 0 (1) and cell 2 (1) → majority = 1 → next = 1
	// Cell 2 (Closed): neighbors are cell 1 (0) and cell 0 (1) → tie → smaller state wins → 0
	CellularAutomata_Classic automaton;
	Boundary_Closed boundary;
	Neighborhood_Center neighborhood(&automaton, 1, &boundary);
	LocalRule_Custom customRule(&automaton);
	Lattice lattice(&automaton, nullptr, {3});

	ASSERT_TRUE(automaton.init());
	lattice.getCell(0)->setCurrentState(State(1));
	lattice.getCell(1)->setCurrentState(State(0));
	lattice.getCell(2)->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell(0)->getCurrentState().getValue(), 0);
	EXPECT_EQ(lattice.getCell(1)->getCurrentState().getValue(), 1);
	EXPECT_EQ(lattice.getCell(2)->getCurrentState().getValue(), 0);
}

// ---------------------------------------------------------------------------
// Region API (setRegionRule / setRegionNeighborhood)
// ---------------------------------------------------------------------------

TEST(CellularAutomataNonUniformRegion, SetRegionRuleAssignsRuleToAllCellsInBox) {
	// 5x5 grid. setRegionRule on the 3x3 interior [{1,1},{3,3}] → 9 cells mapped.
	CellularAutomata_NonUniform automaton;
	Boundary_Fixed boundary;
	Neighborhood_Moore hood(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	LocalRule_Custom regionRule(&automaton);
	automaton.setRegionRule({1, 1}, {3, 3}, &regionRule);

	const auto& rules = automaton.getCellRules();
	EXPECT_EQ(rules.size(), 9u);

	for (int y = 1; y <= 3; ++y) {
		for (int x = 1; x <= 3; ++x) {
			long cellNum = lattice.cellNDimPosition2Number({x, y});
			EXPECT_NE(rules.find(cellNum), rules.end())
				<< "Missing cell at (" << x << "," << y << ")";
		}
	}
}

TEST(CellularAutomataNonUniformRegion, SetRegionNeighborhoodAssignsCorrectNeighborCountAfterInit) {
	// 5x5 grid. Region [{2,2},{2,2}] gets VonNeumann; rest keeps Moore.
	CellularAutomata_NonUniform automaton;
	Boundary_Fixed boundary;
	Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	Neighborhood_VonNeumann vnCenter(&automaton, 1, &boundary, false, /*registerWithCA=*/false);
	automaton.setRegionNeighborhood({2, 2}, {2, 2}, &vnCenter);

	ASSERT_TRUE(automaton.init());

	// Center cell {2,2} must have 4 neighbors (VonNeumann)
	EXPECT_EQ(lattice.getCell({2, 2})->getNeighbors().size(), 4u);
	// Corner cell {0,0} keeps Moore: Fixed boundary returns fixedCell placeholder for
	// out-of-bounds positions, so all 8 slots are filled (some with the placeholder cell).
	EXPECT_EQ(lattice.getCell({0, 0})->getNeighbors().size(), 8u);
}

TEST(CellularAutomataNonUniformRegion, CellRuleOverridesRegionRule) {
	// setRegionRule for all 25 cells, then setCellRule on {2,2}: the map entry
	// for that cell must point to the overriding rule.
	CellularAutomata_NonUniform automaton;
	Boundary_Fixed boundary;
	Neighborhood_Moore hood(&automaton, 1, &boundary);
	LocalRule_GameOfLife gol(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	LocalRule_Custom regionRule(&automaton);
	automaton.setRegionRule({0, 0}, {4, 4}, &regionRule);

	LocalRule_GameOfLife overrideRule(&automaton);
	automaton.setCellRule({2, 2}, &overrideRule);

	long centerNum = lattice.cellNDimPosition2Number({2, 2});
	const auto& rules = automaton.getCellRules();
	auto it = rules.find(centerNum);
	ASSERT_NE(it, rules.end());
	EXPECT_EQ(it->second, &overrideRule);
}
