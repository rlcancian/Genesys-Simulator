#include "ContinuousDistribution_if.h"
#include "DataAnalyser_if.h"
#include "DataSet_if.h"
#include "DiscreteDistribution_if.h"
#include "Distribution_if.h"
#include "FactorialDesign/FactorialDesign.h"
#include "Fitter_if.h"
#include "HypothesisTester_if.h"
#include "OdeSolver_if.h"
#include "OdeSystem_if.h"
#include "Optimizer_if.h"
#include "Quadrature_if.h"
#include "RootFinder_if.h"
#include "Solver_if.h"
#include "TraitsTools.h"

#include <iostream>
#include <string_view>

namespace {
void printToolsOverview() {
	std::cout << "GenESyS tools interfaces\n"
	          << "  analysis: DataSet_if, DataAnalyser_if\n"
	          << "  fitting: Fitter_if -> TraitsTools<Fitter_if>::Implementation\n"
	          << "  hypothesis: HypothesisTester_if -> TraitsTools<HypothesisTester_if>::Implementation\n"
	          << "  distributions: Distribution_if, ContinuousDistribution_if, DiscreteDistribution_if\n"
	          << "  numerics: Solver_if, Quadrature_if, RootFinder_if, OdeSystem_if, OdeSolver_if\n"
	          << "  optimization: Optimizer_if\n"
	          << "  design: FactorialDesign\n";
}
}

int main(int argc, char* argv[]) {
	const std::string_view command = argc > 1 ? argv[1] : "--list";
	if (command == "--list" || command == "--help") {
		printToolsOverview();
		return 0;
	}

	std::cerr << "Unknown tools command: " << command << "\n";
	std::cerr << "Use --list to print the available tools interfaces.\n";
	return 1;
}
