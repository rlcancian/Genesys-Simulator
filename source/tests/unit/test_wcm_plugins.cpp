#include <gtest/gtest.h>

#include <cmath>
#include <map>
#include <string>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/WholeCellModeling/MolecularSpecies.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

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
