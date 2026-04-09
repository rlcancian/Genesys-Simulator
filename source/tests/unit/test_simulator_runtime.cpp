#include <gtest/gtest.h>
#include <algorithm>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Entity.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

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

class SnapshotDataDefinitionProbe : public ModelDataDefinition {
public:
    SnapshotDataDefinitionProbe(Model* model, const std::string& name)
        : ModelDataDefinition(model, "SnapshotDataDefinitionProbe", name, true) {}
};

static unsigned int g_countingControlProbeDestructorCount = 0;
static unsigned int g_countingChildProbeDestructorCount = 0;

// Tracks owned-property deletion through ModelDataDefinition teardown.
class CountingSimulationControlProbe : public SimulationControl {
public:
    CountingSimulationControlProbe(const std::string& elementName, const std::string& propertyName)
        : SimulationControl("CountingSimulationControlProbe", elementName, propertyName, "") {}

    ~CountingSimulationControlProbe() override {
        ++g_countingControlProbeDestructorCount;
    }

    std::string getValue() const override {
        return "fixed-value";
    }

    void setValue(std::string value, bool remove = false) override {
        (void)value;
        (void)remove;
    }
};

// Exposes protected lifecycle APIs to build focused ownership/teardown tests.
class LifecycleModelDataDefinitionProbe : public ModelDataDefinition {
public:
    LifecycleModelDataDefinitionProbe(Model* model, const std::string& name)
        : ModelDataDefinition(model, "LifecycleModelDataDefinitionProbe", name, true) {}

    void AttachInternalData(const std::string& key, ModelDataDefinition* child) {
        _internalDataInsert(key, child);
    }

    void AttachOwnedProperty(SimulationControl* property) {
        _addProperty(property);
    }

    void AttachData(const std::string& key, ModelDataDefinition* data) {
        _attachedDataInsert(key, data);
    }
};

// Tracks owned-internal-data deletion triggered by owner destruction.
class CountingChildDataDefinitionProbe : public ModelDataDefinition {
public:
    CountingChildDataDefinitionProbe(Model* model, const std::string& name)
        : ModelDataDefinition(model, "CountingChildDataDefinitionProbe", name, true) {}

    ~CountingChildDataDefinitionProbe() override {
        ++g_countingChildProbeDestructorCount;
    }
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

    // Captures the runtime control baseline and validates clear() can run repeatedly without breaking infrastructure.
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

    auto* first = new SnapshotDataDefinitionProbe(model, "First");
    auto* second = new SnapshotDataDefinitionProbe(model, "Second");

    // Takes two independent snapshots and verifies local mutations never affect manager state or another snapshot instance.
    auto names1 = model->getDataManager()->getDataDefinitionClassnames();
    auto names2 = model->getDataManager()->getDataDefinitionClassnames();

    const std::string expectedType = "SnapshotDataDefinitionProbe";
    EXPECT_NE(std::find(names1.begin(), names1.end(), expectedType), names1.end());
    EXPECT_NE(std::find(names2.begin(), names2.end(), expectedType), names2.end());

    names1.clear();
    EXPECT_TRUE(names1.empty());
    EXPECT_NE(std::find(names2.begin(), names2.end(), expectedType), names2.end());

    const auto namesFromManager = model->getDataManager()->getDataDefinitionClassnames();
    EXPECT_NE(std::find(namesFromManager.begin(), namesFromManager.end(), expectedType), namesFromManager.end());

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

TEST(SimulatorRuntimeTest, EntityAttributeValuesRoundTripByNameAndIndex) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Entity* entity = model->createEntity("EntityA", true);
    ASSERT_NE(entity, nullptr);

    // Writes both scalar and indexed attribute values, creating missing attributes on demand.
    entity->setAttributeValue("AttrScalar", 42.5, "", true);
    entity->setAttributeValue("AttrIndexed", 7.25, "idx", true);

    EXPECT_DOUBLE_EQ(entity->getAttributeValue("AttrScalar", ""), 42.5);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("AttrIndexed", "idx"), 7.25);

    model->removeEntity(entity);
}

TEST(SimulatorRuntimeTest, RemovingEntityRemovesItFromDataManagerRegistry) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    const unsigned int entitiesBefore = model->getDataManager()->getNumberOfDataDefinitions("Entity");

    Entity* entity = model->createEntity("EntityB", true);
    ASSERT_NE(entity, nullptr);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions("Entity"), entitiesBefore + 1);

    model->removeEntity(entity);
    EXPECT_EQ(model->getDataManager()->getNumberOfDataDefinitions("Entity"), entitiesBefore);
}

TEST(SimulatorRuntimeTest, ModelDataDefinitionDestructorRemovesOwnedPropertyFromModelControls) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    g_countingControlProbeDestructorCount = 0;
    const unsigned int controlsBefore = model->getControls()->size();

    auto* owner = new LifecycleModelDataDefinitionProbe(model, "OwnerLifecycle");
    const unsigned int controlsAfterOwnerConstruction = model->getControls()->size();

    auto* ownedProperty = new CountingSimulationControlProbe(owner->getName(), "OwnedProperty");
    // Registers the owned property in the model controls and owner property list so destructor cleanup is observable.
    model->getControls()->insert(ownedProperty);
    owner->AttachOwnedProperty(ownedProperty);

    EXPECT_EQ(model->getControls()->size(), controlsAfterOwnerConstruction + 1);

    delete owner;

    EXPECT_EQ(model->getControls()->size(), controlsBefore);
    EXPECT_EQ(g_countingControlProbeDestructorCount, 1u);
}

TEST(SimulatorRuntimeTest, ModelDataDefinitionDestructorDeletesOwnedInternalData) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    g_countingChildProbeDestructorCount = 0;

    auto* owner = new LifecycleModelDataDefinitionProbe(model, "OwnerWithChild");
    auto* child = new CountingChildDataDefinitionProbe(model, "OwnedChild");

    // Declares child as owned internal data so owner teardown must delete it.
    owner->AttachInternalData("child", child);

    delete owner;

    EXPECT_EQ(g_countingChildProbeDestructorCount, 1u);
}

TEST(SimulatorRuntimeTest, ModelDataDefinitionDestructorDoesNotDeleteAttachedDataTarget) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* owner = new LifecycleModelDataDefinitionProbe(model, "OwnerAttached");
    auto* attached = new LifecycleModelDataDefinitionProbe(model, "AttachedTarget");

    // Registers a non-owning attachment that must survive owner destruction.
    owner->AttachData("attached", attached);

    delete owner;

    EXPECT_NE(model->getDataManager()->getDataDefinition("LifecycleModelDataDefinitionProbe", "AttachedTarget"), nullptr);

    delete attached;
}
