#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include "plugins/PluginConnectorDummyImpl1.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
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
    EXPECT_DOUBLE_EQ(colony->advanceColonyTime(), 2.25);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(program, errorMessage)) << errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(colony, errorMessage)) << errorMessage;
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
        "program colony() { tick(); observe(); raw + expression; tick(); }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    GroProgramRuntimeState state;
    state.colonyTime = 2.0;
    state.simulationStep = 0.25;

    GroProgramRuntime runtime;
    GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(result.executedCommands, 2u);
    EXPECT_DOUBLE_EQ(state.colonyTime, 2.5);
    EXPECT_EQ(state.tickCount, 2u);
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
}
