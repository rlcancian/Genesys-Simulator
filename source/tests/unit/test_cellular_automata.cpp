#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"

namespace {

class CellularAutomataFixture : public ::testing::Test {
protected:
	void SetUp() override {
		genesys = std::make_unique<Simulator>();
		genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L0_noTraces);
		model = genesys->getModelManager()->newModel();
	}

	CellularAutomataComp* CreateComponent() {
		return new CellularAutomataComp(model);
	}

	std::unique_ptr<Simulator> genesys;
	Model* model = nullptr;
};

void ConfigureRule90(CellularAutomataComp* cellularAutomata,
		CellularAutomataComp::BoundaryType boundaryType = CellularAutomataComp::BoundaryType::FIXED,
		CellularAutomataComp::StateSetType stateSetType = CellularAutomataComp::StateSetType::ENUMERATED) {
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({7});
	cellularAutomata->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::CENTERED);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(boundaryType);
	cellularAutomata->setStateSetType(stateSetType);
	cellularAutomata->setElementaryRuleNumber(90);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::ELEMENTAR_CA);
}

void ConfigureGameOfLife(CellularAutomataComp* cellularAutomata,
		CellularAutomataComp::NeighboorhoodType neighborhoodType = CellularAutomataComp::NeighboorhoodType::MOORE,
		CellularAutomataComp::StateSetType stateSetType = CellularAutomataComp::StateSetType::ENUMERATED) {
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions({5, 5});
	cellularAutomata->setNeighboorhoodType(neighborhoodType);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	cellularAutomata->setStateSetType(stateSetType);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::GAME_OF_LIFE);
}

void ConfigureGenericAutomata(CellularAutomataComp* cellularAutomata,
		const std::vector<unsigned short>& dimensions,
		CellularAutomataComp::NeighboorhoodType neighborhoodType,
		CellularAutomataComp::StateSetType stateSetType = CellularAutomataComp::StateSetType::ENUMERATED) {
	cellularAutomata->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	cellularAutomata->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	cellularAutomata->getlattice()->setDimensions(dimensions);
	cellularAutomata->setNeighboorhoodType(neighborhoodType);
	cellularAutomata->getNeighboorhood()->setRadius(1);
	cellularAutomata->setBoundaryType(CellularAutomataComp::BoundaryType::CLOSED);
	cellularAutomata->setStateSetType(stateSetType);
	cellularAutomata->setLocalRuleType(CellularAutomataComp::LocalRuleType::BIASED_COMPETITION);
}

void SetInitialPattern(CellularAutomataComp* cellularAutomata, const std::string& pattern) {
	for (unsigned long cellNumber = 0; cellNumber < pattern.size(); ++cellNumber) {
		const long value = pattern.at(cellNumber) == '1' ? 1 : 0;
		ASSERT_TRUE(cellularAutomata->setCellState(static_cast<long>(cellNumber), value));
	}
}

std::vector<std::string> RunRule90(CellularAutomataComp* cellularAutomata, unsigned int steps) {
	std::string errorMessage;
	EXPECT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;
	SetInitialPattern(cellularAutomata, "1001000");

	std::vector<std::string> states;
	states.emplace_back(cellularAutomata->showCellularAutomata());
	for (unsigned int step = 0; step < steps; ++step) {
		cellularAutomata->stepCellularAutomata();
		states.emplace_back(cellularAutomata->showCellularAutomata());
	}
	return states;
}

std::string Grid(CellularAutomataComp* cellularAutomata, unsigned short width, unsigned short height) {
	std::string grid;
	for (unsigned short y = 0; y < height; ++y) {
		for (unsigned short x = 0; x < width; ++x)
			grid += std::to_string(cellularAutomata->getlattice()->getCell({static_cast<int>(x), static_cast<int>(y)})->getCurrentState().getValue());
		if (y + 1 < height)
			grid += "\n";
	}
	return grid;
}

void SetBlinker(CellularAutomataComp* cellularAutomata) {
	ASSERT_TRUE(cellularAutomata->setCellState({2, 1}, 1));
	ASSERT_TRUE(cellularAutomata->setCellState({2, 2}, 1));
	ASSERT_TRUE(cellularAutomata->setCellState({2, 3}, 1));
}

unsigned long NeighborCount(CellularAutomataComp* cellularAutomata, const std::vector<int>& position) {
	const long cellNumber = cellularAutomata->getlattice()->cellNDimPosition2Number(position);
	return cellularAutomata->getlattice()->getCell(cellNumber)->getNeighbors().size();
}

}

TEST_F(CellularAutomataFixture, IntegerStateSetRejectsDoubleValues) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureGenericAutomata(cellularAutomata, {3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN, CellularAutomataComp::StateSetType::INTEGERBASED);

	std::string errorMessage;
	ASSERT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;

	EXPECT_TRUE(cellularAutomata->setCellState(0L, 1.0));
	EXPECT_FALSE(cellularAutomata->setCellState(0L, 1.5));
}

TEST_F(CellularAutomataFixture, BitStateSetRejectsNonBitValues) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureRule90(cellularAutomata, CellularAutomataComp::BoundaryType::FIXED, CellularAutomataComp::StateSetType::BITBASED);

	std::string errorMessage;
	ASSERT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;

	EXPECT_TRUE(cellularAutomata->setCellState(0L, 0.0));
	EXPECT_TRUE(cellularAutomata->setCellState(0L, 1.0));
	EXPECT_FALSE(cellularAutomata->setCellState(0L, 0.5));
	EXPECT_FALSE(cellularAutomata->setCellState(0L, 2.0));
}

TEST_F(CellularAutomataFixture, DoubleStateSetAcceptsFractionalValues) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureGenericAutomata(cellularAutomata, {3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN, CellularAutomataComp::StateSetType::DOUBLEBASED);

	std::string errorMessage;
	ASSERT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;

	ASSERT_TRUE(cellularAutomata->setCellState(0L, 1.5));
	EXPECT_DOUBLE_EQ(cellularAutomata->getlattice()->getCell(0L)->getCurrentState().getDoubleValue(), 1.5);
}

TEST_F(CellularAutomataFixture, Rule90FixedBoundaryMatchesExpectedSequence) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureRule90(cellularAutomata);

	const std::vector<std::string> states = RunRule90(cellularAutomata, 6);

	EXPECT_EQ(states, (std::vector<std::string>{
			"1001000",
			"0110100",
			"1110010",
			"1011101",
			"0010100",
			"0100010",
			"1010101"}));
}

TEST_F(CellularAutomataFixture, Rule90BoundaryConditionsHaveExpectedFinalStates) {
	CellularAutomataComp* closed = CreateComponent();
	ConfigureRule90(closed, CellularAutomataComp::BoundaryType::CLOSED);
	EXPECT_EQ(RunRule90(closed, 6).back(), "0000101");

	CellularAutomataComp* reflexive = CreateComponent();
	ConfigureRule90(reflexive, CellularAutomataComp::BoundaryType::REFLEXIVE);
	EXPECT_EQ(RunRule90(reflexive, 6).back(), "0110010");

	CellularAutomataComp* adiabatic = CreateComponent();
	ConfigureRule90(adiabatic, CellularAutomataComp::BoundaryType::ADIABATIC);
	EXPECT_EQ(RunRule90(adiabatic, 6).back(), "0000101");
}

TEST_F(CellularAutomataFixture, Rule90AcceptsBitStateSet) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureRule90(cellularAutomata, CellularAutomataComp::BoundaryType::FIXED, CellularAutomataComp::StateSetType::BITBASED);

	const std::vector<std::string> states = RunRule90(cellularAutomata, 1);

	EXPECT_EQ(states.at(1), "0110100");
}

TEST_F(CellularAutomataFixture, GameOfLifeBlinkerOscillates) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureGameOfLife(cellularAutomata);

	std::string errorMessage;
	ASSERT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;
	SetBlinker(cellularAutomata);

	EXPECT_EQ(Grid(cellularAutomata, 5, 5), "00000\n00100\n00100\n00100\n00000");
	cellularAutomata->stepCellularAutomata();
	EXPECT_EQ(Grid(cellularAutomata, 5, 5), "00000\n00000\n01110\n00000\n00000");
	cellularAutomata->stepCellularAutomata();
	EXPECT_EQ(Grid(cellularAutomata, 5, 5), "00000\n00100\n00100\n00100\n00000");
}

TEST_F(CellularAutomataFixture, GameOfLifeRejectsInvalidConfigurations) {
	CellularAutomataComp* vonNeumann = CreateComponent();
	ConfigureGameOfLife(vonNeumann, CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
	std::string errorMessage;
	EXPECT_FALSE(vonNeumann->initializeCellularAutomata(&errorMessage));

	CellularAutomataComp* integerBased = CreateComponent();
	ConfigureGameOfLife(integerBased, CellularAutomataComp::NeighboorhoodType::MOORE, CellularAutomataComp::StateSetType::INTEGERBASED);
	errorMessage.clear();
	EXPECT_FALSE(integerBased->initializeCellularAutomata(&errorMessage));
}

TEST_F(CellularAutomataFixture, NeighborhoodsHaveExpectedCountsInOneTwoAndThreeDimensions) {
	CellularAutomataComp* moore1D = CreateComponent();
	ConfigureGenericAutomata(moore1D, {3}, CellularAutomataComp::NeighboorhoodType::MOORE);
	std::string errorMessage;
	ASSERT_TRUE(moore1D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(moore1D, {1}), 2u);

	CellularAutomataComp* vonNeumann1D = CreateComponent();
	ConfigureGenericAutomata(vonNeumann1D, {3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
	errorMessage.clear();
	ASSERT_TRUE(vonNeumann1D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(vonNeumann1D, {1}), 2u);

	CellularAutomataComp* moore2D = CreateComponent();
	ConfigureGenericAutomata(moore2D, {3, 3}, CellularAutomataComp::NeighboorhoodType::MOORE);
	errorMessage.clear();
	ASSERT_TRUE(moore2D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(moore2D, {1, 1}), 8u);

	CellularAutomataComp* vonNeumann2D = CreateComponent();
	ConfigureGenericAutomata(vonNeumann2D, {3, 3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
	errorMessage.clear();
	ASSERT_TRUE(vonNeumann2D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(vonNeumann2D, {1, 1}), 4u);

	CellularAutomataComp* moore3D = CreateComponent();
	ConfigureGenericAutomata(moore3D, {3, 3, 3}, CellularAutomataComp::NeighboorhoodType::MOORE);
	errorMessage.clear();
	ASSERT_TRUE(moore3D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(moore3D, {1, 1, 1}), 26u);

	CellularAutomataComp* vonNeumann3D = CreateComponent();
	ConfigureGenericAutomata(vonNeumann3D, {3, 3, 3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
	errorMessage.clear();
	ASSERT_TRUE(vonNeumann3D->initializeCellularAutomata(&errorMessage)) << errorMessage;
	EXPECT_EQ(NeighborCount(vonNeumann3D, {1, 1, 1}), 6u);
}

TEST_F(CellularAutomataFixture, ThreeDimensionalLatticeRejectsInvalidCoordinates) {
	CellularAutomataComp* cellularAutomata = CreateComponent();
	ConfigureGenericAutomata(cellularAutomata, {3, 3, 3}, CellularAutomataComp::NeighboorhoodType::VONNEUMANN);

	std::string errorMessage;
	ASSERT_TRUE(cellularAutomata->initializeCellularAutomata(&errorMessage)) << errorMessage;

	EXPECT_FALSE(cellularAutomata->setCellState({-1, 0, 0}, 1));
}

TEST_F(CellularAutomataFixture, UpdatePoliciesProduceDeterministicSequences) {
	CellularAutomataComp* synchronous = CreateComponent();
	ConfigureRule90(synchronous);
	synchronous->setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::SYNCHRONOUS);
	EXPECT_EQ(RunRule90(synchronous, 4).back(), "0010100");

	CellularAutomataComp* sequential = CreateComponent();
	ConfigureRule90(sequential);
	sequential->setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::SEQUENTIAL);
	EXPECT_EQ(RunRule90(sequential, 4).back(), "1110000");

	CellularAutomataComp* random = CreateComponent();
	ConfigureRule90(random);
	random->setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::RANDOM);
	random->setRandomSeed(7);
	const std::string randomFirstRun = RunRule90(random, 4).back();
	// RANDOM consumes the engine via std::uniform_int_distribution, whose value mapping is
	// implementation-defined (libstdc++ vs libc++); a hardcoded golden is not portable.
	// Assert what the test name promises: the same seed reproduces the same sequence.
	CellularAutomataComp* randomAgain = CreateComponent();
	ConfigureRule90(randomAgain);
	randomAgain->setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::RANDOM);
	randomAgain->setRandomSeed(7);
	EXPECT_EQ(RunRule90(randomAgain, 4).back(), randomFirstRun);

	CellularAutomataComp* blocks = CreateComponent();
	ConfigureRule90(blocks);
	blocks->setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType::BLOCKS);
	blocks->setUpdateBlockSize(2);
	EXPECT_EQ(RunRule90(blocks, 4).back(), "1111100");
}
