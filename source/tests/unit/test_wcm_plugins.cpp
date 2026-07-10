#include <gtest/gtest.h>

#include <cmath>
#include <map>
#include <string>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/WholeCellModeling/BioCompartment.h"
#include "plugins/data/WholeCellModeling/MolecularSpecies.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/CompartmentExchangeComponent.h"
#include "plugins/components/WholeCellModeling/EukaryoticCellCycleComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/PathwayStressResponseComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"
#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"

namespace {

// Test fixture that creates a Simulator + Model once per test case.
class WcmModelFixture : public ::testing::Test {
protected:
    void SetUp() override {
        _simulator.getPluginManager()->autoInsertPlugins();
        _model = _simulator.getModelManager()->newModel();
        ASSERT_NE(_model, nullptr);
    }

    Simulator _simulator;
    Model* _model = nullptr;
};

class CellGrowthComponentProbe : public CellGrowthComponent {
public:
    CellGrowthComponentProbe(Model* model, const std::string& name = "") : CellGrowthComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class CellCycleCheckpointComponentProbe : public CellCycleCheckpointComponent {
public:
    CellCycleCheckpointComponentProbe(Model* model, const std::string& name = "")
        : CellCycleCheckpointComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class CellFateDecisionComponentProbe : public CellFateDecisionComponent {
public:
    CellFateDecisionComponentProbe(Model* model, const std::string& name = "")
        : CellFateDecisionComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class CompartmentExchangeComponentProbe : public CompartmentExchangeComponent {
public:
    CompartmentExchangeComponentProbe(Model* model, const std::string& name = "")
        : CompartmentExchangeComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class PathwayStressResponseComponentProbe : public PathwayStressResponseComponent {
public:
    PathwayStressResponseComponentProbe(Model* model, const std::string& name = "")
        : PathwayStressResponseComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class EukaryoticCellCycleComponentProbe : public EukaryoticCellCycleComponent {
public:
    EukaryoticCellCycleComponentProbe(Model* model, const std::string& name = "")
        : EukaryoticCellCycleComponent(model, name) {}

    void DispatchEventProbe(Entity* entity = nullptr, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

// ---------------------------------------------------------------------------
// StochasticReactionRule — propensity logic
// ---------------------------------------------------------------------------

class WcmStochasticReactionRule : public WcmModelFixture {};

TEST_F(WcmStochasticReactionRule, PropensityIsZeroWhenRateIsZero) {
    auto* rule = new StochasticReactionRule(_model, "R_zero");
    rule->setRateConstant(0.0);
    rule->addReactant("A");
    _model->getDataManager()->insert(rule);

    const std::map<std::string, int> counts = {{"A", 10}};
    EXPECT_DOUBLE_EQ(rule->computePropensity(counts), 0.0);
}

TEST_F(WcmStochasticReactionRule, PropensityIsZeroWhenReactantCountIsZero) {
    auto* rule = new StochasticReactionRule(_model, "R_decay");
    rule->setRateConstant(1.0);
    rule->addReactant("A");
    _model->getDataManager()->insert(rule);

    const std::map<std::string, int> counts = {{"A", 0}};
    EXPECT_DOUBLE_EQ(rule->computePropensity(counts), 0.0);
}

TEST_F(WcmStochasticReactionRule, PropensityEqualsKForUnimolecularCountOne) {
    // A → B, k=2.5, count(A)=1 → propensity = k×1 = 2.5
    auto* rule = new StochasticReactionRule(_model, "R_unimol");
    rule->setRateConstant(2.5);
    rule->addReactant("A");
    _model->getDataManager()->insert(rule);

    const std::map<std::string, int> counts = {{"A", 1}};
    EXPECT_DOUBLE_EQ(rule->computePropensity(counts), 2.5);
}

TEST_F(WcmStochasticReactionRule, PropensityScalesLinearlyWithCountForUnimolecular) {
    // A → B, k=1, count(A)=N → propensity = N
    auto* rule = new StochasticReactionRule(_model, "R_linear");
    rule->setRateConstant(1.0);
    rule->addReactant("A");
    _model->getDataManager()->insert(rule);

    for (int n = 1; n <= 8; ++n) {
        const std::map<std::string, int> counts = {{"A", n}};
        EXPECT_DOUBLE_EQ(rule->computePropensity(counts), static_cast<double>(n))
            << "expected propensity = " << n << " for count = " << n;
    }
}

TEST_F(WcmStochasticReactionRule, PropensityForBimolecularSameSpeciesIsKTimesCn2) {
    // 2A → B (bimolecular same species): propensity = k × C(n,2) = k × n×(n-1)/2
    auto* rule = new StochasticReactionRule(_model, "R_bimol_same");
    rule->setRateConstant(1.0);
    rule->addReactant("A", 2);
    _model->getDataManager()->insert(rule);

    // n=3: C(3,2) = 3
    EXPECT_DOUBLE_EQ(rule->computePropensity({{"A", 3}}), 3.0);
    // n=4: C(4,2) = 6
    EXPECT_DOUBLE_EQ(rule->computePropensity({{"A", 4}}), 6.0);
}

TEST_F(WcmStochasticReactionRule, PropensityForBimolecularDifferentSpeciesIsKTimesProduct) {
    // A + B → C: propensity = k × count(A) × count(B)
    auto* rule = new StochasticReactionRule(_model, "R_bimol_diff");
    rule->setRateConstant(0.5);
    rule->addReactant("A");
    rule->addReactant("B");
    _model->getDataManager()->insert(rule);

    // k × 3 × 4 = 0.5 × 12 = 6.0
    EXPECT_DOUBLE_EQ(rule->computePropensity({{"A", 3}, {"B", 4}}), 6.0);
}

TEST_F(WcmStochasticReactionRule, ReactantsAndProductsStoredCorrectly) {
    auto* rule = new StochasticReactionRule(_model, "R_storage");
    rule->addReactant("ATP", 2);
    rule->addReactant("ADP", 1);
    rule->addProduct("Pi",   3);
    rule->addProduct("H2O",  1);
    _model->getDataManager()->insert(rule);

    ASSERT_EQ(rule->getReactants().size(), 2u);
    EXPECT_EQ(rule->getReactants()[0].speciesName, "ATP");
    EXPECT_EQ(rule->getReactants()[0].stoichiometry, 2);
    EXPECT_EQ(rule->getReactants()[1].speciesName, "ADP");

    ASSERT_EQ(rule->getProducts().size(), 2u);
    EXPECT_EQ(rule->getProducts()[0].speciesName, "Pi");
    EXPECT_EQ(rule->getProducts()[0].stoichiometry, 3);
}

TEST_F(WcmStochasticReactionRule, ClearReactantsAndProducts) {
    auto* rule = new StochasticReactionRule(_model, "R_clear");
    rule->addReactant("A");
    rule->addProduct("B");
    _model->getDataManager()->insert(rule);

    rule->clearReactants();
    rule->clearProducts();
    EXPECT_TRUE(rule->getReactants().empty());
    EXPECT_TRUE(rule->getProducts().empty());
}

// ---------------------------------------------------------------------------
// WholeCellState — data storage
// ---------------------------------------------------------------------------

class WcmWholeCellState : public WcmModelFixture {};

TEST_F(WcmWholeCellState, MoleculeCountDefaultIsZero) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    EXPECT_EQ(state->getMoleculeCount("X"), 0);
    EXPECT_FALSE(state->hasMoleculeCount("X"));
}

TEST_F(WcmWholeCellState, MoleculeCountSetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setMoleculeCount("mRNA_geneA", 42);
    EXPECT_EQ(state->getMoleculeCount("mRNA_geneA"), 42);
    EXPECT_TRUE(state->hasMoleculeCount("mRNA_geneA"));
}

TEST_F(WcmWholeCellState, MetaboliteAmountSetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setMetaboliteAmount("ATP", 1.23e-4);
    EXPECT_DOUBLE_EQ(state->getMetaboliteAmount("ATP"), 1.23e-4);
}

TEST_F(WcmWholeCellState, CompartmentMetaboliteAmountSetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setCompartmentMetaboliteAmount("cytosol", "ATP", 3.5);
    EXPECT_TRUE(state->hasCompartmentMetaboliteAmount("cytosol", "ATP"));
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("cytosol", "ATP"), 3.5);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("membrane", "ATP"), 0.0);
}

TEST_F(WcmWholeCellState, PathwayActivitySetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setPathwayActivity("glycolysis_flux", 7.25);
    EXPECT_TRUE(state->hasPathwayActivity("glycolysis_flux"));
    EXPECT_DOUBLE_EQ(state->getPathwayActivity("glycolysis_flux"), 7.25);
    EXPECT_DOUBLE_EQ(state->getPathwayActivity("ppp_flux"), 0.0);
}

TEST_F(WcmWholeCellState, ResourceBudgetSetClearAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setResourceBudget("A", 10);
    state->setResourceBudget("B", 20);
    EXPECT_EQ(state->getResourceBudget("A"), 10);
    EXPECT_EQ(state->getResourceBudget("B"), 20);

    state->clearResourceBudget();
    EXPECT_EQ(state->getResourceBudget("A"), 0);
}

TEST_F(WcmWholeCellState, CellGeometryDefaults) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    EXPECT_DOUBLE_EQ(state->getCellVolume(), 1.0e-15);
    EXPECT_DOUBLE_EQ(state->getCellMass(),   1.0e-13);
}

TEST_F(WcmWholeCellState, CellGeometrySetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setCellVolume(2.5e-15);
    state->setCellMass(3.7e-13);
    EXPECT_DOUBLE_EQ(state->getCellVolume(), 2.5e-15);
    EXPECT_DOUBLE_EQ(state->getCellMass(),   3.7e-13);
}

TEST_F(WcmWholeCellState, StepCountIncrements) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    EXPECT_EQ(state->getStepCount(), 0);
    state->incrementStep();
    state->incrementStep();
    EXPECT_EQ(state->getStepCount(), 2);
    state->setStepCount(0);
    EXPECT_EQ(state->getStepCount(), 0);
}

TEST_F(WcmWholeCellState, CurrentTimeSetAndGet) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    EXPECT_DOUBLE_EQ(state->getCurrentTime(), 0.0);
    state->setCurrentTime(42.7);
    EXPECT_DOUBLE_EQ(state->getCurrentTime(), 42.7);
}

TEST_F(WcmWholeCellState, LifecycleMetadataSetAndGet) {
    auto* state = new WholeCellState(_model, "LifecycleState");
    _model->getDataManager()->insert(state);

    state->setLifecyclePhase("growth");
    state->setGenerationCount(3);
    state->setViable(false);
    state->setLastDivisionTime(180.0);

    EXPECT_EQ(state->getLifecyclePhase(), "growth");
    EXPECT_EQ(state->getGenerationCount(), 3);
    EXPECT_FALSE(state->isViable());
    EXPECT_DOUBLE_EQ(state->getLastDivisionTime(), 180.0);
}

TEST_F(WcmWholeCellState, MoleculeCountsMapIsAccessible) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    state->setMoleculeCount("A", 10);
    state->setMoleculeCount("B", 20);
    const auto& counts = state->getMoleculeCounts();
    EXPECT_EQ(counts.size(), 2u);
    EXPECT_EQ(counts.at("A"), 10);
    EXPECT_EQ(counts.at("B"), 20);
}

TEST_F(WcmWholeCellState, CheckPassesWithNoFilePaths) {
    auto* state = new WholeCellState(_model, "S");
    _model->getDataManager()->insert(state);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(state, errorMessage)) << errorMessage;
}

// ---------------------------------------------------------------------------
// MolecularSpecies — data storage
// ---------------------------------------------------------------------------

class WcmMolecularSpecies : public WcmModelFixture {};

TEST_F(WcmMolecularSpecies, DefaultCountIsZero) {
    auto* species = new MolecularSpecies(_model, "ATP");
    _model->getDataManager()->insert(species);
    EXPECT_EQ(species->getCount(), 0);
}

TEST_F(WcmMolecularSpecies, SetAndGetCount) {
    auto* species = new MolecularSpecies(_model, "ATP");
    _model->getDataManager()->insert(species);
    species->setCount(100);
    EXPECT_EQ(species->getCount(), 100);
}

TEST_F(WcmMolecularSpecies, NameIsPreserved) {
    auto* species = new MolecularSpecies(_model, "RNAP_free");
    _model->getDataManager()->insert(species);
    EXPECT_EQ(species->getName(), "RNAP_free");
}

// ---------------------------------------------------------------------------
// Plugin information sanity checks
// ---------------------------------------------------------------------------

TEST(WcmPluginInformation, StochasticReactionRuleReturnsValidPluginInfo) {
    PluginInformation* info = StochasticReactionRule::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    EXPECT_FALSE(info->getCategory().empty());
    delete info;
}

TEST(WcmPluginInformation, WholeCellStateReturnsValidPluginInfo) {
    PluginInformation* info = WholeCellState::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    delete info;
}

TEST(WcmPluginInformation, BioCompartmentReturnsValidPluginInfo) {
    PluginInformation* info = BioCompartment::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    delete info;
}

TEST(WcmPluginInformation, MolecularSpeciesReturnsValidPluginInfo) {
    PluginInformation* info = MolecularSpecies::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    delete info;
}

TEST(WcmPluginInformation, StochasticReactionComponentReturnsValidPluginInfo) {
    PluginInformation* info = StochasticReactionComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(),  1u);
    EXPECT_EQ(info->getMaximumInputs(),  1u);
    EXPECT_EQ(info->getMinimumOutputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, CellDivisionEventHasTwoOutputPorts) {
    PluginInformation* info = CellDivisionEvent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumOutputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 2u);
    delete info;
}

TEST(WcmPluginInformation, CellCycleCheckpointComponentReturnsValidPluginInfo) {
    PluginInformation* info = CellCycleCheckpointComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, CellFateDecisionComponentReturnsValidPluginInfo) {
    PluginInformation* info = CellFateDecisionComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 4u);
    delete info;
}

TEST(WcmPluginInformation, CompartmentExchangeComponentReturnsValidPluginInfo) {
    PluginInformation* info = CompartmentExchangeComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, EukaryoticCellCycleComponentReturnsValidPluginInfo) {
    PluginInformation* info = EukaryoticCellCycleComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, PathwayStressResponseComponentReturnsValidPluginInfo) {
    PluginInformation* info = PathwayStressResponseComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, MetabolicStateProjectionComponentReturnsValidPluginInfo) {
    PluginInformation* info = MetabolicStateProjectionComponent::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    delete info;
}

TEST(WcmPluginInformation, StochasticTranscriptionReturnsValidPluginInfo) {
    PluginInformation* info = StochasticTranscription::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    delete info;
}

TEST(WcmPluginInformation, StochasticTranslationReturnsValidPluginInfo) {
    PluginInformation* info = StochasticTranslation::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->getPluginTypename().empty());
    delete info;
}

// ---------------------------------------------------------------------------
// Integration — model-level construction and check()
// ---------------------------------------------------------------------------

class WcmIntegration : public WcmModelFixture {};

TEST_F(WcmIntegration, StochasticReactionComponentCheckFailsWithoutRules) {
    auto* comp = new StochasticReactionComponent(_model, "SSA");
    std::string errorMessage;
    EXPECT_FALSE(ModelDataDefinition::Check(comp, errorMessage));
    EXPECT_FALSE(errorMessage.empty());
}

TEST_F(WcmIntegration, StochasticReactionComponentCheckPassesWithRule) {
    auto* rule = new StochasticReactionRule(_model, "DecayRule");
    rule->setRateConstant(0.1);
    rule->addReactant("A");
    _model->getDataManager()->insert(rule);

    auto* comp = new StochasticReactionComponent(_model, "SSA");
    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(comp, errorMessage)) << errorMessage;
}

TEST_F(WcmIntegration, StochasticReactionComponentShowIncludesExpectedFields) {
    auto* comp = new StochasticReactionComponent(_model, "SSA_Show");
    const std::string showResult = comp->show();
    EXPECT_NE(showResult.find("SSA_Show"), std::string::npos);
    EXPECT_NE(showResult.find("timeWindow"), std::string::npos);
    EXPECT_NE(showResult.find("randomSeed"), std::string::npos);
}

TEST_F(WcmIntegration, CellDivisionEventCheckFailsWithoutWholeCellState) {
    auto* divEvent = new CellDivisionEvent(_model, "CDE");
    std::string errorMessage;
    EXPECT_FALSE(ModelDataDefinition::Check(divEvent, errorMessage));
    EXPECT_FALSE(errorMessage.empty());
}

TEST_F(WcmIntegration, CellDivisionEventCheckPassesWithState) {
    auto* state = new WholeCellState(_model, "WCS_ForDiv");
    _model->getDataManager()->insert(state);

    auto* divEvent = new CellDivisionEvent(_model, "CDE_WithState");
    divEvent->setWholeCellState(state);
    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(divEvent, errorMessage)) << errorMessage;
}

TEST_F(WcmIntegration, CellGrowthComponentUsesEnergyGateWhenConfigured) {
    auto* state = new WholeCellState(_model, "GrowthState");
    _model->getDataManager()->insert(state);
    state->setCellMass(1.0e-15);
    state->setCellVolume(1.0e-3);

    CellGrowthComponentProbe gated(_model, "GrowthGated");
    gated.setWholeCellState(state);
    gated.setGrowthRate(1.0);
    gated.setDeltaT(1.0);
    gated.setDensity(1000.0);
    gated.setEnergyMetaboliteKey("ATP");
    gated.setEnergyHalfSaturation(1.0);

    state->setMetaboliteAmount("ATP", 0.0);
    gated.DispatchEventProbe();
    EXPECT_DOUBLE_EQ(state->getCellMass(), 1.0e-15);

    state->setMetaboliteAmount("ATP", 1.0);
    gated.DispatchEventProbe();
    EXPECT_DOUBLE_EQ(state->getCellMass(), 1.5e-15);
}

TEST_F(WcmIntegration, CellCycleCheckpointComponentAdvancesClockAndAnnotatesLifecycle) {
    auto* state = new WholeCellState(_model, "CheckpointState");
    _model->getDataManager()->insert(state);
    state->setCellMass(1.0e-15);
    state->setMetaboliteAmount("ATP", 2.0);
    state->setMoleculeCount("FtsZ_ring_completion", 0);

    CellCycleCheckpointComponentProbe checkpoint(_model, "Checkpoint");
    checkpoint.setWholeCellState(state);
    checkpoint.setDeltaT(60.0);
    checkpoint.setStarvationAtpThreshold(0.5);
    checkpoint.setDivisionMassThreshold(1.5e-15);
    checkpoint.setFtsZThreshold(0.4);
    checkpoint.DispatchEventProbe();

    EXPECT_DOUBLE_EQ(state->getCurrentTime(), 60.0);
    EXPECT_EQ(state->getStepCount(), 1);
    EXPECT_EQ(state->getLifecyclePhase(), "newborn");

    state->setMetaboliteAmount("ATP", 0.1);
    checkpoint.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "starved");

    state->setMetaboliteAmount("ATP", 2.0);
    state->setCellMass(1.6e-15);
    state->setMoleculeCount("FtsZ_ring_completion", 500);
    checkpoint.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "division_ready");
}

TEST_F(WcmIntegration, CellCycleCheckpointComponentSupportsCompartmentAndPathwayStarvationRules) {
    auto* state = new WholeCellState(_model, "CheckpointCompartmentState");
    _model->getDataManager()->insert(state);
    state->setCellMass(1.0e-15);
    state->setMetaboliteAmount("ATP", 2.0);
    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 0.1);
    state->setPathwayActivity("biomass_objective", 3.0);

    CellCycleCheckpointComponentProbe checkpoint(_model, "CheckpointCompartment");
    checkpoint.setWholeCellState(state);
    checkpoint.setDeltaT(60.0);
    checkpoint.setEnergyMetaboliteKey("ATP");
    checkpoint.setStarvationAtpThreshold(0.5);
    checkpoint.setCompartmentEnergyRegion("cytosol");
    checkpoint.setCompartmentEnergyMetaboliteKey("ATP_c");
    checkpoint.setCompartmentStarvationThreshold(0.5);
    checkpoint.setCriticalPathwayActivityKey("biomass_objective");
    checkpoint.setCriticalPathwayActivityThreshold(2.5);

    checkpoint.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "starved");

    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 1.0);
    checkpoint.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "growth");

    state->setPathwayActivity("biomass_objective", 1.5);
    checkpoint.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "starved");
}

TEST_F(WcmIntegration, CellFateDecisionComponentRoutesByLifecyclePhase) {
    auto* state = new WholeCellState(_model, "FateState");
    _model->getDataManager()->insert(state);

    CellFateDecisionComponentProbe fate(_model, "Fate");
    fate.setWholeCellState(state);

    state->setLifecyclePhase("growth");
    state->setViable(true);
    fate.DispatchEventProbe();
    EXPECT_EQ(fate.getLastRoutedPort(), 0u);

    state->setLifecyclePhase("division_ready");
    fate.DispatchEventProbe();
    EXPECT_EQ(fate.getLastRoutedPort(), 1u);

    state->setLifecyclePhase("starved");
    fate.DispatchEventProbe();
    EXPECT_EQ(fate.getLastRoutedPort(), 2u);

    state->setLifecyclePhase("arrested");
    fate.DispatchEventProbe();
    EXPECT_EQ(fate.getLastRoutedPort(), 2u);

    state->setLifecyclePhase("dead");
    state->setViable(false);
    fate.DispatchEventProbe();
    EXPECT_EQ(fate.getLastRoutedPort(), 3u);
}

TEST_F(WcmIntegration, BioCompartmentSupportsHierarchyValidation) {
    auto* cytosol = new BioCompartment(_model, "cytosol");
    auto* nucleus = new BioCompartment(_model, "nucleus");
    ASSERT_NE(cytosol, nullptr);
    ASSERT_NE(nucleus, nullptr);

    cytosol->setCompartmentType("cytosol");
    cytosol->setRole("central_metabolism");
    nucleus->setCompartmentType("nucleus");
    nucleus->setParentCompartmentName("cytosol");
    nucleus->setMembraneBounded(true);
    nucleus->setRole("replication");

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(cytosol, errorMessage)) << errorMessage;
    errorMessage.clear();
    EXPECT_TRUE(ModelDataDefinition::Check(nucleus, errorMessage)) << errorMessage;
    ASSERT_NE(nucleus->getAttachedData(), nullptr);
    EXPECT_NE(nucleus->getAttachedData()->find("ParentCompartment"), nucleus->getAttachedData()->end());
}

TEST_F(WcmIntegration, EukaryoticCellCycleComponentAdvancesThroughDidacticPhases) {
    auto* cytosol = new BioCompartment(_model, "cytosol");
    auto* nucleus = new BioCompartment(_model, "nucleus");
    auto* mitochondria = new BioCompartment(_model, "mitochondria");
    auto* bud = new BioCompartment(_model, "bud");
    ASSERT_NE(cytosol, nullptr);
    ASSERT_NE(nucleus, nullptr);
    ASSERT_NE(mitochondria, nullptr);
    ASSERT_NE(bud, nullptr);
    nucleus->setParentCompartmentName("cytosol");
    mitochondria->setParentCompartmentName("cytosol");
    bud->setParentCompartmentName("cytosol");

    auto* state = new WholeCellState(_model, "EukCycleState");
    _model->getDataManager()->insert(state);
    state->setMetaboliteAmount("ATP", 3.0);
    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 2.0);
    state->setCompartmentMetaboliteAmount("mitochondria", "ATP_m", 2.0);
    state->setCompartmentMetaboliteAmount("bud", "ATP_bud", 1.0);
    state->setPathwayActivity("biomass_flux", 8.0);
    state->setPathwayActivity("respiration_flux", 4.0);

    EukaryoticCellCycleComponentProbe cycle(_model, "EukCycle");
    cycle.setWholeCellState(state);
    cycle.setDeltaT(60.0);
    cycle.setBudProgressRate(0.60);
    cycle.setDnaReplicationRate(0.60);
    cycle.setSpindleAssemblyRate(0.60);
    cycle.setMitoticExitRate(0.60);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(&cycle, errorMessage)) << errorMessage;

    cycle.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "g1_budding");
    cycle.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "s_phase");
    cycle.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "g2_phase");
    cycle.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "m_phase");
    cycle.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "division_ready");
    EXPECT_GT(state->getPathwayActivity("bud_growth_progress"), 1.0);
    EXPECT_GT(state->getPathwayActivity("dna_replication_progress"), 1.0);
    EXPECT_GT(state->getPathwayActivity("spindle_assembly_progress"), 1.0);
    EXPECT_GT(state->getPathwayActivity("mitotic_exit_progress"), 1.0);
}

TEST_F(WcmIntegration, EukaryoticCellCycleComponentArrestsWhenBudEnergyCheckpointFails) {
    auto* cytosol = new BioCompartment(_model, "cytosol");
    auto* nucleus = new BioCompartment(_model, "nucleus");
    auto* mitochondria = new BioCompartment(_model, "mitochondria");
    auto* bud = new BioCompartment(_model, "bud");
    ASSERT_NE(cytosol, nullptr);
    ASSERT_NE(nucleus, nullptr);
    ASSERT_NE(mitochondria, nullptr);
    ASSERT_NE(bud, nullptr);
    nucleus->setParentCompartmentName("cytosol");
    mitochondria->setParentCompartmentName("cytosol");
    bud->setParentCompartmentName("cytosol");

    auto* state = new WholeCellState(_model, "EukArrestState");
    _model->getDataManager()->insert(state);
    state->setMetaboliteAmount("ATP", 3.0);
    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 2.0);
    state->setCompartmentMetaboliteAmount("mitochondria", "ATP_m", 2.0);
    state->setCompartmentMetaboliteAmount("bud", "ATP_bud", 0.0);
    state->setPathwayActivity("biomass_flux", 8.0);
    state->setPathwayActivity("respiration_flux", 4.0);

    EukaryoticCellCycleComponentProbe cycle(_model, "EukArrestCycle");
    cycle.setWholeCellState(state);
    cycle.setBudProgressRate(0.60);
    cycle.setDnaReplicationRate(0.60);
    cycle.setSpindleAssemblyRate(0.60);
    cycle.setMitoticExitRate(0.60);

    for (int i = 0; i < 4; ++i) {
        cycle.DispatchEventProbe();
    }
    EXPECT_EQ(state->getLifecyclePhase(), "arrested");
}

TEST_F(WcmIntegration, CompartmentExchangeComponentMovesMassBetweenCompartments) {
    auto* state = new WholeCellState(_model, "ExchangeState");
    _model->getDataManager()->insert(state);
    state->setCompartmentMetaboliteAmount("extracellular", "ATP_ext", 10.0);
    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 0.0);

    CompartmentExchangeComponentProbe exchange(_model, "Exchange");
    exchange.setWholeCellState(state);
    exchange.setSourceRegion("extracellular");
    exchange.setSourceMetaboliteKey("ATP_ext");
    exchange.setTargetRegion("cytosol");
    exchange.setTargetMetaboliteKey("ATP_c");
    exchange.setExchangeFraction(0.25);
    exchange.DispatchEventProbe();

    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("extracellular", "ATP_ext"), 7.5);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("cytosol", "ATP_c"), 2.5);
    EXPECT_DOUBLE_EQ(exchange.getLastTransferAmount(), 2.5);
}

TEST_F(WcmIntegration, CompartmentExchangeComponentCanBeDriverLimited) {
    auto* state = new WholeCellState(_model, "ExchangeDriverState");
    _model->getDataManager()->insert(state);
    state->setCompartmentMetaboliteAmount("extracellular", "ATP_ext", 10.0);
    state->setCompartmentMetaboliteAmount("cytosol", "ATP_c", 0.0);
    state->setPathwayActivity("transport_flux", 0.0);

    CompartmentExchangeComponentProbe exchange(_model, "ExchangeDriver");
    exchange.setWholeCellState(state);
    exchange.setSourceRegion("extracellular");
    exchange.setSourceMetaboliteKey("ATP_ext");
    exchange.setTargetRegion("cytosol");
    exchange.setTargetMetaboliteKey("ATP_c");
    exchange.setExchangeFraction(0.25);
    exchange.setDriverPathwayKey("transport_flux");
    exchange.setDriverScale(0.5);
    exchange.setMaxTransferAmount(1.0);

    exchange.DispatchEventProbe();
    EXPECT_DOUBLE_EQ(exchange.getLastTransferAmount(), 0.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("extracellular", "ATP_ext"), 10.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("cytosol", "ATP_c"), 0.0);

    state->setPathwayActivity("transport_flux", 2.0);
    exchange.DispatchEventProbe();
    EXPECT_DOUBLE_EQ(exchange.getLastTransferAmount(), 1.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("extracellular", "ATP_ext"), 9.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("cytosol", "ATP_c"), 1.0);
}

TEST_F(WcmIntegration, CompartmentExchangeComponentCanApplyMultipleRulesPerDispatch) {
    auto* state = new WholeCellState(_model, "ExchangeMultiState");
    _model->getDataManager()->insert(state);
    state->setCompartmentMetaboliteAmount("extracellular", "GLC_ext", 12.0);
    state->setCompartmentMetaboliteAmount("extracellular", "O2_ext", 6.0);
    state->setCompartmentMetaboliteAmount("cytosol", "GLC_c", 0.0);
    state->setCompartmentMetaboliteAmount("mitochondria", "O2_m", 0.0);

    CompartmentExchangeComponentProbe exchange(_model, "ExchangeMulti");
    exchange.setWholeCellState(state);
    exchange.addExchangeRule("glucose_import", "extracellular", "GLC_ext", "cytosol", "GLC_c", 0.25);
    exchange.addExchangeRule("oxygen_import", "extracellular", "O2_ext", "mitochondria", "O2_m", 0.50);
    exchange.DispatchEventProbe();

    EXPECT_EQ(exchange.getExchangeRuleCount(), 2u);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("extracellular", "GLC_ext"), 9.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("cytosol", "GLC_c"), 3.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("extracellular", "O2_ext"), 3.0);
    EXPECT_DOUBLE_EQ(state->getCompartmentMetaboliteAmount("mitochondria", "O2_m"), 3.0);
    EXPECT_DOUBLE_EQ(exchange.getLastTransferAmount(), 6.0);
}

TEST_F(WcmIntegration, PathwayStressResponseComponentTriggersArrestRecoveryAndDeath) {
    auto* state = new WholeCellState(_model, "StressResponseState");
    _model->getDataManager()->insert(state);
    state->setLifecyclePhase("growth");
    state->setViable(true);
    state->setPathwayActivity("biomass_objective", 5.0);

    PathwayStressResponseComponentProbe response(_model, "StressResponse");
    response.setWholeCellState(state);
    response.setMonitoredPathwayKey("biomass_objective");
    response.setStressThreshold(4.0);
    response.setArrestAfterSteps(2u);
    response.setDeathAfterSteps(4u);
    response.setArrestPhase("arrested");
    response.setRecoveryPhase("growth");

    response.DispatchEventProbe();
    EXPECT_EQ(state->getLifecyclePhase(), "growth");
    EXPECT_EQ(response.getStressStreak(), 0u);

    state->setPathwayActivity("biomass_objective", 3.0);
    response.DispatchEventProbe();
    EXPECT_EQ(response.getStressStreak(), 1u);
    EXPECT_EQ(state->getLifecyclePhase(), "growth");

    response.DispatchEventProbe();
    EXPECT_EQ(response.getStressStreak(), 2u);
    EXPECT_EQ(state->getLifecyclePhase(), "arrested");
    EXPECT_TRUE(state->isViable());

    state->setPathwayActivity("biomass_objective", 5.0);
    response.DispatchEventProbe();
    EXPECT_EQ(response.getStressStreak(), 0u);
    EXPECT_EQ(state->getLifecyclePhase(), "growth");

    state->setPathwayActivity("biomass_objective", 3.0);
    response.DispatchEventProbe();
    response.DispatchEventProbe();
    response.DispatchEventProbe();
    response.DispatchEventProbe();
    EXPECT_EQ(response.getStressStreak(), 4u);
    EXPECT_EQ(state->getLifecyclePhase(), "dead");
    EXPECT_FALSE(state->isViable());
}

TEST_F(WcmIntegration, StochasticTranscriptionDefaultProperties) {
    auto* trans = new StochasticTranscription(_model, "Transcription");
    EXPECT_DOUBLE_EQ(trans->getElongationRate(),     50.0);
    EXPECT_DOUBLE_EQ(trans->getMeanGeneLength(),    900.0);
    EXPECT_DOUBLE_EQ(trans->getBindingProbability(),  1.0);
    EXPECT_DOUBLE_EQ(trans->getTimeWindow(),           1.0);
    EXPECT_EQ(trans->getMRNASpeciesPrefix(), "mRNA_");
    EXPECT_EQ(trans->getRnapCountKey(),      "RNAP_free");
}

TEST_F(WcmIntegration, StochasticTranslationDefaultProperties) {
    auto* trans = new StochasticTranslation(_model, "Translation");
    EXPECT_DOUBLE_EQ(trans->getElongationRate(),    16.0);
    EXPECT_DOUBLE_EQ(trans->getMeanProteinLength(), 300.0);
    EXPECT_DOUBLE_EQ(trans->getTimeWindow(),          1.0);
    EXPECT_EQ(trans->getMRNASpeciesPrefix(),    "mRNA_");
    EXPECT_EQ(trans->getProteinSpeciesPrefix(), "prot_");
    EXPECT_EQ(trans->getRibosomeCountKey(),     "ribosome_free");
}

} // namespace
