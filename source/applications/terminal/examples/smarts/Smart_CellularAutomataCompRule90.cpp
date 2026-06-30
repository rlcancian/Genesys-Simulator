/*
 * File:   Smart_CellularAutomataCompRule90.cpp
 *
 * Minimal terminal example that runs elementary cellular automaton Rule 90
 * through the GenESyS CellularAutomataComp component.
 */

#include "Smart_CellularAutomataCompRule90.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

void SetInitialPattern(CellularAutomataComp* cellularAutomata, const std::string& pattern) {
	for (unsigned long cellNumber = 0; cellNumber < pattern.size(); ++cellNumber) {
		const long value = pattern.at(cellNumber) == '1' ? 1 : 0;
		cellularAutomata->setCellState(static_cast<long>(cellNumber), value);
	}
}

void PrintNeighborhoodCount(Model* model, CellularAutomataComp::NeighboorhoodType neighborhoodType, const std::string& neighborhoodName, const std::vector<unsigned short>& dimensions) {
	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions(dimensions);
	cellularAutomata->setNeighboorhoodType(neighborhoodType);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::CLOSED);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::BIASED_COMPETITION);

	std::string errorMessage;
	if (!cellularAutomata->initializeCellularAutomata(&errorMessage)) {
		std::cout << neighborhoodName << " " << dimensions.size() << "D radius 1: initialization failed - " << errorMessage << std::endl;
		return;
	}

	std::vector<int> centerPosition;
	for (unsigned short dimension : dimensions)
		centerPosition.emplace_back(static_cast<int>(dimension / 2));

	const long centerCellNumber = cellularAutomata->getlattice()->cellNDimPosition2Number(centerPosition);
	const unsigned long neighbors = cellularAutomata->getlattice()->getCell(centerCellNumber)->getNeighbors().size();
	std::cout << neighborhoodName << " " << dimensions.size() << "D radius 1: " << neighbors << " neighbors" << std::endl;
}

void RunRule90(Model* model, CellularAutomataComp::BoundaryType boundaryType, const std::string& boundaryName) {
	CellularAutomataComp* cellularAutomata = new CellularAutomataComp(model);
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({7});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(boundaryType);
	cellularAutomata->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	cellularAutomata->setElementaryRuleNumber(90);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::ELEMENTAR_CA);

	std::string errorMessage;
	if (!cellularAutomata->initializeCellularAutomata(&errorMessage)) {
		std::cout << "Could not initialize CellularAutomataComp: " << errorMessage << std::endl;
		return;
	}

	SetInitialPattern(cellularAutomata, "1001000");

	std::cout << "1D lattice, 7 cells, centered radius-1 neighborhood, " << boundaryName << " boundary" << std::endl;
	std::cout << std::endl;

	const unsigned int steps = 6;
	std::cout << "t0: " << cellularAutomata->showCellularAutomata() << std::endl;
	for (unsigned int step = 1; step <= steps; ++step) {
		cellularAutomata->stepCellularAutomata();
		std::cout << "t" << step << ": " << cellularAutomata->showCellularAutomata() << std::endl;
	}
}

}

Smart_CellularAutomataCompRule90::Smart_CellularAutomataCompRule90() {
}

int Smart_CellularAutomataCompRule90::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
	setDefaultTraceHandlers(genesys->getTraceManager());

	Model* model = genesys->getModelManager()->newModel();

	std::cout << "Elementary cellular automaton through CellularAutomataComp - Rule 90" << std::endl;
	std::cout << std::endl;

	RunRule90(model, CellularAutomataComp::BoundaryType::CLOSED, "closed");
	std::cout << std::endl;
	RunRule90(model, CellularAutomataComp::BoundaryType::FIXED, "fixed");
	std::cout << std::endl;
	RunRule90(model, CellularAutomataComp::BoundaryType::REFLEXIVE, "reflexive");
	std::cout << std::endl;
	RunRule90(model, CellularAutomataComp::BoundaryType::ADIABATIC, "adiabatic");

	delete genesys;
	return 0;
}
