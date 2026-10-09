#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"
#include "../../kernel/simulator/essentialPlugins/Counter.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include "plugins/PluginConnectorDummyImpl1.h"
#include "../../plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "../../plugins/components/Decisions/DropOff.h"
#include "../../plugins/components/Logic/Create.h"
#include "../../plugins/components/Logic/Dispose.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"
#include "plugins/data/BiochemicalSimulation/GroProgramCompiler.h"
#include "plugins/data/BiochemicalSimulation/GroProgramParser.h"
#include "plugins/data/BiochemicalSimulation/GroProgramRuntime.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <vector>

namespace {

std::vector<std::string> g_capturedTraceMessages;

void CaptureTraceSimulationEvent(TraceSimulationEvent event) {
    g_capturedTraceMessages.push_back(event.getText());
}

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
        "conveyor.so",
        "defaultnode.so",
        "distance.so",
        "dummyelement.so",
        "groprogram.so",
        "move.so",
        "old_odeelement.so",
        "petriplace.so",
        "rsimulator.so",
        "rsimulatorrunner.so",
        "segment.so",
        "submodel.so",
        "transporter.so"
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

TEST(RuntimePluginManagerClassTest, DropOffDoesNotDeclareEssentialAttributeAsDynamicDependency) {
    std::unique_ptr<PluginInformation> info(DropOff::GetPluginInformation());
    ASSERT_NE(info, nullptr);
    ASSERT_NE(info->getDynamicLibFilenameDependencies(), nullptr);

    const auto* dependencies = info->getDynamicLibFilenameDependencies();
    EXPECT_NE(std::find(dependencies->begin(), dependencies->end(), "entitygroup.so"), dependencies->end());
    EXPECT_EQ(std::find(dependencies->begin(), dependencies->end(), "attribute.so"), dependencies->end());
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
    EXPECT_EQ(signalGrid->getWidth(), 32u);
    EXPECT_EQ(signalGrid->getHeight(), 32u);
    EXPECT_GT(signalGrid->getDiffusionRate(), 0.0);
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

TEST(RuntimePluginManagerClassTest, BacteriaSignalGridRejectsInvalidRelaxationCoefficients) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_InvalidRates");
    ASSERT_NE(signalGrid, nullptr);

    const auto expectInvalid = [&](double diffusion, double decay) {
        signalGrid->setDiffusionRate(diffusion);
        signalGrid->setDecayRate(decay);
        std::string errorMessage;
        EXPECT_FALSE(ModelDataDefinition::Check(signalGrid, errorMessage));
        EXPECT_NE(errorMessage.find("finite value in the [0,1] interval"), std::string::npos) << errorMessage;
    };

    expectInvalid(-0.01, 0.1);
    expectInvalid(1.01, 0.1);
    expectInvalid(0.1, -0.01);
    expectInvalid(0.1, 1.01);
    expectInvalid(std::numeric_limits<double>::quiet_NaN(), 0.1);
    expectInvalid(0.1, std::numeric_limits<double>::infinity());

    signalGrid->setDiffusionRate(0.0);
    signalGrid->setDecayRate(1.0);
    std::string endpointError;
    EXPECT_TRUE(ModelDataDefinition::Check(signalGrid, endpointError)) << endpointError;
}

TEST(RuntimePluginManagerClassTest, GroProgramCanCreateDefaultStarterAndKeepEmptySourceValid) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_Default");
    ASSERT_NE(program, nullptr);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(program, errorMessage)) << errorMessage;

    const std::string filename = "/tmp/genesys_default_gro_program_test.gro";
    EXPECT_TRUE(program->createDefaultGroProgram(filename));
    EXPECT_FALSE(program->getSourceCode().empty());
    EXPECT_NE(program->getSourceCode().find("program leader()"), std::string::npos);
    EXPECT_NE(program->getSourceCode().find("program follower()"), std::string::npos);
    EXPECT_NE(program->getSourceCode().find("program main()"), std::string::npos);
    EXPECT_NE(program->getSourceCode().find("emit_signal ( wave, 22 )"), std::string::npos);
    EXPECT_TRUE(program->validateSyntax(errorMessage)) << errorMessage;

    std::ifstream createdFile(filename);
    ASSERT_TRUE(createdFile.is_open());
    const std::string fileContent((std::istreambuf_iterator<char>(createdFile)),
                                  std::istreambuf_iterator<char>());
    EXPECT_EQ(fileContent, program->getSourceCode());
    std::remove(filename.c_str());
}

TEST(RuntimePluginManagerClassTest, DefaultGroProgramProducesVisibleColonyDynamics) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_DefaultDynamics");
    ASSERT_NE(program, nullptr);

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_DefaultDynamics");
    ASSERT_NE(signalGrid, nullptr);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_DefaultDynamics");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);

    ModelDataDefinition::InitBetweenReplications(colony);

    ASSERT_EQ(colony->getInternalBacteriaCount(), 5u);
    EXPECT_EQ(colony->getBacteriumState(0).programName, "leader");
    EXPECT_EQ(colony->getGridWidth(), 32u);
    EXPECT_EQ(colony->getGridHeight(), 32u);

    double maxSignal = 0.0;
    bool dynamicsObserved = false;
    for (unsigned int step = 0; step < 3; ++step) {
        GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
        ASSERT_TRUE(result.succeeded) << result.errorMessage;

        maxSignal = 0.0;
        for (unsigned int y = 0; y < colony->getGridHeight(); ++y) {
            for (unsigned int x = 0; x < colony->getGridWidth(); ++x) {
                maxSignal = std::max(maxSignal, colony->getSignalValueAt(x, y));
            }
        }

        if (maxSignal > 0.0 &&
            colony->getBacteriumPositionX(0) != 16.0 &&
            colony->getBacteriumPositionY(0) != 16.0 &&
            colony->getBacteriumSize(0) > 0.0 &&
            colony->getBacteriumVolume(0) > 1.0 &&
            colony->getBacteriumRuntimeVariableValue(0, "gfp") > 0.0) {
            dynamicsObserved = true;
            break;
        }
    }

    EXPECT_TRUE(dynamicsObserved);
    EXPECT_EQ(colony->getPopulationSize(), colony->getInternalBacteriaCount());
    EXPECT_GT(maxSignal, 0.0);
    EXPECT_NE(colony->getBacteriumPositionX(0), 16.0);
    EXPECT_NE(colony->getBacteriumPositionY(0), 16.0);
    EXPECT_GT(colony->getBacteriumSize(0), 0.0);
    EXPECT_GT(colony->getBacteriumVolume(0), 1.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "gfp"), 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriumScopedGroPreservesDivisionFlagsAndAbsorbSignalAlias) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_DivisionFlags");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "wave := signal(1, 0.05); "
        "program bacterium() := { "
        "  just_divided & daughter : { gfp := 40 } "
        "  !just_divided : { emit_signal(wave, 5), divide() } "
        "  just_divided : { absorb_signal(wave, 1) } "
        "}; "
        "ecoli([x:=1, y:=1], program bacterium());");

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_DivisionFlags");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(8);
    signalGrid->setHeight(8);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_DivisionFlags");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult first = colony->executeGroProgram();
    ASSERT_TRUE(first.succeeded) << first.errorMessage;
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);

    GroProgramRuntime::ExecutionResult second = colony->executeGroProgram();
    ASSERT_TRUE(second.succeeded) << second.errorMessage;

    bool foundDaughterMarker = false;
    for (std::size_t index = 0; index < colony->getInternalBacteriaCount(); ++index) {
        if (colony->getBacteriumGfp(index) > 0.0) {
            foundDaughterMarker = true;
            break;
        }
    }
    EXPECT_TRUE(foundDaughterMarker);
    EXPECT_GE(colony->getSignalValueAt(1, 1), 0.0);
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
    colony->setNumSteps(4);
    colony->setColonyTimeUnit(Util::TimeUnit::second);
    colony->setInitialPopulation(8);
    colony->setGridWidth(4);
    colony->setGridHeight(5);

    ModelDataDefinition::InitBetweenReplications(colony);

    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_EQ(colony->getNumSteps(), 4u);
    EXPECT_EQ(colony->getColonyTimeUnit(), Util::TimeUnit::second);
    EXPECT_EQ(colony->getPopulationSize(), 8u);
    EXPECT_EQ(colony->getInternalBacteriaCount(), 8u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 0.0);

    std::string errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(program, errorMessage)) << errorMessage;
    EXPECT_TRUE(ModelDataDefinition::Check(colony, errorMessage)) << errorMessage;
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyKeepsBioNetworkTimeIndependentAndUsesSignalGridDimensions) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BioNetwork* network = manager->newInstance<BioNetwork>(model, "BioNetwork_ColonyAuthority");
    ASSERT_NE(network, nullptr);
    network->setStartTime(2.0);
    network->setStopTime(5.0);
    network->setStepSize(0.25);
    network->setCurrentTime(2.0);

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_ColonyAuthority");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(2);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_Authority");
    ASSERT_NE(colony, nullptr);
    colony->setBioNetwork(network);
    colony->setSignalGrid(signalGrid);

    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_DOUBLE_EQ(colony->getSimulationStep(), 0.01);
    EXPECT_EQ(colony->getGridWidth(), 3u);
    EXPECT_EQ(colony->getGridHeight(), 2u);

    colony->setSimulationStep(0.5);
    colony->setGridWidth(4);
    colony->setGridHeight(6);

    EXPECT_DOUBLE_EQ(network->getStepSize(), 0.25);
    EXPECT_DOUBLE_EQ(network->getStartTime(), 2.0);
    EXPECT_DOUBLE_EQ(network->getCurrentTime(), 2.0);
    EXPECT_DOUBLE_EQ(network->getStopTime(), 5.0);
    EXPECT_EQ(signalGrid->getWidth(), 4u);
    EXPECT_EQ(signalGrid->getHeight(), 6u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyDiffusesExplicitSignalGridValues) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SignalDiffusion");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program colony() { skip(); }");

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_Diffusion");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(3);
    signalGrid->setInitialSignal(0.0);
    signalGrid->setDiffusionRate(0.5);
    signalGrid->setDecayRate(0.0);
    signalGrid->setInitialValues("0,0,0,0,10,0,0,0,0");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SignalDiffusion");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    colony->setInitialPopulation(0);

    ModelDataDefinition::InitBetweenReplications(colony);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 1), 10.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 0), 0.0);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_LT(colony->getSignalValueAt(1, 1), 10.0);
    EXPECT_GT(colony->getSignalValueAt(0, 1), 0.0);
    EXPECT_GT(colony->getSignalValueAt(1, 0), 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyCharacterizesLegacySignalRelaxationOnSmallGrids) {
    // Characterization contract for the existing dimensionless per-step
    // relaxation operator. Expected values below are hand-calculated from
    // the graph degrees and are intentionally not claims of physical mass
    // conservation.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SignalStencilCharacterization");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("program colony() { tick(); }");
    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_StencilCharacterization");
    ASSERT_NE(signalGrid, nullptr);
    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_StencilCharacterization");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    colony->setInitialPopulation(0);

    const auto run = [&](unsigned int width, unsigned int height, const std::vector<double>& initial,
                         double diffusion, double decay, double dt) {
        signalGrid->setWidth(width);
        signalGrid->setHeight(height);
        signalGrid->setDiffusionRate(diffusion);
        signalGrid->setDecayRate(decay);
        std::string serialized;
        for (std::size_t i = 0; i < initial.size(); ++i) {
            if (i != 0) serialized += ",";
            serialized += std::to_string(initial[i]);
        }
        signalGrid->setInitialValues(serialized);
        colony->setSimulationStep(dt);
        ModelDataDefinition::InitBetweenReplications(colony);
        GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
        EXPECT_TRUE(result.succeeded) << result.errorMessage;
        std::vector<double> actual;
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) actual.push_back(colony->getSignalValueAt(x, y));
        }
        return actual;
    };

    // 1x1 has no neighbors: diffusion cannot act; decay is applied once.
    EXPECT_NEAR(run(1, 1, {8.0}, 0.75, 0.25, 0.01).at(0), 6.0, 1e-12);

    // Every node in 2x2 has degree two. A corner impulse of 10 with
    // diffusion=0.5 splits to 5 at the source and 2.5 at each adjacent node.
    const std::vector<double> corner2 = run(2, 2, {10, 0, 0, 0}, 0.5, 0.0, 0.1);
    EXPECT_NEAR(corner2[0], 5.0, 1e-12);
    EXPECT_NEAR(corner2[1], 2.5, 1e-12);
    EXPECT_NEAR(corner2[2], 2.5, 1e-12);
    EXPECT_NEAR(corner2[3], 0.0, 1e-12);
    EXPECT_NEAR(corner2[0] + corner2[1] + corner2[2] + corner2[3], 10.0, 1e-12);

    // Uniform values are unchanged by the neighbor relaxation and then
    // multiplied by 1-decay, including at edges with smaller degree.
    const std::vector<double> uniform3 = run(3, 3, std::vector<double>(9, 4.0), 0.7, 0.25, 0.1);
    for (double value : uniform3) EXPECT_NEAR(value, 3.0, 1e-12);

    // A center impulse has degree four; its orthogonal neighbors have degree
    // three. Thus 10 -> 5 at center, and 10/3*0.5 at each adjacent node.
    const std::vector<double> center3 = run(3, 3, {0,0,0, 0,10,0, 0,0,0}, 0.5, 0.0, 0.1);
    EXPECT_NEAR(center3[4], 5.0, 1e-12);
    EXPECT_NEAR(center3[1], 5.0 / 3.0, 1e-12);
    EXPECT_NEAR(center3[3], 5.0 / 3.0, 1e-12);
    EXPECT_NEAR(center3[5], 5.0 / 3.0, 1e-12);
    EXPECT_NEAR(center3[7], 5.0 / 3.0, 1e-12);
    const double centerMass = std::accumulate(center3.begin(), center3.end(), 0.0);
    EXPECT_NEAR(centerMass, 35.0 / 3.0, 1e-12);
    EXPECT_NE(centerMass, 10.0); // boundary degree variation breaks conservation.

    // A corner impulse in 3x3 has degree two while its adjacent edge nodes
    // have degree three: resulting total is 25/3, not the initial 10.
    const std::vector<double> corner3 = run(3, 3, {10,0,0, 0,0,0, 0,0,0}, 0.5, 0.0, 0.1);
    EXPECT_NEAR(corner3[0], 5.0, 1e-12);
    EXPECT_NEAR(corner3[1], 5.0 / 3.0, 1e-12);
    EXPECT_NEAR(corner3[3], 5.0 / 3.0, 1e-12);
    EXPECT_NEAR(std::accumulate(corner3.begin(), corner3.end(), 0.0), 25.0 / 3.0, 1e-12);

    // Zero diffusion and zero degradation is the identity. Full degradation
    // zeroes the field. dt is supplied to the runtime but this operator does
    // not use it: the same initial state returns bit-identical values.
    const std::vector<double> identity = run(3, 3, {1,2,3,4,5,6,7,8,9}, 0.0, 0.0, 0.01);
    EXPECT_EQ(identity, (std::vector<double>{1,2,3,4,5,6,7,8,9}));
    const std::vector<double> fullDecay = run(3, 3, {1,2,3,4,5,6,7,8,9}, 0.0, 1.0, 0.01);
    for (double value : fullDecay) EXPECT_DOUBLE_EQ(value, 0.0);
    EXPECT_EQ(run(3, 3, {0,0,0,0,10,0,0,0,0}, 0.5, 0.0, 0.01),
              run(3, 3, {0,0,0,0,10,0,0,0,0}, 0.5, 0.0, 2.0));
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
    colony->setInitialPopulation(3);
    colony->setGridWidth(2);
    colony->setGridHeight(2);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 3u);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).parentId, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).generation, 0u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 0u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).birthTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 0.0);
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
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
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
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).birthTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastUpdateTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastDivisionTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumAge(0), 0.0);
    EXPECT_EQ(colony->getBacteriumState(3).id, 4u);
    EXPECT_EQ(colony->getBacteriumState(3).parentId, 0u);
    EXPECT_EQ(colony->getBacteriumState(3).generation, 0u);
    EXPECT_EQ(colony->getBacteriumState(3).divisionCount, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).birthTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).lastUpdateTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(3).lastDivisionTime, 0.0);
    EXPECT_EQ(colony->getBacteriumState(8).id, 9u);
    EXPECT_EQ(colony->getBacteriumState(8).parentId, 4u);
    EXPECT_EQ(colony->getBacteriumState(8).generation, 1u);
    EXPECT_EQ(colony->getBacteriumState(8).divisionCount, 0u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(8).birthTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(8).lastDivisionTime, 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumAge(8), 0.0);
    EXPECT_LT(colony->getBacteriumState(8).gridX, colony->getGridWidth());
    EXPECT_LT(colony->getBacteriumState(8).gridY, colony->getGridHeight());
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
    colony->setNumSteps(1);
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
    // NumSteps=1 means the entity executes one delayed colony step and is then
    // forwarded on the following self-dispatch at the same model time.
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
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
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

TEST(RuntimePluginManagerClassTest, GroProgramCompilerParsesCommaSeparatedNeedsClauseAsOneStatement) {
    // "needs a, b;" must be consumed as a single statement, matching the
    // comma-separated member list from the original Gro grammar. It must not
    // be split into unrelated fragments by the generic ';'/',' statement
    // terminator used for comma-separated rule-body actions.
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program report() { needs q, t; selected : { grow(1); } }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    ASSERT_EQ(ir.commands.size(), 2u);
    EXPECT_EQ(ir.commands[0].sourceText, "needs q, t");
    EXPECT_TRUE(ir.commands[1].isIfStatement());
    EXPECT_EQ(ir.commands[1].expressionText, "selected");
}

TEST(RuntimePluginManagerClassTest, GroProgramParserAndCompilerCaptureNamedProgramsAndGroRules) {
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "include gro; "
        "ahl := signal(1, 1); "
        "program leader() := { "
        "  p := [ t := 2.4 ]; "
        "  true : { p.t := p.t + dt } "
        "}; "
        "program follower() := { "
        "  p := [ mode := 0, t := 0 ]; "
        "  p.mode = 0 & get_signal(ahl) > 0.01 : { p.mode := 1, p.t := 0 } "
        "}; "
        "ecoli([x:=0, y:=0], program leader());");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;
    EXPECT_TRUE(parsed.ast.hasNamedPrograms());
    ASSERT_EQ(parsed.ast.namedPrograms.size(), 2u);
    EXPECT_EQ(parsed.ast.namedPrograms[0].name, "leader");
    EXPECT_TRUE(parsed.ast.namedPrograms[0].parameters.empty());
    EXPECT_EQ(parsed.ast.namedPrograms[1].name, "follower");
    EXPECT_TRUE(parsed.ast.namedPrograms[1].parameters.empty());

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    EXPECT_TRUE(ir.hasNamedProgram("leader"));
    EXPECT_TRUE(ir.hasNamedProgram("follower"));
    ASSERT_FALSE(ir.commands.empty());
    bool foundSignalAssignment = false;
    for (const GroProgramIr::Command& command : ir.commands) {
        if (command.isAssignment() && command.assignmentTarget == "ahl") {
            foundSignalAssignment = true;
            EXPECT_EQ(command.expressionText, "signal(1, 1)");
        }
    }
    EXPECT_TRUE(foundSignalAssignment);

    const GroProgramIr::NamedProgramDefinition& leaderDefinition = ir.namedPrograms.at("leader");
    const std::vector<GroProgramIr::Command>& leaderCommands = leaderDefinition.commands;
    ASSERT_EQ(leaderCommands.size(), 2u);
    EXPECT_TRUE(leaderCommands[0].isAssignment());
    EXPECT_EQ(leaderCommands[0].assignmentTarget, "p.t");
    EXPECT_TRUE(leaderCommands[0].assignmentOnlyIfUnset);
    EXPECT_TRUE(leaderCommands[1].isIfStatement());
    EXPECT_EQ(leaderCommands[1].expressionText, "true");

    const GroProgramIr::NamedProgramDefinition& followerDefinition = ir.namedPrograms.at("follower");
    const std::vector<GroProgramIr::Command>& followerCommands = followerDefinition.commands;
    ASSERT_EQ(followerCommands.size(), 3u);
    EXPECT_TRUE(followerCommands[0].isAssignment());
    EXPECT_EQ(followerCommands[0].assignmentTarget, "p.mode");
    EXPECT_TRUE(followerCommands[0].assignmentOnlyIfUnset);
    EXPECT_TRUE(followerCommands[1].isAssignment());
    EXPECT_EQ(followerCommands[1].assignmentTarget, "p.t");
    EXPECT_TRUE(followerCommands[1].assignmentOnlyIfUnset);
    EXPECT_TRUE(followerCommands[2].isIfStatement());
    EXPECT_EQ(followerCommands[2].expressionText, "p.mode = 0 & get_signal(ahl) > 0.01");
}

TEST(RuntimePluginManagerClassTest, GroProgramCompilerExpandsComposedNamedProgramsWithoutBraces) {
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program helper(delta) := grow(delta); "
        "program reporter() := observe(); "
        "program all() := helper(2) + helper(3) sharing local_counter + reporter() sharing local_counter;");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;
    ASSERT_EQ(parsed.ast.namedPrograms.size(), 3u);
    EXPECT_EQ(parsed.ast.namedPrograms[2].name, "all");
    EXPECT_EQ(parsed.ast.namedPrograms[2].bodySource,
              "helper(2) + helper(3) sharing local_counter + reporter() sharing local_counter");

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);
    ASSERT_TRUE(ir.hasNamedProgram("all"));

    const GroProgramIr::NamedProgramDefinition& allDefinition = ir.namedPrograms.at("all");
    ASSERT_EQ(allDefinition.commands.size(), 5u);
    EXPECT_TRUE(allDefinition.commands[0].isAssignment());
    EXPECT_EQ(allDefinition.commands[0].assignmentTarget, "delta");
    EXPECT_EQ(allDefinition.commands[0].expressionText, "2");
    EXPECT_TRUE(allDefinition.commands[1].isFunctionCall());
    EXPECT_EQ(allDefinition.commands[1].functionName, "grow");
    EXPECT_TRUE(allDefinition.commands[2].isAssignment());
    EXPECT_EQ(allDefinition.commands[2].expressionText, "3");
    EXPECT_TRUE(allDefinition.commands[3].isFunctionCall());
    EXPECT_EQ(allDefinition.commands[3].functionName, "grow");
    EXPECT_TRUE(allDefinition.commands[4].isFunctionCall());
    EXPECT_EQ(allDefinition.commands[4].functionName, "observe");
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeTimeExpressionReturnsColonyTimeInsteadOfFailing) {
    // "time()" is not a required builtin for the selected GenESyS Gro
    // subset, but an expression using an unsupported function must not
    // abort the whole statement/program: it must either be a recognized,
    // defined expression function or fail only the specific construct, not
    // the entire execution. Defensively support it directly as the colony
    // time, matching the original Gro "time()" intent.
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse("program bacterium() { t := time(); }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    GroProgramRuntimeState state;
    state.colonyTime = 3.5;

    GroProgramRuntime runtime;
    GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(state.variables.at("t"), 3.5);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeAssignsIndependentOrdinalSignalChannelHandles) {
    // Corpus B (signals) acceptance criterion: two "signal(...)" channel
    // declarations must yield two distinct, stable handles, and reading
    // one channel must never reflect what was emitted into the other.
    // Handle 1 (the first declaration) intentionally maps to the colony's
    // existing legacy single field ("local_signal", channel 0 downstream)
    // so every pre-multi-channel program/fixture keeps its exact behavior;
    // handle 2+ addresses genuinely new, independently stored channels.
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program bacterium() { "
        "s0 := signal(1, 0.1); "
        "s1 := signal(1, 0.1); "
        "emit_signal(s1, 10); "
        "}");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    GroProgramRuntimeState state;
    state.contextVariables["local_signal"] = 2.0;
    state.contextVariables["local_signal_2"] = 5.0;

    GroProgramRuntime runtime;
    GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(state.variables.at("s0"), 1.0);
    EXPECT_DOUBLE_EQ(state.variables.at("s1"), 2.0);

    ASSERT_EQ(result.colonyMutations.size(), 2u);
    EXPECT_EQ(result.colonyMutations[0].type, GroProgramRuntime::ColonyMutation::Type::EnsureSignalChannel);
    ASSERT_EQ(result.colonyMutations[0].numericArguments.size(), 3u);
    EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[0], 1.0);
    EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[1], 1.0);
    EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[2], 0.1);
    EXPECT_EQ(result.colonyMutations[1].type, GroProgramRuntime::ColonyMutation::Type::EnsureSignalChannel);
    EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[0], 2.0);

    ASSERT_EQ(result.signalMutations.size(), 1u);
    EXPECT_EQ(result.signalMutations[0].channel, 2u);
    EXPECT_DOUBLE_EQ(result.signalMutations[0].value, 10.0);

    double s0Value = 0.0, s1Value = 0.0;
    std::string errorMessage;
    ASSERT_TRUE(GroProgramRuntime::evaluateExpression("get_signal(s0)", state, s0Value, errorMessage)) << errorMessage;
    ASSERT_TRUE(GroProgramRuntime::evaluateExpression("get_signal(s1)", state, s1Value, errorMessage)) << errorMessage;
    EXPECT_DOUBLE_EQ(s0Value, 2.0);
    EXPECT_DOUBLE_EQ(s1Value, 5.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeRejectsInvalidSignalCoefficientsBeforeChannelMutation) {
    struct InvalidDeclaration {
        std::string expression;
        std::map<std::string, double> context;
    };
    const std::vector<InvalidDeclaration> invalidDeclarations = {
        {"signal(-0.01, 0.5)", {}},
        {"signal(1.01, 0.5)", {}},
        {"signal(0.5, -0.01)", {}},
        {"signal(0.5, 1.01)", {}},
        {"signal(nonfinite, 0.5)", {{"nonfinite", std::numeric_limits<double>::quiet_NaN()}}},
        {"signal(0.5, nonfinite)", {{"nonfinite", std::numeric_limits<double>::infinity()}}}
    };

    GroProgramParser parser;
    GroProgramCompiler compiler;
    GroProgramRuntime runtime;
    for (std::size_t index = 0; index < invalidDeclarations.size(); ++index) {
        const GroProgramParser::Result parsed = parser.parse(
            "program bacterium() { channel := " + invalidDeclarations[index].expression + "; }");
        ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;
        const GroProgramIr ir = compiler.compile(parsed.ast);
        GroProgramRuntimeState state;
        state.contextVariables = invalidDeclarations[index].context;

        const GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);
        EXPECT_FALSE(result.succeeded) << invalidDeclarations[index].expression;
        EXPECT_NE(result.errorMessage.find("signal coefficients must be finite values in the [0,1] interval"),
                  std::string::npos) << result.errorMessage;
        EXPECT_TRUE(result.colonyMutations.empty()) << invalidDeclarations[index].expression;
        EXPECT_EQ(state.signalDeclarationOrdinal, 0u);
        EXPECT_EQ(state.variables.count("channel"), 0u);
    }

    const GroProgramParser::Result validParsed = parser.parse(
        "program bacterium() { low := signal(0, 1); high := signal(1, 0); middle := signal(0.25, 0.75); }");
    ASSERT_TRUE(validParsed.accepted) << validParsed.errorMessage;
    GroProgramRuntimeState validState;
    const GroProgramRuntime::ExecutionResult validResult = runtime.execute(compiler.compile(validParsed.ast), validState);
    ASSERT_TRUE(validResult.succeeded) << validResult.errorMessage;
    EXPECT_EQ(validResult.colonyMutations.size(), 3u);
    EXPECT_EQ(validState.signalDeclarationOrdinal, 3u);
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

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsRunAndTumbleCommands) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse("program bacterium() { run(0.2); tumble(0.4); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	state.simulationStep = 0.25;
	state.variables["speed"] = 0.1;
	state.variables["direction"] = 1.0;
	state.variables["theta"] = 1.0;
	state.contextVariables["bacterium_speed"] = 0.1;
	state.contextVariables["bacterium_direction"] = 1.0;

	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.motionMutations.size(), 2u);
	EXPECT_EQ(result.motionMutations[0].type, GroProgramRuntime::MotionMutationType::Run);
	EXPECT_EQ(result.motionMutations[1].type, GroProgramRuntime::MotionMutationType::Tumble);
	EXPECT_GT(result.motionMutations[0].resultingSpeed, result.motionMutations[0].previousSpeed);
	EXPECT_NE(result.motionMutations[1].resultingDirection, result.motionMutations[1].previousDirection);
	EXPECT_NE(state.variables.at("speed"), 0.1);
	EXPECT_NE(state.variables.at("direction"), 1.0);
	EXPECT_DOUBLE_EQ(result.assignedVariables.at("speed"), state.variables.at("speed"));
	EXPECT_DOUBLE_EQ(result.assignedVariables.at("direction"), state.variables.at("direction"));
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsChemostatAndBarrierCommands) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse("program colony() { chemostat(true); barrier(1, 2, 3, 4); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.colonyMutations.size(), 2u);
	EXPECT_EQ(result.colonyMutations[0].type, GroProgramRuntime::ColonyMutation::Type::SetChemostatMode);
	EXPECT_EQ(result.colonyMutations[1].type, GroProgramRuntime::ColonyMutation::Type::AddBarrier);
	ASSERT_EQ(result.colonyMutations[0].numericArguments.size(), 1u);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[0], 1.0);
	ASSERT_EQ(result.colonyMutations[1].numericArguments.size(), 4u);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[0], 1.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[1], 2.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[2], 3.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[3], 4.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsMapToCellsCommand) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse("program colony() { map_to_cells(bacterium_index); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.colonyMutations.size(), 1u);
	EXPECT_EQ(result.colonyMutations[0].type, GroProgramRuntime::ColonyMutation::Type::MapToCells);
	EXPECT_EQ(result.colonyMutations[0].expressionText, "bacterium_index");
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsSetSignalCommands) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse(
	    "program colony() { set_signal(0, 0, 0, 7.5); set_signal_rect(0, 0, 0, 1, 1, 2.5); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.colonyMutations.size(), 2u);
	EXPECT_EQ(result.colonyMutations[0].type, GroProgramRuntime::ColonyMutation::Type::SetSignalAt);
	EXPECT_EQ(result.colonyMutations[1].type, GroProgramRuntime::ColonyMutation::Type::SetSignalRect);
	ASSERT_EQ(result.colonyMutations[0].numericArguments.size(), 4u);
	ASSERT_EQ(result.colonyMutations[1].numericArguments.size(), 6u);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[0], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[1], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[2], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[3], 7.5);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[0], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[1], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[2], 0.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[3], 1.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[4], 1.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[5], 2.5);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsSignalGridConfigurationCommands) {
	GroProgramParser parser;
	GroProgramParser::Result parsed = parser.parse(
	    "program colony() { set(\"signal_grid_width\", 7); set(\"signal_grid_height\", 5); set(\"signal_element_size\", 3); }");
	ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

	GroProgramCompiler compiler;
	GroProgramIr ir = compiler.compile(parsed.ast);

	GroProgramRuntimeState state;
	GroProgramRuntime runtime;
	GroProgramRuntime::ExecutionResult result = runtime.execute(ir, state);

	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.colonyMutations.size(), 2u);
	EXPECT_EQ(result.colonyMutations[0].type, GroProgramRuntime::ColonyMutation::Type::SetSignalGridWidth);
	EXPECT_EQ(result.colonyMutations[1].type, GroProgramRuntime::ColonyMutation::Type::SetSignalGridHeight);
	ASSERT_EQ(result.colonyMutations[0].numericArguments.size(), 1u);
	ASSERT_EQ(result.colonyMutations[1].numericArguments.size(), 1u);
	EXPECT_DOUBLE_EQ(result.colonyMutations[0].numericArguments[0], 7.0);
	EXPECT_DOUBLE_EQ(result.colonyMutations[1].numericArguments[0], 5.0);
	EXPECT_DOUBLE_EQ(result.assignedVariables.at("signal_grid_width"), 7.0);
	EXPECT_DOUBLE_EQ(result.assignedVariables.at("signal_grid_height"), 5.0);
	EXPECT_DOUBLE_EQ(state.variables.at("signal_element_size"), 3.0);
	EXPECT_DOUBLE_EQ(result.assignedVariables.at("signal_element_size"), 3.0);
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

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsGroRuleSyntaxAndPersistentRecordFields) {
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse(
        "program bacterium() { "
        "  p := [ mode := 0, t := 0 ]; "
        "  p.mode = 0 & get_signal(ahl) > 0.01 : { emit_signal(ahl, 2), p.mode := 1, p.t := 0 } "
        "  p.mode = 1 : { p.t := p.t + dt } "
        "}");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;

    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    GroProgramRuntimeState state;
    state.simulationStep = 0.5;
    state.variables["ahl"] = 1.0;
    state.contextVariables["local_signal"] = 1.0;

    GroProgramRuntime runtime;
    GroProgramRuntime::ExecutionResult firstResult = runtime.execute(ir, state);

    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    ASSERT_EQ(firstResult.signalMutations.size(), 1u);
    EXPECT_DOUBLE_EQ(firstResult.signalMutations[0].value, 2.0);
    EXPECT_DOUBLE_EQ(state.variables.at("p.mode"), 1.0);
    EXPECT_DOUBLE_EQ(state.variables.at("p.t"), 0.5);

    state.contextVariables["local_signal"] = 0.0;
    GroProgramRuntime::ExecutionResult secondResult = runtime.execute(ir, state);

    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_TRUE(secondResult.signalMutations.empty());
    EXPECT_DOUBLE_EQ(state.variables.at("p.mode"), 1.0);
    EXPECT_DOUBLE_EQ(state.variables.at("p.t"), 1.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsProgramParametersRateSkipAndFluorescenceVariables) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_FluorescentDefault");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "include gro; "
        "set(\"dt\", 0.1); "
        "program oscillator(g0) := { "
        "  p := [ mode := 0, t := 0, x := g0 ]; "
        "  true : { skip() } "
        "  p.mode = 0 & rate(1000000) : { gfp := 0.5 * volume * g0, p.mode := 1, p.t := 0 } "
        "}; "
        "ecoli([x:=0, y:=0], program oscillator(3));");

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_FluorescentDefault");
    ASSERT_NE(signalGrid, nullptr);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_FluorescentDefault");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 1u);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "g0"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "g0"), 3.0);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "gfp"), 0.0);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "p.mode"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "p.t"));
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

TEST(RuntimePluginManagerClassTest, BacteriaColonyReportsUnsupportedGroConstructsThroughExecutionResult) {
    // Unsupported builtins (e.g. the original Gro "reaction()", out of the
    // selected GenESyS subset by maintainer decision) and malformed/raw
    // statements must never disappear silently: BacteriaColony must keep
    // reporting them through the already-existing ExecutionResult
    // diagnostics lists, per bacterium, all the way up from
    // GroProgramRuntime through bacterium-scoped execution.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_UnsupportedConstructs");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "reaction({}, {}, 1); "
        "oops; "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_UnsupportedConstructs");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    ASSERT_EQ(result.unsupportedCommands.size(), 1u);
    EXPECT_NE(result.unsupportedCommands[0].find("reaction("), std::string::npos);
    ASSERT_EQ(result.skippedRawStatements.size(), 1u);
    EXPECT_NE(result.skippedRawStatements[0].find("oops"), std::string::npos);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyMaintainsTwoIndependentSignalChannelsWithoutAliasing) {
    // Acceptance corpus B (signals): creation of two independent channels,
    // emission into one, and diffusion, must never leak into the other
    // channel's field. GenESyS-authored fixture, not copied from any
    // original Gro example.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_TwoChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        // kdiff=kdeg=0 on both channels: this fixture checks channel
        // independence/aliasing, not the (separately tested) diffusion
        // formula, so the emitted value must stay exactly as emitted.
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0, 0); "
        "emit_signal(s1, 100); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_TwoChannels");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 1u);
    const BacteriaColony::BacteriumState& bacterium = colony->getBacteriumState(0);
    const unsigned int gridX = bacterium.gridX;
    const unsigned int gridY = bacterium.gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;

    // Channel 1 (s0, the legacy/default field) must stay untouched: nothing
    // emits into it.
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 0.0);
    // Channel 2 (s1) must reflect exactly what was emitted into it, at the
    // bacterium's own cell: no aliasing with channel 1/s0.
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, gridX, gridY), 100.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "s0"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "s1"), 2.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeRejectsInvalidSignalChannelHandles) {
    // Phase 3 handle contract: a handle must be a non-negative integer
    // actually returned by a prior "signal(...)" declaration. Negative,
    // non-integer, and not-yet-declared handles must be diagnosed, never
    // silently aliased to the legacy channel (channel 0).
    GroProgramCompiler compiler;

    auto compileAndRun = [&](const std::string& source, GroProgramRuntimeState& state) {
        GroProgramParser parser;
        GroProgramParser::Result parsed = parser.parse(source);
        EXPECT_TRUE(parsed.accepted) << parsed.errorMessage;
        GroProgramIr ir = compiler.compile(parsed.ast);
        GroProgramRuntime runtime;
        return runtime.execute(ir, state);
    };

    {
        GroProgramRuntimeState state;
        GroProgramRuntime::ExecutionResult result = compileAndRun(
            "program bacterium() { s0 := signal(1, 0.1); x := get_signal(-1); }", state);
        EXPECT_FALSE(result.succeeded);
        EXPECT_NE(result.errorMessage.find("invalid signal channel handle"), std::string::npos) << result.errorMessage;
    }
    {
        GroProgramRuntimeState state;
        GroProgramRuntime::ExecutionResult result = compileAndRun(
            "program bacterium() { s0 := signal(1, 0.1); emit_signal(-3, 5); }", state);
        EXPECT_FALSE(result.succeeded);
        EXPECT_NE(result.errorMessage.find("invalid signal channel handle"), std::string::npos) << result.errorMessage;
    }
    {
        // s0 resolves to handle 1; 1.5 is not an integer handle.
        GroProgramRuntimeState state;
        GroProgramRuntime::ExecutionResult result = compileAndRun(
            "program bacterium() { s0 := signal(1, 0.1); absorb_signal(1.5, 5); }", state);
        EXPECT_FALSE(result.succeeded);
        EXPECT_NE(result.errorMessage.find("invalid signal channel handle"), std::string::npos) << result.errorMessage;
    }
    {
        // Only one channel has been declared (ordinal == 1); handle 5 was
        // never returned by any "signal(...)" call in this program.
        GroProgramRuntimeState state;
        GroProgramRuntime::ExecutionResult result = compileAndRun(
            "program bacterium() { s0 := signal(1, 0.1); emit_signal(5, 5); }", state);
        EXPECT_FALSE(result.succeeded);
        EXPECT_NE(result.errorMessage.find("invalid signal channel handle"), std::string::npos) << result.errorMessage;
    }
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeRejectsConditionalSignalDeclarations) {
    const std::vector<std::string> sources = {
        "program bacterium() { if (true) { s0 := signal(0.2, 0.1); } }",
        "program bacterium() { if (false) { s0 := signal(0.2, 0.1); } }",
        "program bacterium() { if (true) { x := 0; } else { s0 := signal(0.2, 0.1); } }",
        "program bacterium() { if (false) { if (true) { s0 := signal(0.2, 0.1); } } }"};
    for (const std::string& source : sources) {
        GroProgramParser parser;
        GroProgramParser::Result parsed = parser.parse(source);
        ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;
        GroProgramCompiler compiler;
        GroProgramRuntime runtime;
        GroProgramRuntimeState state;
        GroProgramRuntime::ExecutionResult result = runtime.execute(compiler.compile(parsed.ast), state);
        EXPECT_FALSE(result.succeeded) << "source=" << source;
        EXPECT_NE(result.errorMessage.find("not supported inside conditional branches"), std::string::npos)
            << "source=" << source << ": " << result.errorMessage;
    }
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeKeepsSignalHandleStableAcrossRepeatedExecuteCallsOnReusedState) {
    // Problem C (Phase 3 review): GroProgramRuntimeState::signalDeclarationOrdinal
    // is only correct if every caller happens to construct a fresh state per
    // execution pass. GroProgramRuntime::execute() itself did not enforce
    // this, so a caller reusing the same state object across repeated
    // executions of the same IR would see the handle for the same
    // declaration drift upward on every call (1, then 2, then 3, ...)
    // instead of staying stable.
    GroProgramParser parser;
    GroProgramParser::Result parsed = parser.parse("program bacterium() { s0 := signal(1, 0.1); }");
    ASSERT_TRUE(parsed.accepted) << parsed.errorMessage;
    GroProgramCompiler compiler;
    GroProgramIr ir = compiler.compile(parsed.ast);

    GroProgramRuntimeState state;
    GroProgramRuntime runtime;

    GroProgramRuntime::ExecutionResult firstRun = runtime.execute(ir, state);
    EXPECT_TRUE(firstRun.succeeded) << firstRun.errorMessage;
    EXPECT_DOUBLE_EQ(state.variables.at("s0"), 1.0);

    // Re-executing the exact same IR against the SAME (not freshly
    // constructed) state must assign "s0" the same handle again, not 2.0.
    GroProgramRuntime::ExecutionResult secondRun = runtime.execute(ir, state);
    EXPECT_TRUE(secondRun.succeeded) << secondRun.errorMessage;
    EXPECT_DOUBLE_EQ(state.variables.at("s0"), 1.0);

    GroProgramRuntime::ExecutionResult thirdRun = runtime.execute(ir, state);
    EXPECT_TRUE(thirdRun.succeeded) << thirdRun.errorMessage;
    EXPECT_DOUBLE_EQ(state.variables.at("s0"), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPropagatesGlobalSignalHandlesToMultipleNamedPrograms) {
    // Problem C (Phase 3 review): exercises the full "global declarations
    // -> prelude -> named bacterium program -> get_signal(handle)" path
    // with TWO global channels consumed by TWO different named programs,
    // confirming each program resolves the same stable handle for its own
    // channel and that the two channels remain independent.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_GlobalHandlesTwoPrograms");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "ahl := signal(0, 0); "
        "beta := signal(0, 0); "
        "program leader() := { true : { emit_signal(ahl, 5) } }; "
        "program follower() := { true : { emit_signal(beta, 7) } }; "
        "ecoli([x:=0, y:=0], program leader()); "
        "ecoli([x:=1, y:=0], program follower());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_GlobalHandlesTwoPrograms");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setGridWidth(4);
    colony->setGridHeight(4);

    ModelDataDefinition::InitBetweenReplications(colony);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    const unsigned int leaderX = colony->getBacteriumState(0).gridX;
    const unsigned int leaderY = colony->getBacteriumState(0).gridY;
    const unsigned int followerX = colony->getBacteriumState(1).gridX;
    const unsigned int followerY = colony->getBacteriumState(1).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;

    // leader() emitted into "ahl" (handle 1, legacy field) only; follower()
    // emitted into "beta" (handle 2, additional channel) only. Each must
    // land exactly where it was emitted and nowhere else.
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(leaderX, leaderY), 5.0);
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, leaderX, leaderY), 0.0);
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, followerX, followerY), 7.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(followerX, followerY), 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyKeepsGlobalSignalHandleStableAcrossReplications) {
    // Problem C (Phase 3 review): a global "signal(...)" declaration must
    // resolve to the same handle in every replication, not drift because
    // of leftover ordinal/variable state from a previous replication.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_GlobalHandleReplicationStable");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "ahl := signal(0, 0); "
        "program leader() := { true : { emit_signal(ahl, 5) } }; "
        "ecoli([x:=0, y:=0], program leader());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_GlobalHandleReplicationStable");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setGridWidth(2);
    colony->setGridHeight(2);

    ModelDataDefinition::InitBetweenReplications(colony);
    GroProgramRuntime::ExecutionResult firstReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(firstReplicationResult.succeeded) << firstReplicationResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "ahl"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 1), 5.0);

    ModelDataDefinition::InitBetweenReplications(colony);
    GroProgramRuntime::ExecutionResult secondReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(secondReplicationResult.succeeded) << secondReplicationResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "ahl"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(1, 1), 5.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonySetSignalAndSetSignalRectRespectChannelHandle) {
    // Acceptance corpus B: set_signal/set_signal_rect must address the
    // channel named by their handle argument, independent of the legacy
    // field, and must reject a handle that was never declared.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SetSignalChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program main() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0, 0); "
        "set_signal(s1, 0, 0, 7.5); "
        "set_signal_rect(s1, 0, 0, 1, 1, 2.5); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SetSignalChannels");
    ASSERT_NE(colony, nullptr);
    colony->setInitialPopulation(0);
    colony->setGridWidth(32);
    colony->setGridHeight(32);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;

    // Legacy/channel-1 field must stay exactly zero: nothing ever targets it.
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(16, 16), 0.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(17, 17), 0.0);
    // Channel 2 (s1) must reflect both the point and rect writes.
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, 16, 16), 2.5);
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, 17, 17), 2.5);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyRejectsSetSignalWithUndeclaredChannelHandle) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SetSignalInvalidHandle");
    ASSERT_NE(program, nullptr);
    // No "signal(...)" declaration ever runs, so handle 5 was never issued.
    program->setSourceCode("program main() { set_signal(5, 0, 0, 7.5); }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SetSignalInvalidHandle");
    ASSERT_NE(colony, nullptr);
    colony->setInitialPopulation(0);
    colony->setGridWidth(8);
    colony->setGridHeight(8);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_FALSE(result.succeeded);
    EXPECT_NE(result.errorMessage.find("invalid signal channel handle"), std::string::npos) << result.errorMessage;
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAbsorbsSignalIndependentlyPerChannel) {
    // Acceptance corpus B: absorb_signal must decrement only the channel
    // named by its handle; the legacy channel must stay untouched.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_AbsorbChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0, 0); "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { "
        "  emit_signal(s0, 10); "
        "  emit_signal(s1, 10); "
        "} "
        "if (bacterium_id == 1 && steps == 2) { "
        "  absorb_signal(s1, 4); "
        "} "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_AbsorbChannels");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);
    const BacteriaColony::BacteriumState& bacterium = colony->getBacteriumState(0);
    const unsigned int gridX = bacterium.gridX;
    const unsigned int gridY = bacterium.gridY;

    GroProgramRuntime::ExecutionResult firstStep = colony->executeGroProgram();
    EXPECT_TRUE(firstStep.succeeded) << firstStep.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 10.0);
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, gridX, gridY), 10.0);

    GroProgramRuntime::ExecutionResult secondStep = colony->executeGroProgram();
    EXPECT_TRUE(secondStep.succeeded) << secondStep.errorMessage;
    // Only channel 2 (s1) was absorbed from; channel 1 (s0, legacy) is
    // untouched by the absorb_signal(s1, ...) call.
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 10.0);
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, gridX, gridY), 6.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyDoesNotLeakAdditionalSignalChannelsBetweenReplications) {
    // Acceptance corpus B / contract item: additional channels are pure
    // runtime state and must never leak a value from one replication into
    // the next, even when the grid dimensions are unchanged so the
    // underlying field vector would otherwise be reused as-is.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ReplicationLeak");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0, 0); "
        "if (bacterium_id == 1 && tick_count == 0) { emit_signal(s1, 100); } "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ReplicationLeak");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int firstGridX = colony->getBacteriumState(0).gridX;
    const unsigned int firstGridY = colony->getBacteriumState(0).gridY;
    GroProgramRuntime::ExecutionResult firstReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(firstReplicationResult.succeeded) << firstReplicationResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, firstGridX, firstGridY), 100.0);

    // Start a second replication: channel 2 must come back with no trace of
    // the value emitted during the first replication, even before the Gro
    // program runs again.
    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int secondGridX = colony->getBacteriumState(0).gridX;
    const unsigned int secondGridY = colony->getBacteriumState(0).gridY;
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, secondGridX, secondGridY), 0.0);

    GroProgramRuntime::ExecutionResult secondReplicationFirstStep = colony->executeGroProgram();
    EXPECT_TRUE(secondReplicationFirstStep.succeeded) << secondReplicationFirstStep.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, secondGridX, secondGridY), 100.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyResizesAdditionalSignalChannelsWithGridDimensionChanges) {
    // Dimension-coherence contract: an additional channel declared before a
    // grid resize must still be addressable (correctly sized, not stale)
    // afterwards.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ChannelResize");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program main() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0, 0); "
        "set(\"signal_grid_width\", 5); "
        "set(\"signal_grid_height\", 5); "
        "set_signal(s1, 2, 2, 9.0); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ChannelResize");
    ASSERT_NE(colony, nullptr);
    colony->setInitialPopulation(0);
    colony->setGridWidth(2);
    colony->setGridHeight(2);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(colony->getGridWidth(), 5u);
    EXPECT_EQ(colony->getGridHeight(), 5u);
    // The point written after the resize, at a coordinate that did not
    // exist in the original 2x2 grid, must land correctly in channel 2's
    // (now correctly resized) field, not be silently dropped or
    // misindexed against the old, smaller size.
    EXPECT_DOUBLE_EQ(colony->getAdditionalSignalValueAt(2, 4, 4), 9.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyMaintainsSignalChannelIndependenceAcrossMultipleSteps) {
    // Acceptance corpus B: independence must hold not just immediately
    // after one mutation, but across several colony steps that each apply
    // the per-channel diffusion/decay step.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_MultiStepChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0.1, 0.05); "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { emit_signal(s1, 100); } "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_MultiStepChannels");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(5);
    colony->setGridHeight(5);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    for (int step = 0; step < 4; ++step) {
        GroProgramRuntime::ExecutionResult stepResult = colony->executeGroProgram();
        EXPECT_TRUE(stepResult.succeeded) << stepResult.errorMessage;
        // Channel 1 (s0, legacy) never receives any mutation and has zero
        // diffusion/decay: it must stay exactly zero on every single step,
        // regardless of how channel 2 (s1) evolves under its own
        // diffusion/decay.
        EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 0.0);
        if (step == 0) {
            // The emission is at the center of a 5x5 grid. Its four
            // neighbors start at zero, so channel 2 computes
            // (100 + 0.1*(0-100))*(1-0.05) = 85.5 exactly; the legacy
            // channel remains zero under its distinct (0,0) coefficients.
            EXPECT_NEAR(colony->getAdditionalSignalValueAt(2, gridX, gridY), 85.5, 1e-12);
        }
    }
    // Channel 2 must have decayed from its initial emission (kdeg=0.05 > 0)
    // but still be nonzero after only 4 steps, and must never have leaked
    // into channel 1 above.
    const double channel2Value = colony->getAdditionalSignalValueAt(2, gridX, gridY);
    EXPECT_GT(channel2Value, 0.0);
    EXPECT_LT(channel2Value, 100.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyStepsAdditionalSignalChannelsWithSignalGridAttached) {
    // Problem A (Phase 3 review): _applySignalFieldStep() only called
    // _applyAdditionalSignalChannelsStep() in the branch taken when no
    // BacteriaSignalGrid is attached (_signalGrid == nullptr). Every
    // colony with a real BacteriaSignalGrid attached silently stopped
    // stepping its additional channels entirely. The legacy field's own
    // diffusion/decay is set to exactly zero here so the pre-fix code also
    // hits its second early return (before ever reaching the additional
    // channel step), reproducing both symptoms with one fixture.
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ChannelsWithSignalGrid");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "s1 := signal(0.5, 0); "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { emit_signal(s1, 10); } "
        "}");

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_ChannelsAttached");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(3);
    signalGrid->setInitialSignal(0.0);
    signalGrid->setDiffusionRate(0.0);
    signalGrid->setDecayRate(0.0);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ChannelsWithSignalGrid");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    // The per-bacterium emission and the once-per-colony-step channel
    // diffusion both happen within this same executeGroProgram() call
    // (population loop, then the single post-loop signal-field step), so
    // channel 2's own diffusion (kdiff=0.5) must already have spread the
    // just-emitted value by the time this call returns. Before the fix,
    // _signalGrid being attached meant the additional-channel step never
    // ran at all, so the emission cell stayed frozen at exactly the raw
    // emitted value (10.0) instead of diffusing.
    GroProgramRuntime::ExecutionResult firstStep = colony->executeGroProgram();
    EXPECT_TRUE(firstStep.succeeded) << firstStep.errorMessage;
    const double emissionCellValue = colony->getAdditionalSignalValueAt(2, gridX, gridY);
    EXPECT_LT(emissionCellValue, 10.0);
    EXPECT_GT(emissionCellValue, 0.0);
}

namespace {
// Shared helper for the Problem B precedence test group below: builds a
// single-bacterium colony whose program declares the legacy (handle 1)
// channel with the given coefficients and emits a fixed value into it on
// the first step only, optionally with a BacteriaSignalGrid attached.
BacteriaColony* BuildFirstChannelPrecedenceColony(Simulator& simulator, Model* model,
                                                   const std::string& testLabel,
                                                   double declaredDiffusionRate,
                                                   double declaredDecayRate,
                                                   BacteriaSignalGrid* signalGrid) {
    PluginManager* manager = simulator.getPluginManager();
    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_" + testLabel);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(" + std::to_string(declaredDiffusionRate) + ", " + std::to_string(declaredDecayRate) + "); "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { emit_signal(s0, 10); } "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_" + testLabel);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    if (signalGrid != nullptr) {
        colony->setSignalGrid(signalGrid);
    } else {
        colony->setGridWidth(5);
        colony->setGridHeight(5);
    }
    return colony;
}
} // namespace

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesFirstChannelDeclaredCoefficientsWhenNoSignalGridAttached) {
    // Problem B (Phase 3, authorized precedence, decision 1): without an
    // attached BacteriaSignalGrid, the first "signal(kdiff,kdeg)"
    // declaration (handle 1) must control the legacy field's own
    // diffusion/decay. kdiff=0.9/kdeg=0.1 (an "intermediate" coefficient
    // pair, neither 0 nor 1) on a 5x5 grid: the emission cell has 4
    // neighbors (all 0), so relaxed = 10 + 0.9*(0-10) = 1.0, then
    // decayFactor = 1 - 0.1 = 0.9, giving exactly 0.9.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelIntermediateCoefficients", 0.9, 0.1, nullptr);
    ASSERT_NE(colony, nullptr);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_NEAR(colony->getSignalValueAt(gridX, gridY), 0.9, 1e-9);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesFirstChannelZeroDecayCoefficientWhenNoSignalGridAttached) {
    // kdeg=0: decayFactor=1, so only diffusion applies. On a 5x5 grid with
    // 4 zero neighbors: relaxed = 10 + 0.9*(0-10) = 1.0, decayFactor=1 ->
    // exactly 1.0. Confirms kdeg=0 is honored as "no decay", not silently
    // clamped or ignored.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelZeroDecay", 0.9, 0.0, nullptr);
    ASSERT_NE(colony, nullptr);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_NEAR(colony->getSignalValueAt(gridX, gridY), 1.0, 1e-9);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesFirstChannelFullDecayCoefficientWhenNoSignalGridAttached) {
    // kdeg=1: decayFactor=0, so the field is deliberately zeroed by the
    // end of the same step regardless of diffusion. This is the same
    // effect exercised at BacteriaColony level by
    // BacteriaColonyExecutesSeededNamedGroPrograms's "ahl := signal(1,1)"
    // (there on a 1x1 grid); this test isolates the same kdeg=1 contract
    // on a larger grid where diffusion is also active.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelFullDecay", 0.9, 1.0, nullptr);
    ASSERT_NE(colony, nullptr);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPrioritizesSignalGridOverFirstChannelDeclaredCoefficients) {
    // Problem B (Phase 3, authorized precedence, decision 2): when a
    // BacteriaSignalGrid IS attached, its persisted coefficients remain
    // authoritative for the legacy field, even though the program
    // declares materially different ones. Grid: diffusionRate=0.0,
    // decayRate=0.0 (a true no-op); declared: kdiff=0.9, kdeg=1.0
    // (would aggressively zero the field if honored). The grid's no-op
    // coefficients must win: the raw emitted value must survive exactly.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* signalGrid = simulator.getPluginManager()->newInstance<BacteriaSignalGrid>(
        model, "SignalGrid_FirstChannelGridPriority");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(5);
    signalGrid->setHeight(5);
    signalGrid->setDiffusionRate(0.0);
    signalGrid->setDecayRate(0.0);

    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelGridPriority", 0.9, 1.0, signalGrid);
    ASSERT_NE(colony, nullptr);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    // If the declared kdeg=1.0 had been honored instead of the grid's
    // decayRate=0.0, this would be 0.0 (as in the FullDecay test above).
    // The grid's persisted (no-op) coefficients must win instead.
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 10.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyEmitsFirstChannelCoefficientMismatchDiagnosticOnceWhenSignalGridAttached) {
    // Problem B diagnostic contract: a declared first-channel coefficient
    // that differs from an attached BacteriaSignalGrid's persisted value
    // must be surfaced (the program's intent is otherwise silently
    // discarded), but at most once per replication - not once per step
    // or per bacterium.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BacteriaSignalGrid* signalGrid = simulator.getPluginManager()->newInstance<BacteriaSignalGrid>(
        model, "SignalGrid_FirstChannelMismatchDiagnostic");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(3);
    signalGrid->setHeight(3);
    signalGrid->setDiffusionRate(0.18);
    signalGrid->setDecayRate(0.02);

    // Declared coefficients (0.9, 0.9) differ from the grid's (0.18, 0.02).
    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelMismatchDiagnostic", 0.9, 0.9, signalGrid);
    ASSERT_NE(colony, nullptr);
    ASSERT_NE(model->getTracer(), nullptr);
    model->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
    model->getTracer()->addTraceSimulationExceptionRuleModelData(colony);
    model->getTracer()->addTraceSimulationHandler(&CaptureTraceSimulationEvent);
    g_capturedTraceMessages.clear();

    ModelDataDefinition::InitBetweenReplications(colony);
    // Three steps: the declaration re-executes every bacterium/every
    // step, so a naive implementation could warn on every one of them.
    for (int step = 0; step < 3; ++step) {
        GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
        EXPECT_TRUE(result.succeeded) << result.errorMessage;
    }
    auto mismatchDiagnosticCount = [] {
        return std::count_if(g_capturedTraceMessages.begin(), g_capturedTraceMessages.end(), [](const std::string& text) {
            return text.find("first signal(") != std::string::npos;
        });
    };
    EXPECT_EQ(mismatchDiagnosticCount(), 1);
    // The grid's coefficients must still have won throughout (behavioral
    // confirmation that the mismatch diagnostic is purely informative and
    // does not change precedence).
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;
    EXPECT_GT(colony->getSignalValueAt(gridX, gridY), 0.0);

    // A second replication must be able to warn again exactly once (the
    // latch resets), not stay silenced forever from the first
    // replication - confirmed together with the reset test below.
    ModelDataDefinition::InitBetweenReplications(colony);
    GroProgramRuntime::ExecutionResult secondReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(secondReplicationResult.succeeded) << secondReplicationResult.errorMessage;
    EXPECT_EQ(mismatchDiagnosticCount(), 2);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPreservesLegacyBehaviorWhenNoSignalDeclarationExists) {
    // Decision 4: a program that never calls "signal(...)" at all must
    // keep the exact prior behavior - the legacy field never diffuses or
    // decays on its own (no configuration source exists for it).
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_NoSignalDeclaration");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { emit_signal(10); } "
        "}");

    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(model, "BacteriaColony_NoSignalDeclaration");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);
    colony->setGridWidth(5);
    colony->setGridHeight(5);

    ModelDataDefinition::InitBetweenReplications(colony);
    const unsigned int gridX = colony->getBacteriumState(0).gridX;
    const unsigned int gridY = colony->getBacteriumState(0).gridY;

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 10.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyResetsFirstChannelCoefficientsBetweenReplications) {
    // Reset contract: a declared first-channel coefficient from one
    // replication must not leak into the next, mirroring the equivalent
    // contract already enforced for additional (handle >= 2) channels.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    // kdeg=1: if the coefficient leaked forward as "still applied" with
    // no fresh declaration, a stale decay would still show up; if it
    // leaked forward as "still 0" after a replication that legitimately
    // declared kdeg=1, the contract would also be violated the other
    // way. The real test is in the second replication, after a FRESH
    // declaration with kdeg=0 runs: the field must NOT be forced to
    // zero by a stale kdeg=1 from the first replication.
    BacteriaColony* colony = BuildFirstChannelPrecedenceColony(
        simulator, model, "FirstChannelReplicationReset", 0.0, 1.0, nullptr);
    ASSERT_NE(colony, nullptr);

    ModelDataDefinition::InitBetweenReplications(colony);
    unsigned int gridX = colony->getBacteriumState(0).gridX;
    unsigned int gridY = colony->getBacteriumState(0).gridY;
    GroProgramRuntime::ExecutionResult firstReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(firstReplicationResult.succeeded) << firstReplicationResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 0.0);

    // Second replication: before the program's own declaration re-runs,
    // the stored coefficients and declaration latch must be reset. Change
    // the program to kdeg=0; if stale kdeg=1 survived, its new emission
    // would still be zeroed instead of remaining at 10.
    GroProgram* program = colony->getGroProgram();
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "s0 := signal(0, 0); "
        "steps = steps + 1; "
        "if (bacterium_id == 1 && steps == 1) { emit_signal(s0, 10); } "
        "}");
    ModelDataDefinition::InitBetweenReplications(colony);
    gridX = colony->getBacteriumState(0).gridX;
    gridY = colony->getBacteriumState(0).gridY;
    GroProgramRuntime::ExecutionResult secondReplicationResult = colony->executeGroProgram();
    EXPECT_TRUE(secondReplicationResult.succeeded) << secondReplicationResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(gridX, gridY), 10.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyResetBuiltinClearsFirstChannelCoefficientMetadata) {
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_ResetSignalMetadata");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program main() := { s0 := signal(0, 1); set_signal(s0, 0, 0, 10); };");
    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(model, "BacteriaColony_ResetSignalMetadata");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 0.0);

    // reset() clears the previous channel configuration; the new declaration
    // must be accepted and its zero-decay setting must preserve the value.
    program->setSourceCode(
        "program main() := { reset(); s0 := signal(0, 0); set_signal(s0, 0, 0, 10); };");
    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 10.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPreservesExistingBacteriaColonyGroFixtureBehavior) {
    // Preservation check for the real persisted fixture
    // models/Smart_BacteriaColony_GRO.gen: a BacteriaSignalGrid is
    // attached (diffusionRate=0.22, decayRate=0.04) and the program body
    // (originally named "colony()", seeded via ecoli(...) in that
    // fixture's "main()"; simplified here to the equivalent
    // "program bacterium()" seeding form) uses the legacy single-argument
    // "emit_signal(value)" form without ever calling "signal(kdiff,kdeg)"
    // - so neither Problem B code path is exercised (no first-channel
    // declaration exists at all) and the grid's persisted coefficients
    // drive diffusion exactly as before.
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_BacteriaColonyGroFixture");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { "
        "speed := 0.12 + 0.05 * local_signal + 0.01 * bacterium_generation; "
        "gfp := 18 + 28 * local_signal + 2 * volume; "
        "emit_signal ( 0.55 + 0.15 * local_signal ); "
        "}");

    BacteriaSignalGrid* signalGrid = simulator.getPluginManager()->newInstance<BacteriaSignalGrid>(
        model, "SignalGrid_BacteriaColonyGroFixture");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(16);
    signalGrid->setHeight(16);
    signalGrid->setDiffusionRate(0.22);
    signalGrid->setDecayRate(0.04);

    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(
        model, "BacteriaColony_BacteriaColonyGroFixture");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setSignalGrid(signalGrid);
    colony->setSimulationStep(0.1);
    colony->setInitialPopulation(4);
    colony->setGridWidth(16);
    colony->setGridHeight(16);

    ModelDataDefinition::InitBetweenReplications(colony);
    GroProgramRuntime::ExecutionResult firstStep = colony->executeGroProgram();
    EXPECT_TRUE(firstStep.succeeded) << firstStep.errorMessage;
    // Every bacterium emitted 0.55 (local_signal starts at 0); the grid's
    // own diffusion/decay (0.22/0.04) applies exactly as before Problem B.
    bool anyNonZero = false;
    for (std::size_t index = 0; index < colony->getInternalBacteriaCount(); ++index) {
        const BacteriaColony::BacteriumState& bacterium = colony->getBacteriumState(index);
        if (colony->getSignalValueAt(bacterium.gridX, bacterium.gridY) > 0.0) {
            anyNonZero = true;
        }
    }
    EXPECT_TRUE(anyNonZero);

    GroProgramRuntime::ExecutionResult secondStep = colony->executeGroProgram();
    EXPECT_TRUE(secondStep.succeeded) << secondStep.errorMessage;
}

TEST(RuntimePluginManagerClassTest, ExistingGrowthAndLifecycleBacteriaColonyGenFixturesStillLoad) {
    const std::vector<std::pair<std::string, std::string>> fixtures = {
        {"Smart_GroColonyGrowth.gen", "BacteriaColony_Growth"},
        {"Smart_GroColonyLifecycle.gen", "BacteriaColony_Lifecycle"},
    };
    std::filesystem::path repositoryRoot = std::filesystem::current_path();
    while (!repositoryRoot.empty() &&
           !std::filesystem::exists(repositoryRoot / "models" / fixtures.front().first)) {
        const std::filesystem::path parent = repositoryRoot.parent_path();
        if (parent == repositoryRoot) {
            repositoryRoot.clear();
            break;
        }
        repositoryRoot = parent;
    }
    ASSERT_FALSE(repositoryRoot.empty()) << "Could not locate the repository models directory from the test cwd";

    for (const auto& [fixtureName, colonyName] : fixtures) {
        Simulator simulator;
        ASSERT_NE(simulator.getPluginManager(), nullptr);
        simulator.getPluginManager()->autoInsertPlugins();
        const std::filesystem::path fixturePath = repositoryRoot / "models" / fixtureName;
        Model* model = simulator.getModelManager()->loadModel(fixturePath.string());
        ASSERT_NE(model, nullptr) << "Failed to load " << fixturePath;
        ModelComponent* component = model->getComponentManager()->find(colonyName);
        EXPECT_NE(dynamic_cast<BacteriaColony*>(component), nullptr)
            << "Missing persisted colony " << colonyName << " in " << fixturePath;
    }
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
    colony->setInitialPopulation(2);
    colony->setGridWidth(3);
    colony->setGridHeight(3);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_EQ(firstResult.executedCommands, 10u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_EQ(colony->getPopulationSize(), 2u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "age_steps"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "seen_age"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "age_steps"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_age"), 0.0);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).lastDivisionTime, 0.0);
    EXPECT_EQ(colony->getBacteriumState(1).parentId, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).generation, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(1).birthTime, 0.0);

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_EQ(secondResult.executedCommands, 8u);
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_EQ(colony->getPopulationSize(), 2u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "age_steps"), 2.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "seen_age"), 0.0);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "age_steps"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "seen_age"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "age_steps"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "seen_age"), 0.0);
    EXPECT_EQ(colony->getBacteriumState(1).parentId, 1u);
    EXPECT_EQ(colony->getBacteriumState(1).generation, 1u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonySupportsBacteriumScopedDivide) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_PerBacteriumDivide");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { if (bacterium_id == 1) { ecoli_growth_rate := 0; volume := 0.2; divide(); } }");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_PerBacteriumDivide");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setInitialPopulation(2);

    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(colony->getPopulationSize(), 3u);
    ASSERT_EQ(colony->getInternalBacteriaCount(), 3u);
    EXPECT_EQ(colony->getBacteriumState(0).id, 1u);
    EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
    EXPECT_EQ(colony->getBacteriumState(2).parentId, 1u);
    EXPECT_EQ(colony->getBacteriumState(2).generation, 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0) + colony->getBacteriumVolume(2), 0.2);
    EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0), 0.1);
    EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(2), 0.1);
    EXPECT_DOUBLE_EQ(colony->getBacteriumSize(0), std::sqrt(0.1));
    EXPECT_DOUBLE_EQ(colony->getBacteriumSize(2), std::sqrt(0.1));
    EXPECT_DOUBLE_EQ(colony->getBacteriumDirectionRadians(0), colony->getBacteriumDirectionRadians(2));
    ASSERT_EQ(result.populationMutations.size(), 1u);
    EXPECT_EQ(result.populationMutations[0].type, GroProgramRuntime::PopulationMutationType::Divide);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAutomaticDivisionIsOptionalAndConservesVolume) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_AutomaticDivision");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0); "
			"program grower() { volume := 2.5; } "
			"ecoli([x:=0,y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_AutomaticDivision");
	ASSERT_NE(colony, nullptr);
	EXPECT_FALSE(colony->getAutomaticDivisionEnabled());
	EXPECT_DOUBLE_EQ(colony->getDivisionThresholdVolume(), 2.0);
	colony->setGroProgram(program);

	GroProgramRuntime::ExecutionResult disabledResult = colony->executeGroProgram();
	ASSERT_TRUE(disabledResult.succeeded) << disabledResult.errorMessage;
	EXPECT_EQ(colony->getInternalBacteriaCount(), 1u);
	EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0), 2.5);

	colony->setAutomaticDivisionEnabled(true);
	colony->setDivisionThresholdVolume(2.0);
	GroProgramRuntime::ExecutionResult enabledResult = colony->executeGroProgram();
	ASSERT_TRUE(enabledResult.succeeded) << enabledResult.errorMessage;
	ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
	EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
	EXPECT_EQ(colony->getBacteriumState(1).parentId, colony->getBacteriumState(0).id);
	EXPECT_TRUE(colony->getBacteriumState(0).justDivided);
	EXPECT_TRUE(colony->getBacteriumState(1).justDivided);
	EXPECT_TRUE(colony->getBacteriumState(1).daughter);
	EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0) + colony->getBacteriumVolume(1), 2.5);
	EXPECT_DOUBLE_EQ(colony->getBacteriumDirectionRadians(0), colony->getBacteriumDirectionRadians(1));
	EXPECT_LT(std::hypot(colony->getBacteriumPositionX(1) - colony->getBacteriumPositionX(0),
	                      colony->getBacteriumPositionY(1) - colony->getBacteriumPositionY(0)), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyGroDividePreventsSecondAutomaticDivisionInSameStep) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ManualAndAutomaticDivision");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0); "
			"program grower() { volume := 3; divide(); } "
			"ecoli([x:=0,y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ManualAndAutomaticDivision");
	ASSERT_NE(colony, nullptr);
	colony->setAutomaticDivisionEnabled(true);
	colony->setDivisionThresholdVolume(2.0);
	colony->setGroProgram(program);

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	ASSERT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
	EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0) + colony->getBacteriumVolume(1), 3.0);
	EXPECT_EQ(colony->getBacteriumState(0).divisionCount, 1u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyDiagnosesRepeatedGroDivisionInOneStep) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_RepeatedDivision");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0); "
			"program grower() { volume := 3; divide(); divide(); } "
			"ecoli([x:=0,y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_RepeatedDivision");
	ASSERT_NE(colony, nullptr);
	colony->setGroProgram(program);

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	EXPECT_FALSE(result.succeeded);
	EXPECT_NE(result.errorMessage.find("at most one divide()"), std::string::npos);
	EXPECT_EQ(colony->getInternalBacteriaCount(), 1u);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyPersistsAutomaticDivisionConfiguration) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_PersistedDivision");
	ASSERT_NE(colony, nullptr);
	colony->setAutomaticDivisionEnabled(true);
	colony->setDivisionThresholdVolume(3.25);

	const auto uniqueSuffix = std::chrono::steady_clock::now().time_since_epoch().count();
	const std::filesystem::path path = std::filesystem::temp_directory_path() /
	                                   ("genesys_bacteria_division_" + std::to_string(uniqueSuffix) + ".gen");
	ASSERT_TRUE(simulator.getModelManager()->saveModel(path.string()));
	Model* loadedModel = simulator.getModelManager()->loadModel(path.string());
	ASSERT_NE(loadedModel, nullptr);
	BacteriaColony* loadedColony = dynamic_cast<BacteriaColony*>(
			loadedModel->getComponentManager()->find("BacteriaColony_PersistedDivision"));
	ASSERT_NE(loadedColony, nullptr);
	EXPECT_TRUE(loadedColony->getAutomaticDivisionEnabled());
	EXPECT_DOUBLE_EQ(loadedColony->getDivisionThresholdVolume(), 3.25);
	std::filesystem::remove(path);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAggregateDivideConservesCombinedVolume) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_AggregateDivideVolume");
	ASSERT_NE(program, nullptr);
	program->setSourceCode("program colony() { divide(); }");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_AggregateDivideVolume");
	ASSERT_NE(colony, nullptr);
	colony->setInitialPopulation(3);
	colony->setGroProgram(program);
	ModelDataDefinition::InitBetweenReplications(colony);
	double initialVolume = 0.0;
	for (std::size_t index = 0; index < colony->getInternalBacteriaCount(); ++index) {
		initialVolume += colony->getBacteriumVolume(index);
	}

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	ASSERT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(colony->getInternalBacteriaCount(), 6u);
	double finalVolume = 0.0;
	for (std::size_t index = 0; index < colony->getInternalBacteriaCount(); ++index) {
		finalVolume += colony->getBacteriumVolume(index);
	}
	EXPECT_DOUBLE_EQ(finalVolume, initialVolume);
	for (std::size_t index = 3; index < 6; ++index) {
		EXPECT_EQ(colony->getBacteriumState(index).generation, 1u);
		EXPECT_TRUE(colony->getBacteriumState(index).daughter);
		EXPECT_TRUE(colony->getBacteriumState(index).justDivided);
	}
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyDoesNotExecuteNewMainDivisionDaughterInSameStep) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_MainDivisionSnapshot");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0); "
			"program main() := { divide(); }; "
			"program grower() { visits := visits + 1; } "
			"ecoli([x:=0,y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_MainDivisionSnapshot");
	ASSERT_NE(colony, nullptr);
	colony->setAutomaticDivisionEnabled(true);
	colony->setDivisionThresholdVolume(0.4);
	colony->setGroProgram(program);

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	ASSERT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
	EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "visits"), 1.0);
	EXPECT_FALSE(colony->hasBacteriumRuntimeVariable(1, "visits"));
	EXPECT_TRUE(colony->getBacteriumState(0).justDivided);
	EXPECT_TRUE(colony->getBacteriumState(1).justDivided);
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

TEST(RuntimePluginManagerClassTest, BacteriaColonyExecutesSeededNamedGroPrograms) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_LeaderFollower");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "include gro; "
        "set(\"dt\", 0.25); "
        "ahl := signal(1, 1); "
        "program leader() := { "
        "  p := [ t := 0.3 ]; "
        "  true : { p.t := p.t + dt } "
        "  p.t > 0.4 : { emit_signal(ahl, 5), p.t := 0 } "
        "}; "
        "program follower() := { "
        "  p := [ mode := 0, t := 0 ]; "
        "  p.mode = 0 & get_signal(ahl) > 0.01 : { p.mode := 1, p.t := 0 } "
        "  p.mode = 1 : { p.t := p.t + dt } "
        "}; "
        "ecoli([x:=0, y:=0], program leader()); "
        "ecoli([x:=0, y:=0], program follower());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_LeaderFollower");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    ModelDataDefinition::InitBetweenReplications(colony);

    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_EQ(colony->getBacteriumState(0).programName, "leader");
    EXPECT_EQ(colony->getBacteriumState(1).programName, "follower");
    EXPECT_DOUBLE_EQ(colony->getSimulationStep(), 0.25);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_DOUBLE_EQ(colony->getSimulationStep(), 0.25);
    EXPECT_EQ(colony->getPopulationSize(), 2u);
    // Problem B (Phase 3, authorized precedence): no BacteriaSignalGrid is
    // attached here, so "ahl := signal(1, 1)" (kdiff=1, kdeg=1) now
    // controls the legacy field's own diffusion/decay. leader() emits 5.0
    // into cell (0,0) during the per-bacterium loop (read correctly by
    // follower()'s get_signal(ahl) below, before the once-per-step decay
    // runs - see the leader/follower assertions), but the single-cell
    // grid has no neighbors (diffusion is a no-op) and decayFactor =
    // 1 - kdeg = 0, so the field is deliberately zeroed by the end of
    // this same step. This is the correct, intended effect of kdeg=1 on
    // a 1x1 grid, not a leftover of the old "coefficients ignored" gap
    // (previously this value stayed exactly 5.0 because the legacy field
    // never honored the declared coefficients at all).
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 0.0);
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "p.t"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "p.mode"));
    EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(1, "p.t"));
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "p.t"), 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "p.mode"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "p.t"), 0.25);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyRejectsDifferentCoefficientSignalDeclarationsAcrossNamedPrograms) {
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_ConflictingChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program first() := { s := signal(0.2, 0.1); true : { emit_signal(s, 1) } }; "
        "program second() := { s := signal(0.8, 0.1); true : { emit_signal(s, 1) } }; "
        "ecoli([x:=0, y:=0], program first()); "
        "ecoli([x:=1, y:=0], program second());");
    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(model, "BacteriaColony_ConflictingChannels");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_FALSE(result.succeeded);
    EXPECT_NE(result.errorMessage.find("independently declared"), std::string::npos) << result.errorMessage;
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyRejectsIndependentEqualCoefficientSignalDeclarationsAcrossNamedPrograms) {
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_IndependentChannels");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program first() := { first_signal := signal(0, 0); true : { emit_signal(first_signal, 5) } }; "
        "program second() := { second_signal := signal(0, 0); true : { emit_signal(second_signal, 7) } }; "
        "ecoli([x:=0, y:=0], program first()); "
        "ecoli([x:=1, y:=0], program second());");
    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(model, "BacteriaColony_IndependentChannels");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setGridWidth(4);
    colony->setGridHeight(2);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_FALSE(result.succeeded) << "independent declarations must not silently alias channel 1";
    EXPECT_NE(result.errorMessage.find("independently declared"), std::string::npos) << result.errorMessage;
}

TEST(RuntimePluginManagerClassTest, BacteriaColonySharesOneDeclaredSignalAcrossBacteriaOfSameNamedProgram) {
    Simulator simulator;
    ASSERT_NE(simulator.getPluginManager(), nullptr);
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = simulator.getPluginManager()->newInstance<GroProgram>(model, "GroProgram_SharedNamedChannel");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program shared() := { s := signal(0, 0); true : { emit_signal(s, bacterium_id) } }; "
        "ecoli([x:=0, y:=0], program shared()); "
        "ecoli([x:=1, y:=0], program shared());");
    BacteriaColony* colony = simulator.getPluginManager()->newInstance<BacteriaColony>(model, "BacteriaColony_SharedNamedChannel");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    ASSERT_TRUE(result.succeeded) << result.errorMessage;
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    const auto& first = colony->getBacteriumState(0);
    const auto& second = colony->getBacteriumState(1);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(first.gridX, first.gridY), 1.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(second.gridX, second.gridY), 2.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonySharesBacteriumProgramSignalAcrossBacteria) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_BacteriumScopeSharedChannel");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program bacterium() { s := signal(0, 0); emit_signal(s, bacterium_id); }");
    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_BacteriumScopeSharedChannel");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setInitialPopulation(2);
    colony->setGridWidth(4);
    colony->setGridHeight(2);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    ASSERT_TRUE(result.succeeded) << result.errorMessage;
    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "s"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "s"), 1.0);
    const auto& first = colony->getBacteriumState(0);
    const auto& second = colony->getBacteriumState(1);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(first.gridX, first.gridY), 1.0);
    EXPECT_DOUBLE_EQ(colony->getSignalValueAt(second.gridX, second.gridY), 2.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesGroSeedsBeforeFirstGuiDrivenStep) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_GuiDrivenSeeds");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "set(\"dt\", 0.1); "
        "program leader() := { p := [ t := 0 ]; true : { p.t := p.t + dt } }; "
        "program follower() := { p := [ mode := 0 ]; true : { p.mode := 1 } }; "
        "ecoli([x:=0, y:=0], program leader()); "
        "ecoli([x:=0, y:=10], program follower());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_GuiDrivenSeeds");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    EXPECT_EQ(colony->getBacteriumState(0).programName, "leader");
    EXPECT_EQ(colony->getBacteriumState(1).programName, "follower");
    EXPECT_EQ(colony->getBacteriumState(1).gridY, 10u);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getColonyTime(), 0.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "p.t"), 0.1);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(1, "p.mode"), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyExecutesMainProgramBeforeSeededBacteriaPrograms) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_GlobalMain");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "set(\"dt\", 0.2); "
        "program main() := { warmup := warmup + 1; tick(); }; "
        "program leader() := { p := [ seen_warmup := 0 ]; true : { p.seen_warmup := warmup } }; "
        "ecoli([x:=0, y:=0], program leader());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_GlobalMain");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    GroProgramRuntime::ExecutionResult first = colony->executeGroProgram();
    EXPECT_TRUE(first.succeeded) << first.errorMessage;
    EXPECT_TRUE(colony->hasRuntimeVariable("warmup"));
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("warmup"), 1.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "p.seen_warmup"), 1.0);

    GroProgramRuntime::ExecutionResult second = colony->executeGroProgram();
    EXPECT_TRUE(second.succeeded) << second.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("warmup"), 2.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "p.seen_warmup"), 2.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyMainCanResetAndRespawnSeeds) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ResetAndRespawn");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "program main() := { reset(); ecoli([x:=2, y:=3], program leader()); }; "
        "program leader() := { p := [ seen := 0 ]; true : { p.seen := p.seen + 1 } }; "
        "ecoli([x:=0, y:=0], program leader());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ResetAndRespawn");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_EQ(colony->getInternalBacteriaCount(), 1u);
    EXPECT_EQ(colony->getPopulationSize(), 1u);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).positionX, 2.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).positionY, 3.0);
    EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "p.seen"), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesChemostatAndBarrierMutations) {
    Simulator simulator;
    PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_Environment");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("chemostat(true); barrier(1, 2, 3, 4);");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_Environment");
    ASSERT_NE(colony, nullptr);
    colony->setInitialPopulation(0);
    colony->setGroProgram(program);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_TRUE(colony->getChemostatMode());
    ASSERT_EQ(colony->getBarriers().size(), 1u);
    EXPECT_DOUBLE_EQ(colony->getBarriers().front().x1, 1.0);
    EXPECT_DOUBLE_EQ(colony->getBarriers().front().y1, 2.0);
    EXPECT_DOUBLE_EQ(colony->getBarriers().front().x2, 3.0);
    EXPECT_DOUBLE_EQ(colony->getBarriers().front().y2, 4.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesMapToCellsValues) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_MapToCells");
    ASSERT_NE(program, nullptr);
    program->setSourceCode("map_to_cells(bacterium_index);");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_MapToCells");
    ASSERT_NE(colony, nullptr);
    colony->setInitialPopulation(2);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    ASSERT_EQ(colony->getMappedCellValues().size(), 2u);
    EXPECT_DOUBLE_EQ(colony->getMappedCellValues()[0], 0.0);
    EXPECT_DOUBLE_EQ(colony->getMappedCellValues()[1], 1.0);
    EXPECT_EQ(colony->getMappedCellExpression(), "bacterium_index");
	ASSERT_EQ(result.mappedCellValues.size(), 2u);
	EXPECT_DOUBLE_EQ(result.mappedCellValues[0], 0.0);
	EXPECT_DOUBLE_EQ(result.mappedCellValues[1], 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesSetSignalMutationsToCenteredGridCoordinates) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SetSignal");
	ASSERT_NE(program, nullptr);
	program->setSourceCode("program main() { set_signal(0, 0, 0, 7.5); set_signal_rect(0, 0, 0, 1, 1, 2.5); }");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SetSignal");
	ASSERT_NE(colony, nullptr);
	colony->setInitialPopulation(0);
	colony->setGridWidth(32);
	colony->setGridHeight(32);
	colony->setGroProgram(program);
	ModelDataDefinition::InitBetweenReplications(colony);

	GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_DOUBLE_EQ(colony->getSignalValueAt(16, 16), 2.5);
	EXPECT_DOUBLE_EQ(colony->getSignalValueAt(17, 17), 2.5);
	EXPECT_DOUBLE_EQ(colony->getSignalValueAt(0, 0), 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesSignalGridConfigurationMutations) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SignalGridConfig");
	ASSERT_NE(program, nullptr);
	program->setSourceCode("program main() { set(\"signal_grid_width\", 7); set(\"signal_grid_height\", 5); }");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SignalGridConfig");
	ASSERT_NE(colony, nullptr);
	colony->setInitialPopulation(0);
	colony->setGroProgram(program);
	ModelDataDefinition::InitBetweenReplications(colony);

	GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_EQ(colony->getGridWidth(), 7u);
	EXPECT_EQ(colony->getGridHeight(), 5u);
	EXPECT_DOUBLE_EQ(colony->getSignalValueAt(6, 4), 0.0);
}

TEST(RuntimePluginManagerClassTest, GroProgramRuntimeSupportsSignalMatrixAndDumpCommands) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_SignalMatrixDump");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
	    "program colony() { "
	    "set(\"signal_grid_width\", 3); "
	    "set(\"signal_grid_height\", 2); "
	    "set_signal_rect(0, -1, -1, 0, 0, 4.5); "
	    "get_signal_matrix(); "
	    "dump_signal_field(1, 2); "
	    "}");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_SignalMatrixDump");
	ASSERT_NE(colony, nullptr);
	colony->setInitialPopulation(0);
	colony->setGroProgram(program);
	ModelDataDefinition::InitBetweenReplications(colony);

	GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_EQ(result.signalMatrixWidth, 3u);
	EXPECT_EQ(result.signalMatrixHeight, 2u);
	ASSERT_EQ(result.signalMatrixValues.size(), 6u);
	EXPECT_DOUBLE_EQ(result.signalMatrixValues[0], 4.5);
	EXPECT_DOUBLE_EQ(result.signalMatrixValues[1], 4.5);
	EXPECT_DOUBLE_EQ(result.signalMatrixValues[3], 4.5);
	EXPECT_DOUBLE_EQ(result.signalMatrixValues[4], 4.5);
	EXPECT_NE(result.signalFieldDump.find("signal_matrix 3x2"), std::string::npos);
	EXPECT_NE(result.signalFieldDump.find("(preview)"), std::string::npos);
	EXPECT_NE(result.signalFieldDump.find("row 0:"), std::string::npos);
	EXPECT_NE(result.signalFieldDump.find("4.5"), std::string::npos);
	EXPECT_EQ(result.signalFieldDump.find("row 1:"), std::string::npos);
	EXPECT_EQ(colony->getSignalMatrix().size(), 2u);
	EXPECT_EQ(colony->getSignalMatrix()[0].size(), 3u);
	EXPECT_DOUBLE_EQ(colony->getSignalMatrix()[0][0], 4.5);
	EXPECT_NE(colony->getSignalMatrixDump(2, 3).find("signal_matrix 3x2"), std::string::npos);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesVisibleGrowthToBacteria) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_VisibleGrowth");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "set(\"ecoli_growth_rate\", 0.6); "
        "program grower() := { skip(); }; "
        "ecoli([x:=1, y:=1], program grower());");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_VisibleGrowth");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);

    GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
    EXPECT_TRUE(result.succeeded) << result.errorMessage;
    EXPECT_GT(colony->getBacteriumVolume(0), 1.0);
    EXPECT_GT(colony->getBacteriumSize(0), 1.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "volume"), 1.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "size"), 1.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyKeepsExplicitZeroGrowthRateAtZero) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ZeroGrowthRate");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0); "
			"program grower() := { skip(); }; "
			"ecoli([x:=0, y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ZeroGrowthRate");
	ASSERT_NE(colony, nullptr);
	colony->setGroProgram(program);
	const double initialVolume = colony->getBacteriumVolume(0);
	const double initialSize = colony->getBacteriumSize(0);
	const double initialSpeed = colony->getBacteriumState(0).speed;

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	ASSERT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0), initialVolume);
	EXPECT_DOUBLE_EQ(colony->getBacteriumSize(0), initialSize);
	EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).speed, initialSpeed);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyDoesNotGrowAtZeroSimulationStep) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);
	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_ZeroSimulationStep");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
			"set(\"ecoli_growth_rate\", 0.6); "
			"program grower() := { skip(); }; "
			"ecoli([x:=0, y:=0], program grower());");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_ZeroSimulationStep");
	ASSERT_NE(colony, nullptr);
	colony->setSimulationStep(0.0);
	colony->setGroProgram(program);
	const double initialVolume = colony->getBacteriumVolume(0);
	const double initialSize = colony->getBacteriumSize(0);
	const double initialSpeed = colony->getBacteriumState(0).speed;

	const GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	ASSERT_TRUE(result.succeeded) << result.errorMessage;
	EXPECT_DOUBLE_EQ(colony->getBacteriumVolume(0), initialVolume);
	EXPECT_DOUBLE_EQ(colony->getBacteriumSize(0), initialSize);
	EXPECT_DOUBLE_EQ(colony->getBacteriumState(0).speed, initialSpeed);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyProducesVisibleMotionSignalsAndFluorescence) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
    ASSERT_NE(manager, nullptr);
    manager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_MotionSignalFluorescence");
    ASSERT_NE(program, nullptr);
    program->setSourceCode(
        "set(\"dt\", 0.18); "
        "set(\"ecoli_growth_rate\", 0.12); "
        "wave := signal(0.85, 0.035); "
        "program leader() := { "
        "  p.phase := 0; p.timer := 0; "
        "  true : { "
        "    p.timer := p.timer + dt, "
        "    volume := volume + 0.05, "
        "    size := volume, "
        "    speed := 0.22, "
        "    gfp := 30 + 18 * volume, "
        "    direction := direction + 0.08, "
        "    emit_signal(wave, 8), "
        "    rfp := 6 + 8 * get_signal(wave) "
        "  } "
        "}; "
        "program follower() := { "
        "  p.t := 0; sense := get_signal(wave); "
        "  true : { "
        "    p.t := p.t + dt, "
        "    volume := volume + 0.035 + 0.02 * sense, "
        "    size := volume, "
        "    speed := 0.08 + 0.12 * sense, "
        "    yfp := 12 + 24 * sense, "
        "    cfp := 8 + 14 * p.t "
        "  } "
        "  sense > 0.08 : { direction := direction + 0.22, emit_signal(wave, 6) } "
        "  sense > 0.20 : { rfp := 24 + 40 * sense } "
        "}; "
        "ecoli([x:=10, y:=10], program leader()); "
        "ecoli([x:=12, y:=12], program follower());");

    BacteriaSignalGrid* signalGrid = manager->newInstance<BacteriaSignalGrid>(model, "SignalGrid_MotionSignalFluorescence");
    ASSERT_NE(signalGrid, nullptr);
    signalGrid->setWidth(16);
    signalGrid->setHeight(16);

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_MotionSignalFluorescence");
    ASSERT_NE(colony, nullptr);
    colony->setSignalGrid(signalGrid);
    colony->setGroProgram(program);
    ModelDataDefinition::InitBetweenReplications(colony);

    ASSERT_EQ(colony->getInternalBacteriaCount(), 2u);
    const double initialX = colony->getBacteriumPositionX(0);
    const double initialY = colony->getBacteriumPositionY(0);

    for (unsigned int step = 0; step < 8; ++step) {
        GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
        ASSERT_TRUE(result.succeeded) << result.errorMessage;
    }

    double maxSignal = 0.0;
    for (unsigned int y = 0; y < colony->getGridHeight(); ++y) {
        for (unsigned int x = 0; x < colony->getGridWidth(); ++x) {
            maxSignal = std::max(maxSignal, colony->getSignalValueAt(x, y));
        }
    }

    EXPECT_NE(colony->getBacteriumPositionX(0), initialX);
    EXPECT_NE(colony->getBacteriumPositionY(0), initialY);
    EXPECT_GT(colony->getBacteriumVolume(0), 1.0);
    EXPECT_GT(colony->getBacteriumSize(0), 1.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "gfp"), 0.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(0, "rfp"), 0.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(1, "yfp"), 0.0);
    EXPECT_GT(colony->getBacteriumRuntimeVariableValue(1, "cfp"), 0.0);
    EXPECT_GT(maxSignal, 0.0);
}

TEST(RuntimePluginManagerClassTest, BacteriaColonyAppliesRunAndTumbleToBacteriumScopedState) {
	Simulator simulator;
	PluginManager* manager = simulator.getPluginManager();
	ASSERT_NE(manager, nullptr);
	manager->autoInsertPlugins();

	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	GroProgram* program = manager->newInstance<GroProgram>(model, "GroProgram_RunAndTumble");
	ASSERT_NE(program, nullptr);
	program->setSourceCode(
	    "program bacterium() { run(0.25); tumble(0.5); }");

	BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_RunAndTumble");
	ASSERT_NE(colony, nullptr);
	colony->setGroProgram(program);
	colony->setGridWidth(12);
	colony->setGridHeight(12);
	colony->setInitialPopulation(1);

	ModelDataDefinition::InitBetweenReplications(colony);
	ASSERT_EQ(colony->getInternalBacteriaCount(), 1u);

	const double initialDirection = colony->getBacteriumDirectionRadians(0);
	const double initialSpeed = colony->getBacteriumState(0).speed;

	GroProgramRuntime::ExecutionResult result = colony->executeGroProgram();
	EXPECT_TRUE(result.succeeded) << result.errorMessage;
	ASSERT_EQ(result.motionMutations.size(), 2u);
	EXPECT_EQ(result.motionMutations[0].type, GroProgramRuntime::MotionMutationType::Run);
	EXPECT_EQ(result.motionMutations[1].type, GroProgramRuntime::MotionMutationType::Tumble);
	EXPECT_NE(colony->getBacteriumDirectionRadians(0), initialDirection);
	EXPECT_NE(colony->getBacteriumState(0).speed, initialSpeed);
	EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "speed"));
	EXPECT_TRUE(colony->hasBacteriumRuntimeVariable(0, "direction"));
	EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "speed"), colony->getBacteriumState(0).speed);
	EXPECT_DOUBLE_EQ(colony->getBacteriumRuntimeVariableValue(0, "direction"), colony->getBacteriumDirectionRadians(0));
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
        "tick(); "
        "}");

    BacteriaColony* colony = manager->newInstance<BacteriaColony>(model, "BacteriaColony_BioAware");
    ASSERT_NE(colony, nullptr);
    colony->setGroProgram(program);
    colony->setBioNetwork(bioNetwork);
    colony->setSimulationStep(0.5);
    colony->setInitialPopulation(1);

    ModelDataDefinition::InitBetweenReplications(substrate);
    ModelDataDefinition::InitBetweenReplications(product);
    ModelDataDefinition::InitBetweenReplications(bioNetwork);
    ModelDataDefinition::InitBetweenReplications(colony);

    GroProgramRuntime::ExecutionResult firstResult = colony->executeGroProgram();
    EXPECT_TRUE(firstResult.succeeded) << firstResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_substrate"), 10.0);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_product"), 0.0);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.0);
    EXPECT_DOUBLE_EQ(substrate->getAmount(), 10.0);
    EXPECT_DOUBLE_EQ(product->getAmount(), 0.0);

    const double substrateAfterFirstStep = substrate->getAmount();
    const double productAfterFirstStep = product->getAmount();

    GroProgramRuntime::ExecutionResult secondResult = colony->executeGroProgram();
    EXPECT_TRUE(secondResult.succeeded) << secondResult.errorMessage;
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_substrate"), substrateAfterFirstStep);
    EXPECT_DOUBLE_EQ(colony->getRuntimeVariableValue("seen_product"), productAfterFirstStep);
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.0);
    EXPECT_DOUBLE_EQ(substrate->getAmount(), substrateAfterFirstStep);
    EXPECT_DOUBLE_EQ(product->getAmount(), productAfterFirstStep);
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
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.0);
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
    EXPECT_DOUBLE_EQ(bioNetwork->getCurrentTime(), 0.0);
    EXPECT_FALSE(colony->hasBacteriumRuntimeVariable(0, "bio_species_sharedsignal"));
    EXPECT_FALSE(colony->hasBacteriumRuntimeVariable(1, "bio_species_sharedsignal"));
}
