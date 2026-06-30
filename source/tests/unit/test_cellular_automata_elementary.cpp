// Verifies that the existing engine reproduces, bit-perfect, the textbook/Wolfram ground truth for
// elementary (1D, binary, radius-1) cellular automata. This is the deterministic "comparison to
// known analytical results / consistency" verification (book Cap. 11 §11.10.3 and Cap. 13).
//
// Ground truth:
//   Rule 30 (00011110, next = left XOR (center OR right)), single central 1, fixed-0 boundary, width 9
//     t0=000010000 t1=000111000 t2=001100100 t3=011011110   (book p.242 worked example)
//   Rule 90 (01011010, next = left XOR right -> Sierpinski), single central 1, fixed-0 boundary, width 11
//     t0=00000100000 t1=00001010000 t2=00010001000 t3=00101010100 t4=01000000010

#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

#include <cstdint>
#include <string>
#include <vector>

namespace {

std::string rowOf(Lattice& lattice) {
	std::string row;
	for (unsigned long cellNumber = 0; cellNumber < lattice.getCellsSize(); ++cellNumber) {
		row += std::to_string(lattice.getCell(static_cast<long>(cellNumber))->getCurrentState().getValue());
	}
	return row;
}

// Runs an elementary CA: 1D lattice of `width` cells, binary states, centered radius-1 neighborhood,
// fixed-0 boundary, a single 1 at the center. Returns the space-time rows t0..t(steps).
std::vector<std::string> runElementary(unsigned short width, std::uint8_t ruleNumber, unsigned int steps) {
	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {width}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	LocalRule_Elementary rule(&cellularAutomata, ruleNumber, &stateSet);

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);

	lattice.init();
	State centerOn(1);
	lattice.setCellState(static_cast<long>(width / 2), &centerOn);

	std::vector<std::string> rows;
	rows.emplace_back(rowOf(lattice));
	for (unsigned int t = 0; t < steps; ++t) {
		cellularAutomata.step();
		rows.emplace_back(rowOf(lattice));
	}
	return rows;
}

} // namespace

// Rule30MatchesTextbookGroundTruth — runs elementary Rule 30 from a single central 1 on a width-9 lattice
// and asserts rows t0..t3 equal the worked example from the textbook (Cap. 11), bit for bit.
TEST(ElementaryCA, Rule30MatchesTextbookGroundTruth) {
	const std::vector<std::string> rows = runElementary(9, 30, 3);
	ASSERT_EQ(rows.size(), 4u);
	EXPECT_EQ(rows[0], "000010000");
	EXPECT_EQ(rows[1], "000111000");
	EXPECT_EQ(rows[2], "001100100");
	EXPECT_EQ(rows[3], "011011110");
}

// Rule90MatchesSierpinskiGroundTruth — runs Rule 90 (next = left XOR right) from a single central 1 on a
// width-11 lattice and asserts rows t0..t4 match the Sierpinski-triangle ground truth exactly.
TEST(ElementaryCA, Rule90MatchesSierpinskiGroundTruth) {
	const std::vector<std::string> rows = runElementary(11, 90, 4);
	ASSERT_EQ(rows.size(), 5u);
	EXPECT_EQ(rows[0], "00000100000");
	EXPECT_EQ(rows[1], "00001010000");
	EXPECT_EQ(rows[2], "00010001000");
	EXPECT_EQ(rows[3], "00101010100");
	EXPECT_EQ(rows[4], "01000000010");
}

// IsDeterministicAcrossRuns — two independent Rule 30 runs with identical setup must produce identical
// histories, confirming the engine has no hidden nondeterminism (uninitialized state, ordering, etc.).
TEST(ElementaryCA, IsDeterministicAcrossRuns) {
	EXPECT_EQ(runElementary(11, 30, 6), runElementary(11, 30, 6));
}

// EmptyGridStaysEmptyUnderRule30 — degeneracy/quiescence check: with no live cell, Rule 30 (000 -> 0) must
// leave the lattice all-zero after several steps, verifying the rule has the expected stable empty state.
TEST(ElementaryCA, EmptyGridStaysEmptyUnderRule30) {
	// Degeneracy test: with no live cell, rule 30 (000 -> 0) keeps the lattice all-zero.
	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {9}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	LocalRule_Elementary rule(&cellularAutomata, 30, &stateSet);
	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);
	lattice.init();
	for (int t = 0; t < 5; ++t) {
		cellularAutomata.step();
	}
	EXPECT_EQ(rowOf(lattice), "000000000");
}
