#include <gtest/gtest.h>

#include <algorithm>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"

namespace {
struct SimulationStartObserver {
    bool called = false;
    bool running = false;
    bool paused = true;
    unsigned int replication = 0;

    void OnSimulationStart(SimulationEvent* event) {
        called = true;
        running = event->isRunning();
        paused = event->isPaused();
        replication = event->getCurrentReplicationNumber();
    }
};

// Exposes protected attached-data hooks so unit tests can verify non-owning attachment semantics directly.
class AttachedDataAccessProbe : public ModelDataDefinition {
public:
    AttachedDataAccessProbe(Model* model, const std::string& name)
        : ModelDataDefinition(model, "AttachedDataAccessProbe", name, true) {}

    void Attach(std::string key, ModelDataDefinition* data) {
        _attachedDataInsert(key, data);
    }

    void Detach(std::string key) {
        _attachedDataRemove(key);
    }
};

// Provides a minimal concrete data definition type to exercise manager class-name snapshot semantics.
class DataDefinitionSnapshotProbe : public ModelDataDefinition {
public:
    DataDefinitionSnapshotProbe(Model* model, const std::string& name)
        : ModelDataDefinition(model, "DataDefinitionSnapshotProbe", name, true) {}
};
}

TEST(SimulatorRuntimeTest, CanConstructSimulatorAndAccessManagers) {
    Simulator simulator;

    EXPECT_NE(simulator.getPluginManager(), nullptr);
    EXPECT_NE(simulator.getModelManager(), nullptr);
    EXPECT_NE(simulator.getTraceManager(), nullptr);
    EXPECT_NE(simulator.getParserManager(), nullptr);
    EXPECT_NE(simulator.getExperimentManager(), nullptr);
}

TEST(SimulatorRuntimeTest, VersionAndNameAreAvailable) {
    Simulator simulator;

    EXPECT_FALSE(simulator.getName().empty());
    EXPECT_FALSE(simulator.getVersion().empty());
    EXPECT_GT(simulator.getVersionNumber(), 0u);
}

TEST(SimulatorRuntimeTest, ConstructAndDestroySimulatorRepeatedly) {
    for (int i = 0; i < 5; ++i) {
        auto simulator = std::make_unique<Simulator>();
        ASSERT_NE(simulator->getLicenceManager(), nullptr);
        ASSERT_NE(simulator->getPluginManager(), nullptr);
        ASSERT_NE(simulator->getModelManager(), nullptr);
        ASSERT_NE(simulator->getTraceManager(), nullptr);
        ASSERT_NE(simulator->getParserManager(), nullptr);
        ASSERT_NE(simulator->getExperimentManager(), nullptr);
    }
}


TEST(SimulatorRuntimeTest, ModelHasChangedReflectsNestedSubsystemUpdates) {
    Simulator simulator;

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EXPECT_FALSE(model->hasChanged());

    model->getInfos()->setName("ChangedName");

    EXPECT_TRUE(model->hasChanged());
}

TEST(SimulatorRuntimeTest, SimulationStartHandlerReceivesInitializedStateSnapshot) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SimulationStartObserver observer;
    model->getOnEventManager()->addOnSimulationStartHandler(&observer, &SimulationStartObserver::OnSimulationStart);

    model->getSimulation()->start();

    EXPECT_TRUE(observer.called);
    EXPECT_TRUE(observer.running);
    EXPECT_FALSE(observer.paused);
    EXPECT_EQ(observer.replication, 1u);
}

// Ensures creating a new current model repeatedly keeps runtime usable and updates current() consistently.
TEST(SimulatorRuntimeTest, NewModelCanBeCalledMultipleTimesAndUpdatesCurrentModel) {
    Simulator simulator;

    Model* first = simulator.getModelManager()->newModel();
    ASSERT_NE(first, nullptr);

    Model* second = simulator.getModelManager()->newModel();
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(simulator.getModelManager()->current(), second);
    EXPECT_NO_THROW(second->getSimulation()->start());
}

TEST(SimulatorRuntimeTest, ModelClearPreservesBaseSimulationControlsCount) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);
    // Capture the baseline controls created by ModelSimulation and ensure clear() keeps them available.
    const unsigned int controlsBefore = model->getControls()->size();
    ASSERT_GT(controlsBefore, 0u);

    model->clear();

    EXPECT_EQ(model->getControls()->size(), controlsBefore);
}

TEST(SimulatorRuntimeTest, ModelClearIsIdempotentAndKeepsRuntimeUsable) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    // Captures baseline simulation controls and validates clear() can run twice without invalidating core runtime managers.
    const unsigned int controlsBefore = model->getControls()->size();
    ASSERT_GT(controlsBefore, 0u);

    model->clear();
    model->clear();

    EXPECT_EQ(model->getControls()->size(), controlsBefore);
    EXPECT_NE(model->getSimulation(), nullptr);
    EXPECT_NE(model->getDataManager(), nullptr);
    EXPECT_NE(model->getComponentManager(), nullptr);
}

TEST(SimulatorRuntimeTest, DataDefinitionClassnamesSnapshotIsReturnedByValue) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    // Creates managed data definitions so the manager registry exposes at least one class name for snapshot checks.
    auto* first = new DataDefinitionSnapshotProbe(model, "SnapshotA");
    auto* second = new DataDefinitionSnapshotProbe(model, "SnapshotB");
    (void)second;

    // Captures independent snapshots and verifies mutating one local copy does not affect manager state or another snapshot.
    auto names1 = model->getDataManager()->getDataDefinitionClassnames();
    auto names2 = model->getDataManager()->getDataDefinitionClassnames();
    ASSERT_FALSE(names1.empty());
    ASSERT_FALSE(names2.empty());
    EXPECT_NE(std::find(names1.begin(), names1.end(), "DataDefinitionSnapshotProbe"), names1.end());
    EXPECT_NE(std::find(names2.begin(), names2.end(), "DataDefinitionSnapshotProbe"), names2.end());

    names1.remove("DataDefinitionSnapshotProbe");

    EXPECT_EQ(std::find(names1.begin(), names1.end(), "DataDefinitionSnapshotProbe"), names1.end());
    EXPECT_NE(std::find(names2.begin(), names2.end(), "DataDefinitionSnapshotProbe"), names2.end());
    auto names3 = model->getDataManager()->getDataDefinitionClassnames();
    EXPECT_NE(std::find(names3.begin(), names3.end(), "DataDefinitionSnapshotProbe"), names3.end());

    // Explicitly releases test-owned instances to keep teardown deterministic inside this test body.
    delete first;
    delete second;
}

TEST(SimulatorRuntimeTest, AttachedDataRemoveOnlyDetachesRegistryEntry) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    // Creates two managed model data objects and links one as a non-owning attachment of the other.
    auto* owner = new AttachedDataAccessProbe(model, "Owner");
    auto* attached = new AttachedDataAccessProbe(model, "Attached");
    owner->Attach("Ref", attached);

    // Detaches only the attachment mapping and verifies the attached object remains registered in the manager.
    owner->Detach("Ref");
    EXPECT_NE(model->getDataManager()->getDataDefinition("AttachedDataAccessProbe", "Attached"), nullptr);

    delete owner;
    delete attached;
}
