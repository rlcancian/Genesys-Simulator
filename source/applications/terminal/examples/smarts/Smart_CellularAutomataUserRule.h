/*
 * File:   Smart_CellularAutomataUserRule.h
 *
 * Terminal example (Tema 6, line B): runs a cellular automaton whose local rule
 * is written by the USER as C++ source and compiled at runtime through
 * CellularAutomataComp (LocalRule_UserDefined + CppCompiler). Demonstrates how a
 * user drives GenESyS to see any rule (e.g. rule 90, rule 30) without recompiling
 * the simulator.
 */

#ifndef SMART_CELLULARAUTOMATAUSERRULE_H
#define SMART_CELLULARAUTOMATAUSERRULE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_CellularAutomataUserRule : public BaseGenesysTerminalApplication {
public:
	Smart_CellularAutomataUserRule();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_CELLULARAUTOMATAUSERRULE_H */
