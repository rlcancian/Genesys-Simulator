#include <gtest/gtest.h>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/components/InputOutput/Read.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/data/Logic/Variable.h"

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <limits>
#include <string>
#include <vector>
#include <unistd.h>

namespace {
class TemporaryInput {
public:
    explicit TemporaryInput(const std::string& text, const std::string& extension = ".txt") {
        static std::atomic<unsigned long> next{0};
        path = std::filesystem::temp_directory_path() /
            ("genesys_read_" + std::to_string(::getpid()) + "_" +
             std::to_string(++next) + extension);
        std::ofstream output(path);
        output << text;
        if (!output.good()) throw std::runtime_error("cannot write Read test fixture");
    }
    ~TemporaryInput() { std::error_code ignored; std::filesystem::remove(path, ignored); }
    std::filesystem::path path;
};

class ReadProbe final : public Read {
public:
    using Read::Read;
    void dispatch(Entity* entity) { _onDispatchEvent(entity, 0); }
    void resetReplication() { _initBetweenReplications(); }
    bool check(std::string& error) { return _check(error); }
};

struct ReadFixture : testing::Test {
    Simulator simulator;
    Model* model = nullptr;

    void SetUp() override {
        simulator.getPluginManager()->autoInsertPlugins();
        model = simulator.getModelManager()->newModel();
        ASSERT_NE(model, nullptr);
    }

    ReadProbe* reader(const std::string& name = "Input") {
        auto* component = new ReadProbe(model, name);
        component->connectTo(new Dispose(model, name + "_Dispose"));
        return component;
    }

    static void addAttribute(Read* read, const std::string& name) {
        read->addDestination(new ReadDestination(name, true));
    }
    static void addVariable(Read* read, const std::string& name) {
        read->addDestination(new ReadDestination(name, false));
    }

    Variable* variable(const std::string& name) {
        return dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(
            Util::TypeOf<Variable>(), name));
    }
};
} // namespace

TEST_F(ReadFixture, MetadataAndStaticPluginRegistration) {
    std::unique_ptr<PluginInformation> info(Read::GetPluginInformation());
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->getPluginTypename(), "Read");
    EXPECT_EQ(info->getCategory(), "InputOutput");
    EXPECT_EQ(info->getMinimumInputs(), 1u);
    EXPECT_EQ(info->getMaximumInputs(), 1u);
    EXPECT_EQ(info->getMinimumOutputs(), 1u);
    EXPECT_EQ(info->getMaximumOutputs(), 1u);
    EXPECT_NE(simulator.getPluginManager()->find("Read"), nullptr);
}

TEST(ReadPluginIntrospectionRegression, EmptyRecordTemplateCompletionIsNotReportedAsError) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    ASSERT_NE(simulator.getPluginManager()->find("Read"), nullptr);
    EXPECT_TRUE(simulator.getTraceManager()->errorMessages()->empty());
}

TEST_F(ReadFixture, PropertyEditorCreatesNamedDestinationsAndExposesChildControls) {
    auto* read = reader();
    SimulationControl* destinationsControl = nullptr;
    for (SimulationControl* control : *read->getSimulationControls()->list()) {
        if (control->getName() == "Destinations") {
            destinationsControl = control;
            break;
        }
    }
    ASSERT_NE(destinationsControl, nullptr);
    ASSERT_TRUE(destinationsControl->supportsListEditor());
    ASSERT_TRUE(destinationsControl->supportsNewListElementCreation());
    ASSERT_TRUE(destinationsControl->createNewListElement("Measurement"));
    ASSERT_EQ(read->getDestinations()->size(), 1u);
    auto* destination = read->getDestinations()->getAtRank(0);
    ASSERT_NE(destination, nullptr);
    EXPECT_EQ(destination->getDestination(), "Measurement");
    ASSERT_NE(destinationsControl->getChildSimulationControls(0), nullptr);
    EXPECT_EQ(destinationsControl->getChildSimulationControls(0)->size(), 2u);
    std::unique_ptr<List<std::string>> labels(destinationsControl->getStrValues());
    ASSERT_NE(labels, nullptr);
    EXPECT_EQ(labels->getAtRank(0), "Measurement");
}

TEST_F(ReadFixture, PropertyEditorDefaultCreationAllowsChildNamingAndReordering) {
    auto* read = reader();
    SimulationControl* destinationsControl = nullptr;
    for (SimulationControl* control : *read->getSimulationControls()->list()) {
        if (control->getName() == "Destinations") {
            destinationsControl = control;
            break;
        }
    }
    ASSERT_NE(destinationsControl, nullptr);
    // The actual GUI calls createNewListElement() without a name.
    ASSERT_TRUE(destinationsControl->createNewListElement());
    ASSERT_EQ(read->getDestinations()->size(), 1u);
    EXPECT_TRUE(read->getDestinations()->getAtRank(0)->getDestination().empty());
    auto* firstControls = destinationsControl->getChildSimulationControls(0);
    ASSERT_NE(firstControls, nullptr);
    SimulationControl* firstDestination = nullptr;
    for (SimulationControl* control : *firstControls->list()) {
        if (control->getName() == "Destination") firstDestination = control;
    }
    ASSERT_NE(firstDestination, nullptr);
    firstDestination->setValue("FirstInput");
    EXPECT_EQ(read->getDestinations()->getAtRank(0)->getDestination(), "FirstInput");

    ASSERT_TRUE(destinationsControl->createNewListElement());
    auto* secondControls = destinationsControl->getChildSimulationControls(1);
    ASSERT_NE(secondControls, nullptr);
    for (SimulationControl* control : *secondControls->list()) {
        if (control->getName() == "Destination") control->setValue("SecondInput");
    }
    ASSERT_EQ(read->getDestinations()->size(), 2u);
    EXPECT_EQ(read->getDestinations()->getAtRank(1)->getDestination(), "SecondInput");

    destinationsControl->moveListElement(0, 1);
    EXPECT_EQ(read->getDestinations()->getAtRank(0)->getDestination(), "SecondInput");
    EXPECT_EQ(read->getDestinations()->getAtRank(1)->getDestination(), "FirstInput");
}

TEST_F(ReadFixture, FileRecordsAreConsumedInOrderAndRoutedAfterCompleteAssignments) {
    TemporaryInput file("# data\n\n10 25 3\n12 30 4\n15 28 2\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "ProcessingTime");
    addVariable(read, "CurrentLoad");
    addAttribute(read, "Priority");
    std::string error;
    ASSERT_TRUE(read->check(error)) << error;

    const double expected[3][3] = {{10,25,3}, {12,30,4}, {15,28,2}};
    for (const auto& row : expected) {
        Entity* entity = model->createEntity("ReadEntity", true);
        read->dispatch(entity);
        EXPECT_DOUBLE_EQ(entity->getAttributeValue("ProcessingTime"), row[0]);
        EXPECT_DOUBLE_EQ(entity->getAttributeValue("Priority"), row[2]);
        ASSERT_NE(variable("CurrentLoad"), nullptr);
        EXPECT_DOUBLE_EQ(variable("CurrentLoad")->getValue(), row[1]);
    }
    ASSERT_EQ(model->getFutureEvents()->size(), 3u);
    for (Event* event : *model->getFutureEvents()->list()) {
        EXPECT_EQ(event->getComponent(), read->getConnectionManager()->getFrontConnection()->component);
    }
}

TEST_F(ReadFixture, KeyboardProviderProcessesValuesWithoutHumanInteraction) {
    auto* read = reader();
    read->setReadFromType(Read::ReadFromType::KEYBOARD);
    addAttribute(read, "X");
    addVariable(read, "Y");
    std::vector<std::string> records{"1 2", "3 4"};
    std::size_t offset = 0;
    read->setKeyboardInputProvider([&](const std::string& prompt, std::string& record) {
        EXPECT_NE(prompt.find("2 numeric"), std::string::npos);
        if (offset == records.size()) return false;
        record = records[offset++];
        return true;
    });
    for (int i = 0; i < 2; ++i) {
        Entity* entity = model->createEntity("KeyboardEntity", true);
        read->dispatch(entity);
        EXPECT_DOUBLE_EQ(entity->getAttributeValue("X"), i == 0 ? 1 : 3);
    }
    ASSERT_NE(variable("Y"), nullptr);
    EXPECT_DOUBLE_EQ(variable("Y")->getValue(), 4);
    EXPECT_EQ(model->getFutureEvents()->size(), 2u);
}

TEST_F(ReadFixture, IncompleteAndInvalidRecordsDoNotPartiallyAssignOrForward) {
    TemporaryInput file("1 2\nnot-a-number 9\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "X");
    addVariable(read, "Y");
    Entity* first = model->createEntity("First", true);
    read->dispatch(first);
    EXPECT_DOUBLE_EQ(first->getAttributeValue("X"), 1);
    ASSERT_NE(variable("Y"), nullptr);
    EXPECT_DOUBLE_EQ(variable("Y")->getValue(), 2);
    auto* second = model->createEntity("Second", true);
    read->dispatch(second);
    EXPECT_EQ(model->getFutureEvents()->size(), 1u);
    EXPECT_DOUBLE_EQ(second->getAttributeValue("X"), 0);
    EXPECT_DOUBLE_EQ(variable("Y")->getValue(), 2);
}

TEST_F(ReadFixture, OversizedNumericIndexCannotPartiallyAssignEarlierFields) {
    TemporaryInput file("12 34\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "FirstField");
    addVariable(read, "UnsafeIndex[" + std::to_string(std::numeric_limits<unsigned int>::max()) + "]");
    std::string error;
    EXPECT_FALSE(read->check(error));
    EXPECT_NE(error.find("invalid [index]"), std::string::npos);

    Entity* entity = model->createEntity("UnsafeIndexEntity", true);
    read->dispatch(entity);
    EXPECT_EQ(model->getFutureEvents()->size(), 0u);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("FirstField"), 0.0);
}

TEST_F(ReadFixture, MalformedNumericIndexCannotPartiallyAssignEarlierFields) {
    TemporaryInput file("7 8\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "FirstField");
    addVariable(read, "MalformedIndex[1,,2]");
    std::string error;
    EXPECT_FALSE(read->check(error));

    Entity* entity = model->createEntity("MalformedIndexEntity", true);
    read->dispatch(entity);
    EXPECT_TRUE(model->getFutureEvents()->empty());
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("FirstField"), 0.0);
}

TEST_F(ReadFixture, LargeButRepresentableIndexCannotExhaustMemoryOrPartiallyAssign) {
    // UINT_MAX-1 passed the previous overflow-only guard, but the dense
    // SparseValueStore would try to allocate billions of double values.
    TemporaryInput file("12 34\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "Untouched");
    addVariable(read, "HugeIndex[" +
        std::to_string(std::numeric_limits<unsigned int>::max() - 1u) + "]");
    std::string error;
    EXPECT_FALSE(read->check(error));
    EXPECT_NE(error.find("invalid [index]"), std::string::npos);
    Entity* entity = model->createEntity("HugeIndexEntity", true);
    read->dispatch(entity);
    EXPECT_EQ(model->getFutureEvents()->size(), 0u);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Untouched"), 0.0);
}

TEST_F(ReadFixture, MultidimensionalIndexProductIsBoundedBeforeWrites) {
    TemporaryInput file("9 10\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "Untouched");
    addVariable(read, "HugeMatrix[9999,9999]");
    std::string error;
    EXPECT_FALSE(read->check(error));
    Entity* entity = model->createEntity("HugeMatrixEntity", true);
    read->dispatch(entity);
    EXPECT_TRUE(model->getFutureEvents()->empty());
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Untouched"), 0.0);
}

TEST_F(ReadFixture, KeyboardProviderExceptionStopsInsteadOfEscapingDispatch) {
    auto* read = reader();
    read->setReadFromType(Read::ReadFromType::KEYBOARD);
    addVariable(read, "Value");
    read->setKeyboardInputProvider([](const std::string&, std::string&) -> bool {
        throw std::runtime_error("input bridge unavailable");
    });
    EXPECT_NO_THROW(read->dispatch(model->createEntity("ProviderException", true)));
    EXPECT_TRUE(model->getFutureEvents()->empty());
    ASSERT_NE(variable("Value"), nullptr);
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 0.0);
    const auto* errors = model->getTracer()->errorMessages();
    ASSERT_NE(errors, nullptr);
    EXPECT_FALSE(errors->empty());
}

TEST_F(ReadFixture, RemovedDestinationControlsAreReleasedOnComponentTeardown) {
    const auto controlsBefore = model->getControls()->size();
    auto* read = new Read(model, "ControlLifetime");
    const auto controlsWithComponent = model->getControls()->size();
    auto* destination = new ReadDestination("", false);
    read->addDestination(destination);
    EXPECT_EQ(model->getControls()->size(), controlsWithComponent + 2u);
    read->removeDestination(destination);
    // Intentionally retained while a GUI editor could still have bindings.
    EXPECT_EQ(model->getControls()->size(), controlsWithComponent + 2u);
    delete read;
    EXPECT_EQ(model->getControls()->size(), controlsBefore);
}

TEST_F(ReadFixture, EndOfFileDoesNotForwardAnExtraEntity) {
    TemporaryInput file("8\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addVariable(read, "Value");
    read->dispatch(model->createEntity("First", true));
    EXPECT_EQ(model->getFutureEvents()->size(), 1u);
    read->dispatch(model->createEntity("Second", true));
    EXPECT_EQ(model->getFutureEvents()->size(), 1u);
    ASSERT_NE(variable("Value"), nullptr);
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 8);
}

TEST_F(ReadFixture, ResetBetweenReplicationsRewindsFileCursor) {
    TemporaryInput file("5\n9\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addVariable(read, "Value");
    read->dispatch(model->createEntity("First", true));
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 5);
    read->dispatch(model->createEntity("Second", true));
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 9);
    read->resetReplication();
    read->dispatch(model->createEntity("Third", true));
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 5);
    EXPECT_EQ(model->getFutureEvents()->size(), 3u);
}

TEST_F(ReadFixture, ModelCheckRejectsMissingFileAndEmptyDestinations) {
    auto* read = reader();
    std::string error;
    EXPECT_FALSE(read->check(error));
    EXPECT_NE(error.find("Filename"), std::string::npos);
    EXPECT_NE(error.find("destination"), std::string::npos);
    addVariable(read, "Only");
    read->setFilename("/this/is/a/nonexistent/file/genesys-read.txt");
    error.clear();
    EXPECT_FALSE(read->check(error));
    EXPECT_NE(error.find("cannot open"), std::string::npos);
}

TEST_F(ReadFixture, KeyboardCancellationDoesNotForwardOrApplyChanges) {
    auto* read = reader();
    read->setReadFromType(Read::ReadFromType::KEYBOARD);
    addVariable(read, "Value");
    read->setKeyboardInputProvider([](const std::string&, std::string&) { return false; });
    read->dispatch(model->createEntity("Cancelled", true));
    EXPECT_TRUE(model->getFutureEvents()->empty());
    ASSERT_NE(variable("Value"), nullptr);
    EXPECT_DOUBLE_EQ(variable("Value")->getValue(), 0);
}

TEST_F(ReadFixture, PersistenceRestoresSourceFileAndOrderedDestinations) {
    TemporaryInput file("4 7\n");
    auto* read = reader();
    read->setFilename(file.path.string());
    addAttribute(read, "Speed");
    addVariable(read, "Load");
    std::string error;
    ASSERT_TRUE(read->check(error)) << error;
    TemporaryInput serialized("", ".gen");
    ASSERT_TRUE(model->save(serialized.path.string()));
    Model* restored = simulator.getModelManager()->newModel();
    ASSERT_NE(restored, nullptr);
    ASSERT_TRUE(restored->load(serialized.path.string()));
    auto* loaded = dynamic_cast<Read*>(restored->getComponentManager()->find("Input"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->readFromType(), Read::ReadFromType::FILE);
    EXPECT_EQ(loaded->filename(), file.path.string());
    ASSERT_EQ(loaded->getDestinations()->size(), 2u);
    EXPECT_EQ(loaded->getDestinations()->getAtRank(0)->getDestination(), "Speed");
    EXPECT_TRUE(loaded->getDestinations()->getAtRank(0)->isAttributeNotVariable());
    EXPECT_EQ(loaded->getDestinations()->getAtRank(1)->getDestination(), "Load");
    EXPECT_FALSE(loaded->getDestinations()->getAtRank(1)->isAttributeNotVariable());
    EXPECT_EQ(loaded->getDestinations()->getAtRank(0)->getSimulationControls()->size(), 2u);
}

TEST_F(ReadFixture, CreateReadDisposeModelRunsToCompletion) {
    TemporaryInput file("11\n22\n");
    auto* source = new Create(model, "CreateReadEntities");
    auto* read = new Read(model, "ReadFileRecords");
    auto* sink = new Dispose(model, "DisposeReadEntities");
    auto* entityType = new EntityType(model, "ReadEntityType");
    source->setEntityType(entityType);
    source->setFirstCreation(0.0);
    source->setTimeBetweenCreationsExpression("1.0");
    source->setMaxCreations(2);
    read->setFilename(file.path.string());
    addVariable(read, "LastValue");
    source->connectTo(read);
    read->connectTo(sink);
    model->getSimulation()->setReplicationLength(10.0, Util::TimeUnit::second);
    model->getSimulation()->setNumberOfReplications(1);
    ASSERT_TRUE(model->check());
    model->getSimulation()->start();
    ASSERT_NE(variable("LastValue"), nullptr);
    EXPECT_DOUBLE_EQ(variable("LastValue")->getValue(), 22);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions(Util::TypeOf<Entity>()), 0u);
}
