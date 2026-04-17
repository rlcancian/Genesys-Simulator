#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Counter.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include "plugins/PluginConnectorDummyImpl1.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
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
    colony->setInitialPopulation(8);
    colony->setGridWidth(4);
    colony->setGridHeight(5);

    ModelDataDefinition::InitBetweenReplications(colony);

    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 2.0);
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
    colony->setInitialPopulation(4);

    Dispose* dispose = manager->newInstance<Dispose>(model, "Dispose_Flow");
    ASSERT_NE(dispose, nullptr);

    create->connectTo(colony);
    colony->connectTo(dispose);

    model->getSimulation()->setReplicationLength(1.0);
    model->getSimulation()->start();

    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 2.25);
    EXPECT_EQ(colony->getPopulationSize(), 6u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 6u);

    Counter* disposed = dynamic_cast<Counter*>(dispose->getInternalData("CountNumberIn"));
    ASSERT_NE(disposed, nullptr);
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
	EXPECT_NE(invalidPopulationResult.errorMessage.find("set_population command expects one positive integer argument"),
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
