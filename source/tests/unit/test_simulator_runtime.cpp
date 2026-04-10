#include <gtest/gtest.h>
#include <algorithm>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Entity.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Persistence.h"
#include "plugins/data/Queue.h"
#include "plugins/data/Variable.h"

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
static unsigned int g_countingWaitingProbeDestructorCount = 0;

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

class QueueLifecycleProbe : public Queue {
public:
    QueueLifecycleProbe(Model* model, const std::string& name = "") : Queue(model, name) {}

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }
};

class QueueValidationProbe : public Queue {
public:
    QueueValidationProbe(Model* model, const std::string& name = "") : Queue(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }
};

class VariableLifecycleProbe : public Variable {
public:
    VariableLifecycleProbe(Model* model, const std::string& name = "") : Variable(model, name) {}

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }
};

class FakeModelPersistenceRuntime : public ModelPersistence_if {
public:
    bool save(std::string) override { return false; }
    bool load(std::string) override { return false; }
    bool hasChanged() override { return false; }
    bool getOption(ModelPersistence_if::Options) override { return false; }
    void setOption(ModelPersistence_if::Options, bool) override {}
    std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

class CountingWaitingProbe final : public Waiting {
public:
    CountingWaitingProbe(Entity* entity, double timeStartedWaiting, ModelComponent* thisComponent, unsigned int thisComponentOutputPort = 0)
        : Waiting(entity, timeStartedWaiting, thisComponent, thisComponentOutputPort) {}

    ~CountingWaitingProbe() override {
        ++g_countingWaitingProbeDestructorCount;
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

TEST(SimulatorRuntimeTest, ModelAccessorsExposeStableRuntimeReferencesAndFlags) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EXPECT_EQ(model->getParentSimulator(), &simulator);
    EXPECT_EQ(model->getTracer(), simulator.getTraceManager());
    EXPECT_NE(model->getPersistence(), nullptr);
    EXPECT_EQ(model->getLevel(), 0u);

    const bool initialAutoCreate = model->isAutomaticallyCreatesModelDataDefinitions();
    model->setAutomaticallyCreatesModelDataDefinitions(!initialAutoCreate);
    EXPECT_EQ(model->isAutomaticallyCreatesModelDataDefinitions(), !initialAutoCreate);
}

TEST(SimulatorRuntimeTest, ModelSetTracerRebindsTracerPointer) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    TraceManager alternateTracer(&simulator);
    model->setTracer(&alternateTracer);

    EXPECT_EQ(model->getTracer(), &alternateTracer);
    EXPECT_NE(model->getTracer(), simulator.getTraceManager());
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

TEST(SimulatorRuntimeTest, ModelDataManagerSupportsLookupByNameIdAndRank) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* alpha = new SnapshotDataDefinitionProbe(model, "Alpha");
    auto* beta = new SnapshotDataDefinitionProbe(model, "Beta");
    auto* manager = model->getDataManager();
    const std::string typeName = "SnapshotDataDefinitionProbe";

    EXPECT_EQ(manager->getNumberOfDataDefinitions(typeName), 2u);
    EXPECT_GE(manager->getNumberOfDataDefinitions(), 2u);
    EXPECT_EQ(manager->getDataDefinition(typeName, "Alpha"), alpha);
    EXPECT_EQ(manager->getDataDefinition(typeName, beta->getId()), beta);
    EXPECT_EQ(manager->getRankOf(typeName, "Alpha"), 0);
    EXPECT_EQ(manager->getRankOf(typeName, "Beta"), 1);
    EXPECT_EQ(manager->getRankOf(typeName, "Missing"), -1);

    delete alpha;
    delete beta;
}

TEST(SimulatorRuntimeTest, ModelDataManagerHasChangedCanBeToggledAndRecomputed) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* manager = model->getDataManager();
    auto* data = new SnapshotDataDefinitionProbe(model, "ChangeProbe");
    ASSERT_NE(data, nullptr);

    EXPECT_TRUE(manager->hasChanged());

    manager->setHasChanged(false);
    EXPECT_FALSE(manager->hasChanged());

    data->setName("ChangeProbeRenamed");
    EXPECT_TRUE(manager->hasChanged());

    delete data;
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

TEST(SimulatorRuntimeTest, EntityAttributesCanBeSetAndReadByAttributeId) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Entity* first = model->createEntity("EntityC", true);
    Entity* second = model->createEntity("EntityD", true);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    auto* attribute = new Attribute(model, "Cost");
    ASSERT_NE(attribute, nullptr);
    const Util::identification attributeId = attribute->getId();

    first->setAttributeValue(attributeId, 13.5);
    second->setAttributeValue(attributeId, 21.0, "batch");

    EXPECT_DOUBLE_EQ(first->getAttributeValue(attributeId), 13.5);
    EXPECT_DOUBLE_EQ(second->getAttributeValue(attributeId, "batch"), 21.0);

    model->removeEntity(first);
    model->removeEntity(second);
    delete attribute;
}

TEST(SimulatorRuntimeTest, ModelDataDefinitionAccessorsExposeStableStateAndMetadata) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* probe = new SnapshotDataDefinitionProbe(model, "ProbeMetadata");
    ASSERT_NE(probe, nullptr);

    EXPECT_FALSE(probe->getClassname().empty());
    EXPECT_GT(probe->getId(), 0u);
    EXPECT_EQ(probe->getName(), "ProbeMetadata");
    EXPECT_FALSE(probe->hasChanged());

    probe->setModelLevel(3u);
    EXPECT_EQ(probe->getLevel(), 3u);

    const bool initialReportStatistics = probe->isReportStatistics();
    probe->setReportStatistics(!initialReportStatistics);
    EXPECT_EQ(probe->isReportStatistics(), !initialReportStatistics);
    EXPECT_TRUE(probe->hasChanged());

    delete probe;
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

TEST(SimulatorRuntimeTest, ModelDataDefinitionDestructorDeletesMultipleOwnedInternalDataChildren) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    g_countingChildProbeDestructorCount = 0;

    auto* owner = new LifecycleModelDataDefinitionProbe(model, "OwnerWithMultipleChildren");
    auto* childA = new CountingChildDataDefinitionProbe(model, "OwnedChildA");
    auto* childB = new CountingChildDataDefinitionProbe(model, "OwnedChildB");

    // Declares two owned internal data children so owner teardown must delete both instances.
    owner->AttachInternalData("childA", childA);
    owner->AttachInternalData("childB", childB);

    delete owner;

    EXPECT_EQ(g_countingChildProbeDestructorCount, 2u);
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

TEST(SimulatorRuntimeTest, ModelDataDefinitionDestructorRemovesOwnedPropertyAlsoRegisteredAsResponse) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    g_countingControlProbeDestructorCount = 0;
    const unsigned int controlsBefore = model->getControls()->size();
    const unsigned int responsesBefore = model->getResponses()->size();

    auto* owner = new LifecycleModelDataDefinitionProbe(model, "OwnerControlAndResponse");
    const unsigned int controlsAfterOwnerConstruction = model->getControls()->size();
    const unsigned int responsesAfterOwnerConstruction = model->getResponses()->size();

    auto* ownedProperty = new CountingSimulationControlProbe(owner->getName(), "OwnedControlAndResponse");
    // Registers the same owned property in controls and responses so destructor cleanup must remove both references.
    model->getControls()->insert(ownedProperty);
    model->getResponses()->insert(static_cast<SimulationResponse*>(ownedProperty));
    owner->AttachOwnedProperty(ownedProperty);

    EXPECT_EQ(model->getControls()->size(), controlsAfterOwnerConstruction + 1);
    EXPECT_EQ(model->getResponses()->size(), responsesAfterOwnerConstruction + 1);

    delete owner;

    EXPECT_EQ(model->getControls()->size(), controlsBefore);
    EXPECT_EQ(model->getResponses()->size(), responsesBefore);
    EXPECT_EQ(g_countingControlProbeDestructorCount, 1u);
}

TEST(SimulatorRuntimeTest, QueueFirstOnEmptyQueueReturnsNullptr) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueEmpty");
    queue.setReportStatistics(false);

    EXPECT_EQ(queue.first(), nullptr);
    EXPECT_EQ(queue.size(), 0u);
}

TEST(SimulatorRuntimeTest, QueueRemoveElementDeletesOwnedWaiting) {
    g_countingWaitingProbeDestructorCount = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueRemove");
    queue.setReportStatistics(false);
    Entity* entity = model->createEntity("QueueRemoveEntity", true);
    ASSERT_NE(entity, nullptr);

    auto* waiting = new CountingWaitingProbe(entity, 0.0, nullptr);
    queue.insertElement(waiting);
    queue.removeElement(waiting);

    EXPECT_EQ(queue.size(), 0u);
    EXPECT_EQ(g_countingWaitingProbeDestructorCount, 1u);
}

TEST(SimulatorRuntimeTest, QueueInitBetweenReplicationsDeletesOwnedWaiting) {
    g_countingWaitingProbeDestructorCount = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    QueueLifecycleProbe queue(model, "QueueInit");
    queue.setReportStatistics(false);
    Entity* entityA = model->createEntity("QueueInitEntityA", true);
    Entity* entityB = model->createEntity("QueueInitEntityB", true);
    ASSERT_NE(entityA, nullptr);
    ASSERT_NE(entityB, nullptr);

    queue.insertElement(new CountingWaitingProbe(entityA, 0.0, nullptr));
    queue.insertElement(new CountingWaitingProbe(entityB, 0.0, nullptr));
    queue.InitBetweenReplicationsProbe();

    EXPECT_EQ(queue.size(), 0u);
    EXPECT_EQ(g_countingWaitingProbeDestructorCount, 2u);
}

TEST(SimulatorRuntimeTest, QueueDestructorDeletesRemainingOwnedWaiting) {
    g_countingWaitingProbeDestructorCount = 0;

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Entity* entityA = model->createEntity("QueueDestructorEntityA", true);
    Entity* entityB = model->createEntity("QueueDestructorEntityB", true);
    ASSERT_NE(entityA, nullptr);
    ASSERT_NE(entityB, nullptr);

    auto* queue = new Queue(model, "QueueDestructor");
    queue->setReportStatistics(false);
    queue->insertElement(new CountingWaitingProbe(entityA, 0.0, nullptr));
    queue->insertElement(new CountingWaitingProbe(entityB, 0.0, nullptr));
    delete queue;

    EXPECT_EQ(g_countingWaitingProbeDestructorCount, 2u);
}

TEST(SimulatorRuntimeTest, QueueOrderRuleFifoKeepsArrivalOrder) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueFIFO");
    queue.setReportStatistics(false);
    queue.setOrderRule(Queue::OrderRule::FIFO);

    Entity* firstEntity = model->createEntity("QueueFIFOEntityA", true);
    Entity* secondEntity = model->createEntity("QueueFIFOEntityB", true);
    Entity* thirdEntity = model->createEntity("QueueFIFOEntityC", true);
    ASSERT_NE(firstEntity, nullptr);
    ASSERT_NE(secondEntity, nullptr);
    ASSERT_NE(thirdEntity, nullptr);

    queue.insertElement(new Waiting(firstEntity, 0.0, nullptr));
    queue.insertElement(new Waiting(secondEntity, 0.0, nullptr));
    queue.insertElement(new Waiting(thirdEntity, 0.0, nullptr));

    ASSERT_NE(queue.getAtRank(0), nullptr);
    ASSERT_NE(queue.getAtRank(1), nullptr);
    ASSERT_NE(queue.getAtRank(2), nullptr);
    EXPECT_EQ(queue.getAtRank(0)->getEntity(), firstEntity);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), secondEntity);
    EXPECT_EQ(queue.getAtRank(2)->getEntity(), thirdEntity);
}

TEST(SimulatorRuntimeTest, QueueOrderRuleLifoPlacesLatestArrivalFirst) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "QueueLIFO");
    queue.setReportStatistics(false);
    queue.setOrderRule(Queue::OrderRule::LIFO);

    Entity* firstEntity = model->createEntity("QueueLIFOEntityA", true);
    Entity* secondEntity = model->createEntity("QueueLIFOEntityB", true);
    Entity* thirdEntity = model->createEntity("QueueLIFOEntityC", true);
    ASSERT_NE(firstEntity, nullptr);
    ASSERT_NE(secondEntity, nullptr);
    ASSERT_NE(thirdEntity, nullptr);

    queue.insertElement(new Waiting(firstEntity, 0.0, nullptr));
    queue.insertElement(new Waiting(secondEntity, 0.0, nullptr));
    queue.insertElement(new Waiting(thirdEntity, 0.0, nullptr));

    ASSERT_NE(queue.first(), nullptr);
    EXPECT_EQ(queue.first()->getEntity(), thirdEntity);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), secondEntity);
    EXPECT_EQ(queue.getAtRank(2)->getEntity(), firstEntity);
}

TEST(SimulatorRuntimeTest, QueueOrderRuleHighestValueRanksByAttributeDescending) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* priority = new Attribute(model, "Priority");
    ASSERT_NE(priority, nullptr);
    const Util::identification priorityId = priority->getId();

    Queue queue(model, "QueueHighestValue");
    queue.setReportStatistics(false);
    queue.setAttributeName(priority->getName());
    queue.setOrderRule(Queue::OrderRule::HIGHESTVALUE);

    Entity* low = model->createEntity("QueueHighestLow", true);
    Entity* high = model->createEntity("QueueHighestHigh", true);
    Entity* mid = model->createEntity("QueueHighestMid", true);
    ASSERT_NE(low, nullptr);
    ASSERT_NE(high, nullptr);
    ASSERT_NE(mid, nullptr);
    low->setAttributeValue(priorityId, 1.0);
    high->setAttributeValue(priorityId, 9.0);
    mid->setAttributeValue(priorityId, 5.0);

    queue.insertElement(new Waiting(low, 0.0, nullptr));
    queue.insertElement(new Waiting(high, 0.0, nullptr));
    queue.insertElement(new Waiting(mid, 0.0, nullptr));

    EXPECT_EQ(queue.getAtRank(0)->getEntity(), high);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), mid);
    EXPECT_EQ(queue.getAtRank(2)->getEntity(), low);

    delete priority;
}

TEST(SimulatorRuntimeTest, QueueOrderRuleSmallestValueRanksByAttributeAscending) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* priority = new Attribute(model, "PrioritySmallest");
    ASSERT_NE(priority, nullptr);
    const Util::identification priorityId = priority->getId();

    Queue queue(model, "QueueSmallestValue");
    queue.setReportStatistics(false);
    queue.setAttributeName(priority->getName());
    queue.setOrderRule(Queue::OrderRule::SMALLESTVALUE);

    Entity* high = model->createEntity("QueueSmallestHigh", true);
    Entity* low = model->createEntity("QueueSmallestLow", true);
    Entity* mid = model->createEntity("QueueSmallestMid", true);
    ASSERT_NE(high, nullptr);
    ASSERT_NE(low, nullptr);
    ASSERT_NE(mid, nullptr);
    high->setAttributeValue(priorityId, 9.0);
    low->setAttributeValue(priorityId, 1.0);
    mid->setAttributeValue(priorityId, 5.0);

    queue.insertElement(new Waiting(high, 0.0, nullptr));
    queue.insertElement(new Waiting(low, 0.0, nullptr));
    queue.insertElement(new Waiting(mid, 0.0, nullptr));

    EXPECT_EQ(queue.getAtRank(0)->getEntity(), low);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), mid);
    EXPECT_EQ(queue.getAtRank(2)->getEntity(), high);

    delete priority;
}

TEST(SimulatorRuntimeTest, QueueOrderRuleAttributeTieUsesFifoTiebreaker) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* priority = new Attribute(model, "PriorityTie");
    ASSERT_NE(priority, nullptr);
    const Util::identification priorityId = priority->getId();

    Queue queue(model, "QueueTieFIFO");
    queue.setReportStatistics(false);
    queue.setAttributeName(priority->getName());
    queue.setOrderRule(Queue::OrderRule::HIGHESTVALUE);

    Entity* firstTie = model->createEntity("QueueTieFirst", true);
    Entity* secondTie = model->createEntity("QueueTieSecond", true);
    Entity* highest = model->createEntity("QueueTieHighest", true);
    ASSERT_NE(firstTie, nullptr);
    ASSERT_NE(secondTie, nullptr);
    ASSERT_NE(highest, nullptr);
    firstTie->setAttributeValue(priorityId, 5.0);
    secondTie->setAttributeValue(priorityId, 5.0);
    highest->setAttributeValue(priorityId, 7.0);

    queue.insertElement(new Waiting(firstTie, 0.0, nullptr));
    queue.insertElement(new Waiting(secondTie, 0.0, nullptr));
    queue.insertElement(new Waiting(highest, 0.0, nullptr));

    EXPECT_EQ(queue.getAtRank(0)->getEntity(), highest);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), firstTie);
    EXPECT_EQ(queue.getAtRank(2)->getEntity(), secondTie);

    delete priority;
}

TEST(SimulatorRuntimeTest, QueueCheckFailsWhenAttributeRuleHasNoAttributeName) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    QueueValidationProbe queue(model, "QueueCheckMissingAttr");
    queue.setOrderRule(Queue::OrderRule::HIGHESTVALUE);

    std::string errorMessage;
    EXPECT_FALSE(queue.CheckProbe(errorMessage));
    EXPECT_FALSE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, QueueCheckPassesWhenAttributeRuleHasValidAttributeName) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* priority = new Attribute(model, "PriorityCheck");
    ASSERT_NE(priority, nullptr);

    QueueValidationProbe queue(model, "QueueCheckValidAttr");
    queue.setOrderRule(Queue::OrderRule::SMALLESTVALUE);
    queue.setAttributeName(priority->getName());

    std::string errorMessage;
    EXPECT_TRUE(queue.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());

    delete priority;
}

TEST(SimulatorRuntimeTest, VariableInitBetweenReplicationsCopiesWithoutAliasingInitialValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe variable(model, "VariableResetNoAlias");
    variable.setInitialValue(10.0, "idx");
    variable.InitBetweenReplicationsProbe();

    variable.setValue(77.0, "idx");

    EXPECT_DOUBLE_EQ(variable.getValue("idx"), 77.0);
    EXPECT_DOUBLE_EQ(variable.getInitialValue("idx"), 10.0);
}

TEST(SimulatorRuntimeTest, VariableInitBetweenReplicationsRestoresCurrentValueFromInitial) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe variable(model, "VariableResetRestores");
    variable.setInitialValue(21.5, "slot");
    variable.setValue(3.0, "slot");

    variable.InitBetweenReplicationsProbe();

    EXPECT_DOUBLE_EQ(variable.getValue("slot"), 21.5);
}

TEST(SimulatorRuntimeTest, VariableSavePersistsMultipleDimensionsWithIncreasingIndexes) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe variable(model, "VariablePersistDimensions");
    variable.insertDimentionSize(3u);
    variable.insertDimentionSize(5u);
    variable.insertDimentionSize(7u);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    variable.SaveInstanceProbe(&fields, true);

    EXPECT_EQ(fields.loadField("dimensions", 0u), 3u);
    EXPECT_EQ(fields.loadField("dimension[0]", 0u), 3u);
    EXPECT_EQ(fields.loadField("dimension[1]", 0u), 5u);
    EXPECT_EQ(fields.loadField("dimension[2]", 0u), 7u);
}

TEST(SimulatorRuntimeTest, VariableSaveAndLoadPreservesInitialValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe source(model, "VariablePersistValuesSource");
    source.setInitialValue(4.25, "");
    source.setInitialValue(8.5, "1,1");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    VariableLifecycleProbe loaded(model, "VariablePersistValuesLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

    EXPECT_DOUBLE_EQ(loaded.getInitialValue(""), 4.25);
    EXPECT_DOUBLE_EQ(loaded.getInitialValue("1,1"), 8.5);
}

TEST(SimulatorRuntimeTest, VariableLoadedCurrentAndInitialContainersRemainIndependentAfterReset) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe source(model, "VariableContainerIndependenceSource");
    source.setInitialValue(12.0, "shared");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    VariableLifecycleProbe loaded(model, "VariableContainerIndependenceLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    loaded.InitBetweenReplicationsProbe();

    ASSERT_NE(loaded.getValues(), nullptr);
    EXPECT_DOUBLE_EQ(loaded.getValue("shared"), 12.0);
    loaded.setValue(44.0, "shared");
    EXPECT_DOUBLE_EQ(loaded.getValue("shared"), 44.0);
    EXPECT_DOUBLE_EQ(loaded.getInitialValue("shared"), 12.0);
    loaded.setInitialValue(66.0, "shared");
    EXPECT_DOUBLE_EQ(loaded.getInitialValue("shared"), 66.0);
    EXPECT_DOUBLE_EQ(loaded.getValue("shared"), 44.0);
}

TEST(SimulatorRuntimeTest, VariableShowIncludesVariableSpecificValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    VariableLifecycleProbe variable(model, "VariableShowDetails");
    variable.setValue(3.14, "pi");

    const std::string shown = variable.show();
    EXPECT_NE(shown.find("values:{"), std::string::npos);
    EXPECT_NE(shown.find("pi="), std::string::npos);
}
