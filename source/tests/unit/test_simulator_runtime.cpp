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
#include "plugins/data/Station.h"
#include "plugins/data/Set.h"
#include "plugins/data/Label.h"
#include "plugins/data/Storage.h"
#define private public
#define protected public
#include "plugins/data/EntityGroup.h"
#undef protected
#undef private
#include "plugins/components/Delay.h"
#include "plugins/components/Batch.h"
#include "plugins/components/Separate.h"
#include "plugins/components/Match.h"
#include "plugins/components/Search.h"
#include "plugins/components/Remove.h"
#define private public
#define protected public
#include "plugins/components/Process.h"
#undef protected
#undef private
#define private public
#define protected public
#include "plugins/components/Wait.h"
#include "plugins/components/Signal.h"
#undef protected
#undef private

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

class StationTestProbe : public Station {
public:
    StationTestProbe(Model* model, const std::string& name = "") : Station(model, name) {}

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }

    StatisticsCollector* NumberInStationCollectorProbe() const {
        return _cstatNumberInStation;
    }

    StatisticsCollector* TimeInStationCollectorProbe() const {
        return _cstatTimeInStation;
    }

    unsigned int NumberInStationProbe() const {
        return _numberInStation;
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

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void EnqueueEntityProbe(Entity* entity, ModelComponent* sourceComponent, double timeStartedWaiting = 0.0) {
        getQueue()->insertElement(new Waiting(entity, timeStartedWaiting, sourceComponent));
    }

    bool IsScanConditionHandlerRegisteredProbe() const {
        return _isScanConditionHandlerRegistered;
    }

    unsigned int CountReleasesWithCurrentBoundaryProbe(unsigned int queuedEntities, unsigned int globalSignalLimit, unsigned int localWaitLimit) const {
        unsigned int freed = 0;
        while (queuedEntities > 0 && globalSignalLimit > 0 && freed < localWaitLimit) {
            --queuedEntities;
            --globalSignalLimit;
            ++freed;
        }
        return freed;
    }

    unsigned int TriggerSignalHandlerProbe(SignalData* signalData) {
        return _handlerForSignalDataEvent(signalData);
    }

    void TriggerAfterProcessHandlerProbe(SimulationEvent* event) {
        _handlerForAfterProcessEventEvent(event);
    }
};

class SignalProbe : public Signal {
public:
    SignalProbe(Model* model, const std::string& name = "") : Signal(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    SignalData* SignalDataPtrProbe() const {
        return _signalData;
    }
};

class ProcessProbe : public Process {
public:
    ProcessProbe(Model* model, const std::string& name = "") : Process(model, name) {}

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

    Seize* SeizePtrProbe() const { return _seize; }
    Delay* DelayPtrProbe() const { return _delay; }
    Release* ReleasePtrProbe() const { return _release; }
};

class CollectorSinkComponentProbe : public ModelComponent {
public:
    CollectorSinkComponentProbe(Model* model, const std::string& name = "")
        : ModelComponent(model, "CollectorSinkComponentProbe", name) {}

    const std::vector<Entity*>& ReceivedEntities() const {
        return _received;
    }

protected:
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
        (void)inputPortNumber;
        _received.push_back(entity);
    }

    bool _loadInstance(PersistenceRecord* fields) override {
        return ModelComponent::_loadInstance(fields);
    }

    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override {
        ModelComponent::_saveInstance(fields, saveDefaultValues);
    }

private:
    std::vector<Entity*> _received;
};

class BatchProbe : public Batch {
public:
    BatchProbe(Model* model, const std::string& name = "") : Batch(model, name) {}

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void DispatchEventProbe(Entity* entity, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class SeparateProbe : public Separate {
public:
    SeparateProbe(Model* model, const std::string& name = "") : Separate(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void DispatchEventProbe(Entity* entity, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }
};

class EntityGroupProbe : public EntityGroup {
public:
    EntityGroupProbe(Model* model, const std::string& name = "") : EntityGroup(model, name) {}

    void InitBetweenReplicationsProbe() {
        _initBetweenReplications();
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    StatisticsCollector* NumberInGroupCollectorProbe() const {
        return _cstatNumberInGroup;
    }
};

class MatchProbe : public Match {
public:
    MatchProbe(Model* model, const std::string& name = "") : Match(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }

    void DispatchEventProbe(Entity* entity, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }
};

class SearchProbe : public Search {
public:
    SearchProbe(Model* model, const std::string& name = "") : Search(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void DispatchEventProbe(Entity* entity, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }
};

class RemoveProbe : public Remove {
public:
    RemoveProbe(Model* model, const std::string& name = "") : Remove(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void DispatchEventProbe(Entity* entity, unsigned int inputPortNumber = 0) {
        _onDispatchEvent(entity, inputPortNumber);
    }

    void CreateInternalAndAttachedDataProbe() {
        _createInternalAndAttachedData();
    }
};

static void DrainFutureEvents(Model* model) {
    while (!model->getFutureEvents()->empty()) {
        Event* event = model->getFutureEvents()->front();
        model->getFutureEvents()->pop_front();
        ModelComponent::DispatchEvent(event);
        delete event;
    }
}

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

class SetProbe : public Set {
public:
    SetProbe(Model* model, const std::string& name = "") : Set(model, name) {}

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
};

class LabelProbe : public Label {
public:
    LabelProbe(Model* model, const std::string& name = "") : Label(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
    }
};

class StorageProbe : public Storage {
public:
    StorageProbe(Model* model, const std::string& name = "") : Storage(model, name) {}

    bool CheckProbe(std::string& errorMessage) {
        return _check(errorMessage);
    }

    bool LoadInstanceProbe(PersistenceRecord* fields) {
        return _loadInstance(fields);
    }

    void SaveInstanceProbe(PersistenceRecord* fields, bool saveDefaultValues = false) {
        _saveInstance(fields, saveDefaultValues);
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

TEST(SimulatorRuntimeTest, LabelCheckFailsWithoutEnteringComponent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    LabelProbe label(model, "LabelCheckMissingDestination");
    std::string errorMessage;
    EXPECT_FALSE(label.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("entering component was not defined"), std::string::npos);
    EXPECT_EQ(label.getAttachedData()->count("EnteringLabelComponent"), 0u);
}

TEST(SimulatorRuntimeTest, LabelCheckPassesWithValidEnteringComponentAndAttachesIt) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    LabelProbe label(model, "LabelCheckValidDestination");
    CollectorSinkComponentProbe sink(model, "LabelCheckSink");
    label.setEnterIntoLabelComponent(&sink);

    std::string errorMessage;
    EXPECT_TRUE(label.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
    ASSERT_EQ(label.getAttachedData()->count("EnteringLabelComponent"), 1u);
    EXPECT_EQ(label.getAttachedData()->at("EnteringLabelComponent"), &sink);
}

TEST(SimulatorRuntimeTest, LabelLoadWithMissingEnteringComponentRemainsTraceableAndCheckFails) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    fields.saveField("typename", Util::TypeOf<Label>());
    fields.saveField("id", 1u);
    fields.saveField("name", "LabelLoadMissingDestination");
    fields.saveField("reportStatistics", true);
    fields.saveField("label", "Dock-A");
    fields.saveField("enteringComponentName", "ComponentThatDoesNotExist");

    LabelProbe loaded(model, "LabelLoadMissingDestination");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    EXPECT_EQ(loaded.getEnterIntoLabelComponent(), nullptr);
    EXPECT_EQ(loaded.getLabel(), "Dock-A");

    std::string errorMessage;
    EXPECT_FALSE(loaded.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("entering component was not defined"), std::string::npos);
}

TEST(SimulatorRuntimeTest, LabelSendEntityWithoutDestinationDoesNotCrash) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    LabelProbe label(model, "LabelSafeSendWithoutDestination");
    EXPECT_NO_THROW(label.sendEntityToLabelComponent(nullptr, 0.0));
}

TEST(SimulatorRuntimeTest, LabelShowIncludesLabelAndEnteringComponent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    LabelProbe label(model, "LabelShowProbe");
    CollectorSinkComponentProbe sink(model, "LabelShowSink");
    label.setLabel("Station-Transfer");
    label.setEnterIntoLabelComponent(&sink);

    const std::string shown = label.show();
    EXPECT_NE(shown.find("label=\"Station-Transfer\""), std::string::npos);
    EXPECT_NE(shown.find("enteringComponent=LabelShowSink"), std::string::npos);
}

TEST(SimulatorRuntimeTest, LabelSaveAndLoadRoundTripPreservesLabelAndEnteringComponentName) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    CollectorSinkComponentProbe sink(model, "LabelPersistSink");
    LabelProbe source(model, "LabelPersistSource");
    source.setLabel("QueueToSink");
    source.setEnterIntoLabelComponent(&sink);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    LabelProbe loaded(model, "LabelPersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    EXPECT_EQ(loaded.getLabel(), "QueueToSink");
    EXPECT_EQ(loaded.getEnterIntoLabelComponent(), &sink);

    std::string errorMessage;
    EXPECT_TRUE(loaded.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, SetDestructorDeletesOwnedContainerButNotMembers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource memberA(model, "SetLifecycleMemberA");
    Resource memberB(model, "SetLifecycleMemberB");

    auto* set = new SetProbe(model, "SetLifecycle");
    set->addElementSet(&memberA);
    set->addElementSet(&memberB);
    ASSERT_EQ(set->getElementSet()->size(), 2u);

    delete set;
    EXPECT_EQ(memberA.getName(), "SetLifecycleMemberA");
    EXPECT_EQ(memberB.getName(), "SetLifecycleMemberB");
}

TEST(SimulatorRuntimeTest, SetLoadInstanceReplacesStateWithoutResidualMembers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource memberA(model, "SetLoadMemberA");
    Resource memberB(model, "SetLoadMemberB");
    Resource memberC(model, "SetLoadMemberC");
    SetProbe set(model, "SetLoad");

    set.addElementSet(&memberA);
    set.addElementSet(&memberB);
    FakeModelPersistenceRuntime persistence;
    PersistenceRecord firstFields(persistence);
    set.SaveInstanceProbe(&firstFields, true);

    set.getElementSet()->clear();
    set.addElementSet(&memberC);
    PersistenceRecord secondFields(persistence);
    set.SaveInstanceProbe(&secondFields, true);

    ASSERT_TRUE(set.LoadInstanceProbe(&firstFields));
    ASSERT_EQ(set.getElementSet()->size(), 2u);
    ASSERT_TRUE(set.LoadInstanceProbe(&secondFields));
    ASSERT_EQ(set.getElementSet()->size(), 1u);
    EXPECT_EQ(set.getElementSet()->front(), &memberC);
}

TEST(SimulatorRuntimeTest, SetCheckFailsForMixedMemberTypes) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource resource(model, "SetMixedResource");
    Queue queue(model, "SetMixedQueue");
    SetProbe set(model, "SetMixed");
    set.addElementSet(&resource);
    set.addElementSet(&queue);

    std::string errorMessage;
    EXPECT_FALSE(set.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("member["), std::string::npos);
    EXPECT_NE(errorMessage.find("SetMixedQueue"), std::string::npos);
    EXPECT_NE(errorMessage.find("Queue"), std::string::npos);
}

TEST(SimulatorRuntimeTest, SetCheckPassesForHomogeneousMembersAndPreservesOrder) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource memberA(model, "SetHomogeneousA");
    Resource memberB(model, "SetHomogeneousB");
    SetProbe set(model, "SetHomogeneous");
    set.addElementSet(&memberA);
    set.addElementSet(&memberB);

    std::string errorMessage;
    EXPECT_TRUE(set.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
    ASSERT_EQ(set.getElementSet()->size(), 2u);
    auto members = set.getElementSet()->list();
    EXPECT_EQ(members->front(), &memberA);
    EXPECT_EQ(members->back(), &memberB);
}

TEST(SimulatorRuntimeTest, SetCheckCreatesDistinctIndexedAttachedMembers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource memberA(model, "SetAttachedIndexA");
    Resource memberB(model, "SetAttachedIndexB");
    SetProbe set(model, "SetAttachedIndex");
    set.addElementSet(&memberA);
    set.addElementSet(&memberB);

    std::string errorMessage;
    ASSERT_TRUE(set.CheckProbe(errorMessage));
    auto* attached = set.getAttachedData();
    const std::string member0Key = "Member" + Util::StrIndex(0);
    const std::string member1Key = "Member" + Util::StrIndex(1);
    ASSERT_EQ(attached->count(member0Key), 1u);
    ASSERT_EQ(attached->count(member1Key), 1u);
    EXPECT_EQ(attached->at(member0Key), &memberA);
    EXPECT_EQ(attached->at(member1Key), &memberB);
}

TEST(SimulatorRuntimeTest, SetRecheckRemovesObsoleteAttachedMembers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Resource memberA(model, "SetRecheckA");
    Resource memberB(model, "SetRecheckB");
    SetProbe set(model, "SetRecheck");
    set.addElementSet(&memberA);
    set.addElementSet(&memberB);
    set.CreateInternalAndAttachedDataProbe();

    auto* attached = set.getAttachedData();
    ASSERT_EQ(attached->count("SetRecheck.SetRecheckA"), 1u);
    ASSERT_EQ(attached->count("SetRecheck.SetRecheckB"), 1u);

    set.getElementSet()->clear();
    set.addElementSet(&memberB);
    set.CreateInternalAndAttachedDataProbe();

    EXPECT_EQ(attached->count("SetRecheck.SetRecheckA"), 0u);
    ASSERT_EQ(attached->count("SetRecheck.SetRecheckB"), 1u);
    EXPECT_EQ(attached->at("SetRecheck.SetRecheckB"), &memberB);
}

TEST(SimulatorRuntimeTest, StorageDefaultsAreInitializedAsExpected) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Storage storage(model, "StorageDefaults");
    EXPECT_EQ(storage.getCapacity(), 10u);
    EXPECT_DOUBLE_EQ(storage.getTotalArea(), 1.0);
    EXPECT_DOUBLE_EQ(storage.getUnitsPerArea(), 1.0);
}

TEST(SimulatorRuntimeTest, StorageSettersAndGettersRemainCoherent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Storage storage(model, "StorageSetters");
    storage.setCapacity(25u);
    storage.setTotalArea(42.5);
    storage.setUnitsPerArea(3.75);

    EXPECT_EQ(storage.getCapacity(), 25u);
    EXPECT_DOUBLE_EQ(storage.getTotalArea(), 42.5);
    EXPECT_DOUBLE_EQ(storage.getUnitsPerArea(), 3.75);
}

TEST(SimulatorRuntimeTest, StorageCheckFailsForInvalidValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StorageProbe storage(model, "StorageInvalid");

    storage.setCapacity(0u);
    storage.setTotalArea(1.0);
    storage.setUnitsPerArea(1.0);
    std::string invalidCapacityError;
    EXPECT_FALSE(storage.CheckProbe(invalidCapacityError));
    EXPECT_NE(invalidCapacityError.find("Capacity must be greater than zero"), std::string::npos);

    storage.setCapacity(1u);
    storage.setTotalArea(0.0);
    storage.setUnitsPerArea(1.0);
    std::string invalidTotalAreaError;
    EXPECT_FALSE(storage.CheckProbe(invalidTotalAreaError));
    EXPECT_NE(invalidTotalAreaError.find("TotalArea must be greater than zero"), std::string::npos);

    storage.setCapacity(1u);
    storage.setTotalArea(1.0);
    storage.setUnitsPerArea(-1.0);
    std::string invalidUnitsPerAreaError;
    EXPECT_FALSE(storage.CheckProbe(invalidUnitsPerAreaError));
    EXPECT_NE(invalidUnitsPerAreaError.find("UnitsPerArea must be greater than zero"), std::string::npos);
}

TEST(SimulatorRuntimeTest, StorageCheckPassesForValidConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StorageProbe storage(model, "StorageValid");
    storage.setCapacity(7u);
    storage.setTotalArea(2.5);
    storage.setUnitsPerArea(4.0);

    std::string errorMessage;
    EXPECT_TRUE(storage.CheckProbe(errorMessage));
    EXPECT_TRUE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, StorageShowIncludesMainConfiguredParameters) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Storage storage(model, "StorageShow");
    storage.setCapacity(15u);
    storage.setTotalArea(6.5);
    storage.setUnitsPerArea(2.25);

    const std::string info = storage.show();
    EXPECT_NE(info.find("capacity=15"), std::string::npos);
    EXPECT_NE(info.find("totalArea=6.5"), std::string::npos);
    EXPECT_NE(info.find("unitsPerArea=2.25"), std::string::npos);
}

TEST(SimulatorRuntimeTest, StorageSaveAndLoadRoundTripPreservesParameters) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StorageProbe source(model, "StorageSaveSource");
    source.setCapacity(18u);
    source.setTotalArea(11.5);
    source.setUnitsPerArea(5.25);

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    StorageProbe loaded(model, "StorageSaveLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    EXPECT_EQ(loaded.getCapacity(), 18u);
    EXPECT_DOUBLE_EQ(loaded.getTotalArea(), 11.5);
    EXPECT_DOUBLE_EQ(loaded.getUnitsPerArea(), 5.25);
}

TEST(SimulatorRuntimeTest, StorageRegistersMainControlsAsOwnedProperties) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Storage storage(model, "StorageProperties");
    auto* controls = storage.getSimulationControls();
    ASSERT_NE(controls, nullptr);
    EXPECT_GE(controls->size(), 3u);

    bool hasCapacity = false;
    bool hasTotalArea = false;
    bool hasUnitsPerArea = false;
    for (SimulationControl* control : *controls->list()) {
        ASSERT_NE(control, nullptr);
        hasCapacity = hasCapacity || control->getName() == "Capacity";
        hasTotalArea = hasTotalArea || control->getName() == "TotalArea";
        hasUnitsPerArea = hasUnitsPerArea || control->getName() == "UnitsPerArea";
    }

    EXPECT_TRUE(hasCapacity);
    EXPECT_TRUE(hasTotalArea);
    EXPECT_TRUE(hasUnitsPerArea);
}

TEST(SimulatorRuntimeTest, StationCreateInternalInitiallyCreatesCollectorsWhenStatisticsEnabled) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationCreateStats");
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();

    EXPECT_NE(station.NumberInStationCollectorProbe(), nullptr);
    EXPECT_NE(station.TimeInStationCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, StationRecheckWithStatisticsEnabledIsIdempotentAndKeepsCollectors) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationIdempotentStats");
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    StatisticsCollector* numberCollector = station.NumberInStationCollectorProbe();
    StatisticsCollector* timeCollector = station.TimeInStationCollectorProbe();
    ASSERT_NE(numberCollector, nullptr);
    ASSERT_NE(timeCollector, nullptr);

    station.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(station.NumberInStationCollectorProbe(), numberCollector);
    EXPECT_EQ(station.TimeInStationCollectorProbe(), timeCollector);
}

TEST(SimulatorRuntimeTest, StationDisablingStatisticsOnRecheckClearsCollectorsPointers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationDisableStats");
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(station.NumberInStationCollectorProbe(), nullptr);
    ASSERT_NE(station.TimeInStationCollectorProbe(), nullptr);

    station.setReportStatistics(false);
    station.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(station.NumberInStationCollectorProbe(), nullptr);
    EXPECT_EQ(station.TimeInStationCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, StationReenableStatisticsOnRecheckRecreatesCollectors) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationReenableStats");
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    ASSERT_NE(station.NumberInStationCollectorProbe(), nullptr);
    ASSERT_NE(station.TimeInStationCollectorProbe(), nullptr);

    station.setReportStatistics(false);
    station.CreateInternalAndAttachedDataProbe();
    ASSERT_EQ(station.NumberInStationCollectorProbe(), nullptr);
    ASSERT_EQ(station.TimeInStationCollectorProbe(), nullptr);

    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    EXPECT_NE(station.NumberInStationCollectorProbe(), nullptr);
    EXPECT_NE(station.TimeInStationCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, StationInitBetweenReplicationsHookResetsLocalCount) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationReplicationReset");
    station.setReportStatistics(false);
    station.CreateInternalAndAttachedDataProbe();
    Entity* entityA = model->createEntity("StationReplicationEntityA", true);
    Entity* entityB = model->createEntity("StationReplicationEntityB", true);
    station.enter(entityA);
    station.enter(entityB);
    ASSERT_EQ(station.NumberInStationProbe(), 2u);

    station.InitBetweenReplicationsProbe();
    EXPECT_EQ(station.NumberInStationProbe(), 0u);
}

TEST(SimulatorRuntimeTest, StationRenameRecheckKeepsOnlyCurrentArrivalAttributeAttached) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationRenameOld");
    station.CreateInternalAndAttachedDataProbe();
    auto* attached = station.getAttachedData();
    ASSERT_EQ(attached->count("Entity.ArrivalAtStationRenameOld"), 1u);

    station.setName("StationRenameNew");
    station.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(attached->count("Entity.ArrivalAtStationRenameNew"), 1u);
    EXPECT_EQ(attached->count("Entity.ArrivalAtStationRenameOld"), 0u);
    EXPECT_EQ(attached->count("Entity.Station"), 1u);
}

TEST(SimulatorRuntimeTest, StationEnterLeaveFlowRemainsCoherentAfterRechecksAndResets) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    StationTestProbe station(model, "StationFlow");
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    station.setReportStatistics(false);
    station.CreateInternalAndAttachedDataProbe();
    station.setReportStatistics(true);
    station.CreateInternalAndAttachedDataProbe();
    station.setReportStatistics(false);
    station.CreateInternalAndAttachedDataProbe();

    Entity* entity = model->createEntity("StationFlowEntity", true);
    station.enter(entity);
    EXPECT_EQ(station.NumberInStationProbe(), 1u);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.Station"), static_cast<double>(station.getId()));
    EXPECT_NO_THROW(entity->getAttributeValue("Entity.ArrivalAtStationFlow"));

    station.leave(entity);
    EXPECT_EQ(station.NumberInStationProbe(), 0u);
    EXPECT_DOUBLE_EQ(entity->getAttributeValue("Entity.Station"), 0.0);
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

TEST(SimulatorRuntimeTest, WaitAndSignalPersistenceRoundTripPreservesSharedSignalDataReference) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe sharedSignalData(model, "SharedSignalData");
    WaitProbe sourceWait(model, "WaitPersistSource");
    sourceWait.setWaitType(Wait::WaitType::WaitForSignal);
    sourceWait.setSignalData(&sharedSignalData);

    SignalProbe sourceSignal(model, "SignalPersistSource");
    sourceSignal.setSignalData(&sharedSignalData);
    sourceSignal.setLimitExpression("2");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord waitFields(persistence);
    PersistenceRecord signalFields(persistence);
    sourceWait.SaveInstanceProbe(&waitFields, true);
    sourceSignal.SaveInstanceProbe(&signalFields, true);

    WaitProbe loadedWait(model, "WaitPersistLoaded");
    SignalProbe loadedSignal(model, "SignalPersistLoaded");
    ASSERT_TRUE(loadedWait.LoadInstanceProbe(&waitFields));
    ASSERT_TRUE(loadedSignal.LoadInstanceProbe(&signalFields));

    ASSERT_NE(loadedWait._signalData, nullptr);
    ASSERT_NE(loadedSignal.SignalDataPtrProbe(), nullptr);
    EXPECT_EQ(loadedWait._signalData, &sharedSignalData);
    EXPECT_EQ(loadedSignal.SignalDataPtrProbe(), &sharedSignalData);
    EXPECT_EQ(loadedWait._signalData, loadedSignal.SignalDataPtrProbe());
}

TEST(SimulatorRuntimeTest, SignalCheckRequiresSignalDataAndValidLimitExpression) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signalData(model, "SignalCheckData");
    SignalProbe signal(model, "SignalCheckProbe");

    signal.setLimitExpression("1");
    std::string noSignalDataError;
    EXPECT_FALSE(signal.CheckProbe(noSignalDataError));
    EXPECT_NE(noSignalDataError.find("SignalData is null"), std::string::npos);

    signal.setSignalData(&signalData);
    std::string validError;
    EXPECT_TRUE(signal.CheckProbe(validError));
    EXPECT_TRUE(validError.empty());

    signal.setLimitExpression("invalid +");
    std::string invalidExpressionError;
    EXPECT_FALSE(signal.CheckProbe(invalidExpressionError));
    EXPECT_NE(invalidExpressionError.find("LimitExpression"), std::string::npos);
}

TEST(SimulatorRuntimeTest, WaitSignalHandlerRespectsLocalLimitWithoutOffByOne) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    WaitProbe wait(model, "WaitSignalLimit");
    EXPECT_EQ(wait.CountReleasesWithCurrentBoundaryProbe(4u, 10u, 3u), 3u);
    EXPECT_EQ(wait.CountReleasesWithCurrentBoundaryProbe(4u, 2u, 3u), 2u);
    EXPECT_EQ(wait.CountReleasesWithCurrentBoundaryProbe(2u, 10u, 3u), 2u);
}

TEST(SimulatorRuntimeTest, WaitScanForConditionRecheckDoesNotReRegisterHandlerFlag) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    WaitProbe wait(model, "WaitScanRecheck");
    Queue queue(model, "WaitScanQueue");
    wait.setWaitType(Wait::WaitType::ScanForCondition);
    wait.setCondition("1");
    wait.setQueue(&queue);

    std::string firstCheckError;
    EXPECT_TRUE(wait.CheckProbe(firstCheckError));
    EXPECT_TRUE(firstCheckError.empty());
    EXPECT_TRUE(wait.IsScanConditionHandlerRegisteredProbe());

    std::string secondCheckError;
    EXPECT_TRUE(wait.CheckProbe(secondCheckError));
    EXPECT_TRUE(secondCheckError.empty());
    EXPECT_TRUE(wait.IsScanConditionHandlerRegisteredProbe());
}

TEST(SimulatorRuntimeTest, WaitCheckValidatesWaitForSignalContractAndLimitExpression) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    WaitProbe wait(model, "WaitForSignalCheck");
    Queue queue(model, "WaitForSignalQueue");
    SignalDataProbe signalData(model, "WaitForSignalData");

    wait.setQueue(&queue);
    wait.setWaitType(Wait::WaitType::WaitForSignal);
    wait.setLimitExpression("1");

    std::string missingSignalError;
    EXPECT_FALSE(wait.CheckProbe(missingSignalError));
    EXPECT_NE(missingSignalError.find("SignalData is null"), std::string::npos);

    wait.setSignalData(&signalData);
    wait.setLimitExpression("invalid +");

    std::string invalidLimitError;
    EXPECT_FALSE(wait.CheckProbe(invalidLimitError));
    EXPECT_NE(invalidLimitError.find("LimitExpression"), std::string::npos);

    wait.setLimitExpression("1");
    std::string validLimitError;
    EXPECT_TRUE(wait.CheckProbe(validLimitError));
    EXPECT_TRUE(validLimitError.empty());
}

TEST(SimulatorRuntimeTest, WaitCheckValidatesScanConditionExpression) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    WaitProbe wait(model, "WaitScanConditionCheck");
    Queue queue(model, "WaitScanConditionQueue");
    wait.setQueue(&queue);
    wait.setWaitType(Wait::WaitType::ScanForCondition);
    wait.setCondition("invalid +");

    std::string errorMessage;
    EXPECT_FALSE(wait.CheckProbe(errorMessage));
    EXPECT_NE(errorMessage.find("Condition"), std::string::npos);
}

TEST(SimulatorRuntimeTest, WaitForSignalLimitZeroDoesNotReleaseQueuedEntities) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    WaitProbe wait(model, "WaitSignalLimitZero");
    Queue* queue = new Queue(model, "WaitSignalLimitZeroQueue");
    SignalDataProbe signalData(model, "WaitSignalLimitZeroData");

    wait.setQueue(queue);
    wait.setWaitType(Wait::WaitType::WaitForSignal);
    wait.setSignalData(&signalData);
    wait.setLimitExpression("0");

    signalData.generateSignal(0.0, 10u);
    EXPECT_EQ(wait.TriggerSignalHandlerProbe(&signalData), 0u);
    EXPECT_EQ(wait.getQueue()->size(), 0u);
}

TEST(SimulatorRuntimeTest, SignalAndWaitSharedSignalDataRemainCoherentAfterRecheck) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SignalDataProbe signalData(model, "RecheckSharedSignalData");
    Queue* queue = new Queue(model, "RecheckSharedWaitQueue");

    WaitProbe wait(model, "RecheckSharedWait");
    wait.setQueue(queue);
    wait.setWaitType(Wait::WaitType::WaitForSignal);
    wait.setSignalData(&signalData);
    wait.setLimitExpression("1");

    SignalProbe signal(model, "RecheckSharedSignal");
    signal.setSignalData(&signalData);
    signal.setLimitExpression("1");

    wait.CreateInternalAndAttachedDataProbe();
    std::string firstWaitError;
    std::string firstSignalError;
    EXPECT_TRUE(wait.CheckProbe(firstWaitError));
    EXPECT_TRUE(signal.CheckProbe(firstSignalError));

    std::string secondWaitError;
    std::string secondSignalError;
    EXPECT_TRUE(wait.CheckProbe(secondWaitError));
    EXPECT_TRUE(signal.CheckProbe(secondSignalError));

    EXPECT_EQ(wait._signalData, &signalData);
    EXPECT_EQ(signal.SignalDataPtrProbe(), &signalData);
    EXPECT_EQ(wait._signalData, signal.SignalDataPtrProbe());
    EXPECT_TRUE(signalData.hasSignalDataEventHandler(&wait));
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

TEST(SimulatorRuntimeTest, MatchAnyReleasesOnlyWhenAllQueuesHaveEnoughEntities) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    MatchProbe match(model, "MatchAnyBasic");
    CollectorSinkComponentProbe sink(model, "MatchAnyBasicSink");
    match.connectTo(&sink);
    match.setRule(Match::Rule::Any);
    match.setNumberOfQueues(2);
    match.setMatchSize("1");
    match.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "MatchAnyBasicPart");
    Entity* q0e1 = model->createEntity("MatchAnyQ0E1", true);
    Entity* q1e1 = model->createEntity("MatchAnyQ1E1", true);
    q0e1->setEntityType(partType);
    q1e1->setEntityType(partType);

    match.DispatchEventProbe(q0e1, 0);
    DrainFutureEvents(model);
    EXPECT_TRUE(sink.ReceivedEntities().empty());

    match.DispatchEventProbe(q1e1, 1);
    DrainFutureEvents(model);

    ASSERT_EQ(sink.ReceivedEntities().size(), 2u);
    EXPECT_EQ(sink.ReceivedEntities()[0], q0e1);
    EXPECT_EQ(sink.ReceivedEntities()[1], q1e1);
}

TEST(SimulatorRuntimeTest, MatchAnyWithMatchSizeTwoReleasesExactlyTwoPerQueueAndKeepsOverflowWaiting) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    MatchProbe match(model, "MatchAnySizeTwo");
    CollectorSinkComponentProbe sink(model, "MatchAnySizeTwoSink");
    match.connectTo(&sink);
    match.setRule(Match::Rule::Any);
    match.setNumberOfQueues(2);
    match.setMatchSize("2");
    match.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "MatchAnySizeTwoPart");
    Entity* q0e1 = model->createEntity("MatchAny2Q0E1", true);
    Entity* q0e2 = model->createEntity("MatchAny2Q0E2", true);
    Entity* q0e3 = model->createEntity("MatchAny2Q0E3", true);
    Entity* q1e1 = model->createEntity("MatchAny2Q1E1", true);
    Entity* q1e2 = model->createEntity("MatchAny2Q1E2", true);
    q0e1->setEntityType(partType);
    q0e2->setEntityType(partType);
    q0e3->setEntityType(partType);
    q1e1->setEntityType(partType);
    q1e2->setEntityType(partType);

    match.DispatchEventProbe(q0e1, 0);
    match.DispatchEventProbe(q0e2, 0);
    match.DispatchEventProbe(q0e3, 0);
    match.DispatchEventProbe(q1e1, 1);
    match.DispatchEventProbe(q1e2, 1);
    DrainFutureEvents(model);

    Queue* q0 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "MatchAnySizeTwo.Queue0"));
    Queue* q1 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "MatchAnySizeTwo.Queue1"));
    ASSERT_NE(q0, nullptr);
    ASSERT_NE(q1, nullptr);
    EXPECT_EQ(q0->size(), 1u);
    EXPECT_EQ(q1->size(), 0u);
    ASSERT_EQ(sink.ReceivedEntities().size(), 4u);
}

TEST(SimulatorRuntimeTest, MatchByAttributeSynchronizesOnlyCompatibleAttributeValues) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    (void)new Attribute(model, "Color");
    MatchProbe match(model, "MatchByAttribute");
    CollectorSinkComponentProbe sink(model, "MatchByAttributeSink");
    match.connectTo(&sink);
    match.setRule(Match::Rule::ByAttribute);
    match.setNumberOfQueues(2);
    match.setMatchSize("1");
    match.setAttributeName("Color");
    match.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "MatchByAttributePart");
    Entity* q0Color1 = model->createEntity("MatchByAttrQ0Color1", true);
    Entity* q1Color2 = model->createEntity("MatchByAttrQ1Color2", true);
    Entity* q0Color2 = model->createEntity("MatchByAttrQ0Color2", true);
    q0Color1->setEntityType(partType);
    q1Color2->setEntityType(partType);
    q0Color2->setEntityType(partType);
    q0Color1->setAttributeValue("Color", 1.0);
    q1Color2->setAttributeValue("Color", 2.0);
    q0Color2->setAttributeValue("Color", 2.0);

    match.DispatchEventProbe(q0Color1, 0);
    match.DispatchEventProbe(q1Color2, 1);
    DrainFutureEvents(model);
    EXPECT_TRUE(sink.ReceivedEntities().empty());

    match.DispatchEventProbe(q0Color2, 0);
    DrainFutureEvents(model);

    Queue* q0 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "MatchByAttribute.Queue0"));
    Queue* q1 = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "MatchByAttribute.Queue1"));
    ASSERT_NE(q0, nullptr);
    ASSERT_NE(q1, nullptr);
    ASSERT_EQ(sink.ReceivedEntities().size(), 2u);
    EXPECT_EQ(q0->size(), 1u);
    EXPECT_EQ(q1->size(), 0u);
    EXPECT_EQ(sink.ReceivedEntities()[0], q0Color2);
    EXPECT_EQ(sink.ReceivedEntities()[1], q1Color2);
}

TEST(SimulatorRuntimeTest, MatchPersistenceRoundTripPreservesRuleQueueCountMatchSizeAndAttribute) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    MatchProbe source(model, "MatchPersistSource");
    source.setRule(Match::Rule::ByAttribute);
    source.setNumberOfQueues(4);
    source.setMatchSize("3");
    source.setAttributeName("Color");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    MatchProbe loaded(model, "MatchPersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    EXPECT_EQ(loaded.getRule(), Match::Rule::ByAttribute);
    EXPECT_EQ(loaded.getNumberOfQueues(), 4u);
    EXPECT_EQ(loaded.getMatchSize(), "3");
    EXPECT_EQ(loaded.getAttributeName(), "Color");
}

TEST(SimulatorRuntimeTest, MatchCheckValidatesQueueCountMatchSizeAndByAttributeContract) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    MatchProbe invalidQueues(model, "MatchCheckInvalidQueues");
    invalidQueues.setRule(Match::Rule::Any);
    invalidQueues.setNumberOfQueues(1);
    invalidQueues.setMatchSize("1");
    std::string invalidQueuesError;
    EXPECT_FALSE(invalidQueues.CheckProbe(invalidQueuesError));
    EXPECT_NE(invalidQueuesError.find("NumberOfQueues"), std::string::npos);

    MatchProbe invalidMatchSize(model, "MatchCheckInvalidExpression");
    invalidMatchSize.setRule(Match::Rule::Any);
    invalidMatchSize.setNumberOfQueues(2);
    invalidMatchSize.setMatchSize("bad_expr(");
    std::string invalidExpressionError;
    EXPECT_FALSE(invalidMatchSize.CheckProbe(invalidExpressionError));
    EXPECT_NE(invalidExpressionError.find("MatchSize"), std::string::npos);

    MatchProbe invalidAttribute(model, "MatchCheckInvalidAttribute");
    invalidAttribute.setRule(Match::Rule::ByAttribute);
    invalidAttribute.setNumberOfQueues(2);
    invalidAttribute.setMatchSize("1");
    invalidAttribute.setAttributeName("MissingColorAttribute");
    std::string invalidAttributeError;
    EXPECT_FALSE(invalidAttribute.CheckProbe(invalidAttributeError));
    EXPECT_NE(invalidAttributeError.find("AttributeName"), std::string::npos);

    (void)new Attribute(model, "Color");
    MatchProbe valid(model, "MatchCheckValid");
    valid.setRule(Match::Rule::ByAttribute);
    valid.setNumberOfQueues(2);
    valid.setMatchSize("1");
    valid.setAttributeName("Color");
    std::string validError;
    EXPECT_TRUE(valid.CheckProbe(validError));
    EXPECT_TRUE(validError.empty());
}

TEST(SimulatorRuntimeTest, BatchAnyFormsSingleBatchWithAtLeastBatchSizeAndKeepsOverflowQueued) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BatchProbe batch(model, "BatchAny");
    CollectorSinkComponentProbe sink(model, "BatchAnySink");
    batch.connectTo(&sink);
    batch.setBatchSize("2");
    batch.setRule(Batch::Rule::Any);
    batch.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "BatchAnyPart");
    Entity* e1 = model->createEntity("AnyE1", true);
    Entity* e2 = model->createEntity("AnyE2", true);
    Entity* e3 = model->createEntity("AnyE3", true);
    e1->setEntityType(partType);
    e2->setEntityType(partType);
    e3->setEntityType(partType);

    batch.DispatchEventProbe(e1);
    batch.DispatchEventProbe(e2);
    batch.DispatchEventProbe(e3);
    DrainFutureEvents(model);

    Queue* queue = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "BatchAny.Queue"));
    ASSERT_NE(queue, nullptr);
    EXPECT_EQ(queue->size(), 1u);
    ASSERT_EQ(sink.ReceivedEntities().size(), 1u);
}

TEST(SimulatorRuntimeTest, BatchByAttributeFormsExactBatchSizeOfFirstCompatibleEntities) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BatchProbe batch(model, "BatchByAttribute");
    CollectorSinkComponentProbe sink(model, "BatchByAttributeSink");
    batch.connectTo(&sink);
    batch.setBatchSize("2");
    batch.setRule(Batch::Rule::ByAttribute);
    batch.setAttributeName("Color");
    batch.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "BatchByAttrPart");
    (void)new Attribute(model, "Color");
    Entity* e1 = model->createEntity("ByAttrE1", true);
    Entity* e2 = model->createEntity("ByAttrE2", true);
    Entity* e3 = model->createEntity("ByAttrE3", true);
    Entity* e4 = model->createEntity("ByAttrE4", true);
    e1->setEntityType(partType);
    e2->setEntityType(partType);
    e3->setEntityType(partType);
    e4->setEntityType(partType);
    e1->setAttributeValue("Color", 1.0);
    e2->setAttributeValue("Color", 2.0);
    e3->setAttributeValue("Color", 1.0);
    e4->setAttributeValue("Color", 1.0);

    batch.DispatchEventProbe(e1);
    batch.DispatchEventProbe(e2);
    batch.DispatchEventProbe(e3);
    batch.DispatchEventProbe(e4);
    DrainFutureEvents(model);

    Queue* queue = dynamic_cast<Queue*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), "BatchByAttribute.Queue"));
    ASSERT_NE(queue, nullptr);
    EXPECT_EQ(queue->size(), 2u);
    ASSERT_EQ(sink.ReceivedEntities().size(), 1u);
}

TEST(SimulatorRuntimeTest, BatchTemporaryThenSeparateReleasesOriginalMembersInOrder) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BatchProbe batch(model, "BatchTemp");
    SeparateProbe separate(model, "SeparateAfterTemp");
    CollectorSinkComponentProbe sink(model, "BatchTempSink");
    batch.connectTo(&separate);
    separate.connectTo(&sink);
    batch.setBatchType(Batch::BatchType::Temporary);
    batch.setBatchSize("2");
    batch.setRule(Batch::Rule::Any);
    batch.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "BatchTempPart");
    Entity* e1 = model->createEntity("TempE1", true);
    Entity* e2 = model->createEntity("TempE2", true);
    e1->setEntityType(partType);
    e2->setEntityType(partType);

    batch.DispatchEventProbe(e1);
    batch.DispatchEventProbe(e2);
    DrainFutureEvents(model);

    ASSERT_EQ(sink.ReceivedEntities().size(), 2u);
    EXPECT_EQ(sink.ReceivedEntities()[0], e1);
    EXPECT_EQ(sink.ReceivedEntities()[1], e2);
}

TEST(SimulatorRuntimeTest, BatchPermanentRepresentativePassesThroughSeparateWithoutResurrectingMembers) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BatchProbe batch(model, "BatchPermanent");
    SeparateProbe separate(model, "SeparateAfterPermanent");
    CollectorSinkComponentProbe sink(model, "BatchPermanentSink");
    batch.connectTo(&separate);
    separate.connectTo(&sink);
    batch.setBatchType(Batch::BatchType::Permanent);
    batch.setBatchSize("2");
    batch.setRule(Batch::Rule::Any);
    batch.CreateInternalAndAttachedDataProbe();

    EntityType* partType = new EntityType(model, "BatchPermanentPart");
    Entity* e1 = model->createEntity("PermE1", true);
    Entity* e2 = model->createEntity("PermE2", true);
    const Util::identification e1Id = e1->getId();
    const Util::identification e2Id = e2->getId();
    e1->setEntityType(partType);
    e2->setEntityType(partType);

    batch.DispatchEventProbe(e1);
    batch.DispatchEventProbe(e2);
    DrainFutureEvents(model);

    ASSERT_EQ(sink.ReceivedEntities().size(), 1u);
    EXPECT_NE(sink.ReceivedEntities()[0], e1);
    EXPECT_NE(sink.ReceivedEntities()[0], e2);
    EXPECT_EQ(dynamic_cast<Entity*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), e1Id)), nullptr);
    EXPECT_EQ(dynamic_cast<Entity*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), e2Id)), nullptr);
}

TEST(SimulatorRuntimeTest, BatchCheckValidatesByAttributeRequirementAndMinimalValidConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    BatchProbe invalidBatch(model, "BatchCheckInvalid");
    invalidBatch.setRule(Batch::Rule::ByAttribute);
    invalidBatch.setBatchSize("2");
    invalidBatch.setAttributeName("");
    std::string invalidError;
    EXPECT_FALSE(invalidBatch.CheckProbe(invalidError));
    EXPECT_FALSE(invalidError.empty());

    BatchProbe validBatch(model, "BatchCheckValid");
    validBatch.setRule(Batch::Rule::Any);
    validBatch.setBatchSize("2");
    std::string validError;
    EXPECT_TRUE(validBatch.CheckProbe(validError));
    EXPECT_TRUE(validError.empty());
}

TEST(SimulatorRuntimeTest, SeparateHandlesUngroupedEntitySafely) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SeparateProbe separate(model, "SeparateUngrouped");
    CollectorSinkComponentProbe sink(model, "SeparateUngroupedSink");
    separate.connectTo(&sink);

    EntityType* partType = new EntityType(model, "SeparateUngroupedPart");
    Entity* entity = model->createEntity("UngroupedEntity", true);
    entity->setEntityType(partType);
    entity->setAttributeValue("Entity.Group", 0.0, "", true);

    separate.DispatchEventProbe(entity);
    DrainFutureEvents(model);

    ASSERT_EQ(sink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(sink.ReceivedEntities()[0], entity);
}

TEST(SimulatorRuntimeTest, SeparateCheckAndValidGroupReferenceRemainCoherent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SeparateProbe separate(model, "SeparateValidGroup");
    CollectorSinkComponentProbe sink(model, "SeparateValidGroupSink");
    separate.connectTo(&sink);

    std::string checkError;
    EXPECT_TRUE(separate.CheckProbe(checkError));
    EXPECT_TRUE(checkError.empty());

    EntityType* partType = new EntityType(model, "SeparateGroupedPart");
    EntityGroup* entityGroup = new EntityGroup(model, "ManualEntityGroup");
    Entity* representative = model->createEntity("Representative", true);
    Entity* member = model->createEntity("Member", true);
    representative->setEntityType(partType);
    member->setEntityType(partType);
    representative->setAttributeValue("Entity.Group", entityGroup->getId(), "", true);
    entityGroup->insertElement(representative->getId(), member);

    separate.DispatchEventProbe(representative);
    DrainFutureEvents(model);

    ASSERT_EQ(sink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(sink.ReceivedEntities()[0], member);
    EXPECT_EQ(member->getAttributeValue("Entity.Group"), 0.0);
}

TEST(SimulatorRuntimeTest, EntityGroupInsertAndGetExistingGroupPreservesInsertionOrder) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EntityGroupProbe entityGroup(model, "EntityGroupExisting");
    EntityType* partType = new EntityType(model, "EntityGroupExistingPart");
    Entity* e1 = model->createEntity("GroupE1", true);
    Entity* e2 = model->createEntity("GroupE2", true);
    e1->setEntityType(partType);
    e2->setEntityType(partType);

    const unsigned int groupKey = 101u;
    entityGroup.insertElement(groupKey, e1);
    entityGroup.insertElement(groupKey, e2);
    List<Entity*>* members = entityGroup.getGroup(groupKey);

    ASSERT_NE(members, nullptr);
    ASSERT_EQ(members->size(), 2u);
    EXPECT_EQ(members->getAtRank(0), e1);
    EXPECT_EQ(members->getAtRank(1), e2);
}

TEST(SimulatorRuntimeTest, EntityGroupGetGroupReturnsNullptrForMissingGroup) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EntityGroupProbe entityGroup(model, "EntityGroupMissing");
    EXPECT_EQ(entityGroup.getGroup(999u), nullptr);
}

TEST(SimulatorRuntimeTest, EntityGroupRemoveElementDeletesEmptyGroupEntry) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EntityGroupProbe entityGroup(model, "EntityGroupRemove");
    EntityType* partType = new EntityType(model, "EntityGroupRemovePart");
    Entity* e1 = model->createEntity("GroupRemoveE1", true);
    e1->setEntityType(partType);

    const unsigned int groupKey = 202u;
    entityGroup.insertElement(groupKey, e1);
    ASSERT_NE(entityGroup.getGroup(groupKey), nullptr);

    entityGroup.removeElement(groupKey, e1);
    EXPECT_EQ(entityGroup.getGroup(groupKey), nullptr);
}

TEST(SimulatorRuntimeTest, EntityGroupInitBetweenReplicationsClearsAllRuntimeGroups) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EntityGroupProbe entityGroup(model, "EntityGroupReplicationReset");
    EntityType* partType = new EntityType(model, "EntityGroupReplicationResetPart");
    Entity* e1 = model->createEntity("GroupResetE1", true);
    Entity* e2 = model->createEntity("GroupResetE2", true);
    e1->setEntityType(partType);
    e2->setEntityType(partType);

    entityGroup.insertElement(301u, e1);
    entityGroup.insertElement(302u, e2);
    ASSERT_NE(entityGroup.getGroup(301u), nullptr);
    ASSERT_NE(entityGroup.getGroup(302u), nullptr);

    entityGroup.InitBetweenReplicationsProbe();
    EXPECT_EQ(entityGroup.getGroup(301u), nullptr);
    EXPECT_EQ(entityGroup.getGroup(302u), nullptr);
}

TEST(SimulatorRuntimeTest, EntityGroupStatisticsToggleResetsAndRecreatesCollectorOnRecheck) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EntityGroupProbe entityGroup(model, "EntityGroupStatsToggle");
    ASSERT_NE(entityGroup.NumberInGroupCollectorProbe(), nullptr);

    entityGroup.setReportStatistics(false);
    entityGroup.CreateInternalAndAttachedDataProbe();
    EXPECT_EQ(entityGroup.NumberInGroupCollectorProbe(), nullptr);

    entityGroup.setReportStatistics(true);
    entityGroup.CreateInternalAndAttachedDataProbe();
    EXPECT_NE(entityGroup.NumberInGroupCollectorProbe(), nullptr);
}

TEST(SimulatorRuntimeTest, EntityGroupDestructorCleansOwnedGroupContainersSafely) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    EXPECT_NO_FATAL_FAILURE({
        EntityGroupProbe entityGroup(model, "EntityGroupDestructorCleanup");
        EntityType* partType = new EntityType(model, "EntityGroupDestructorPart");
        Entity* e1 = model->createEntity("GroupDestructorE1", true);
        Entity* e2 = model->createEntity("GroupDestructorE2", true);
        e1->setEntityType(partType);
        e2->setEntityType(partType);
        entityGroup.insertElement(401u, e1);
        entityGroup.insertElement(401u, e2);
        entityGroup.insertElement(402u, e1);
    });
}

TEST(SimulatorRuntimeTest, ProcessCreateInternalMacroComponentKeepsSeizeDelayReleaseChainCoherent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ProcessProbe process(model, "ProcessCreateMacro");
    Delay sink(model, "ProcessCreateMacroSink");
    process.getConnectionManager()->insert(&sink);
    process.CreateInternalAndAttachedDataProbe();

    ASSERT_NE(process.SeizePtrProbe(), nullptr);
    ASSERT_NE(process.DelayPtrProbe(), nullptr);
    ASSERT_NE(process.ReleasePtrProbe(), nullptr);
    EXPECT_EQ(process.SeizePtrProbe()->getLevel(), process.getId());
    EXPECT_EQ(process.DelayPtrProbe()->getLevel(), process.getId());
    EXPECT_EQ(process.ReleasePtrProbe()->getLevel(), process.getId());
    ASSERT_NE(process.SeizePtrProbe()->getConnectionManager()->getFrontConnection(), nullptr);
    ASSERT_NE(process.DelayPtrProbe()->getConnectionManager()->getFrontConnection(), nullptr);
    EXPECT_EQ(process.SeizePtrProbe()->getConnectionManager()->getFrontConnection()->component, process.DelayPtrProbe());
    EXPECT_EQ(process.DelayPtrProbe()->getConnectionManager()->getFrontConnection()->component, process.ReleasePtrProbe());
    ASSERT_NE(process.getConnectionManager()->getFrontConnection(), nullptr);
    EXPECT_EQ(process.getConnectionManager()->getFrontConnection()->component, process.SeizePtrProbe());
    ASSERT_NE(process.ReleasePtrProbe()->getConnectionManager()->getFrontConnection(), nullptr);
    EXPECT_EQ(process.ReleasePtrProbe()->getConnectionManager()->getFrontConnection()->component, &sink);
}

TEST(SimulatorRuntimeTest, ProcessRecheckReconcilesReleaseRequestsFromSeizeRequests) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ProcessProbe process(model, "ProcessReconcile");
    Queue* queue = new Queue(model, "ProcessReconcileQueue");
    process.setQueueableItem(new QueueableItem(queue));
    Resource* r1 = new Resource(model, "ProcessReconcileR1");
    Resource* r2 = new Resource(model, "ProcessReconcileR2");
    process.addSeizeRequest(new SeizableItem(r1, "1", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY, "Entity.CustomSaveR1"));
    process.addSeizeRequest(new SeizableItem(r2, "2", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY));
    process.CreateInternalAndAttachedDataProbe();

    std::string errorMessage;
    ASSERT_TRUE(process.CheckProbe(errorMessage)) << errorMessage;
    ASSERT_NE(process.ReleasePtrProbe(), nullptr);
    ASSERT_EQ(process.SeizePtrProbe()->getSeizeRequests()->size(), process.ReleasePtrProbe()->getReleaseRequests()->size());
    ASSERT_EQ(process.ReleasePtrProbe()->getReleaseRequests()->size(), 2u);
    EXPECT_EQ(process.ReleasePtrProbe()->getReleaseRequests()->getAtRank(0)->getSelectionRule(), SeizableItem::SelectionRule::SPECIFICMEMBER);
    EXPECT_EQ(process.ReleasePtrProbe()->getReleaseRequests()->getAtRank(1)->getSelectionRule(), SeizableItem::SelectionRule::SPECIFICMEMBER);
    EXPECT_EQ(process.ReleasePtrProbe()->getReleaseRequests()->getAtRank(0)->getSaveAttribute(), "Entity.CustomSaveR1");
    EXPECT_FALSE(process.ReleasePtrProbe()->getReleaseRequests()->getAtRank(1)->getSaveAttribute().empty());
}

TEST(SimulatorRuntimeTest, ProcessPersistenceRoundTripPreservesConfigurationAndReconcilesInternals) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ProcessProbe source(model, "ProcessPersistSource");
    Queue* queue = new Queue(model, "ProcessPersistQueue");
    source.setAllocationType(Util::AllocationType::ValueAdded);
    source.setPriority(7u);
    source.setPriorityExpression("2+3");
    source.setQueueableItem(new QueueableItem(queue));
    source.addSeizeRequest(new SeizableItem(new Resource(model, "ProcessPersistResource"), "3", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY));
    source.setDelayExpression("4");
    source.setDelayTimeUnit(Util::TimeUnit::minute);
    Delay sink(model, "ProcessPersistSink");
    source.getConnectionManager()->insert(&sink);
    source.CreateInternalAndAttachedDataProbe();

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    ProcessProbe loaded(model, "ProcessPersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));

    EXPECT_EQ(loaded.getAllocationType(), Util::AllocationType::ValueAdded);
    EXPECT_EQ(loaded.getPriority(), 7u);
    EXPECT_EQ(loaded.getPriorityExpression(), "2+3");
    ASSERT_NE(loaded.getQueueableItem(), nullptr);
    EXPECT_EQ(loaded.getQueueableItem()->getQueueableName(), "ProcessPersistQueue");
    ASSERT_EQ(loaded.getSeizeRequests()->size(), 1u);
    EXPECT_EQ(loaded.getSeizeRequests()->getAtRank(0)->getResourceName(), "ProcessPersistResource");
    EXPECT_EQ(loaded.delayExpression(), "4");
    EXPECT_EQ(loaded.delayTimeUnit(), Util::TimeUnit::minute);
    ASSERT_NE(loaded.ReleasePtrProbe(), nullptr);
    EXPECT_EQ(loaded.SeizePtrProbe()->getSeizeRequests()->size(), loaded.ReleasePtrProbe()->getReleaseRequests()->size());
    EXPECT_EQ(fields.loadField("nextId", -1), sink.getId());
}

TEST(SimulatorRuntimeTest, ProcessCheckFailsWhenInternalCompositionIsInconsistent) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ProcessProbe process(model, "ProcessInvalid");
    process.setQueueableItem(new QueueableItem(new Queue(model, "ProcessInvalidQueue")));
    process.addSeizeRequest(new SeizableItem(new Resource(model, "ProcessInvalidResource"), "1", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY));
    process.CreateInternalAndAttachedDataProbe();
    process.SeizePtrProbe()->getConnectionManager()->connections()->clear();
    process.DelayPtrProbe()->getConnectionManager()->connections()->clear();
    process.ReleasePtrProbe()->getReleaseRequests()->clear();

    std::string errorMessage;
    EXPECT_FALSE(process.CheckProbe(errorMessage));
    EXPECT_FALSE(errorMessage.empty());
}

TEST(SimulatorRuntimeTest, ProcessCheckPassesWithMinimalValidConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ProcessProbe process(model, "ProcessValid");
    process.setQueueableItem(new QueueableItem(new Queue(model, "ProcessValidQueue")));
    process.addSeizeRequest(new SeizableItem(new Resource(model, "ProcessValidResource"), "1", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY));
    process.setDelayExpression("1");
    process.setDelayTimeUnit(Util::TimeUnit::second);
    process.CreateInternalAndAttachedDataProbe();

    std::string errorMessage;
    EXPECT_TRUE(process.CheckProbe(errorMessage)) << errorMessage;
}

TEST(SimulatorRuntimeTest, DISABLED_SearchQueueFindsEntityInRangeSavesRankAndRoutesToFoundPort) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SearchProbe search(model, "SearchFind");
    Queue queue(model, "SearchFindQueue");
    CollectorSinkComponentProbe notFoundSink(model, "SearchFindNotFound");
    CollectorSinkComponentProbe foundSink(model, "SearchFindFound");
    search.getConnectionManager()->insert(&notFoundSink);
    search.getConnectionManager()->insert(&foundSink);
    search.setSearchInType(Search::SearchInType::QUEUE);
    search.setSearchIn(&queue);
    search.setStartRank("1");
    search.setEndRank("3");
    search.setSearchCondition("1");
    search.setSaveFounRankAttribute("SearchFoundRankAttr");
    Attribute searchFoundRankAttr(model, "SearchFoundRankAttr");

    CollectorSinkComponentProbe producer(model, "SearchFindProducer");
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE0", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE1", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchFindQueueE2", true), 0.0, &producer));

    Entity* trigger = model->createEntity("SearchFindTrigger", true);
    search.DispatchEventProbe(trigger);

    EXPECT_EQ(trigger->getAttributeValue("SearchFoundRankAttr"), 1.0);
    EXPECT_EQ(notFoundSink.ReceivedEntities().size(), 0u);
    ASSERT_EQ(foundSink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(foundSink.ReceivedEntities().front(), trigger);
}

TEST(SimulatorRuntimeTest, DISABLED_SearchQueueNotFoundRoutesToPortZeroAndSavesZeroRank) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SearchProbe search(model, "SearchNotFound");
    Queue queue(model, "SearchNotFoundQueue");
    CollectorSinkComponentProbe notFoundSink(model, "SearchNotFoundOut0");
    CollectorSinkComponentProbe foundSink(model, "SearchNotFoundOut1");
    search.getConnectionManager()->insert(&notFoundSink);
    search.getConnectionManager()->insert(&foundSink);
    search.setSearchInType(Search::SearchInType::QUEUE);
    search.setSearchIn(&queue);
    search.setStartRank("0");
    search.setEndRank("2");
    search.setSearchCondition("0");
    search.setSaveFounRankAttribute("SearchNotFoundRankAttr");
    Attribute searchNotFoundRankAttr(model, "SearchNotFoundRankAttr");

    CollectorSinkComponentProbe producer(model, "SearchNotFoundProducer");
    queue.insertElement(new Waiting(model->createEntity("SearchNotFoundQueueE0", true), 0.0, &producer));
    queue.insertElement(new Waiting(model->createEntity("SearchNotFoundQueueE1", true), 0.0, &producer));

    Entity* trigger = model->createEntity("SearchNotFoundTrigger", true);
    search.DispatchEventProbe(trigger);

    EXPECT_EQ(trigger->getAttributeValue("SearchNotFoundRankAttr"), 0.0);
    ASSERT_EQ(notFoundSink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(notFoundSink.ReceivedEntities().front(), trigger);
    EXPECT_EQ(foundSink.ReceivedEntities().size(), 0u);
}

TEST(SimulatorRuntimeTest, SearchPersistenceRoundTripPreservesConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "SearchPersistQueue");
    SearchProbe source(model, "SearchPersistSource");
    source.setSearchInType(Search::SearchInType::QUEUE);
    source.setSearchIn(&queue);
    source.setStartRank("2");
    source.setEndRank("4");
    source.setSearchCondition("Entity.Value > 0");
    source.setSaveFounRankAttribute("SearchPersistFoundRank");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    SearchProbe loaded(model, "SearchPersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    ASSERT_NE(loaded.getSearchIn(), nullptr);
    EXPECT_EQ(loaded.getSearchInType(), Search::SearchInType::QUEUE);
    EXPECT_EQ(loaded.getStartRank(), "2");
    EXPECT_EQ(loaded.getEndRank(), "4");
    EXPECT_EQ(loaded.getSearchCondition(), "Entity.Value > 0");
    EXPECT_EQ(loaded.getSaveFounRankAttribute(), "SearchPersistFoundRank");
    EXPECT_EQ(loaded.getSearchInName(), "SearchPersistQueue");
}

TEST(SimulatorRuntimeTest, SearchCheckValidatesConditionSearchInAndMinimalQueueConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    SearchProbe invalidCondition(model, "SearchCheckInvalidCondition");
    invalidCondition.setSearchInType(Search::SearchInType::QUEUE);
    invalidCondition.setSearchIn(new Queue(model, "SearchCheckInvalidConditionQueue"));
    invalidCondition.setStartRank("0");
    invalidCondition.setEndRank("1");
    invalidCondition.setSearchCondition("1+");
    invalidCondition.setSaveFounRankAttribute("SearchCheckInvalidConditionAttr");
    std::string invalidConditionMessage;
    EXPECT_FALSE(invalidCondition.CheckProbe(invalidConditionMessage));
    EXPECT_FALSE(invalidConditionMessage.empty());

    SearchProbe missingSearchIn(model, "SearchCheckMissingSearchIn");
    missingSearchIn.setSearchInType(Search::SearchInType::QUEUE);
    missingSearchIn.setStartRank("0");
    missingSearchIn.setEndRank("1");
    missingSearchIn.setSearchCondition("1");
    missingSearchIn.setSaveFounRankAttribute("SearchCheckMissingSearchInAttr");
    std::string missingSearchInMessage;
    EXPECT_FALSE(missingSearchIn.CheckProbe(missingSearchInMessage));
    EXPECT_NE(missingSearchInMessage.find("SearchIn was not defined"), std::string::npos);

    SearchProbe valid(model, "SearchCheckValid");
    Attribute validSearchAttribute(model, "SearchCheckValidAttr");
    valid.setSearchInType(Search::SearchInType::QUEUE);
    valid.setSearchIn(new Queue(model, "SearchCheckValidQueue"));
    valid.setStartRank("0");
    valid.setEndRank("1");
    valid.setSearchCondition("1");
    valid.setSaveFounRankAttribute("SearchCheckValidAttr");
    std::string validMessage;
    EXPECT_TRUE(valid.CheckProbe(validMessage)) << validMessage;
}

TEST(SimulatorRuntimeTest, DISABLED_RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RemoveProbe remove(model, "RemoveSingleRank");
    Queue queue(model, "RemoveSingleRankQueue");
    CollectorSinkComponentProbe mainSink(model, "RemoveSingleRankMain");
    CollectorSinkComponentProbe removedSink(model, "RemoveSingleRankRemoved");
    remove.getConnectionManager()->insert(&mainSink);
    remove.getConnectionManager()->insert(&removedSink);
    remove.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    remove.setRemoveFrom(&queue);
    remove.setRemoveStartRank("1");
    remove.setRemoveEndRank("1");

    CollectorSinkComponentProbe producer(model, "RemoveSingleRankProducer");
    Entity* q0 = model->createEntity("RemoveSingleRankQ0", true);
    Entity* q1 = model->createEntity("RemoveSingleRankQ1", true);
    Entity* q2 = model->createEntity("RemoveSingleRankQ2", true);
    queue.insertElement(new Waiting(q0, 0.0, &producer));
    queue.insertElement(new Waiting(q1, 0.0, &producer));
    queue.insertElement(new Waiting(q2, 0.0, &producer));

    Entity* trigger = model->createEntity("RemoveSingleRankTrigger", true);
    remove.DispatchEventProbe(trigger);

    ASSERT_EQ(removedSink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(removedSink.ReceivedEntities().front(), q1);
    EXPECT_EQ(queue.size(), 2u);
    ASSERT_EQ(mainSink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(mainSink.ReceivedEntities().front(), trigger);
}

TEST(SimulatorRuntimeTest, DISABLED_RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RemoveProbe remove(model, "RemoveRange");
    Queue queue(model, "RemoveRangeQueue");
    CollectorSinkComponentProbe mainSink(model, "RemoveRangeMain");
    CollectorSinkComponentProbe removedSink(model, "RemoveRangeRemoved");
    remove.getConnectionManager()->insert(&mainSink);
    remove.getConnectionManager()->insert(&removedSink);
    remove.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    remove.setRemoveFrom(&queue);
    remove.setRemoveStartRank("1");
    remove.setRemoveEndRank("2");

    CollectorSinkComponentProbe producer(model, "RemoveRangeProducer");
    Entity* q0 = model->createEntity("RemoveRangeQ0", true);
    Entity* q1 = model->createEntity("RemoveRangeQ1", true);
    Entity* q2 = model->createEntity("RemoveRangeQ2", true);
    Entity* q3 = model->createEntity("RemoveRangeQ3", true);
    queue.insertElement(new Waiting(q0, 0.0, &producer));
    queue.insertElement(new Waiting(q1, 0.0, &producer));
    queue.insertElement(new Waiting(q2, 0.0, &producer));
    queue.insertElement(new Waiting(q3, 0.0, &producer));

    Entity* trigger = model->createEntity("RemoveRangeTrigger", true);
    remove.DispatchEventProbe(trigger);

    ASSERT_EQ(removedSink.ReceivedEntities().size(), 2u);
    EXPECT_EQ(removedSink.ReceivedEntities().at(0), q1);
    EXPECT_EQ(removedSink.ReceivedEntities().at(1), q2);
    EXPECT_EQ(queue.size(), 2u);
    EXPECT_EQ(queue.getAtRank(0)->getEntity(), q0);
    EXPECT_EQ(queue.getAtRank(1)->getEntity(), q3);
    ASSERT_EQ(mainSink.ReceivedEntities().size(), 1u);
    EXPECT_EQ(mainSink.ReceivedEntities().front(), trigger);
}

TEST(SimulatorRuntimeTest, RemovePersistenceRoundTripPreservesConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    Queue queue(model, "RemovePersistQueue");
    RemoveProbe source(model, "RemovePersistSource");
    source.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    source.setRemoveFrom(&queue);
    source.setRemoveStartRank("3");
    source.setRemoveEndRank("5");

    FakeModelPersistenceRuntime persistence;
    PersistenceRecord fields(persistence);
    source.SaveInstanceProbe(&fields, true);

    RemoveProbe loaded(model, "RemovePersistLoaded");
    ASSERT_TRUE(loaded.LoadInstanceProbe(&fields));
    ASSERT_NE(loaded.getRemoveFrom(), nullptr);
    EXPECT_EQ(loaded.getRemoveFromType(), Remove::RemoveFromType::QUEUE);
    EXPECT_EQ(loaded.getRemoveStartRank(), "3");
    EXPECT_EQ(loaded.getRemoveEndRank(), "5");
    EXPECT_EQ(loaded.getRemoveFrom()->getName(), "RemovePersistQueue");
}

TEST(SimulatorRuntimeTest, RemoveCheckValidatesRankExpressionsAndMinimalQueueConfiguration) {
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    RemoveProbe invalidStart(model, "RemoveCheckInvalidStart");
    invalidStart.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    invalidStart.setRemoveFrom(new Queue(model, "RemoveCheckInvalidStartQueue"));
    invalidStart.setRemoveStartRank("1+");
    invalidStart.setRemoveEndRank("1");
    std::string invalidStartMessage;
    EXPECT_FALSE(invalidStart.CheckProbe(invalidStartMessage));
    EXPECT_FALSE(invalidStartMessage.empty());

    RemoveProbe invalidEnd(model, "RemoveCheckInvalidEnd");
    invalidEnd.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    invalidEnd.setRemoveFrom(new Queue(model, "RemoveCheckInvalidEndQueue"));
    invalidEnd.setRemoveStartRank("0");
    invalidEnd.setRemoveEndRank("2+");
    std::string invalidEndMessage;
    EXPECT_FALSE(invalidEnd.CheckProbe(invalidEndMessage));
    EXPECT_FALSE(invalidEndMessage.empty());

    RemoveProbe valid(model, "RemoveCheckValid");
    valid.setRemoveFromType(Remove::RemoveFromType::QUEUE);
    valid.setRemoveFrom(new Queue(model, "RemoveCheckValidQueue"));
    valid.setRemoveStartRank("0");
    valid.setRemoveEndRank("0");
    std::string validMessage;
    EXPECT_TRUE(valid.CheckProbe(validMessage)) << validMessage;
}
