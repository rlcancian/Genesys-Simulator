#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Counter.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include "plugins/PluginConnectorDummyImpl1.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/BacteriaSignalGrid.h"
#include "plugins/data/BiologicalModeling/GroProgramCompiler.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"
#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace {

PluginInformation* BuildPluginWithMissingSystemDependency() {
    auto* info = new PluginInformation(
        "PluginWithMissingSystemDependency",
        static_cast<StaticLoaderDataDefinitionInstance>(nullptr),
        static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
    info->setCategory("Test");
    info->insertSystemDependency(SystemDependency(
        SystemDependency::OS::Any,
        "MissingDependency",
        "install-missing-dependency",
        "check-missing-dependency"));
    return info;
}

SystemCommandResult RuntimeCommandResultWithExitCode(int exitCode) {
    SystemCommandResult result;
    result.started = true;
    result.exitCode = exitCode;
    return result;
}

class RuntimeFakeCommandExecutor : public SystemCommandExecutor_if {
public:
    std::map<std::string, std::vector<SystemCommandResult>> results;
    std::vector<std::string> commands;

    SystemCommandResult run(const std::string& command) override {
        commands.push_back(command);
        auto it = results.find(command);
        if (it == results.end() || it->second.empty()) {
            return {};
        }
        SystemCommandResult result = it->second.front();
        it->second.erase(it->second.begin());
        return result;
    }
};

class RuntimeFakePluginConnector : public PluginConnector_if {
public:
    Plugin* check(const std::string) override {
        return new Plugin(&BuildPluginWithMissingSystemDependency);
    }

    Plugin* connect(const std::string) override {
        connectCalls++;
        return new Plugin(&BuildPluginWithMissingSystemDependency);
    }

    List<std::string>* find() override {
        return new List<std::string>();
    }

    bool disconnect(const std::string) override {
        return true;
    }

    bool disconnect(Plugin* plugin) override {
        delete plugin;
        disconnectedPlugins++;
        return true;
    }

    unsigned int disconnectedPlugins = 0;
    unsigned int connectCalls = 0;
};

}

TEST(RuntimePluginManagerClassTest, SimulatorProvidesPluginManagerWithDefaultPlugins) {
    Simulator simulator;

    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    EXPECT_GE(manager->size(), 4u);
    EXPECT_NE(manager->front(), nullptr);
}

TEST(RuntimePluginManagerClassTest, InsertReturnsNullptrAndDoesNotChangeSizeWhenLibraryIsMissing) {
    Simulator simulator;

    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);

    const auto before = manager->size();
    Plugin* inserted = manager->insert("definitely_missing_plugin_library.so");

    EXPECT_EQ(inserted, nullptr);
    EXPECT_EQ(manager->size(), before);
    ASSERT_EQ(manager->getPluginLoadIssues()->size(), 1u);
    const PluginLoadIssue issue = manager->getPluginLoadIssues()->front();
    EXPECT_EQ(issue.getFilename(), "definitely_missing_plugin_library.so");
    EXPECT_EQ(issue.getReason(), PluginLoadIssue::Reason::InvalidPlugin);
    EXPECT_FALSE(issue.getMessage().empty());
}

TEST(RuntimePluginManagerClassTest, InsertRefusesPluginWhenSystemDependencyIsMissingWithoutConfirmation) {
    Simulator simulator;
    auto* executor = new RuntimeFakeCommandExecutor();
    auto* connector = new RuntimeFakePluginConnector();
    executor->results["check-missing-dependency"] = {RuntimeCommandResultWithExitCode(1)};
    PluginManager manager(&simulator, connector, executor);

    const auto before = manager.size();
    Plugin* inserted = manager.insert("fake_plugin_library.so");

    EXPECT_EQ(inserted, nullptr);
    EXPECT_EQ(manager.size(), before);
    ASSERT_EQ(executor->commands.size(), 1u);
    EXPECT_EQ(executor->commands.front(), "check-missing-dependency");
    EXPECT_EQ(connector->disconnectedPlugins, 0u);
    EXPECT_EQ(connector->connectCalls, 0u);
    ASSERT_EQ(manager.getPluginLoadIssues()->size(), 1u);
    const PluginLoadIssue issue = manager.getPluginLoadIssues()->front();
    EXPECT_EQ(issue.getFilename(), "fake_plugin_library.so");
    EXPECT_EQ(issue.getPluginTypename(), "PluginWithMissingSystemDependency");
    EXPECT_EQ(issue.getReason(), PluginLoadIssue::Reason::MissingSystemDependency);
    EXPECT_TRUE(issue.hasSystemDependencyResult());
    EXPECT_NE(issue.diagnosticText().find("install-missing-dependency"), std::string::npos);
}

TEST(RuntimePluginManagerClassTest, InsertInstallsAndRevalidatesSystemDependencyWhenUserConfirms) {
    Simulator simulator;
    auto* executor = new RuntimeFakeCommandExecutor();
    executor->results["check-missing-dependency"] = {
        RuntimeCommandResultWithExitCode(1),
        RuntimeCommandResultWithExitCode(0),
        RuntimeCommandResultWithExitCode(0)
    };
    executor->results["install-missing-dependency"] = {RuntimeCommandResultWithExitCode(0)};
    auto* connector = new RuntimeFakePluginConnector();
    PluginManager manager(&simulator, connector, executor);
    PluginInsertionOptions options;
    bool confirmationAsked = false;
    options.confirmSystemDependencyInstallation = [&confirmationAsked](const SystemDependencyCheckResult& result) {
        confirmationAsked = true;
        EXPECT_FALSE(result.canInsertPlugin());
        return true;
    };

    const auto before = manager.size();
    Plugin* inserted = manager.insert("fake_plugin_library.so", options);

    ASSERT_NE(inserted, nullptr);
    EXPECT_TRUE(confirmationAsked);
    EXPECT_EQ(manager.size(), before + 1);
    EXPECT_EQ(connector->connectCalls, 1u);
    ASSERT_EQ(executor->commands.size(), 4u);
    EXPECT_EQ(executor->commands[0], "check-missing-dependency");
    EXPECT_EQ(executor->commands[1], "install-missing-dependency");
    EXPECT_EQ(executor->commands[2], "check-missing-dependency");
    EXPECT_EQ(executor->commands[3], "check-missing-dependency");
    EXPECT_TRUE(manager.getPluginLoadIssues()->empty());
}

TEST(RuntimePluginManagerClassTest, SuccessfulRetryClearsStoredPluginLoadIssue) {
    Simulator simulator;
    auto* executor = new RuntimeFakeCommandExecutor();
    executor->results["check-missing-dependency"] = {
        RuntimeCommandResultWithExitCode(1),
        RuntimeCommandResultWithExitCode(0),
        RuntimeCommandResultWithExitCode(0)
    };
    auto* connector = new RuntimeFakePluginConnector();
    PluginManager manager(&simulator, connector, executor);

    EXPECT_EQ(manager.insert("fake_plugin_library.so"), nullptr);
    ASSERT_EQ(manager.getPluginLoadIssues()->size(), 1u);

    Plugin* inserted = manager.insert("fake_plugin_library.so");

    ASSERT_NE(inserted, nullptr);
    EXPECT_TRUE(manager.getPluginLoadIssues()->empty());
    EXPECT_EQ(connector->connectCalls, 1u);
}

TEST(RuntimePluginManagerClassTest, DummyConnectorRegistersConcreteModelPlugins) {
    PluginConnectorDummyImpl1 connector;
    std::unique_ptr<List<std::string>> filenames(connector.find());
    ASSERT_NE(filenames, nullptr);

    const std::vector<std::string> expectedPluginFiles = {
        "bacteriacolony.so",
        "bacteriasignalgrid.so",
        "biosimulatorrunner.so",
        "cellularautomata.so",
        "defaultnode.so",
        "dummyelement.so",
        "groprogram.so",
        "old_odeelement.so",
        "petriplace.so",
        "rsimulator.so",
        "rsimulatorrunner.so",
        "submodel.so"
    };

    for (const std::string& filename : expectedPluginFiles) {
        EXPECT_NE(std::find(filenames->list()->begin(), filenames->list()->end(), filename), filenames->list()->end()) << filename;

        std::unique_ptr<Plugin> plugin(connector.connect(filename));
        ASSERT_NE(plugin, nullptr) << filename;
        ASSERT_NE(plugin->getPluginInfo(), nullptr) << filename;
        EXPECT_TRUE(plugin->isIsValidPlugin()) << filename;
        EXPECT_FALSE(plugin->getPluginInfo()->getPluginTypename().empty()) << filename;
    }
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPluginExposesFlowPorts) {
    PluginConnectorDummyImpl1 connector;
    std::unique_ptr<Plugin> plugin(connector.connect("bacteriacolony.so"));
    ASSERT_NE(plugin, nullptr);
    ASSERT_NE(plugin->getPluginInfo(), nullptr);

    PluginInformation* info = plugin->getPluginInfo();
    EXPECT_FALSE(info->isSource());
    EXPECT_FALSE(info->isSink());
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumInputs(), 1u);
    EXPECT_EQ(info->getMinimumOutputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
}

TEST(RuntimePluginManagerClassTest, BacteriaSignalGridCanBeCreatedAndValidated) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_1");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(2);
    signalGrid->setInitialSignal(0.5);
    signalGrid->setDiffusionRate(0.25);
    signalGrid->setDecayRate(0.1);
    signalGrid->setInitialValues("1.0, 0.5, 0.0, 2.0, 1.5, 1.0");

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(signalGrid, errorMessage)) << errorMessage;

    std::vector<double> values;
    ASSERT_TRUE(signalGrid->buildInitialField(values, errorMessage)) << errorMessage;
    ASSERT_EQ(values.size(), 6u);
    EXPECT_DOUBLE_EQ(values[0], 1.0);
    EXPECT_DOUBLE_EQ(values[3], 2.0);
    EXPECT_DOUBLE_EQ(values[5], 1.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramAndBacteriaColonyCanBeCreatedAndStepped) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_1");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program main() { tick(); }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_1");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.25);
    colony->setInitialColonyTime(2.0);
    colony->setFinalColonyTime(3.0);
    colony->setColonyTimeUnit(Util::TimeUnit::second);
    colony->setInitialPopulation(8);
    colony->setGridWidth(4);
    colony->setGridHeight(5);

    ModelDataDefinition::InitBetweenReplications(colony);

    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 2.0);
    EXPECT_DOUBLE_EQ(colony->getFinalColonyTime(), 3.0);
    EXPECT_EQ(colony->getColonyTimeUnit(), Util::TimeUnit::second);
    EXPECT_EQ(colony->getPopulationSize(), 8u);
    EXPECT_EQ(colony->getInternalBacteriaCount(), 8u);
    EXPECT_DOUBLE_EQ(colony->advanceColonyTime(), 2.25);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 2.25);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(program, errorMessage)) << errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(colony, errorMessage)) << errorMessage;
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyExecutesConfiguredGroProgram) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_2");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program colony() { tick(); grow(2); divide(); set_population(9); }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_2");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(1.0);
    colony->setInitialPopulation(3);
    colony->setGridWidth(2);
    colony->setGridHeight(2);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 3u);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).parentId, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).generation, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 0u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).birthTime, 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastDivisionTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumAge(0), 0.0);
    EXPECT_EQ(colony->getBacteriumState(0).gridX, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).gridY, 0u);
    EXPECT_EQ(colony->getBacteriumState(1).gridX, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).gridY, 0u);
    EXPECT_EQ(colony->getBacteriumState(2).gridX, 0u);
    EXPECT_EQ(colony->getBacteriumState(2).gridY, 1u);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();

    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(result.executedCommands, 4u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 1.5);
    EXPECT_EQ(colony->getPopulationSize(), 9u);
    ASSERT_EQ(result.populationMutations.size(), 3u);
    EXPECT_EQ(result.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Grow);
    EXPECT_EQ(result.populationMutations[0].value, 2u);
    EXPECT_EQ(result.populationMutations[0].previousPopulationSize, 3u);
    EXPECT_EQ(result.populationMutations[0].resultingPopulationSize, 5u);
    EXPECT_EQ(result.populationMutations[1].type, GroProgramRuntime::PopulationMutationType::Divide);
    EXPECT_EQ(result.populationMutations[1].value, 5u);
    EXPECT_EQ(result.populationMutations[1].previousPopulationSize, 5u);
    EXPECT_EQ(result.populationMutations[1].resultingPopulationSize, 10u);
    EXPECT_EQ(result.populationMutations[2].type, GroProgramRuntime::PopulationMutationType::SetPopulation);
    EXPECT_EQ(result.populationMutations[2].value, 9u);
    EXPECT_EQ(result.populationMutations[2].previousPopulationSize, 10u);
    EXPECT_EQ(result.populationMutations[2].resultingPopulationSize, 9u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 9u);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).parentId, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).generation, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).birthTime, 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 1.5);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastDivisionTime, 1.5);
    EXPECT_DOUBLE_EQ(colony->getBacteriumAge(0), 0.5);
    EXPECT_EQ(colony->getBacteriumState(3).id, 4u);
    EXPECT_EQ(colony->getBacteriumState(3).parentId, 0u);
    EXPECT_EQ(colony->getBacteriumState(3).generation, 0u);
    EXPECT_EQ(colony->getBacteriumState(3).divisionCount, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).birthTime, 1.5);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).lastUpdateTime, 1.5);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).lastDivisionTime, 1.5);
    EXPECT_EQ(colony->getBacteriumState(8).id, 9u);
    EXPECT_EQ(colony->getBacteriumState(8).parentId, 4u);
    EXPECT_EQ(colony->getBacteriumState(8).generation, 1u);
    EXPECT_EQ(colony->getBacteriumState(8).divisionCount, 0u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(8).birthTime, 1.5);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(8).lastDivisionTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumAge(8), 0.0);
    EXPECT_EQ(colony->getBacteriumState(8).gridX, 0u);
    EXPECT_EQ(colony->getBacteriumState(8).gridY, 0u);
    EXPECT_TRUE(result.unsupportedCommands.empty());
    EXPECT_TRUE(result.skippedRawStatements.empty());
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyForwardsEntityAfterRuntimeStep) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_Flow");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program colony() { tick(); grow(2); }");

    Create* create = manager->newInstance<Create>(model, "Create_Flow");
    ASSERT_NE(create, nullptr);
    create->setFirstCreation(0.0);
    create->setTimeBetweenCreationsExpression("1.0");
    create->setMaxCreations(1);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_Flow");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.25);
    colony->setInitialColonyTime(2.0);
    colony->setFinalColonyTime(2.5);
    colony->setColonyTimeUnit(Util::TimeUnit::second);
    colony->setInitialPopulation(4);

    Dispose* dispose = manager->newInstance<Dispose>(model, "Dispose_Flow");
    ASSERT_NE(dispose, nullptr);

    create->connectTo(colony);
    colony->connectTo(dispose);

    model->getSimulation()->setReplicationLength(1.0);
    model->getSimulation()->start();

    Counter* disposed = dynamic_cast<Counter*>(dispose->getInternalData("CountNumberIn"));
    ASSERT_NE(disposed, nullptr);
    // The entity should remain inside the colony until the internal colony
    // clock reaches the configured final time and is then forwarded once.
    EXPECT_DOUBLE_EQ(disposed->getCountValue(), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesDieCommandToInternalState) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_Die");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program colony() { tick(); die(2); }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_Die");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(1.0);
    colony->setInitialPopulation(5);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();

    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(result.executedCommands, 2u);
    ASSERT_EQ(result.populationMutations.size(), 1u);
    EXPECT_EQ(result.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Die);
    EXPECT_EQ(result.populationMutations[0].value, 2u);
    EXPECT_EQ(result.populationMutations[0].previousPopulationSize, 5u);
    EXPECT_EQ(result.populationMutations[0].resultingPopulationSize, 3u);
    EXPECT_EQ(colony->getPopulationSize(), 3u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 3u);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).id, 2u);
    EXPECT_EQ(colony->getBacteriumState(2).id, 3u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 1.5);
}

TEST(RuntimePluginManagerClassTest, GroProgramParserKeepsLexicalValidationBoundary) {
    GroProgramParser parser;

    GroProgramParser::Result commented = parser.parse("program main() { tick(\"}\"); /* ignored { */ }");
    EXPECT_TRUE(commented.accepted) << commented.errorMessage;
    EXPECT_TRUE(commented.ast.isProgramBlock());

    GroProgramParser::Result accepted = parser.parse("program main() { tick(\"}\"); divide(); }");
    EXPECT_TRUE(accepted.accepted) << accepted.errorMessage;
    EXPECT_TRUE(accepted.errorMessage.empty());
    EXPECT_TRUE(accepted.ast.isProgramBlock());
    EXPECT_EQ(accepted.ast.programName, "main");
    EXPECT_EQ(accepted.ast.bodySource, "tick(\"}\"); divide();");
    ASSERT_EQ(accepted.ast.statements.size(), 2u);
    EXPECT_EQ(accepted.ast.statements[0].sourceText, "tick(\"}\")");
    EXPECT_EQ(accepted.ast.statements[1].sourceText, "divide()");

    GroProgramParser::Result rawStatements = parser.parse("tick(); grow();");
    EXPECT_TRUE(rawStatements.accepted) << rawStatements.errorMessage;
    EXPECT_EQ(rawStatements.ast.sourceForm, GroProgramAst::SourceForm::RawStatements);
    ASSERT_EQ(rawStatements.ast.statements.size(), 2u);
    EXPECT_EQ(rawStatements.ast.statements[0].sourceText, "tick()");
    EXPECT_EQ(rawStatements.ast.statements[1].sourceText, "grow()");

    GroProgramParser::Result rejected = parser.parse("program main() { tick(); ");
    EXPECT_FALSE(rejected.accepted);
    EXPECT_NE(rejected.errorMessage.find("unmatched opening delimiters"), std::string::npos);
}

TEST(RuntimePluginManagerClassTest, GroProgramCompilerBuildsInitialSemanticIr) {
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program colony() { tick(); set_rate(mu, 0.2); observe(\"a,b\"); raw + expression; }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    EXPECT_TRUE(ir.isProgramBlock());
    EXPECT_EQ(ir.programName, "colony");
    ASSERT_EQ(ir.commands.size(), 4u);

    EXPECT_TRUE(ir.commands[0].isFunctionCall());
    EXPECT_EQ(ir.commands[0].functionName, "tick");
    EXPECT_TRUE(ir.commands[0].arguments.empty());
    EXPECT_EQ(ir.commands[0].sourceText, "tick()");

    EXPECT_TRUE(ir.commands[1].isFunctionCall());
    EXPECT_EQ(ir.commands[1].functionName, "set_rate");
    ASSERT_EQ(ir.commands[1].arguments.size(), 2u);
    EXPECT_EQ(ir.commands[1].arguments[0], "mu");
    EXPECT_EQ(ir.commands[1].arguments[1], "0.2");

    EXPECT_TRUE(ir.commands[2].isFunctionCall());
    EXPECT_EQ(ir.commands[2].functionName, "observe");
    ASSERT_EQ(ir.commands[2].arguments.size(), 1u);
    EXPECT_EQ(ir.commands[2].arguments[0], "\"a,b\"");

    EXPECT_FALSE(ir.commands[3].isFunctionCall());
    EXPECT_EQ(ir.commands[3].sourceText, "raw + expression");
}

TEST(RuntimePluginManagerClassTest, GroProgramCompilerBuildsAssignmentsAndConditionals) {
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program colony() { accumulator = population + 1; if (accumulator > 3) { grow(accumulator - 1); } else { die(); } }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    ASSERT_EQ(ir.commands.size(), 2u);
    EXPECT_TRUE(ir.commands[0].isAssignment());
    EXPECT_EQ(ir.commands[0].assignmentTarget, "accumulator");
    EXPECT_EQ(ir.commands[0].expressionText, "population + 1");

    EXPECT_TRUE(ir.commands[1].isIfStatement());
    EXPECT_EQ(ir.commands[1].expressionText, "accumulator > 3");
    ASSERT_EQ(ir.commands[1].thenCommands.size(), 1u);
    ASSERT_EQ(ir.commands[1].elseCommands.size(), 1u);
    EXPECT_TRUE(ir.commands[1].thenCommands[0].isFunctionCall());
    EXPECT_EQ(ir.commands[1].thenCommands[0].functionName, "grow");
    ASSERT_EQ(ir.commands[1].thenCommands[0].arguments.size(), 1u);
    EXPECT_EQ(ir.commands[1].thenCommands[0].arguments[0], "accumulator - 1");
    EXPECT_TRUE(ir.commands[1].elseCommands[0].isFunctionCall());
    EXPECT_EQ(ir.commands[1].elseCommands[0].functionName, "die");
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeExecutesInitialTickCommand) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse(
	    "program colony() { tick(); grow(); grow(3); divide(); set_population(7); observe(); raw + expression; tick(); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	state.colonyTime = 2.0;
	state.simulationStep = 0.25;
	state.populationSize = 2;

	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_EQ(result.executedCommands, 6u);
	EXPECT_DOUBLE_EQ(state.colonyTime, 2.5);
	EXPECT_EQ(state.tickCount, 2u);
	EXPECT_EQ(state.populationSize, 7u);
	ASSERT_EQ(result.populationMutations.size(), 4u);
	EXPECT_EQ(result.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Grow);
	EXPECT_EQ(result.populationMutations[0].value, 1u);
	EXPECT_EQ(result.populationMutations[0].previousPopulationSize, 2u);
	EXPECT_EQ(result.populationMutations[0].resultingPopulationSize, 3u);
	EXPECT_EQ(result.populationMutations[1].type, GroProgramRuntime::PopulationMutationType::Grow);
	EXPECT_EQ(result.populationMutations[1].value, 3u);
	EXPECT_EQ(result.populationMutations[1].previousPopulationSize, 3u);
	EXPECT_EQ(result.populationMutations[1].resultingPopulationSize, 6u);
	EXPECT_EQ(result.populationMutations[2].type, GroProgramRuntime::PopulationMutationType::Divide);
	EXPECT_EQ(result.populationMutations[2].value, 6u);
	EXPECT_EQ(result.populationMutations[2].previousPopulationSize, 6u);
	EXPECT_EQ(result.populationMutations[2].resultingPopulationSize, 12u);
	EXPECT_EQ(result.populationMutations[3].type, GroProgramRuntime::PopulationMutationType::SetPopulation);
	EXPECT_EQ(result.populationMutations[3].value, 7u);
	EXPECT_EQ(result.populationMutations[3].previousPopulationSize, 12u);
	EXPECT_EQ(result.populationMutations[3].resultingPopulationSize, 7u);
	ASSERT_EQ(result.unsupportedCommands.size(), 1u);
	EXPECT_EQ(result.unsupportedCommands[0], "observe()");
    ASSERT_EQ(result.skippedRawStatements.size(), 1u);
    EXPECT_EQ(result.skippedRawStatements[0], "raw + expression");

    GroProgramParser::Result invalidTick = parser.parse("tick(1);");
    ASSERT_TRUE(invalidTick.accepted) << invalidTick.errorMessage;
    GroProgramIr invalidIr = compiler.compile(invalidTick.ast);

	GroProgramRuntime::ExecutionResult invalidResult = runtime.execute(invalidIr, state);
	EXPECT_FALSE(invalidResult.succeeded);
	EXPECT_NE(invalidResult.errorMessage.find("tick command does not accept arguments"), std::string::npos);

	GroProgramParser::Result invalidPopulation = parser.parse("set_population(0);");
	ASSERT_TRUE(invalidPopulation.accepted) << invalidPopulation.errorMessage;
	GroProgramIr invalidPopulationIr = compiler.compile(invalidPopulation.ast);

	GroProgramRuntime::ExecutionResult invalidPopulationResult = runtime.execute(invalidPopulationIr, state);
	EXPECT_FALSE(invalidPopulationResult.succeeded);
	EXPECT_NE(invalidPopulationResult.errorMessage.find("set_population command expects one positive integer expression argument"),
	          std::string::npos);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsDieCommand) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse("program colony() { die(); die(2); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	state.populationSize = 5;

	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_EQ(result.executedCommands, 2u);
	EXPECT_EQ(state.populationSize, 2u);
	ASSERT_EQ(result.populationMutations.size(), 2u);
	EXPECT_EQ(result.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Die);
	EXPECT_EQ(result.populationMutations[0].value, 1u);
	EXPECT_EQ(result.populationMutations[0].previousPopulationSize, 5u);
	EXPECT_EQ(result.populationMutations[0].resultingPopulationSize, 4u);
	EXPECT_EQ(result.populationMutations[1].type, GroProgramRuntime::PopulationMutationType::Die);
	EXPECT_EQ(result.populationMutations[1].value, 2u);
	EXPECT_EQ(result.populationMutations[1].previousPopulationSize, 4u);
	EXPECT_EQ(result.populationMutations[1].resultingPopulationSize, 2u);

	GroProgramParser::Result invalidDeath = parser.parse("die(3);");
	ASSERT_TRUE(invalidDeath.accepted) << invalidDeath.errorMessage;
	GroProgramIr invalidIr = compiler.compile(invalidDeath.ast);

	GroProgramRuntime::ExecutionResult invalidResult = runtime.execute(invalidIr, state);
	EXPECT_FALSE(invalidResult.succeeded);
	EXPECT_NE(invalidResult.errorMessage.find("die command would remove more bacteria"), std::string::npos);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeExecutesAssignmentsAndConditionalsAcrossRuns) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse(
	    "program colony() { acc = acc + 1; if (acc == 1) { grow(acc + 1); tick(); } else { die(acc - 1); } }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	state.colonyTime = 2.0;
	state.simulationStep = 0.25;
	state.populationSize = 2;

	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult firstResult = runtime.execute(ir, state);

	EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
	EXPECT_EQ(firstResult.executedCommands, 4u);
	EXPECT_DOUBLE_EQ(state.colonyTime, 2.25);
	EXPECT_EQ(state.populationSize, 4u);
	ASSERT_EQ(firstResult.populationMutations.size(), 1u);
	EXPECT_EQ(firstResult.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Grow);
	EXPECT_EQ(firstResult.populationMutations[0].value, 2u);
	ASSERT_EQ(firstResult.assignedVariables.size(), 1u);
	EXPECT_DOUBLE_EQ(firstResult.assignedVariables.at("acc"), 1.0);
	EXPECT_DOUBLE_EQ(state.variables.at("acc"), 1.0);

	GroProgramRuntime::ExecutionResult secondResult = runtime.execute(ir, state);

	EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
	EXPECT_EQ(secondResult.executedCommands, 3u);
	EXPECT_DOUBLE_EQ(state.colonyTime, 2.25);
	EXPECT_EQ(state.populationSize, 3u);
	ASSERT_EQ(secondResult.populationMutations.size(), 1u);
	EXPECT_EQ(secondResult.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Die);
	EXPECT_EQ(secondResult.populationMutations[0].value, 1u);
	EXPECT_DOUBLE_EQ(secondResult.assignedVariables.at("acc"), 2.0);
	EXPECT_DOUBLE_EQ(state.variables.at("acc"), 2.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPreservesRuntimeVariablesAcrossExecutions) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_Stateful");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program colony() { counter = counter + 1; if (counter == 1) { grow(counter + 1); } else { die(counter - 1); } }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_Stateful");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(1.0);
    colony->setInitialPopulation(2);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_FALSE(colony->hasRuntimeVariable("counter"));

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_TRUE(colony->hasRuntimeVariable("counter"));
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("counter"), 1.0);
    EXPECT_EQ(colony->getPopulationSize(), 4u);

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("counter"), 2.0);
    EXPECT_EQ(colony->getPopulationSize(), 3u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyExecutesBacteriumScopedProgramsWithPerBacteriumState) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_PerBacterium");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "age_steps = age_steps + 1; "
        "seen_age = bacterium_age; "
        "if (bacterium_id == 1 && age_steps == 1) { grow(); } "
        "if (bacterium_generation == 0 && bacterium_id != 1) { die(); } "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_PerBacterium");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(1.0);
    colony->setInitialPopulation(2);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_EQ(firstResult.executedCommands, 10u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 1.5);
    EXPECT_EQ(colony->getPopulationSize(), 2u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "age_steps"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "seen_age"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "age_steps"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_age"), 0.5);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastDivisionTime, 1.5);
    EXPECT_EQ(colony->getBacteriumState(1).parentId, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).generation, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(1).birthTime, 1.5);

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_EQ(secondResult.executedCommands, 8u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 2.0);
    EXPECT_EQ(colony->getPopulationSize(), 2u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "age_steps"), 2.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_age"), 1.0);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "age_steps"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "seen_age"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "age_steps"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "seen_age"), 0.5);
    EXPECT_EQ(colony->getBacteriumState(1).parentId, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).generation, 1u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyExecutesSignalAwareBacteriumProgram) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_Runtime");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(1);
    signalGrid->setDiffusionRate(0.5);
    signalGrid->setDecayRate(0.0);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SignalAware");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "seen_signal = local_signal; "
        "if (bacterium_id == 1) { emit_signal(4); } "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SignalAware");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(1.0);
    colony->setInitialPopulation(1);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_EQ(firstResult.signalMutations.size(), 1u);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 0), 1.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(2, 0), 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_signal"), 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumLocalSignal(0), 2.0);

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_signal"), 2.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 3.5);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(2, 0), 0.5);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyReusesBioNetworkAsBiochemicalContext) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioSpecies* substrate = manager->newInstance<BioSpecies>(model, "Substrate");
    ASSERT_NE(substrate, nullptr);
    substrate->setInitialAmount(10.0);

    BioSpecies* product = manager->newInstance<BioSpecies>(model, "Product");
    ASSERT_NE(product, nullptr);
    product->setInitialAmount(0.0);

    BioReaction* reaction = manager->newInstance<BioReaction>(model, "Reaction_AB");
    ASSERT_NE(reaction, nullptr);
    reaction->addReactant("Substrate", 1.0);
    reaction->addProduct("Product", 1.0);
    reaction->setKineticLawExpression("0.5 * Substrate");

    BioNetwork* bioNetwork = manager->newInstance<BioNetwork>(model, "BioNetwork_Colony");
    ASSERT_NE(bioNetwork, nullptr);
    bioNetwork->addSpecies("Substrate");
    bioNetwork->addSpecies("Product");
    bioNetwork->addReaction("Reaction_AB");
    bioNetwork->setStartTime(0.0);
    bioNetwork->setStopTime(1.0);
    bioNetwork->setStepSize(0.5);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_BioAware");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program colony() { "
        "seen_substrate = bio_species_substrate; "
        "seen_product = bio_species_product; "
        "seen_bio_time = bio_current_time; "
        "tick(); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_BioAware");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setBioNetwork(bioNetwork);
    colony->setSimulationStep(0.5);
    colony->setInitialColonyTime(0.0);
    colony->setInitialPopulation(1);

    ModelDataDefinition::InitBetweenReplications(substrate);
    ModelDataDefinition::InitBetweenReplications(product);
    ModelDataDefinition::InitBetweenReplications(bioNetwork);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_substrate"), 10.0);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_product"), 0.0);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_bio_time"), 0.0);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.5);
    EXPECT_LT(substrate->getAmount(), 10.0);
    EXPECT_GT(product->getAmount(), 0.0);

    const double substrateAfterFirstStep = substrate->getAmount();
    const double productAfterFirstStep = product->getAmount();

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_substrate"), substrateAfterFirstStep);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_product"), productAfterFirstStep);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_bio_time"), 0.5);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 1.0);
    EXPECT_LT(substrate->getAmount(), substrateAfterFirstStep);
    EXPECT_GT(product->getAmount(), productAfterFirstStep);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAssignmentsCanWriteBioNetworkSpecies) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioSpecies* substrate = manager->newInstance<BioSpecies>(model, "Substrate");
    ASSERT_NE(substrate, nullptr);
    substrate->setInitialAmount(8.0);

    BioReaction* noop = manager->newInstance<BioReaction>(model, "Reaction_NoOp");
    ASSERT_NE(noop, nullptr);
    noop->addReactant("Substrate", 1.0);
    noop->addProduct("Substrate", 1.0);
    noop->setRateConstant(0.0);

    BioNetwork* bioNetwork = manager->newInstance<BioNetwork>(model, "BioNetwork_Writeback");
    ASSERT_NE(bioNetwork, nullptr);
    bioNetwork->addSpecies("Substrate");
    bioNetwork->addReaction("Reaction_NoOp");
    bioNetwork->setStartTime(0.0);
    bioNetwork->setStopTime(1.0);
    bioNetwork->setStepSize(0.5);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_BioWriteback");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program colony() { "
        "before_value = bio_species_substrate; "
        "bio_species_substrate = bio_species_substrate - 3; "
        "tick(); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_BioWriteback");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setBioNetwork(bioNetwork);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);

    ModelDataDefinition::InitBetweenReplications(substrate);
    ModelDataDefinition::InitBetweenReplications(bioNetwork);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("before_value"), 8.0);
    EXPECT_DOUBLE_EQ(substrate->getAmount(), 5.0);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.5);
    EXPECT_FALSE(colony->hasRuntimeVariable("bio_species_substrate"));
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyBacteriumScopedProgramsCanSequentiallyUpdateBioNetworkSpecies) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioSpecies* shared = manager->newInstance<BioSpecies>(model, "SharedSignal");
    ASSERT_NE(shared, nullptr);
    shared->setInitialAmount(0.0);

    BioReaction* noop = manager->newInstance<BioReaction>(model, "Reaction_SharedNoOp");
    ASSERT_NE(noop, nullptr);
    noop->addReactant("SharedSignal", 1.0);
    noop->addProduct("SharedSignal", 1.0);
    noop->setRateConstant(0.0);

    BioNetwork* bioNetwork = manager->newInstance<BioNetwork>(model, "BioNetwork_Shared");
    ASSERT_NE(bioNetwork, nullptr);
    bioNetwork->addSpecies("SharedSignal");
    bioNetwork->addReaction("Reaction_SharedNoOp");
    bioNetwork->setStartTime(0.0);
    bioNetwork->setStopTime(1.0);
    bioNetwork->setStepSize(0.5);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_BacteriumBioWriteback");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "seen_shared = bio_species_sharedsignal; "
        "bio_species_sharedsignal = bio_species_sharedsignal + 1; "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_BacteriumBioWriteback");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setBioNetwork(bioNetwork);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(2);

    ModelDataDefinition::InitBetweenReplications(shared);
    ModelDataDefinition::InitBetweenReplications(bioNetwork);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_shared"), 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "seen_shared"), 1.0);
    EXPECT_DOUBLE_EQ(shared->getAmount(), 2.0);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.5);
    EXPECT_FALSE(colony->hasBacteriumRuntimeVariable(0, "bio_species_sharedsignal"));
    EXPECT_FALSE(colony->hasBacteriumRuntimeVariable(1, "bio_species_sharedsignal"));
}
