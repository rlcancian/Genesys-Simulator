#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Custom.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_PermissiveLife.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

namespace {

template<typename NeighborhoodType, typename BoundaryType>
std::vector<long> buildNeighborNumbers(std::vector<unsigned short> dimensions, std::vector<int> position, unsigned short radius) {
	CellularAutomata_Classic automaton;
	BoundaryType boundary;
	NeighborhoodType neighborhood(&automaton, radius, &boundary);
	Lattice lattice(&automaton, nullptr, dimensions);

	EXPECT_TRUE(automaton.init());
	std::vector<Cell*> neighbors = neighborhood.getNeighbors(lattice.getCell(position));
	std::vector<long> numbers;
	numbers.reserve(neighbors.size());
	for (Cell* neighbor : neighbors) {
		numbers.emplace_back(neighbor->getCellNumber());
	}
	return numbers;
}

unsigned long powUnsigned(unsigned long base, unsigned short exponent) {
	unsigned long result = 1;
	for (unsigned short i = 0; i < exponent; ++i) {
		result *= base;
	}
	return result;
}

unsigned long expectedMooreCount(unsigned short dimensions, unsigned short radius) {
	return powUnsigned(2 * radius + 1, dimensions) - 1;
}

unsigned long expectedVonNeumannCount(unsigned short dimensions, unsigned short radius) {
	std::vector<int> offset(dimensions, 0);
	unsigned long count = 0;

	const auto visit = [&](const auto& self, unsigned short dimension) -> void {
		if (dimension == dimensions) {
			unsigned int manhattanDistance = 0;
			bool zeroOffset = true;
			for (int delta : offset) {
				manhattanDistance += static_cast<unsigned int>(std::abs(delta));
				zeroOffset = zeroOffset && delta == 0;
			}
			if (!zeroOffset && manhattanDistance <= radius) {
				++count;
			}
			return;
		}

		for (int delta = -static_cast<int>(radius); delta <= static_cast<int>(radius); ++delta) {
			offset.at(dimension) = delta;
			self(self, dimension + 1);
		}
	};

	visit(visit, 0);
	return count;
}

} // namespace

TEST(CellularAutomataLattice, ConvertsLinearCellNumbersAndNDimPositionsRoundTrip) {
	CellularAutomata_Classic automaton;
	Lattice lattice(&automaton, nullptr, {2, 3, 4, 5});
	const unsigned long totalCells = lattice.calculateTotalCells();

	for (long cellNumber = 0; cellNumber < static_cast<long>(totalCells); ++cellNumber) {
		const std::vector<int> position = lattice.cellNumber2NDimPosition(cellNumber);
		EXPECT_EQ(lattice.cellNDimPosition2Number(position), cellNumber);
	}

	EXPECT_EQ(lattice.cellNumber2NDimPosition(0), (std::vector<int>{0, 0, 0, 0}));
	EXPECT_EQ(lattice.cellNumber2NDimPosition(1), (std::vector<int>{1, 0, 0, 0}));
	EXPECT_EQ(lattice.cellNumber2NDimPosition(2), (std::vector<int>{0, 1, 0, 0}));
	EXPECT_EQ(lattice.cellNDimPosition2Number({1, 2, 3, 4}), 119);
	EXPECT_EQ(lattice.cellNDimPosition2Number({2, 0, 0, 0}), -1);
	EXPECT_EQ(lattice.cellNDimPosition2Number({0, -1, 0, 0}), -1);
}

TEST(CellularAutomataMooreNeighborhood, CountsInternalNeighborsForMultipleDimensionsAndRadii) {
	struct Case {
		std::vector<unsigned short> dimensions;
		std::vector<int> position;
		unsigned short radius;
	};

	const std::vector<Case> cases = {
		{{5}, {2}, 1},
		{{5, 5}, {2, 2}, 1},
		{{7, 7}, {3, 3}, 2},
		{{5, 5, 5}, {2, 2, 2}, 1},
		{{7, 7, 7}, {3, 3, 3}, 2},
	};

	for (const Case& currentCase : cases) {
		const auto numbers = buildNeighborNumbers<Neighborhood_Moore, Boundary_Closed>(
			currentCase.dimensions,
			currentCase.position,
			currentCase.radius
		);
		EXPECT_EQ(numbers.size(), expectedMooreCount(currentCase.dimensions.size(), currentCase.radius));
	}
}

TEST(CellularAutomataMooreNeighborhood, ReturnsDeterministicLexicographicOffsets) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Moore, Boundary_Closed>({5, 5}, {2, 2}, 1);

	EXPECT_EQ(numbers, (std::vector<long>{
		6, 11, 16,
		7,     17,
		8, 13, 18,
	}));
}

TEST(CellularAutomataMooreNeighborhood, KeepsFixedBoundaryPositionsAsFixedCells) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Moore, Boundary_Fixed>({3, 3}, {0, 0}, 1);

	EXPECT_EQ(numbers.size(), 8);
	EXPECT_EQ(std::count(numbers.begin(), numbers.end(), -99), 5);
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 1), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 3), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 4), numbers.end());
}

TEST(CellularAutomataVonNeumannNeighborhood, CountsInternalNeighborsForMultipleDimensionsAndRadii) {
	struct Case {
		std::vector<unsigned short> dimensions;
		std::vector<int> position;
		unsigned short radius;
	};

	const std::vector<Case> cases = {
		{{5}, {2}, 1},
		{{5, 5}, {2, 2}, 1},
		{{7, 7}, {3, 3}, 2},
		{{5, 5, 5}, {2, 2, 2}, 1},
		{{7, 7, 7}, {3, 3, 3}, 2},
	};

	for (const Case& currentCase : cases) {
		const auto numbers = buildNeighborNumbers<Neighborhood_VonNeumann, Boundary_Closed>(
			currentCase.dimensions,
			currentCase.position,
			currentCase.radius
		);
		EXPECT_EQ(numbers.size(), expectedVonNeumannCount(currentCase.dimensions.size(), currentCase.radius));
	}
}

TEST(CellularAutomataVonNeumannNeighborhood, ReturnsDeterministicLexicographicOffsets) {
	const auto numbers = buildNeighborNumbers<Neighborhood_VonNeumann, Boundary_Closed>({5, 5}, {2, 2}, 2);

	EXPECT_EQ(numbers, (std::vector<long>{
		10,
		6, 11, 16,
		2, 7, 17, 22,
		8, 13, 18,
		14,
	}));
}

TEST(CellularAutomataVonNeumannNeighborhood, KeepsFixedBoundaryPositionsAsFixedCells) {
	const auto numbers = buildNeighborNumbers<Neighborhood_VonNeumann, Boundary_Fixed>({3, 3}, {0, 0}, 1);

	EXPECT_EQ(numbers.size(), 4);
	EXPECT_EQ(std::count(numbers.begin(), numbers.end(), -99), 2);
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 1), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 3), numbers.end());
}

TEST(CellularAutomataGameOfLife, KeepsTwoDimensionalMooreBlinkerRegression) {
	CellularAutomata_Classic automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore neighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife localRule(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

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
// Non-uniform rule tests
// ---------------------------------------------------------------------------

namespace {

// Minimal local rules for non-uniform CA tests
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

	// All cells start at 0 (default); one step should produce [1, 0, 1, 0, 1]
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
// Non-uniform neighborhood tests
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
// Fully non-uniform CA tests (combined rule + neighborhood)
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

// ---------------------------------------------------------------------------
// NonUniform: interaction proof — same rule, different neighborhood → different outcome
// ---------------------------------------------------------------------------

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

	// Three diagonal neighbors of {4,4}: only visible to Moore, not VonNeumann
	lattice.getCell({3, 3})->setCurrentState(State(1));
	lattice.getCell({5, 3})->setCurrentState(State(1));
	lattice.getCell({3, 5})->setCurrentState(State(1));

	// Three diagonal neighbors of {1,1}: demonstrates Moore birth in the same step
	lattice.getCell({0, 0})->setCurrentState(State(1));
	lattice.getCell({2, 0})->setCurrentState(State(1));
	lattice.getCell({0, 2})->setCurrentState(State(1));

	automaton.step();

	EXPECT_EQ(lattice.getCell({1, 1})->getCurrentState().getValue(), 1); // Moore: 3 diagonal live → born
	EXPECT_EQ(lattice.getCell({4, 4})->getCurrentState().getValue(), 0); // VonNeumann: 0 orthogonal live → stays dead
}

// ---------------------------------------------------------------------------
// CellularAutomataComp dispatch contract: only CellularAutomata_NonUniform
// accepts per-cell rules and neighborhoods; Classic does not.
// ---------------------------------------------------------------------------

TEST(CellularAutomataCompDispatch, NonUniformAcceptsPerCellApiClassicDoesNot) {
	// Verify via a base pointer that CellularAutomataComp::setCellLocalRule/setCellNeighborhood
	// correctly routes calls: NonUniform accepts both APIs, Classic rejects both.
	CellularAutomataBase* nonUniformBase = new CellularAutomata_NonUniform();
	CellularAutomataBase* classicBase    = new CellularAutomata_Classic();

	// Cast through base pointer — not known at compile time, so no spurious warning
	EXPECT_NE(dynamic_cast<CellularAutomata_NonUniform*>(nonUniformBase), nullptr);
	EXPECT_EQ(dynamic_cast<CellularAutomata_NonUniform*>(classicBase),    nullptr);

	delete nonUniformBase;
	delete classicBase;
}

// ---------------------------------------------------------------------------
// LocalRule_PermissiveLife test
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
// LocalRule_Custom (majority vote) test
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
