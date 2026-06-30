#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Hexagonal.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Network.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Triangular.h"
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

// ConvertsLinearCellNumbersAndNDimPositionsRoundTrip — the lattice indexes cells two ways: a flat number
// and an n-dim coordinate. On a 2x3x4x5 lattice it asserts number->position->number is the identity for
// every cell (a bijection), spot-checks a few known mappings, and that out-of-range coords map to -1.
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

// CountsInternalNeighborsForMultipleDimensionsAndRadii (Moore) — for an interior cell the Moore
// neighborhood is the full (2r+1)^d box minus the cell itself. Drives several dimension/radius cases and
// checks the neighbor count matches that closed form, so the generator is correct beyond hand-picked sizes.
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

// ReturnsDeterministicLexicographicOffsets (Moore) — neighbor ordering matters because user rules index
// neighbors[i] by position. Pins the exact cell-number list a center cell on a 5x5 lattice returns,
// locking in the deterministic lexicographic (row-major) offset order.
TEST(CellularAutomataMooreNeighborhood, ReturnsDeterministicLexicographicOffsets) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Moore, Boundary_Closed>({5, 5}, {2, 2}, 1);

	EXPECT_EQ(numbers, (std::vector<long>{
		6, 11, 16,
		7,     17,
		8, 13, 18,
	}));
}

// KeepsFixedBoundaryPositionsAsFixedCells (Moore) — at a corner cell, 5 of the 8 Moore neighbors fall off
// the lattice. Under a Fixed boundary those become the sentinel cell number -99 while the 3 in-bounds
// neighbors keep their real numbers, so the slot count stays 8 and edge handling is explicit.
TEST(CellularAutomataMooreNeighborhood, KeepsFixedBoundaryPositionsAsFixedCells) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Moore, Boundary_Fixed>({3, 3}, {0, 0}, 1);

	EXPECT_EQ(numbers.size(), 8);
	EXPECT_EQ(std::count(numbers.begin(), numbers.end(), -99), 5);
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 1), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 3), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 4), numbers.end());
}

// CountsInternalNeighborsForMultipleDimensionsAndRadii (VonNeumann) — the VonNeumann neighborhood is every
// cell within Manhattan distance r (a diamond, not a box). Compares the generator's count against a
// reference brute-force enumeration across several dimension/radius cases.
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

// ReturnsDeterministicLexicographicOffsets (VonNeumann) — pins the exact ordered neighbor list for a
// radius-2 VonNeumann diamond on a 5x5 lattice, locking the deterministic traversal order user rules rely on.
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

// KeepsFixedBoundaryPositionsAsFixedCells (VonNeumann) — corner-cell counterpart of the Moore boundary
// test: of the 4 VonNeumann neighbors at {0,0}, the 2 off-lattice ones become sentinel -99 and the 2
// in-bounds ones keep real numbers under a Fixed boundary.
TEST(CellularAutomataVonNeumannNeighborhood, KeepsFixedBoundaryPositionsAsFixedCells) {
	const auto numbers = buildNeighborNumbers<Neighborhood_VonNeumann, Boundary_Fixed>({3, 3}, {0, 0}, 1);

	EXPECT_EQ(numbers.size(), 4);
	EXPECT_EQ(std::count(numbers.begin(), numbers.end(), -99), 2);
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 1), numbers.end());
	EXPECT_NE(std::find(numbers.begin(), numbers.end(), 3), numbers.end());
}

// ReturnsThreeNeighborsWithAlternatingOrientation (Triangular) — on a triangular lattice each cell has
// exactly 3 neighbors, and the set depends on whether the cell is an up- or down-pointing triangle. Pins
// the ordered neighbor numbers for an interior cell to lock that orientation-dependent topology.
TEST(CellularAutomataTriangularNeighborhood, ReturnsThreeNeighborsWithAlternatingOrientation) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Triangular, Boundary_Closed>({3, 3}, {1, 1}, 1);

	EXPECT_EQ(numbers, (std::vector<long>{3, 5, 1}));
}

// AppliesClosedAndFixedBoundaries (Triangular) — repeats the triangular neighbor lookup at a corner under
// both boundaries: Closed wraps the missing neighbors around (real numbers), Fixed replaces them with the
// -99 sentinel. Confirms boundary handling works for the special triangular topology, not just rectangular.
TEST(CellularAutomataTriangularNeighborhood, AppliesClosedAndFixedBoundaries) {
	const auto closed = buildNeighborNumbers<Neighborhood_Triangular, Boundary_Closed>({3, 3}, {0, 0}, 1);
	EXPECT_EQ(closed, (std::vector<long>{2, 1, 6}));

	const auto fixed = buildNeighborNumbers<Neighborhood_Triangular, Boundary_Fixed>({3, 3}, {0, 0}, 1);
	EXPECT_EQ(fixed.size(), 3);
	EXPECT_EQ(std::count(fixed.begin(), fixed.end(), -99), 2);
	EXPECT_NE(std::find(fixed.begin(), fixed.end(), 1), fixed.end());
}

// ReturnsSixOffsetNeighbors (Hexagonal) — on a hexagonal lattice each cell has 6 neighbors. Pins the exact
// ordered neighbor numbers for an interior cell, locking the hex offset pattern.
TEST(CellularAutomataHexagonalNeighborhood, ReturnsSixOffsetNeighbors) {
	const auto numbers = buildNeighborNumbers<Neighborhood_Hexagonal, Boundary_Closed>({3, 3}, {1, 1}, 1);

	EXPECT_EQ(numbers, (std::vector<long>{6, 7, 8, 5, 1, 3}));
}

// AppliesClosedAndFixedBoundaries (Hexagonal) — corner hex cell under both boundaries: Closed wraps all 6
// neighbors to real cells, Fixed turns the 4 off-lattice ones into the -99 sentinel. Validates boundary
// behavior for the hexagonal topology.
TEST(CellularAutomataHexagonalNeighborhood, AppliesClosedAndFixedBoundaries) {
	const auto closed = buildNeighborNumbers<Neighborhood_Hexagonal, Boundary_Closed>({3, 3}, {0, 0}, 1);
	EXPECT_EQ(closed, (std::vector<long>{2, 3, 1, 7, 6, 8}));

	const auto fixed = buildNeighborNumbers<Neighborhood_Hexagonal, Boundary_Fixed>({3, 3}, {0, 0}, 1);
	EXPECT_EQ(fixed.size(), 6);
	EXPECT_EQ(std::count(fixed.begin(), fixed.end(), -99), 4);
	EXPECT_NE(std::find(fixed.begin(), fixed.end(), 1), fixed.end());
	EXPECT_NE(std::find(fixed.begin(), fixed.end(), 3), fixed.end());
}

// UsesConfiguredEdgesAsTopology (Network) — a NETWORK lattice has no geometry; adjacency comes from an
// explicit edge list. With edges {0-1,0-2,2-3} it asserts node 0's neighbors are {1,2} and that the
// undirected edge is visible from the other side too (node 1 sees node 0).
TEST(CellularAutomataNetworkNeighborhood, UsesConfiguredEdgesAsTopology) {
	CellularAutomata_Classic automaton;
	Boundary_Fixed boundary;
	Neighborhood_Network neighborhood(&automaton, 1, &boundary);
	Lattice lattice(&automaton, nullptr, {4}, LatticeType::NETWORK);
	lattice.setNetworkEdges({{0, 1}, {0, 2}, {2, 3}});

	ASSERT_TRUE(automaton.init());
	std::vector<Cell*> neighbors = neighborhood.getNeighbors(lattice.getCell(0L));
	std::vector<long> numbers;
	for (Cell* neighbor : neighbors)
		numbers.emplace_back(neighbor->getCellNumber());

	EXPECT_EQ(numbers, (std::vector<long>{1, 2}));
	EXPECT_EQ(lattice.getNetworkNeighborCellNumbers(1), (std::vector<unsigned long>{0}));
}

// SupportsDirectedEdges (Network) — when edges are declared directed (undirected=false), adjacency is
// one-way. With 0->1 and 0->2 it asserts node 0 reaches {1,2} but node 1 has no outgoing neighbors.
TEST(CellularAutomataNetworkNeighborhood, SupportsDirectedEdges) {
	CellularAutomata_Classic automaton;
	Boundary_Fixed boundary;
	Neighborhood_Network neighborhood(&automaton, 1, &boundary);
	Lattice lattice(&automaton, nullptr, {4}, LatticeType::NETWORK);
	lattice.setNetworkEdges({{0, 1}, {0, 2}}, false);

	ASSERT_TRUE(automaton.init());
	EXPECT_EQ(lattice.getNetworkNeighborCellNumbers(0), (std::vector<unsigned long>{1, 2}));
	EXPECT_TRUE(lattice.getNetworkNeighborCellNumbers(1).empty());
}

// KeepsTwoDimensionalMooreBlinkerRegression (GameOfLife) — regression guard wiring the real components
// (Closed boundary, Moore neighborhood, built-in GoL rule) on a 5x5 lattice. Seeds a horizontal blinker
// and asserts that after one step it is vertical, catching any regression in the assembled step pipeline.
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
