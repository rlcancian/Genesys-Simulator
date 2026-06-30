// Verifies the user-defined local rule (Tema 6, Linha B): a rule supplied as C++ source, compiled at
// runtime via CppCompiler, loaded, and invoked per cell. Correctness is checked bit-perfect against
// the same theoretical ground truth used for the built-in engine (Wolfram rules 30/90, Game of Life),
// plus an equivalence check user-rule == built-in preset, plus graceful failure on bad user code.

#include <gtest/gtest.h>

#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_UserDefined.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"

#include "kernel/simulator/Simulator.h"

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

CppCompiler* makeCompiler(Simulator& simulator) {
	Model* model = simulator.getModelManager()->newModel();
	CppCompiler* compiler = new CppCompiler(model, "userRuleCompiler");
	compiler->setOutputDir("/tmp/");
	compiler->setTempDir("/tmp/");
	compiler->setFlagsGeneral("-w -std=c++14"); // drop the default -v verbose noise
	return compiler;
}

// Runs a 1D elementary CA driven by a user-defined rule built from `ruleBody` (a function body wrapped
// into the nextState signature via buildBody). Single central 1, fixed-0 boundary. Returns the
// space-time rows t0..t(steps); empty vector if the build failed.
std::vector<std::string> runUserElementary(Simulator& simulator, unsigned short width,
		const std::string& ruleBody, unsigned int steps, std::string& errorMessage) {
	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {width}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	CppCompiler* compiler = makeCompiler(simulator);
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);
	if (!rule.buildBody(ruleBody, errorMessage)) {
		return {};
	}

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

// Rule30FromUserSourceMatchesTextbook — a user-supplied Rule 30 body, compiled at runtime, must reproduce
// the same textbook ground truth as the built-in engine (rows t0..t3 on width 9).
TEST(UserDefinedCA, Rule30FromUserSourceMatchesTextbook) {
	Simulator simulator;
	std::string error;
	// Rule 30: next = left XOR (center OR right). Neighbors order for 1D centered radius-1 = {left,right}.
	const std::string source = "return neighbors[0] ^ (self | neighbors[1]);";
	const std::vector<std::string> rows = runUserElementary(simulator, 9, source, 3, error);
	ASSERT_EQ(rows.size(), 4u) << "build/compile failed: " << error;
	EXPECT_EQ(rows[0], "000010000");
	EXPECT_EQ(rows[1], "000111000");
	EXPECT_EQ(rows[2], "001100100");
	EXPECT_EQ(rows[3], "011011110");
}

// Rule90FromUserSourceMatchesSierpinski — a user-supplied Rule 90 body must reproduce the Sierpinski
// ground truth (rows t0..t4 on width 11), proving runtime-compiled rules match the analytical result.
TEST(UserDefinedCA, Rule90FromUserSourceMatchesSierpinski) {
	Simulator simulator;
	std::string error;
	// Rule 90: next = left XOR right (center-independent) -> Sierpinski triangle.
	const std::string source = "return neighbors[0] ^ neighbors[1];";
	const std::vector<std::string> rows = runUserElementary(simulator, 11, source, 4, error);
	ASSERT_EQ(rows.size(), 5u) << "build/compile failed: " << error;
	EXPECT_EQ(rows[0], "00000100000");
	EXPECT_EQ(rows[1], "00001010000");
	EXPECT_EQ(rows[2], "00010001000");
	EXPECT_EQ(rows[3], "00101010100");
	EXPECT_EQ(rows[4], "01000000010");
}

// UserRule90EqualsBuiltInElementary90 — strong equivalence check: the user rule and the built-in
// LocalRule_Elementary(90) must produce byte-identical histories over a wide lattice and many steps.
TEST(UserDefinedCA, UserRule90EqualsBuiltInElementary90) {
	// The user-defined rule must produce identical output to the built-in LocalRule_Elementary(90)
	// over many steps and a wider lattice (strong equivalence / consistency check).
	Simulator simulator;
	std::string error;
	const unsigned short width = 31;
	const unsigned int steps = 15;

	const std::vector<std::string> userRows = runUserElementary(simulator, width,
		"return neighbors[0] ^ neighbors[1];", steps, error);
	ASSERT_EQ(userRows.size(), steps + 1u) << "build/compile failed: " << error;

	// Built-in reference run, same configuration.
	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {width}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	LocalRule_Elementary builtIn(&cellularAutomata, 90, &stateSet);
	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&builtIn);
	lattice.init();
	State centerOn(1);
	lattice.setCellState(static_cast<long>(width / 2), &centerOn);
	std::vector<std::string> builtInRows;
	builtInRows.emplace_back(rowOf(lattice));
	for (unsigned int t = 0; t < steps; ++t) {
		cellularAutomata.step();
		builtInRows.emplace_back(rowOf(lattice));
	}

	EXPECT_EQ(userRows, builtInRows);
}

// BadUserCodeFailsGracefully — feeding the compiler invalid C++ must return false, leave the rule
// not-ready, and report a non-empty error instead of crashing the simulator.
TEST(UserDefinedCA, BadUserCodeFailsGracefully) {
	// A compilation error in the user's rule must be reported, not crash, and leave the rule not-ready.
	Simulator simulator;
	CppCompiler* compiler = makeCompiler(simulator);
	CellularAutomata_Classic cellularAutomata;
	StateSet_Enumerable stateSet(&cellularAutomata, {});
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);

	std::string error;
	const bool ok = rule.build("this is not valid c++ code <<<", error);
	EXPECT_FALSE(ok);
	EXPECT_FALSE(rule.isReady());
	EXPECT_FALSE(error.empty());
}

// GameOfLifeBlinkerOscillatesViaUserRule — Game of Life (B3/S23) written as a multi-statement user rule
// over a 2D Moore neighborhood; a horizontal blinker must flip vertical after one step and back after two.
TEST(UserDefinedCA, GameOfLifeBlinkerOscillatesViaUserRule) {
	// 2D Game of Life (B3/S23) expressed as a user rule over a Moore neighborhood. A horizontal
	// blinker (3 cells) must become vertical after one step and horizontal again after two (period 2).
	Simulator simulator;
	std::string error;
	const std::string gol =
		"long live = 0;"
		"for (int i = 0; i < numNeighbors; ++i) live += neighbors[i];"
		"if (self != 0) return (live == 2 || live == 3) ? 1 : 0;"
		"return (live == 3) ? 1 : 0;";

	CellularAutomata_Classic cellularAutomata;
	const unsigned short side = 5;
	Lattice lattice(&cellularAutomata, nullptr, {side, side}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Moore neighborhood(&cellularAutomata, 1, &boundary);
	CppCompiler* compiler = makeCompiler(simulator);
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);
	ASSERT_TRUE(rule.buildBody(gol, error)) << error;

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);
	lattice.init();

	// Horizontal blinker centered at row 2, columns 1..3 (cell number = col + side*row).
	State on(1);
	lattice.setCellState(static_cast<long>(1 + side * 2), &on);
	lattice.setCellState(static_cast<long>(2 + side * 2), &on);
	lattice.setCellState(static_cast<long>(3 + side * 2), &on);

	const std::string horizontal = rowOf(lattice);
	cellularAutomata.step();
	const std::string vertical = rowOf(lattice);
	cellularAutomata.step();
	const std::string horizontalAgain = rowOf(lattice);

	EXPECT_NE(horizontal, vertical) << "blinker should change shape after one step";
	EXPECT_EQ(horizontal, horizontalAgain) << "blinker should return to its shape after two steps (period 2)";

	// Explicit: after one step the live cells are the vertical triple (col 2, rows 1..3).
	std::string expectedVertical(side * side, '0');
	expectedVertical[2 + side * 1] = '1';
	expectedVertical[2 + side * 2] = '1';
	expectedVertical[2 + side * 3] = '1';
	EXPECT_EQ(vertical, expectedVertical);
}

// ClosedBoundaryWrapsAroundTorus — exercises the Closed (periodic) boundary with a "copy left neighbor"
// user rule: a single live cell on a width-5 ring shifts right each step and returns to its start after 5.
TEST(UserDefinedCA, ClosedBoundaryWrapsAroundTorus) {
	// Exercises the Boundary_Closed (periodic) condition, distinct from the Fixed boundary used above.
	// A "copy your left neighbor" rule (next = neighbors[0]) shifts the pattern one cell to the right
	// each step. On a closed 1D ring a single live cell must travel around and return to its start
	// after `width` steps; under a fixed boundary it would instead fall off the edge and vanish.
	Simulator simulator;
	std::string error;
	const unsigned short width = 5;

	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {width}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Closed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	CppCompiler* compiler = makeCompiler(simulator);
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);
	ASSERT_TRUE(rule.buildBody("return neighbors[0];", error)) << error; // next = left neighbor's state

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);
	lattice.init();
	State on(1);
	lattice.setCellState(2, &on);

	EXPECT_EQ(rowOf(lattice), "00100");
	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "00010");
	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "00001"); // at the right edge
	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "10000") << "closed boundary must wrap the live cell around the ring";
	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "01000");
	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "00100") << "after `width` steps the pattern must return to its start";
}

// GameOfLifeGliderTranslatesViaUserRule — the classic GoL glider, run via the user rule on a 10x10 grid,
// must reappear as the same shape shifted by (+1,+1) after 4 generations (backs the shipped evidence diagram).
TEST(UserDefinedCA, GameOfLifeGliderTranslatesViaUserRule) {
	// The classic Game of Life glider returns to its own shape shifted by (+1,+1) after 4 generations.
	// This test-backs the glider space-time diagram shipped in dcs/site/assets/evidence-diagrams.txt
	// (previously only the blinker was asserted) using the user rule over a Moore neighborhood.
	Simulator simulator;
	std::string error;
	const std::string gol =
		"long live = 0;"
		"for (int i = 0; i < numNeighbors; ++i) live += neighbors[i];"
		"if (self != 0) return (live == 2 || live == 3) ? 1 : 0;"
		"return (live == 3) ? 1 : 0;";

	CellularAutomata_Classic cellularAutomata;
	const unsigned short side = 10;
	Lattice lattice(&cellularAutomata, nullptr, {side, side}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Moore neighborhood(&cellularAutomata, 1, &boundary);
	CppCompiler* compiler = makeCompiler(simulator);
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);
	ASSERT_TRUE(rule.buildBody(gol, error)) << error;

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);
	lattice.init();

	// Glider placed in the interior (cell number = col + side*row), matching the evidence file's t0.
	auto setOn = [&](int row, int col) { State s(1); lattice.setCellState(col + side * row, &s); };
	setOn(1, 2);
	setOn(2, 3);
	setOn(3, 1);
	setOn(3, 2);
	setOn(3, 3);

	for (int t = 0; t < 4; ++t) {
		cellularAutomata.step();
	}

	// After 4 generations: same glider, shifted by (+1,+1) — matches the evidence file's t4.
	std::string expected(side * side, '0');
	auto mark = [&](int row, int col) { expected[col + side * row] = '1'; };
	mark(2, 3);
	mark(3, 4);
	mark(4, 2);
	mark(4, 3);
	mark(4, 4);
	EXPECT_EQ(rowOf(lattice), expected) << "glider must reappear shifted by (+1,+1) after 4 steps";
}

// ExtendedContractExposesCellPosition — the extended contract (nextStateEx) passes the cell's coordinates
// to user code; a rule returning the parity of column 0 must paint "010101" after one step, proving the
// position is actually delivered (independent of neighbor states).
TEST(UserDefinedCA, ExtendedContractExposesCellPosition) {
	// The extended user contract (nextStateEx) hands the rule the cell's n-dimensional position.
	// A rule returning the parity of the first coordinate must paint a 1D row 0,1,0,1,... after one
	// step regardless of neighbor states, proving the position is delivered to user code (tema 6 §6).
	Simulator simulator;
	std::string error;
	const unsigned short width = 6;

	CellularAutomata_Classic cellularAutomata;
	Lattice lattice(&cellularAutomata, nullptr, {width}, LatticeType::RETICULAR);
	State zero(0);
	State one(1);
	StateSet_Enumerable stateSet(&cellularAutomata, {&zero, &one});
	Boundary_Fixed boundary(&lattice);
	Neighborhood_Center neighborhood(&cellularAutomata, 1, &boundary);
	CppCompiler* compiler = makeCompiler(simulator);
	LocalRule_UserDefined rule(&cellularAutomata, compiler, &stateSet);
	const std::string source =
		"extern \"C\" long nextStateEx(long self, const long* neighbors, int numNeighbors, "
		"const int* position, int numDimensions) { (void) self; (void) neighbors; (void) numNeighbors; "
		"(void) numDimensions; return position[0] & 1; }";
	ASSERT_TRUE(rule.build(source, error)) << error;
	EXPECT_TRUE(rule.isReady());

	cellularAutomata.setLattice(&lattice);
	cellularAutomata.setStateSet(&stateSet);
	cellularAutomata.setNeighborhood(&neighborhood);
	cellularAutomata.setLocalRule(&rule);
	lattice.init();

	cellularAutomata.step();
	EXPECT_EQ(rowOf(lattice), "010101") << "extended rule must color cells by the parity of their column";
}
