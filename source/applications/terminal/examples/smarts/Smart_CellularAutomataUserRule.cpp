/*
 * File:   Smart_CellularAutomataUserRule.cpp
 *
 * Tema 6, line B: the cellular automaton's local rule is written by the USER as C++
 * source and compiled at runtime by the component
 * (CellularAutomataComp -> LocalRule_UserDefined -> CppCompiler -> dlopen/dlsym).
 *
 * Shows how a user drives GenESyS to see any rule: just change the rule string.
 * Here it runs rule 90 (Sierpinski triangle) and rule 30 (chaotic).
 *
 * Optionally, set the environment variable CA_RULE to the body of a rule to run only
 * that rule, without recompiling the simulator (only the rule is compiled at runtime):
 *     CA_RULE="return n[0] ^ n[1];" ./genesys_terminal_application
 */

#include "Smart_CellularAutomataUserRule.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Wraps the BODY of a rule (statements that return a long) into the full extern "C"
// nextState signature, so the user can write just the logic, e.g. "return n[0] ^ n[1];".
std::string WrapBody(const std::string& body) {
	return "extern \"C\" long nextState(long self, const long* n, int k) {"
	       " (void) self; (void) n; (void) k; " + body + " }";
}

// Renders a row of cells in a readable way: 0 -> '.', 1 -> '#'.
std::string Render(const std::string& cells) {
	std::string out;
	for (char c : cells) out += (c == '0' ? '.' : '#');
	return out;
}

// Runs a 1D cellular automaton whose local rule was written by the user as C++ source.
//   ruleName   : label for printing
//   ruleSource : full user-defined C++ source (defines nextState)
//   width/steps: lattice size and number of steps
void RunUserRule(Model* model, const std::string& ruleName, const std::string& ruleSource,
		unsigned short width, unsigned int steps) {
	CellularAutomataComp* ca = new CellularAutomataComp(model);

	// --- automaton configuration, exactly as a user would do through the public API ---
	ca->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	ca->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	ca->getlattice()->setDimensions({width});                 // 1D, `width` cells
	ca->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	ca->getNeighboorhood()->setRadius(1);                     // neighbors = {left, right}
	ca->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);   // outside the lattice is 0
	ca->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);

	// --- the user-defined rule (line B): the transition written by the user in C++ ---
	ca->setUserDefinedRuleSource(ruleSource);
	ca->setLocalRuleType(CellularAutomataComp::LocalRuleType::USERDEFINED);

	// initialize calls _check, which COMPILES the user rule at runtime.
	std::string errorMessage;
	if (!ca->initializeCellularAutomata(&errorMessage)) {
		std::cout << ruleName << ": initialization failed (rule did not compile) -> "
		          << errorMessage << std::endl;
		return;
	}

	// Initial state: a single cell turned on at the center.
	ca->setCellState(static_cast<long>(width / 2), 1);

	std::cout << "=== " << ruleName << " ===" << std::endl;
	std::cout << "user rule (C++): " << ruleSource << std::endl;
	std::cout << "1D lattice, " << width << " cells, centered radius-1 neighborhood, fixed boundary"
	          << std::endl << std::endl;

	const std::string t0 = ca->showCellularAutomata();
	std::cout << "t 0  " << Render(t0) << "   " << t0 << std::endl;
	for (unsigned int step = 1; step <= steps; ++step) {
		ca->stepCellularAutomata();                          // applies the user rule per cell
		const std::string row = ca->showCellularAutomata();
		std::cout << "t" << (step < 10 ? " " : "") << step << "  " << Render(row)
		          << "   " << row << std::endl;
	}
	std::cout << std::endl;
}

} // namespace

Smart_CellularAutomataUserRule::Smart_CellularAutomataUserRule() {
}

int Smart_CellularAutomataUserRule::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	std::cout << "Cellular automaton with a USER-DEFINED local rule (Tema 6, line B)" << std::endl;
	std::cout << "The rule is written in C++ and compiled at runtime by the component."
	          << std::endl << std::endl;

	// If the CA_RULE environment variable is set, run ONLY that rule (the user writes just the
	// body, e.g. CA_RULE="return n[0] & n[1];"). Useful for a live demo: changing the rule does
	// NOT recompile the simulator, only the rule is compiled at runtime.
	const char* envRule = std::getenv("CA_RULE");
	if (envRule != nullptr && envRule[0] != '\0') {
		RunUserRule(model, "User rule (CA_RULE)", WrapBody(envRule), 31, 15);
		delete genesys;
		return 0;
	}

	// Without CA_RULE: show two classic rules written by the user.
	// Rule 90: next = left XOR right  -> Sierpinski triangle.
	RunUserRule(model, "Rule 90 (Sierpinski)", WrapBody("return n[0] ^ n[1];"), 31, 15);

	// Rule 30: next = left XOR (center OR right)  -> chaotic pattern.
	RunUserRule(model, "Rule 30 (chaotic)", WrapBody("return n[0] ^ (self | n[1]);"), 31, 15);

	delete genesys;
	return 0;
}
