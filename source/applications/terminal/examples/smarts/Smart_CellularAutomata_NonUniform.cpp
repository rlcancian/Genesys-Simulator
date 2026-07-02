#include "Smart_CellularAutomata_NonUniform.h"

#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformRule.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformNeighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Custom.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_PermissiveLife.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"

#include <iostream>
#include <vector>

static void printLattice(Lattice* lattice, int cols, int rows) {
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			long v = lattice->getCell({x, y})->getCurrentState().getValue();
			std::cout << (v ? '#' : '.');
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Scenario 1: NONUNIFORMRULE
//   10x10 lattice, Moore r=1, Closed boundary.
//   Default rule: Game of Life.
//   Left column (x=0): LocalRule_Custom (majority vote).
// ---------------------------------------------------------------------------
static void runScenario1() {
	std::cout << "=== Scenario 1: Non-Uniform Rule (GoL interior / Majority left column) ===\n";

	CellularAutomata_NonUniformRule automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_Custom customRule(&automaton);    // registers → global = customRule
	LocalRule_GameOfLife golRule(&automaton);   // registers → global = golRule (fallback)
	Lattice lattice(&automaton, nullptr, {10, 10});

	for (int y = 0; y < 10; ++y)
		automaton.setCellRule({0, y}, &customRule);

	automaton.init();

	// Seed a glider in the interior
	lattice.getCell({4, 4})->setCurrentState(State(1));
	lattice.getCell({5, 4})->setCurrentState(State(1));
	lattice.getCell({6, 4})->setCurrentState(State(1));
	lattice.getCell({5, 3})->setCurrentState(State(1));
	lattice.getCell({4, 5})->setCurrentState(State(1));
	// Seed the left column with alternating states so the custom rule has something to work with
	for (int y = 0; y < 10; y += 2)
		lattice.getCell({0, y})->setCurrentState(State(1));

	std::cout << "Initial state:\n";
	printLattice(&lattice, 10, 10);

	for (int step = 0; step < 3; ++step) {
		automaton.step();
		std::cout << "After step " << (step + 1) << ":\n";
		printLattice(&lattice, 10, 10);
	}
}

// ---------------------------------------------------------------------------
// Scenario 2: NONUNIFORMNEIGHBOOR
//   10x10 lattice, Moore r=1 default, Game of Life rule.
//   Central column (x=5) uses VonNeumann r=1.
// ---------------------------------------------------------------------------
static void runScenario2() {
	std::cout << "=== Scenario 2: Non-Uniform Neighborhood (Moore default / VonNeumann center column) ===\n";

	CellularAutomata_NonUniformNeighborhood automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_GameOfLife rule(&automaton);
	Lattice lattice(&automaton, nullptr, {10, 10});

	std::vector<Neighborhood_VonNeumann*> centralColumnNeighborhoods;
	for (int y = 0; y < 10; ++y) {
		auto* vn = new Neighborhood_VonNeumann(&automaton, 1, &boundary, false, false);
		centralColumnNeighborhoods.push_back(vn);
		automaton.setCellNeighborhood({5, y}, vn);
	}

	automaton.init();

	lattice.getCell({4, 4})->setCurrentState(State(1));
	lattice.getCell({5, 4})->setCurrentState(State(1));
	lattice.getCell({6, 4})->setCurrentState(State(1));
	lattice.getCell({5, 3})->setCurrentState(State(1));
	lattice.getCell({4, 5})->setCurrentState(State(1));

	std::cout << "Initial state:\n";
	printLattice(&lattice, 10, 10);

	for (int step = 0; step < 5; ++step) {
		automaton.step();
		std::cout << "After step " << (step + 1) << ":\n";
		printLattice(&lattice, 10, 10);
	}

	for (auto* vn : centralColumnNeighborhoods)
		delete vn;
}

// ---------------------------------------------------------------------------
// Scenario 3: NONUNIFORM (combined rule + neighborhood)
//   10x10 lattice. Top row: VonNeumann + Custom (majority). Rest: Moore + GoL.
// ---------------------------------------------------------------------------
static void runScenario3() {
	std::cout << "=== Scenario 3: Fully Non-Uniform (top row: VonNeumann + Majority / rest: Moore + GoL) ===\n";

	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);
	LocalRule_Custom customRule(&automaton);   // registers → global = customRule
	LocalRule_GameOfLife golRule(&automaton);  // registers → global = golRule (fallback)
	Lattice lattice(&automaton, nullptr, {10, 10});

	std::vector<Neighborhood_VonNeumann*> topRowNeighborhoods;
	for (int x = 0; x < 10; ++x) {
		auto* vn = new Neighborhood_VonNeumann(&automaton, 1, &boundary, false, false);
		topRowNeighborhoods.push_back(vn);
		automaton.setCellNeighborhood({x, 0}, vn);
		automaton.setCellRule({x, 0}, &customRule);
	}

	automaton.init();

	for (int x = 0; x < 10; ++x)
		lattice.getCell({x, 0})->setCurrentState(State(1));
	lattice.getCell({4, 4})->setCurrentState(State(1));
	lattice.getCell({5, 4})->setCurrentState(State(1));
	lattice.getCell({6, 4})->setCurrentState(State(1));

	std::cout << "Initial state:\n";
	printLattice(&lattice, 10, 10);

	for (int step = 0; step < 3; ++step) {
		automaton.step();
		std::cout << "After step " << (step + 1) << ":\n";
		printLattice(&lattice, 10, 10);
	}

	for (auto* vn : topRowNeighborhoods)
		delete vn;
}

// ---------------------------------------------------------------------------
// Scenario 4: Ecological Zones (NONUNIFORM combined)
//   20x20 lattice, Closed boundary.
//   Left zone (x < 10): Moore r=1 + Game of Life (global defaults).
//   Right zone (x >= 10): VonNeumann r=1 + PermissiveLife (survives 1-4, born 2-3).
//
//   The zone boundary creates visible emergent behaviour:
//   - A GoL glider seeded in the left zone evolves normally until its cells cross x=10,
//     where the PermissiveLife rule takes over and the pattern spreads differently.
//   - An L-shape seeded in the right zone grows and stabilises under PermissiveLife,
//     forming a structure that would quickly die under GoL.
//   - The zone boundary (printed as '|') is a visual guide only; cells on either side
//     interact freely — their neighbours are determined by their own neighbourhood type.
// ---------------------------------------------------------------------------
static void printLatticeZoned(Lattice* lattice, int cols, int rows, int zoneX) {
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			if (x == zoneX)
				std::cout << '|';
			long v = lattice->getCell({x, y})->getCurrentState().getValue();
			std::cout << (v ? '#' : '.');
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

static void runScenario4() {
	std::cout << "=== Scenario 4: Ecological Zones (GoL left | PermissiveLife right) ===\n";
	std::cout << "    Left of '|': Moore + Game of Life\n";
	std::cout << "    Right of '|': VonNeumann + PermissiveLife (survives 1-4, born 2-3)\n\n";

	const int cols = 20, rows = 20, zoneX = 10;

	CellularAutomata_NonUniform automaton;
	Boundary_Closed boundary;
	Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
	LocalRule_PermissiveLife permissiveRule(&automaton); // global = permissive (temporarily)
	LocalRule_GameOfLife golRule(&automaton);            // global = GoL (left-zone fallback)
	Lattice lattice(&automaton, nullptr, {cols, rows});

	std::vector<Neighborhood_VonNeumann*> rightNeighborhoods;
	for (int y = 0; y < rows; ++y) {
		for (int x = zoneX; x < cols; ++x) {
			auto* vn = new Neighborhood_VonNeumann(&automaton, 1, &boundary, false, false);
			rightNeighborhoods.push_back(vn);
			automaton.setCellNeighborhood({x, y}, vn);
			automaton.setCellRule({x, y}, &permissiveRule);
		}
	}

	automaton.init();

	// Left zone: classic GoL glider (moves right toward the boundary)
	lattice.getCell({6, 7})->setCurrentState(State(1));
	lattice.getCell({7, 8})->setCurrentState(State(1));
	lattice.getCell({5, 9})->setCurrentState(State(1));
	lattice.getCell({6, 9})->setCurrentState(State(1));
	lattice.getCell({7, 9})->setCurrentState(State(1));

	// Right zone: L-shape that spreads under PermissiveLife but would die under GoL
	lattice.getCell({13, 8})->setCurrentState(State(1));
	lattice.getCell({14, 8})->setCurrentState(State(1));
	lattice.getCell({13, 9})->setCurrentState(State(1));

	std::cout << "Initial state:\n";
	printLatticeZoned(&lattice, cols, rows, zoneX);

	for (int step = 0; step < 10; ++step) {
		automaton.step();
		std::cout << "After step " << (step + 1) << ":\n";
		printLatticeZoned(&lattice, cols, rows, zoneX);
	}

	for (auto* vn : rightNeighborhoods)
		delete vn;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int Smart_CellularAutomata_NonUniform::main(int argc, char** argv) {
	runScenario1();
	runScenario2();
	runScenario3();
	runScenario4();
	return 0;
}
