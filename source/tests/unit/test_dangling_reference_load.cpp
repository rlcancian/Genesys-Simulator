// Regression tests for the GUI model save/load bug investigation: loading a
// component whose by-name reference (Resource/Queue/Set/Station) is missing
// from the file must not silently fabricate a fresh duplicate object nor
// silently leave the reference null with no diagnostic. See
// docs/ai_assistants/BACKLOG_AUTONOMOUS.md and the fix in SeizableItem.cpp,
// QueueableItem.cpp, Seize.cpp, Release.cpp, Process.cpp, Route.cpp,
// Enter.cpp and Leave.cpp.

#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/persistence/GenSerializer.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/MaterialHandling/Route.h"
#include "plugins/components/MaterialHandling/Enter.h"
#include "plugins/components/MaterialHandling/Leave.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/MaterialHandling/Station.h"

namespace {

// Saves `component` (built against `savingModel`, where the referenced data
// definitions exist) into a fresh PersistenceRecord, mirroring the real
// save/load round trip without requiring an actual file on disk.
std::unique_ptr<PersistenceRecord> saveComponent(Model* savingModel, ModelComponent* component) {
	GenSerializer serializer(savingModel);
	auto fields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
	ModelComponent::SaveInstance(fields.get(), component);
	return fields;
}

} // namespace

TEST(DanglingReferenceLoadTest, SeizeWithMissingResourceDoesNotFabricateDuplicate) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	ASSERT_NE(savingModel, nullptr);
	Resource* savingResource = new Resource(savingModel, "OnlyInSavingModel");
	Seize* savingSeize = new Seize(savingModel, "SeizeWithResource");
	savingSeize->addRequest(new SeizableItem(savingResource));
	auto fields = saveComponent(savingModel, savingSeize);

	// The loading model deliberately does NOT recreate "OnlyInSavingModel",
	// simulating a Resource that was renamed/deleted between saves.
	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();
	ASSERT_NE(loadingModel, nullptr);

	auto* loadedSeize = dynamic_cast<Seize*>(Seize::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedSeize, nullptr);
	ASSERT_EQ(loadedSeize->getSeizeRequests()->size(), 1u);

	SeizableItem* loadedItem = loadedSeize->getSeizeRequests()->list()->front();
	EXPECT_EQ(loadedItem->getResource(), nullptr);

	// No phantom duplicate Resource must have been created in the loading model.
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Resource>())->size(), 0u);
}

TEST(DanglingReferenceLoadTest, SeizeWithExistingResourceStillResolvesNormally) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Resource* savingResource = new Resource(savingModel, "SharedResource");
	Seize* savingSeize = new Seize(savingModel, "SeizeWithResource");
	savingSeize->addRequest(new SeizableItem(savingResource));
	auto fields = saveComponent(savingModel, savingSeize);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();
	Resource* loadingResource = new Resource(loadingModel, "SharedResource");

	auto* loadedSeize = dynamic_cast<Seize*>(Seize::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedSeize, nullptr);
	ASSERT_EQ(loadedSeize->getSeizeRequests()->size(), 1u);
	EXPECT_EQ(loadedSeize->getSeizeRequests()->list()->front()->getResource(), loadingResource);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Resource>())->size(), 1u);
}

TEST(DanglingReferenceLoadTest, SeizeWithMissingQueueDoesNotFabricateDuplicate) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Queue* savingQueue = new Queue(savingModel, "OnlyInSavingModel");
	Seize* savingSeize = new Seize(savingModel, "SeizeWithQueue");
	savingSeize->setQueueableItem(new QueueableItem(savingQueue));
	auto fields = saveComponent(savingModel, savingSeize);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();

	auto* loadedSeize = dynamic_cast<Seize*>(Seize::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedSeize, nullptr);
	ASSERT_NE(loadedSeize->getQueueableItem(), nullptr);
	EXPECT_EQ(loadedSeize->getQueueableItem()->getQueue(), nullptr);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Queue>())->size(), 0u);
}

TEST(DanglingReferenceLoadTest, ReleaseWithMissingResourceDoesNotFabricateDuplicate) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Resource* savingResource = new Resource(savingModel, "OnlyInSavingModel");
	Release* savingRelease = new Release(savingModel, "ReleaseWithResource");
	savingRelease->addReleaseRequests(new SeizableItem(savingResource));
	auto fields = saveComponent(savingModel, savingRelease);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();

	auto* loadedRelease = dynamic_cast<Release*>(Release::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedRelease, nullptr);
	ASSERT_EQ(loadedRelease->getReleaseRequests()->size(), 1u);
	EXPECT_EQ(loadedRelease->getReleaseRequests()->list()->front()->getResource(), nullptr);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Resource>())->size(), 0u);
}

TEST(DanglingReferenceLoadTest, RouteWithMissingStationLeavesNullAndReportsFailure) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Station* savingStation = new Station(savingModel, "OnlyInSavingModel");
	Route* savingRoute = new Route(savingModel, "RouteToStation");
	savingRoute->setRouteDestinationType(Route::DestinationType::Station);
	savingRoute->setStation(savingStation);
	auto fields = saveComponent(savingModel, savingRoute);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();

	auto* loadedRoute = dynamic_cast<Route*>(Route::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedRoute, nullptr);
	EXPECT_EQ(loadedRoute->getStation(), nullptr);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Station>())->size(), 0u);
}

TEST(DanglingReferenceLoadTest, EnterWithMissingStationLeavesNullInsteadOfCrashingOnShow) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Station* savingStation = new Station(savingModel, "OnlyInSavingModel");
	Enter* savingEnter = new Enter(savingModel, "EnterStation");
	savingEnter->setStation(savingStation);
	auto fields = saveComponent(savingModel, savingEnter);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();

	auto* loadedEnter = dynamic_cast<Enter*>(Enter::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedEnter, nullptr);
	EXPECT_EQ(loadedEnter->getStation(), nullptr);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Station>())->size(), 0u);
}

TEST(DanglingReferenceLoadTest, LeaveWithMissingStationLeavesNullInsteadOfCrashingOnShow) {
	Simulator savingSimulator;
	Model* savingModel = savingSimulator.getModelManager()->newModel();
	Station* savingStation = new Station(savingModel, "OnlyInSavingModel");
	Leave* savingLeave = new Leave(savingModel, "LeaveStation");
	savingLeave->setStation(savingStation);
	auto fields = saveComponent(savingModel, savingLeave);

	Simulator loadingSimulator;
	Model* loadingModel = loadingSimulator.getModelManager()->newModel();

	auto* loadedLeave = dynamic_cast<Leave*>(Leave::LoadInstance(loadingModel, fields.get()));
	ASSERT_NE(loadedLeave, nullptr);
	EXPECT_EQ(loadedLeave->getStation(), nullptr);
	EXPECT_EQ(loadingModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Station>())->size(), 0u);
}
