#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Entity.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Event.h"
#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Persistence.h"
#include "plugins/data/Queue.h"
#include "plugins/data/Variable.h"
#include "plugins/data/Resource.h"
#include "plugins/data/Failure.h"
#include "plugins/data/Schedule.h"
#include "plugins/data/Sequence.h"
#include "plugins/data/SignalData.h"
#include "plugins/components/Delay.h"
#include "plugins/components/Wait.h"

class DelayProbe : public Delay {
public:
    DelayProbe(Model* model, const std::string& name = "") : Delay(model, name) {}

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    StatisticsCollector* WaitTimeStatisticsCollectorProbe() const {
        return _cstatWaitTime;
    }

    void AddWaitTimeValueProbe(double waitTime) {
        _cstatWaitTime->getStatistics()->getCollector()->addValue(waitTime);
    }

    bool HasAttachedDataProbe(const std::string& key) const {
        return getAttachedData()->find(key) != getAttachedData()->end();
    }
};

class ResourceTestProbe {
public:
    static bool HasStatisticsInternals(const Resource& resource) {
        return resource._cstatTimeSeized != nullptr &&
               resource._cstatTimeFailed != nullptr &&
               resource._cstatProportionSeized != nullptr &&
               resource._cstatCapacityUtilization != nullptr &&
               resource._counterTotalTimeSeized != nullptr &&
               resource._counterTotalTimeFailed != nullptr &&
               resource._counterNumSeizes != nullptr &&
               resource._counterNumReleases != nullptr &&
               resource._counterTotalCostPerUse != nullptr &&
               resource._counterTotalCostBusy != nullptr &&
               resource._counterTotalCostIdle != nullptr;
    }

    static bool HasFailure(const Resource& resource, const Failure* failure) {
        return std::find(resource._failures->list()->begin(), resource._failures->list()->end(), failure) != resource._failures->list()->end();
    }

    static size_t FailureCount(const Resource& resource) {
        return resource._failures->size();
    }
};

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
static unsigned int g_countingSequenceStepProbeDestructorCount = 0;

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

class ResourceProbe : public Resource {
public:
    ResourceProbe(Model* model, const std::string& name = "") : Resource(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }
};

class SequenceProbe : public Sequence {
public:
    SequenceProbe(Model* model, const std::string& name = "") : Sequence(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void AttachDataProbe(const std::string& key, ModelDataDefinition* data) {
        _attachedDataInsert(key, data);
    }
};

class SignalDataProbe : public SignalData {
public:
    SignalDataProbe(Model* model, const std::string& name = "") : SignalData(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }
};

class WaitProbe : public Wait {
public:
    WaitProbe(Model* model, const std::string& name = "") : Wait(model, name) {}

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }
};

class FailureProbe : public Failure {
public:
    FailureProbe(Model* model, const std::string& name = "") : Failure(model, name) {}

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }
};

class ScheduleProbe : public Schedule {
public:
    ScheduleProbe(Model* model, const std::string& name = "") : Schedule(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void InternalEventNoopProbe(void* parameter) {
        (void) parameter;
    }
};

struct ReplicationStartEventInjector {
    Model* model = nullptr;
    ScheduleProbe* owner = nullptr;
    bool inserted = false;
    double eventTime = 0.0;
    std::string description;

    void OnReplicationStart(SimulationEvent*) {
        if (inserted || model == nullptr || owner == nullptr) {
            return;
        }
        auto* event = new InternalEvent(eventTime, description);
        event->setEventHandler(owner, &ScheduleProbe::InternalEventNoopProbe, nullptr);
        model->getFutureEvents()->insert(event);
        inserted = true;
    }
};

class CountingSequenceStepProbe : public SequenceStep {
public:
    CountingSequenceStepProbe(Station* station, std::list<Assignment*>* assignments = nullptr)
        : SequenceStep(station, assignments) {}

    ~CountingSequenceStepProbe() override {
        ++g_countingSequenceStepProbeDestructorCount;
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

std::vector<std::string> DelayAllocationAttributeNames() {
    return {
        "Entity.TotalValueAddedTime",
        "Entity.TotalNonValueAddedTime",
        "Entity.TotalTransferTime",
        "Entity.TotalWaitTime",
        "Entity.TotalOthersTime"
    };
}

std::string DelayAllocationAttributeName(Util::AllocationType allocation) {
    return "Entity.Total" + Util::StrAllocation(allocation) + "Time";
}

size_t CountAttachedDelayAllocationAttributes(const DelayProbe& delay) {
    size_t count = 0;
    for (const std::string& attributeName : DelayAllocationAttributeNames()) {
        if (delay.HasAttachedDataProbe(attributeName)) {
            ++count;
        }
    }
    return count;
}
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

TEST(SimulatorRuntimeTest, ScheduleDestructorDeletesOwnedSchedulableItemsSafely) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* schedule = new ScheduleProbe(model, "ScheduleLifecycle");
    schedule->getSchedulableItems()->insert(new SchedulableItem("1", 1.0));
    schedule->getSchedulableItems()->insert(new SchedulableItem("2", 2.0));

    EXPECT_NO_THROW(delete schedule);
}

TEST(SimulatorRuntimeTest, ScheduleLoadInstanceReplacesSchedulableItemsWithoutKeepingOldPointers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ScheduleProbe firstPersisted(model, "ScheduleReloadFirst");
    firstPersisted.setRepeatAfterLast(false);
    firstPersisted.getSchedulableItems()->insert(new SchedulableItem("stale", 4.0));
    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fieldsFirst(persistence);
    firstPersisted.SaveInstanceProbe(&fieldsFirst, true);

    ScheduleProbe secondPersisted(model, "ScheduleReloadSecond");
    secondPersisted.setRepeatAfterLast(false);
    secondPersisted.getSchedulableItems()->insert(new SchedulableItem("11", 3.0));
    secondPersisted.getSchedulableItems()->insert(new SchedulableItem("22", 5.0, SchedulableItem::Rule::WAIT));
    PersistenceRecord fieldsSecond(persistence);
    secondPersisted.SaveInstanceProbe(&fieldsSecond, true);

    ScheduleProbe schedule(model, "ScheduleReloadTarget");
    ASSERT_TRUE(schedule.LoadInstanceProbe(&fieldsFirst));
    ASSERT_EQ(schedule.getSchedulableItems()->size(), 1u);
    ASSERT_EQ(schedule.getSchedulableItems()->getAtRank(0)->getExpression(), "stale");

    ASSERT_TRUE(schedule.LoadInstanceProbe(&fieldsSecond));
    ASSERT_EQ(schedule.getSchedulableItems()->size(), 2u);
    EXPECT_EQ(schedule.getSchedulableItems()->getAtRank(0)->getExpression(), "11");
    EXPECT_DOUBLE_EQ(schedule.getSchedulableItems()->getAtRank(0)->getDuration(), 3.0);
    EXPECT_EQ(schedule.getSchedulableItems()->getAtRank(1)->getExpression(), "22");
    EXPECT_DOUBLE_EQ(schedule.getSchedulableItems()->getAtRank(1)->getDuration(), 5.0);
}

TEST(SimulatorRuntimeTest, ScheduleGetExpressionReturnsSafeFallbackForEmptyList) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Schedule schedule(model, "ScheduleEmptyExpression");
    EXPECT_EQ(schedule.getExpression(), "");
}

TEST(SimulatorRuntimeTest, ScheduleGetExpressionHandlesNonRepeatingAndReturnsLastAfterEnd) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ScheduleProbe schedule(model, "ScheduleNonRepeatExpression");
    schedule.setRepeatAfterLast(false);
    schedule.getSchedulableItems()->insert(new SchedulableItem("1", 2.0));
    schedule.getSchedulableItems()->insert(new SchedulableItem("2", 3.0));

    EXPECT_EQ(schedule.getExpression(), "1");

    ReplicationStartEventInjector injector{model, &schedule, false, 10.0, "AdvanceTimeForSchedule"};
    model->getOnEventManager()->addOnReplicationStartHandler(&injector, &ReplicationStartEventInjector::OnReplicationStart);
    model->getSimulation()->setReplicationLength(20.0);
    model->getSimulation()->start();

    EXPECT_DOUBLE_EQ(model->getSimulation()->getSimulatedTime(), 10.0);
    EXPECT_EQ(schedule.getExpression(), "2");
}

TEST(SimulatorRuntimeTest, ScheduleGetExpressionRepeatsWithPositiveCycleDurations) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ScheduleProbe schedule(model, "ScheduleRepeatExpression");
    schedule.setRepeatAfterLast(true);
    schedule.getSchedulableItems()->insert(new SchedulableItem("1", 2.0));
    schedule.getSchedulableItems()->insert(new SchedulableItem("2", 3.0));

    EXPECT_EQ(schedule.getExpression(), "1");

    ReplicationStartEventInjector injector{model, &schedule, false, 8.0, "AdvanceTimeForScheduleRepeat"};
    model->getOnEventManager()->addOnReplicationStartHandler(&injector, &ReplicationStartEventInjector::OnReplicationStart);
    model->getSimulation()->setReplicationLength(20.0);
    model->getSimulation()->start();

    EXPECT_DOUBLE_EQ(model->getSimulation()->getSimulatedTime(), 8.0);
    EXPECT_EQ(schedule.getExpression(), "2");
}

TEST(SimulatorRuntimeTest, ScheduleCheckFailsForRepeatingCyclesWithZeroTotalDuration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ScheduleProbe schedule(model, "ScheduleInvalidZeroCycle");
    schedule.setRepeatAfterLast(true);
    schedule.getSchedulableItems()->insert(new SchedulableItem("1", 0.0));
    schedule.getSchedulableItems()->insert(new SchedulableItem("2", 0.0));

    std::string errorMessage;
    EXPECT_FALSE(schedule.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("duration > 0"), std::string::npos);
}

TEST(SimulatorRuntimeTest, ScheduleCheckPassesForValidRepeatingConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ScheduleProbe schedule(model, "ScheduleValidCheck");
    schedule.setRepeatAfterLast(true);
    schedule.getSchedulableItems()->insert(new SchedulableItem("1", 1.0));
    schedule.getSchedulableItems()->insert(new SchedulableItem("2", 0.0));

    std::string errorMessage;
    EXPECT_TRUE(schedule.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
    EXPECT_NE(schedule.show().find("items=2"), std::string::npos);
    EXPECT_NE(schedule.show().find("repeatAfterLast=true"), std::string::npos);
    EXPECT_NO_THROW(schedule.CreateInternalAndAttachedDataProbe());
}

TEST(SimulatorRuntimeTest, ResourceSettersUpdateState) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ResourceProbe resource(model, "ResourceSetterCheck");
    Schedule schedule(model, "ResourceSetterSchedule");

    resource.setResourceState(Resource::ResourceState::FAILED);
    resource.setCostBusyTimeUnit(11.25);
    resource.setCostIdleTimeUnit(7.5);
    resource.setCostPerUse(3.5);
    resource.setCapacitySchedule(&schedule);

    EXPECT_EQ(resource.getResourceState(), Resource::ResourceState::FAILED);
    EXPECT_DOUBLE_EQ(resource.getCostBusyTimeUnit(), 11.25);
    EXPECT_DOUBLE_EQ(resource.getCostIdleTimeUnit(), 7.5);
    EXPECT_DOUBLE_EQ(resource.getCostPerUse(), 3.5);
    EXPECT_EQ(resource.getCapacitySchedule(), &schedule);
}

TEST(SimulatorRuntimeTest, ResourceCheckFailsForInvalidConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ResourceProbe capacityZero(model, "ResourceCheckCapacityZero");
    capacityZero.setCapacity(0u);
    std::string errorMessage;
    EXPECT_FALSE(capacityZero.CheckProbe(errorMessage));

    ResourceProbe negativeCost(model, "ResourceCheckNegativeCost");
    negativeCost.setCostPerUse(-1.0);
    errorMessage.clear();
    EXPECT_FALSE(negativeCost.CheckProbe(errorMessage));

    Schedule invalidSchedule(model, "ResourceCheckInvalidSchedule");
    ResourceProbe invalidScheduleResource(model, "ResourceCheckWithInvalidSchedule");
    invalidScheduleResource.setCapacitySchedule(&invalidSchedule);
    errorMessage.clear();
    EXPECT_FALSE(invalidScheduleResource.CheckProbe(errorMessage));

    Failure invalidFailure(model, "ResourceCheckInvalidFailure");
    invalidFailure.setFailureType(Failure::FailureType::COUNT);
    invalidFailure.setCountExpression(")");
    ResourceProbe invalidFailureResource(model, "ResourceCheckWithInvalidFailure");
    invalidFailureResource.insertFailure(&invalidFailure);
    errorMessage.clear();
    EXPECT_FALSE(invalidFailureResource.CheckProbe(errorMessage));
}

TEST(SimulatorRuntimeTest, ResourceCheckPassesForValidConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Schedule schedule(model, "ResourceCheckValidSchedule");
    schedule.getSchedulableItems()->insert(new SchedulableItem("2", 10.0));

    Failure failure(model, "ResourceCheckValidFailure");
    failure.setFailureType(Failure::FailureType::COUNT);
    failure.setCountExpression("3");
    failure.setDownTimeExpression("1");

    ResourceProbe resource(model, "ResourceCheckValid");
    resource.setCapacity(2u);
    resource.setCostBusyTimeUnit(1.0);
    resource.setCostIdleTimeUnit(0.0);
    resource.setCostPerUse(0.5);
    resource.setCapacitySchedule(&schedule);
    resource.insertFailure(&failure);

    std::string errorMessage;
    EXPECT_TRUE(resource.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, ResourceSaveAndLoadPreservesCapacityScheduleReference) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Schedule schedule(model, "ResourcePersistSchedule");
    ResourceProbe source(model, "ResourcePersistScheduleSource");
    source.setCapacitySchedule(&schedule);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    ResourceProbe loaded(model, "ResourcePersistScheduleLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

    EXPECT_EQ(loaded.getCapacitySchedule(), &schedule);
}

TEST(SimulatorRuntimeTest, ResourceSaveAndLoadPreservesFailuresReferenceList) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Failure failureA(model, "ResourcePersistFailureA");
    Failure failureB(model, "ResourcePersistFailureB");
    ResourceProbe source(model, "ResourcePersistFailureSource");
    source.insertFailure(&failureA);
    source.insertFailure(&failureB);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    ResourceProbe loaded(model, "ResourcePersistFailureLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    loaded.CreateInternalAndAttachedDataProbe();

    auto* attached = loaded.getAttachedData();
    const std::string keyPrefix = loaded.getName() + ".Failure.";
    ASSERT_EQ(attached->count(keyPrefix + "ResourcePersistFailureA"), 1u);
    ASSERT_EQ(attached->count(keyPrefix + "ResourcePersistFailureB"), 1u);
    EXPECT_EQ(attached->at(keyPrefix + "ResourcePersistFailureA"), &failureA);
    EXPECT_EQ(attached->at(keyPrefix + "ResourcePersistFailureB"), &failureB);
}

TEST(SimulatorRuntimeTest, ResourceRecheckKeepsAttachedDataConsistent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Schedule scheduleA(model, "ResourceRecheckScheduleA");
    Schedule scheduleB(model, "ResourceRecheckScheduleB");
    Failure failureA(model, "ResourceRecheckFailureA");
    Failure failureB(model, "ResourceRecheckFailureB");

    ResourceProbe resource(model, "ResourceRecheckAttached");
    resource.setCapacitySchedule(&scheduleA);
    resource.insertFailure(&failureA);
    resource.CreateInternalAndAttachedDataProbe();

    auto* attached = resource.getAttachedData();
    ASSERT_EQ(attached->at("ResourceRecheckAttached.CapacitySchedule"), &scheduleA);
    ASSERT_EQ(attached->at("ResourceRecheckAttached.Failure.ResourceRecheckFailureA"), &failureA);

    resource.removeFailure(&failureA);
    resource.insertFailure(&failureB);
    resource.setCapacitySchedule(&scheduleB);
    resource.CreateInternalAndAttachedDataProbe();

    EXPECT_EQ(attached->at("ResourceRecheckAttached.CapacitySchedule"), &scheduleB);
    EXPECT_EQ(attached->at("ResourceRecheckAttached.Failure.ResourceRecheckFailureB"), &failureB);
    EXPECT_EQ(attached->count("ResourceRecheckAttached.Failure.ResourceRecheckFailureA"), 0u);

    resource.setCapacitySchedule(nullptr);
    resource.removeFailure(&failureB);
    resource.CreateInternalAndAttachedDataProbe();

    EXPECT_EQ(attached->count("ResourceRecheckAttached.CapacitySchedule"), 0u);
    EXPECT_EQ(attached->count("ResourceRecheckAttached.Failure.ResourceRecheckFailureB"), 0u);
}

TEST(SimulatorRuntimeTest, ResourceToggleReportStatisticsClearsAndRecreatesInternalPointersSafely) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ResourceProbe resource(model, "ResourceToggleStats");
    resource.setReportStatistics(true);
    resource.CreateInternalAndAttachedDataProbe();
    EXPECT_TRUE(ResourceTestProbe::HasStatisticsInternals(resource));

    resource.setReportStatistics(false);
    resource.CreateInternalAndAttachedDataProbe();
    EXPECT_FALSE(ResourceTestProbe::HasStatisticsInternals(resource));

    resource.setReportStatistics(true);
    resource.CreateInternalAndAttachedDataProbe();
    EXPECT_TRUE(ResourceTestProbe::HasStatisticsInternals(resource));
}

TEST(SimulatorRuntimeTest, ResourceDestructorCleansOwnedHandlersContainers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    std::weak_ptr<int> weakCapture;
    {
        auto* resource = new ResourceProbe(model, "ResourceDestructorLifecycle");
        auto capture = std::make_shared<int>(42);
        weakCapture = capture;

        Resource::ResourceEventHandler handler = [capture](Resource*) {
            (void)capture;
        };
        resource->addReleaseResourceEventHandler(handler, nullptr, 1u);
        ASSERT_FALSE(weakCapture.expired());
        delete resource;
    }

    EXPECT_TRUE(weakCapture.expired());
}

TEST(SimulatorRuntimeTest, FailureDestructorReleasesOwnedContainersAndDetachesResources) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ResourceProbe resourceA(model, "FailureLifecycleResourceA");
    ResourceProbe resourceB(model, "FailureLifecycleResourceB");

    auto* failure = new FailureProbe(model, "FailureLifecycle");
    failure->addResource(&resourceA);
    failure->addResource(&resourceB);

    ASSERT_TRUE(ResourceTestProbe::HasFailure(resourceA, failure));
    ASSERT_TRUE(ResourceTestProbe::HasFailure(resourceB, failure));
    ASSERT_EQ(ResourceTestProbe::FailureCount(resourceA), 1u);
    ASSERT_EQ(ResourceTestProbe::FailureCount(resourceB), 1u);

    delete failure;

    EXPECT_EQ(ResourceTestProbe::FailureCount(resourceA), 0u);
    EXPECT_EQ(ResourceTestProbe::FailureCount(resourceB), 0u);
}

TEST(SimulatorRuntimeTest, FailureAddResourceMaintainsBidirectionalAssociationWithoutDuplicates) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    FailureProbe failure(model, "FailureBidirectionalAdd");
    ResourceProbe resource(model, "FailureBidirectionalAddResource");

    failure.addResource(&resource);
    failure.addResource(&resource); // duplicate attempt

    EXPECT_EQ(failure.falingResources()->size(), 1u);
    EXPECT_EQ(failure.falingResources()->getAtRank(0), &resource);
    EXPECT_TRUE(ResourceTestProbe::HasFailure(resource, &failure));
    EXPECT_EQ(ResourceTestProbe::FailureCount(resource), 1u);
}

TEST(SimulatorRuntimeTest, FailureRemoveResourceMaintainsBidirectionalAssociation) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    FailureProbe failure(model, "FailureBidirectionalRemove");
    ResourceProbe resource(model, "FailureBidirectionalRemoveResource");
    failure.addResource(&resource);
    ASSERT_EQ(failure.falingResources()->size(), 1u);
    ASSERT_TRUE(ResourceTestProbe::HasFailure(resource, &failure));

    failure.removeResource(&resource);

    EXPECT_EQ(failure.falingResources()->size(), 0u);
    EXPECT_EQ(ResourceTestProbe::FailureCount(resource), 0u);
    EXPECT_FALSE(ResourceTestProbe::HasFailure(resource, &failure));
}

TEST(SimulatorRuntimeTest, FailureSaveAndLoadPreservesFalingResourcesBidirectionally) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ResourceProbe resourceA(model, "FailurePersistResourceA");
    ResourceProbe resourceB(model, "FailurePersistResourceB");
    FailureProbe source(model, "FailurePersistSource");
    source.addResource(&resourceA);
    source.addResource(&resourceB);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    FailureProbe loaded(model, "FailurePersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    ASSERT_EQ(loaded.falingResources()->size(), 2u);
    EXPECT_TRUE(std::find(loaded.falingResources()->list()->begin(), loaded.falingResources()->list()->end(), &resourceA) != loaded.falingResources()->list()->end());
    EXPECT_TRUE(std::find(loaded.falingResources()->list()->begin(), loaded.falingResources()->list()->end(), &resourceB) != loaded.falingResources()->list()->end());
    EXPECT_TRUE(ResourceTestProbe::HasFailure(resourceA, &loaded));
    EXPECT_TRUE(ResourceTestProbe::HasFailure(resourceB, &loaded));
}

TEST(SimulatorRuntimeTest, FailureInitBetweenReplicationsSchedulesByFalingResourcesList) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    FailureProbe failure(model, "FailureInitSchedule");
    ResourceProbe resourceA(model, "FailureInitResourceA");
    ResourceProbe resourceB(model, "FailureInitResourceB");
    failure.setFailureType(Failure::FailureType::TIME);
    failure.setUpTimeExpression("1");
    failure.setDownTimeExpression("1");
    failure.addResource(&resourceA);
    failure.addResource(&resourceB);

    const unsigned int eventsBefore = model->getFutureEvents()->size();
    failure.InitBetweenReplicationsProbe();
    const unsigned int eventsAfter = model->getFutureEvents()->size();

    EXPECT_EQ(eventsAfter - eventsBefore, 2u);
}

TEST(SimulatorRuntimeTest, SequenceDestructorDeletesOwnedSteps) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    g_countingSequenceStepProbeDestructorCount = 0;
    Station station(model, "SequenceLifecycleStation");
    auto* sequence = new SequenceProbe(model, "SequenceLifecycle");
    sequence->getSteps()->insert(new CountingSequenceStepProbe(&station));
    sequence->getSteps()->insert(new CountingSequenceStepProbe(&station));

    delete sequence;
    EXPECT_EQ(g_countingSequenceStepProbeDestructorCount, 2u);
}

TEST(SimulatorRuntimeTest, SequenceStepDestructorOwnsAndDeletesAssignments) {
    std::list<Assignment*>* assignments = new std::list<Assignment*>();
    assignments->push_back(new Assignment("Entity.a", "1", true));
    assignments->push_back(new Assignment("Entity.b", "2", true));
    SequenceStep* step = new SequenceStep(static_cast<Station*>(nullptr), assignments);

    delete step;
    SUCCEED();
}

TEST(SimulatorRuntimeTest, SequenceSaveAndLoadPreservesAssignmentsPerStepWithoutCollision) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station stationA(model, "SequencePersistStationA");
    Station stationB(model, "SequencePersistStationB");
    SequenceProbe source(model, "SequencePersistAssignmentsSource");
    auto* stepA = new SequenceStep(&stationA);
    stepA->getAssignments()->push_back(new Assignment("Entity.stepA", "11", true));
    stepA->getAssignments()->push_back(new Assignment("Entity.stepA2", "12", true));
    auto* stepB = new SequenceStep(&stationB);
    stepB->getAssignments()->push_back(new Assignment("Entity.stepB", "21", true));
    source.getSteps()->insert(stepA);
    source.getSteps()->insert(stepB);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    SequenceProbe loaded(model, "SequencePersistAssignmentsLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    ASSERT_EQ(loaded.getSteps()->size(), 2u);

    auto loadedSteps = loaded.getSteps()->list();
    auto it = loadedSteps->begin();
    SequenceStep* loadedStepA = *it++;
    SequenceStep* loadedStepB = *it++;

    ASSERT_EQ(loadedStepA->getAssignments()->size(), 2u);
    ASSERT_EQ(loadedStepB->getAssignments()->size(), 1u);
    EXPECT_EQ(loadedStepA->getAssignments()->front()->getDestination(), "Entity.stepA");
    EXPECT_EQ(loadedStepA->getAssignments()->back()->getDestination(), "Entity.stepA2");
    EXPECT_EQ(loadedStepB->getAssignments()->front()->getDestination(), "Entity.stepB");
    EXPECT_EQ(loadedStepB->getAssignments()->front()->getExpression(), "21");
}

TEST(SimulatorRuntimeTest, SequenceSaveAndLoadPreservesStationAndLabelPerStep) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station station(model, "SequencePersistStation");
    Label label(model, "SequencePersistLabel");
    SequenceProbe source(model, "SequencePersistRoutingSource");
    source.getSteps()->insert(new SequenceStep(&station));
    source.getSteps()->insert(new SequenceStep(&label));

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    SequenceProbe loaded(model, "SequencePersistRoutingLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    ASSERT_EQ(loaded.getSteps()->size(), 2u);

    auto steps = loaded.getSteps()->list();
    auto it = steps->begin();
    SequenceStep* loadedStationStep = *it++;
    SequenceStep* loadedLabelStep = *it++;
    EXPECT_EQ(loadedStationStep->getStation(), &station);
    EXPECT_EQ(loadedStationStep->getLabel(), nullptr);
    EXPECT_EQ(loadedLabelStep->getStation(), nullptr);
    EXPECT_EQ(loadedLabelStep->getLabel(), &label);
}

TEST(SimulatorRuntimeTest, SequenceRecheckRemovesObsoleteAttachedData) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station stationA(model, "SequenceRecheckStationA");
    Station stationB(model, "SequenceRecheckStationB");
    Label labelA(model, "SequenceRecheckLabelA");
    SequenceProbe sequence(model, "SequenceRecheck");
    sequence.getSteps()->insert(new SequenceStep(&stationA));
    sequence.getSteps()->insert(new SequenceStep(&stationB));
    sequence.CreateInternalAndAttachedDataProbe();

    auto* attached = sequence.getAttachedData();
    EXPECT_EQ(attached->count("StepLabel[0]"), 0u);
    sequence.AttachDataProbe("StepStation[77]", &stationA);
    ASSERT_EQ(attached->count("StepStation[77]"), 1u);

    SequenceStep* obsoleteStep = sequence.getSteps()->list()->back();
    sequence.getSteps()->remove(obsoleteStep);
    delete obsoleteStep;
    SequenceStep* firstStationStep = sequence.getSteps()->front();
    sequence.getSteps()->remove(firstStationStep);
    delete firstStationStep;
    sequence.getSteps()->insert(new SequenceStep(&labelA));
    sequence.CreateInternalAndAttachedDataProbe();

    EXPECT_EQ(attached->count("StepStation[77]"), 0u);
    EXPECT_EQ(attached->count("StepStation[1]"), 0u);
    ASSERT_EQ(attached->count("StepLabel[0]"), 1u);
    EXPECT_EQ(attached->at("StepLabel[0]"), &labelA);
}

TEST(SimulatorRuntimeTest, SequenceCheckFailsForEmptyStep) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SequenceProbe sequence(model, "SequenceInvalid");
    sequence.getSteps()->insert(new SequenceStep(static_cast<Station*>(nullptr)));

    std::string errorMessage;
    EXPECT_FALSE(sequence.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("must reference a Station or a Label"), std::string::npos);
}

TEST(SimulatorRuntimeTest, SequenceCheckPassesForValidSteps) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Station station(model, "SequenceValidStation");
    SequenceProbe sequence(model, "SequenceValid");
    auto* stationStep = new SequenceStep(&station);
    stationStep->getAssignments()->push_back(new Assignment("Entity.valid", "1", true));
    sequence.getSteps()->insert(stationStep);
    Station station2(model, "SequenceValidStation2");
    sequence.getSteps()->insert(new SequenceStep(&station2));

    std::string errorMessage;
    EXPECT_TRUE(sequence.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, SignalDataDestructorHandlesOwnedHandlersLifecycle) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    auto* signal = new SignalDataProbe(model, "SignalLifecycle");
    Wait waitA(model, "SignalLifecycleWaitA");
    Wait waitB(model, "SignalLifecycleWaitB");
    signal->addSignalDataEventHandler([](SignalData*) { return 0u; }, &waitA);
    signal->addSignalDataEventHandler([](SignalData*) { return 0u; }, &waitB);

    EXPECT_TRUE(signal->hasSignalDataEventHandler(&waitA));
    EXPECT_TRUE(signal->hasSignalDataEventHandler(&waitB));
    EXPECT_NO_THROW(delete signal);
}

TEST(SimulatorRuntimeTest, SignalDataInitBetweenReplicationsResetsRemainsToLimit) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signal(model, "SignalReset");
    Wait wait(model, "SignalResetWait");
    signal.addSignalDataEventHandler([](SignalData*) { return 0u; }, &wait);

    signal.generateSignal(0.0, 3);
    signal.decreaseRemainLimit();
    ASSERT_EQ(signal.remainsToLimit(), 2u);

    signal.InitBetweenReplicationsProbe();
    EXPECT_EQ(signal.remainsToLimit(), 0u);
}

TEST(SimulatorRuntimeTest, SignalDataAddHandlerDoesNotDuplicateSameComponent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signal(model, "SignalNoDuplicate");
    Wait wait(model, "SignalNoDuplicateWait");
    signal.addSignalDataEventHandler([](SignalData*) { return 1u; }, &wait);
    signal.addSignalDataEventHandler([](SignalData*) { return 10u; }, &wait);

    EXPECT_EQ(signal.generateSignal(0.0, 10), 1u);
}

TEST(SimulatorRuntimeTest, SignalDataRemoveHandlerByComponentRemovesOwnedRegistration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signal(model, "SignalRemove");
    Wait wait(model, "SignalRemoveWait");
    signal.addSignalDataEventHandler([](SignalData*) { return 2u; }, &wait);
    ASSERT_TRUE(signal.hasSignalDataEventHandler(&wait));

    signal.removeSignalDataEventHandler(&wait);
    EXPECT_FALSE(signal.hasSignalDataEventHandler(&wait));
    EXPECT_EQ(signal.generateSignal(0.0, 10), 0u);
}

TEST(SimulatorRuntimeTest, WaitRecheckUpdatesSignalDataHandlersWhenSignalChangesOrTypeChanges) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signalA(model, "SignalRecheckA");
    SignalDataProbe signalB(model, "SignalRecheckB");
    WaitProbe wait(model, "WaitRecheck");
    wait.setWaitType(Wait::WaitType::WaitForSignal);
    wait.setSignalData(&signalA);
    wait.CreateInternalAndAttachedDataProbe();
    ASSERT_TRUE(signalA.hasSignalDataEventHandler(&wait));

    wait.setSignalData(&signalB);
    wait.CreateInternalAndAttachedDataProbe();
    EXPECT_FALSE(signalA.hasSignalDataEventHandler(&wait));
    EXPECT_TRUE(signalB.hasSignalDataEventHandler(&wait));

    wait.setWaitType(Wait::WaitType::InfiniteHold);
    wait.CreateInternalAndAttachedDataProbe();
    EXPECT_FALSE(signalB.hasSignalDataEventHandler(&wait));
}

TEST(SimulatorRuntimeTest, DelayCreateInternalInitiallyCreatesStatisticsCollectorWhenEnabled) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayCreateStats");
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();

    EXPECT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, DelayAttachedAttributeUsesInitialAllocationWhenStatisticsAreEnabled) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayInitialAllocationAttached");
    delay.setReportStatistics(true);
    delay.setAllocation(Util::AllocationType::Wait);
    delay.CreateInternalAndAttachedDataProbe();

    EXPECT_TRUE(delay.HasAttachedDataProbe("Entity.TotalWaitTime"));
    EXPECT_EQ(CountAttachedDelayAllocationAttributes(delay), 1u);
}

TEST(SimulatorRuntimeTest, DelayRecheckWithAllocationChangeKeepsOnlyCurrentAttachedAttribute) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayAllocationRecheck");
    delay.setReportStatistics(true);
    delay.setAllocation(Util::AllocationType::Wait);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_TRUE(delay.HasAttachedDataProbe("Entity.TotalWaitTime"));

    delay.setAllocation(Util::AllocationType::Transfer);
    delay.CreateInternalAndAttachedDataProbe();
    EXPECT_FALSE(delay.HasAttachedDataProbe("Entity.TotalWaitTime"));
    EXPECT_TRUE(delay.HasAttachedDataProbe("Entity.TotalTransferTime"));
    EXPECT_EQ(CountAttachedDelayAllocationAttributes(delay), 1u);
}

TEST(SimulatorRuntimeTest, DelayRecheckWithStatisticsDisabledRemovesAllAllocationAttachedAttributes) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayDisableStatisticsAttached");
    delay.setReportStatistics(true);
    delay.setAllocation(Util::AllocationType::Transfer);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_TRUE(delay.HasAttachedDataProbe("Entity.TotalTransferTime"));

    delay.setReportStatistics(false);
    delay.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(CountAttachedDelayAllocationAttributes(delay), 0u);
    EXPECT_EQ(delay.WaitTimeStatisticsCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, DelayRecheckReenableStatisticsWithDifferentAllocationCreatesSingleExpectedAttachedAttribute) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayReenableDifferentAllocation");
    delay.setReportStatistics(true);
    delay.setAllocation(Util::AllocationType::Wait);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_TRUE(delay.HasAttachedDataProbe("Entity.TotalWaitTime"));

    delay.setReportStatistics(false);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_EQ(CountAttachedDelayAllocationAttributes(delay), 0u);

    delay.setAllocation(Util::AllocationType::Others);
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    EXPECT_TRUE(delay.HasAttachedDataProbe("Entity.TotalOthersTime"));
    EXPECT_FALSE(delay.HasAttachedDataProbe("Entity.TotalWaitTime"));
    EXPECT_EQ(CountAttachedDelayAllocationAttributes(delay), 1u);
}

TEST(SimulatorRuntimeTest, DelayRecheckKeepsComponentConsistentForDispatchPathsAfterAllocationChanges) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayConsistentDispatchAfterRecheck");
    delay.setReportStatistics(true);
    delay.setAllocation(Util::AllocationType::ValueAdded);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);

    delay.setAllocation(Util::AllocationType::Transfer);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_TRUE(delay.HasAttachedDataProbe(DelayAllocationAttributeName(Util::AllocationType::Transfer)));
    ASSERT_EQ(CountAttachedDelayAllocationAttributes(delay), 1u);
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe()->getStatistics(), nullptr);
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe()->getStatistics()->getCollector(), nullptr);
    EXPECT_NO_THROW(delay.AddWaitTimeValueProbe(2.0));
}

TEST(SimulatorRuntimeTest, DelayRecheckWithStatisticsEnabledIsIdempotentAndPreservesInternalCollector) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayIdempotentStats");
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    StatisticsCollector* firstCollector = delay.WaitTimeStatisticsCollectorProbe();
    ASSERT_NE(firstCollector, nullptr);

    delay.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(delay.WaitTimeStatisticsCollectorProbe(), firstCollector);
    EXPECT_NE(delay.WaitTimeStatisticsCollectorProbe()->getStatistics(), nullptr);
}

TEST(SimulatorRuntimeTest, DelayRecheckWithStatisticsDisabledClearsInternalCollectorPointer) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayDisableStats");
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);

    delay.setReportStatistics(false);
    delay.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(delay.WaitTimeStatisticsCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, DelayRecheckCanRecreateCollectorAfterDisablingAndReenablingStatistics) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayRecreateStats");
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);

    delay.setReportStatistics(false);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_EQ(delay.WaitTimeStatisticsCollectorProbe(), nullptr);

    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    StatisticsCollector* recreatedCollector = delay.WaitTimeStatisticsCollectorProbe();
    ASSERT_NE(recreatedCollector, nullptr);
    EXPECT_NE(recreatedCollector->getStatistics(), nullptr);
}

TEST(SimulatorRuntimeTest, DelayCollectorAccessPathRemainsValidAcrossStatisticsRecheckSequence) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    DelayProbe delay(model, "DelayCollectorPath");
    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    delay.CreateInternalAndAttachedDataProbe();

    delay.setReportStatistics(false);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_EQ(delay.WaitTimeStatisticsCollectorProbe(), nullptr);

    delay.setReportStatistics(true);
    delay.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe(), nullptr);
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe()->getStatistics(), nullptr);
    ASSERT_NE(delay.WaitTimeStatisticsCollectorProbe()->getStatistics()->getCollector(), nullptr);

    EXPECT_NO_THROW(delay.AddWaitTimeValueProbe(1.5));
}

TEST(SimulatorRuntimeTest, SignalDataCheckFailsWithoutHandlers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signal(model, "SignalCheckInvalid");
    std::string errorMessage;
    EXPECT_FALSE(signal.CheckProbe(errorMessage));
    EXPECT_FALSE(errorMessage.empty());
    EXPECT_NE(errorMessage.find("requires at least one event handler"), std::string::npos);
}

TEST(SimulatorRuntimeTest, SignalDataCheckPassesWithValidHandler) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signal(model, "SignalCheckValid");
    Wait wait(model, "SignalCheckWait");
    signal.addSignalDataEventHandler([](SignalData*) { return 0u; }, &wait);

    std::string errorMessage;
    EXPECT_TRUE(signal.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
}
