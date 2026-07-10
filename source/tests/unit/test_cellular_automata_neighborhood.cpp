#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_HppLatticeGas.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

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

TEST(CellularAutomataStateSet, SupportsFiniteIntegerRealAndCompositeDomains) {
	State zero(0);
	State one(1);
	State two(2);
	State three(3);
	State four(4);
	StateSet_Enumerable binary(nullptr, {&zero, &one});
	StateSet_Enumerable fiveStates(nullptr, {&zero, &one, &two, &three, &four});

	EXPECT_TRUE(binary.contains(State(0)));
	EXPECT_TRUE(binary.contains(State(1)));
	EXPECT_FALSE(binary.contains(State(2)));
	EXPECT_EQ(binary.createDefaultState()->getValue(), 0);
	EXPECT_TRUE(binary.parseState("1")->equals(State(1)));
	EXPECT_EQ(fiveStates.enumerateStates().size(), 5);

	NaturalStateSet naturals(nullptr);
	EXPECT_TRUE(naturals.contains(State(42)));
	EXPECT_FALSE(naturals.contains(State(-1)));

	RealIntervalStateSet unitInterval(nullptr, 0.0, 1.0);
	EXPECT_TRUE(unitInterval.contains(RealState(0.5)));
	EXPECT_FALSE(unitInterval.contains(RealState(1.5)));
	EXPECT_EQ(unitInterval.parseState("0.25")->typeName(), "RealState");

	HppLatticeGasState hpp({{true, false, true, false}});
	HppLatticeGasStateSet hppSet(nullptr);
	EXPECT_TRUE(hppSet.contains(hpp));
	EXPECT_FALSE(hppSet.contains(State(5)));
	EXPECT_EQ(hppSet.enumerateStates().size(), 16);
	EXPECT_EQ(hpp.toString(), "{N:1,E:0,S:1,W:0}");

	CompositeState composite;
	composite.addField("num_particles", State(2));
	composite.addField("direction", State(1));
	CompositeStateSet compositeSet(nullptr);
	EXPECT_TRUE(compositeSet.contains(composite));
	EXPECT_NE(composite.clone().get(), &composite);
}

TEST(CellularAutomataCell, ClonesStatesAndValidatesAgainstStateSet) {
	State zero(0);
	State one(1);
	StateSet_Enumerable binary(nullptr, {&zero, &one});
	Cell cell(&binary);

	EXPECT_TRUE(cell.setCurrentState(one));
	EXPECT_EQ(cell.getCurrentState().getValue(), 1);

	State rejected(2);
	EXPECT_FALSE(cell.setCurrentState(rejected));
	EXPECT_EQ(cell.getCurrentState().getValue(), 1);

	EXPECT_TRUE(cell.setNextState(zero));
	EXPECT_TRUE(cell.isUpdatePending());
	EXPECT_TRUE(cell.updateState());
	EXPECT_EQ(cell.getCurrentState().getValue(), 0);

	HppLatticeGasState hpp({{true, false, false, false}});
	EXPECT_FALSE(cell.setCurrentState(hpp));
}

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

TEST(CellularAutomataHppLatticeGas, PropagatesAndCollidesCompositeParticleStates) {
	CellularAutomata_Classic automaton;
	HppLatticeGasStateSet stateSet(&automaton);
	Boundary_Closed boundary;
	Neighborhood_VonNeumann neighborhood(&automaton, 1, &boundary);
	LocalRule_HppLatticeGas localRule(&automaton);
	Lattice lattice(&automaton, nullptr, {5, 5});

	ASSERT_TRUE(automaton.init());
	lattice.getCell({2, 1})->setCurrentState(HppLatticeGasState({{false, false, true, false}}));
	lattice.getCell({2, 3})->setCurrentState(HppLatticeGasState({{true, false, false, false}}));

	automaton.step();

	const auto* center = lattice.getCell({2, 2})->getCurrentState().as<HppLatticeGasState>();
	ASSERT_NE(center, nullptr);
	EXPECT_FALSE(center->has(HppLatticeGasState::North));
	EXPECT_TRUE(center->has(HppLatticeGasState::East));
	EXPECT_FALSE(center->has(HppLatticeGasState::South));
	EXPECT_TRUE(center->has(HppLatticeGasState::West));
	EXPECT_TRUE(stateSet.contains(*center));
}
