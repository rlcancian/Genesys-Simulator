#include "animations/AnimationPlaceholder.h"
#include "controllers/SimulationEventController.h"
#include "extensions/GuiExtensionManager.h"
#include "extensions/GuiExtensionPluginCatalog.h"
#include "graphicals/ModelGraphicsScene.h"

#include "kernel/simulator/Event.h"
#include "kernel/simulator/OnEventManager.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/essentialPlugins/StatisticsCollector.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/auxiliar/SeizableItem.h"
#include "plugins/components/MaterialHandling/Enter.h"
#include "plugins/components/MaterialHandling/Leave.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/MaterialHandling/Station.h"

#include <gtest/gtest.h>

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>

#include <memory>
#include <string>

namespace {

class AnimationDispatchTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_NE(_pluginManager, nullptr);
        _pluginManager->autoInsertPlugins();

        _model = _simulator.getModelManager()->newModel();
        ASSERT_NE(_model, nullptr);

        _scene.setSimulator(&_simulator);
        _context.simulator = &_simulator;
        _context.mainWindow = &_mainWindow;
        _context.graphicsScene = &_scene;

        _extensionManager.setPlugins(GuiExtensionPluginCatalog::resolvedPlugins());
        _extensionManager.rebuild(_context);
        _scene.setGuiExtensionManager(&_extensionManager);
    }

    Model* model() const { return _model; }
    ModelGraphicsScene* scene() { return &_scene; }
    GuiExtensionManager* extensionManager() { return &_extensionManager; }

    AnimationPlaceholder* addPlaceholder(const std::string& animationType, const QString& targetName) {
        AnimationPlaceholder* placeholder = _extensionManager.createAnimationPlaceholder(animationType);
        if (placeholder == nullptr) {
            return nullptr;
        }
        placeholder->setTargetName(targetName);
        placeholder->setRect(0.0, 0.0, 80.0, 40.0);
        _scene.addItem(placeholder);
        _scene.addDrawingAnimation(placeholder);
        return placeholder;
    }

    AnimationStatistics* addStatisticsPlaceholder(const QString& targetName) {
        auto* stats = new AnimationStatistics();
        stats->setTargetName(targetName);
        stats->setRect(0.0, 0.0, 80.0, 40.0);
        _scene.addItem(stats);
        _scene.addDrawingAnimation(stats);
        return stats;
    }

protected:
    Simulator _simulator;
    PluginManager* _pluginManager = _simulator.getPluginManager();
    Model* _model = nullptr;
    QMainWindow _mainWindow;
    ModelGraphicsScene _scene{0, 0, 2000, 2000};
    GuiExtensionManager _extensionManager{&_mainWindow};
    GuiExtensionRuntimeContext _context;
};

bool hasAnimationContribution(const GuiExtensionManager& manager, const std::string& animationType) {
    for (const GuiAnimationContribution& contribution : manager.animationContributions()) {
        if (contribution.animationType == animationType) {
            return true;
        }
    }
    return false;
}

struct SimulationControllerUi {
    QLabel replicationLabel;
    QProgressBar progressBar;
    QTableWidget simulationEventsTable;
    QTableWidget entitiesTable;
    QTableWidget variablesTable;
    QTextEdit simulationText;
    QTextEdit reportsText;
    QTabWidget centralTabWidget;
    QAction graphicalSimulationAction;
    QAction animationEnabledAction;
    bool modelChecked = true;

    explicit SimulationControllerUi(QMainWindow* mainWindow)
        : graphicalSimulationAction(mainWindow)
        , animationEnabledAction(mainWindow) {
        graphicalSimulationAction.setCheckable(true);
        graphicalSimulationAction.setChecked(true);
        animationEnabledAction.setCheckable(true);
        animationEnabledAction.setChecked(true);
    }
};

std::unique_ptr<SimulationEventController> makeSimulationEventController(
    Simulator* simulator,
    ModelGraphicsScene* scene,
    SimulationControllerUi* ui)
{
    return std::make_unique<SimulationEventController>(
        simulator,
        scene,
        nullptr,
        &ui->replicationLabel,
        &ui->progressBar,
        &ui->simulationEventsTable,
        &ui->entitiesTable,
        &ui->variablesTable,
        &ui->simulationText,
        &ui->reportsText,
        &ui->centralTabWidget,
        &ui->graphicalSimulationAction,
        &ui->animationEnabledAction,
        &ui->modelChecked,
        0,
        SimulationEventController::Callbacks{
            []() {},
            [](SimulationEvent*) {},
            [](bool) {},
            [](bool) {},
            [](SimulationEvent*) {},
        });
}

class SimulationEventControllerE2EFixture : public AnimationDispatchTestFixture {
protected:
    void SetUp() override {
        AnimationDispatchTestFixture::SetUp();
        _controllerUi = std::make_unique<SimulationControllerUi>(&_mainWindow);
        _controller = makeSimulationEventController(&_simulator, scene(), _controllerUi.get());
    }

    SimulationEventController* controller() { return _controller.get(); }
    SimulationControllerUi* controllerUi() { return _controllerUi.get(); }

    SimulationEvent* makeSimulationEvent(Event* kernelEvent) {
        SimulationEvent* simulationEvent = SimulationEvent::NewUnsetInstance();
        simulationEvent->setCurrentEvent(kernelEvent);
        return simulationEvent;
    }

    std::unique_ptr<SimulationControllerUi> _controllerUi;
    std::unique_ptr<SimulationEventController> _controller;
};

} // namespace

TEST_F(AnimationDispatchTestFixture, AnimationPluginsRegisterQueueResourceAndStationContributions) {
    EXPECT_TRUE(hasAnimationContribution(*extensionManager(), "Queue"));
    EXPECT_TRUE(hasAnimationContribution(*extensionManager(), "Resource"));
    EXPECT_TRUE(hasAnimationContribution(*extensionManager(), "Station"));
    EXPECT_GE(extensionManager()->animationContributions().size(), 3u);
}

TEST_F(AnimationDispatchTestFixture, CreateAnimationPlaceholderReturnsTypedPlaceholders) {
    AnimationPlaceholder* queue = extensionManager()->createAnimationPlaceholder("Queue");
    AnimationPlaceholder* resource = extensionManager()->createAnimationPlaceholder("Resource");
    AnimationPlaceholder* station = extensionManager()->createAnimationPlaceholder("Station");
    AnimationPlaceholder* unknown = extensionManager()->createAnimationPlaceholder("UnknownType");

    ASSERT_NE(queue, nullptr);
    ASSERT_NE(resource, nullptr);
    ASSERT_NE(station, nullptr);
    EXPECT_EQ(unknown, nullptr);

    EXPECT_EQ(queue->getAnimationType(), QStringLiteral("Queue"));
    EXPECT_EQ(resource->getAnimationType(), QStringLiteral("Resource"));
    EXPECT_EQ(station->getAnimationType(), QStringLiteral("Station"));
    EXPECT_EQ(queue->overlayBusyCount(), -1);

    delete queue;
    delete resource;
    delete station;
}

TEST_F(AnimationDispatchTestFixture, StatisticsCollectorLinksWhenTargetMatches) {
    auto* collectorDefinition = new StatisticsCollector(model(), "WaitTimeCollector");
    ASSERT_NE(collectorDefinition, nullptr);
    ASSERT_NE(collectorDefinition->getStatistics(), nullptr);

    AnimationStatistics* stats = addStatisticsPlaceholder(QStringLiteral("WaitTimeCollector"));
    ASSERT_NE(stats, nullptr);
    EXPECT_EQ(stats->getCollector(), nullptr);

    scene()->setStatisticsCollectors();

    EXPECT_NE(stats->getCollector(), nullptr);
    EXPECT_EQ(stats->getCollector(), collectorDefinition->getStatistics()->getCollector());
}

TEST_F(AnimationDispatchTestFixture, StatisticsCollectorClearsWhenTargetDoesNotMatch) {
    new StatisticsCollector(model(), "WaitTimeCollector");
    AnimationStatistics* stats = addStatisticsPlaceholder(QStringLiteral("MissingCollector"));

    scene()->setStatisticsCollectors();

    EXPECT_EQ(stats->getCollector(), nullptr);
}

TEST_F(AnimationDispatchTestFixture, ResourceOverlayIncrementsOnSeizeMove) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    seize->addRequest(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);
    EXPECT_EQ(placeholder->overlayBusyCount(), 0);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);

    EXPECT_EQ(placeholder->overlayBusyCount(), 1);
}

TEST_F(AnimationDispatchTestFixture, ResourceOverlayDecrementsOnReleaseMove) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    auto* release = new Release(model(), "Release_1");
    seize->addRequest(new SeizableItem(resource));
    release->addReleaseRequests(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    ASSERT_EQ(placeholder->overlayBusyCount(), 1);

    scene()->notifyEntityMovePluginAnimations(release, nullptr);

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);
}

TEST_F(AnimationDispatchTestFixture, ResourceOverlayTracksMultipleSeizeAndReleaseMoves) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    auto* release = new Release(model(), "Release_1");
    seize->addRequest(new SeizableItem(resource));
    release->addReleaseRequests(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    EXPECT_EQ(placeholder->overlayBusyCount(), 2);

    scene()->notifyEntityMovePluginAnimations(release, nullptr);
    EXPECT_EQ(placeholder->overlayBusyCount(), 1);
}

TEST_F(AnimationDispatchTestFixture, ResourceOverlayIgnoresMismatchedTargetName) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    seize->addRequest(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("OtherMachine"));
    ASSERT_NE(placeholder, nullptr);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);
}

TEST_F(AnimationDispatchTestFixture, StationOverlayIncrementsOnEnterProcess) {
    auto* station = new Station(model(), "station1");
    auto* enter = new Enter(model(), "Enter_1");
    enter->setStation(station);

    AnimationPlaceholder* placeholder = addPlaceholder("Station", QStringLiteral("station1"));
    ASSERT_NE(placeholder, nullptr);
    EXPECT_EQ(placeholder->overlayBusyCount(), 0);

    scene()->notifyAfterProcessPluginAnimations(enter, nullptr);

    EXPECT_EQ(placeholder->overlayBusyCount(), 1);
}

TEST_F(AnimationDispatchTestFixture, StationOverlayDecrementsOnLeaveProcess) {
    auto* station = new Station(model(), "station1");
    auto* enter = new Enter(model(), "Enter_1");
    auto* leave = new Leave(model(), "Leave_1");
    enter->setStation(station);
    leave->setStation(station);

    AnimationPlaceholder* placeholder = addPlaceholder("Station", QStringLiteral("station1"));
    ASSERT_NE(placeholder, nullptr);

    scene()->notifyAfterProcessPluginAnimations(enter, nullptr);
    ASSERT_EQ(placeholder->overlayBusyCount(), 1);

    scene()->notifyAfterProcessPluginAnimations(leave, nullptr);

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);
}

TEST_F(AnimationDispatchTestFixture, ClearAnimationsValuesResetsPluginOverlayCounts) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    seize->addRequest(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    ASSERT_EQ(placeholder->overlayBusyCount(), 2);

    scene()->clearAnimationsValues();

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);
}

TEST_F(AnimationDispatchTestFixture, StatisticsCollectorRelinksAfterTargetChange) {
    auto* firstCollector = new StatisticsCollector(model(), "FirstCollector");
    auto* secondCollector = new StatisticsCollector(model(), "SecondCollector");
    ASSERT_NE(firstCollector, nullptr);
    ASSERT_NE(secondCollector, nullptr);

    AnimationStatistics* stats = addStatisticsPlaceholder(QStringLiteral("FirstCollector"));
    scene()->setStatisticsCollectors();
    ASSERT_NE(stats->getCollector(), nullptr);
    EXPECT_EQ(stats->getCollector(), firstCollector->getStatistics()->getCollector());

    stats->setTargetName(QStringLiteral("SecondCollector"));
    scene()->setStatisticsCollectors();

    EXPECT_NE(stats->getCollector(), nullptr);
    EXPECT_EQ(stats->getCollector(), secondCollector->getStatistics()->getCollector());
}

TEST_F(SimulationEventControllerE2EFixture, SimulationStartHandlerResetsOverlaysAndLinksStatistics) {
    auto* collectorDefinition = new StatisticsCollector(model(), "WaitTimeCollector");
    ASSERT_NE(collectorDefinition, nullptr);

    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    seize->addRequest(new SeizableItem(resource));

    AnimationPlaceholder* resourcePlaceholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    AnimationStatistics* stats = addStatisticsPlaceholder(QStringLiteral("WaitTimeCollector"));
    ASSERT_NE(resourcePlaceholder, nullptr);
    ASSERT_NE(stats, nullptr);

    scene()->notifyEntityMovePluginAnimations(seize, nullptr);
    ASSERT_EQ(resourcePlaceholder->overlayBusyCount(), 1);
    ASSERT_EQ(stats->getCollector(), nullptr);

    SimulationEvent* startEvent = SimulationEvent::NewUnsetInstance();
    controller()->onSimulationStartHandler(startEvent);
    SimulationEvent::DeleteInstance(startEvent);

    EXPECT_EQ(resourcePlaceholder->overlayBusyCount(), 0);
    EXPECT_NE(stats->getCollector(), nullptr);
    EXPECT_EQ(stats->getCollector(), collectorDefinition->getStatistics()->getCollector());
}

TEST_F(SimulationEventControllerE2EFixture, SimulationEventControllerUpdatesResourceOverlayOnSeizeMove) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    auto* release = new Release(model(), "Release_1");
    seize->addRequest(new SeizableItem(resource));
    release->addReleaseRequests(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);

    Entity* entity = model()->createEntity("Entity_1");
    Event seizeEvent(0.0, entity, seize);
    SimulationEvent* seizeSimulationEvent = makeSimulationEvent(&seizeEvent);
    controller()->onMoveEntityEvent(seizeSimulationEvent);
    EXPECT_EQ(placeholder->overlayBusyCount(), 1);

    Event releaseEvent(1.0, entity, release);
    SimulationEvent* releaseSimulationEvent = makeSimulationEvent(&releaseEvent);
    controller()->onMoveEntityEvent(releaseSimulationEvent);

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);

    SimulationEvent::DeleteInstance(seizeSimulationEvent);
    SimulationEvent::DeleteInstance(releaseSimulationEvent);
}

TEST_F(SimulationEventControllerE2EFixture, SimulationEventControllerUpdatesStationOverlayOnAfterProcess) {
    auto* station = new Station(model(), "station1");
    auto* enter = new Enter(model(), "Enter_1");
    auto* leave = new Leave(model(), "Leave_1");
    enter->setStation(station);
    leave->setStation(station);

    AnimationPlaceholder* placeholder = addPlaceholder("Station", QStringLiteral("station1"));
    ASSERT_NE(placeholder, nullptr);

    Entity* entity = model()->createEntity("Entity_1");
    Event enterEvent(0.0, entity, enter);
    SimulationEvent* enterSimulationEvent = makeSimulationEvent(&enterEvent);
    controller()->onAfterProcessEvent(enterSimulationEvent);
    EXPECT_EQ(placeholder->overlayBusyCount(), 1);

    Event leaveEvent(1.0, entity, leave);
    SimulationEvent* leaveSimulationEvent = makeSimulationEvent(&leaveEvent);
    controller()->onAfterProcessEvent(leaveSimulationEvent);
    EXPECT_EQ(placeholder->overlayBusyCount(), 0);

    SimulationEvent::DeleteInstance(enterSimulationEvent);
    SimulationEvent::DeleteInstance(leaveSimulationEvent);
}

TEST_F(SimulationEventControllerE2EFixture, MoveEntityEventSkippedWhenGraphicalSimulationDisabled) {
    auto* resource = new Resource(model(), "Machine_1");
    auto* seize = new Seize(model(), "Seize_1");
    seize->addRequest(new SeizableItem(resource));

    AnimationPlaceholder* placeholder = addPlaceholder("Resource", QStringLiteral("Machine_1"));
    ASSERT_NE(placeholder, nullptr);

    controllerUi()->graphicalSimulationAction.setChecked(false);

    Entity* entity = model()->createEntity("Entity_1");
    Event seizeEvent(0.0, entity, seize);
    SimulationEvent* seizeSimulationEvent = makeSimulationEvent(&seizeEvent);
    controller()->onMoveEntityEvent(seizeSimulationEvent);

    EXPECT_EQ(placeholder->overlayBusyCount(), 0);

    SimulationEvent::DeleteInstance(seizeSimulationEvent);
}

TEST(GuiAnimationDispatch, NotifyWithNullExtensionManagerDoesNotCrash) {
    Simulator simulator;
    PluginManager* pluginManager = simulator.getPluginManager();
    ASSERT_NE(pluginManager, nullptr);
    pluginManager->autoInsertPlugins();

    Model* model = simulator.getModelManager()->newModel();
    ASSERT_NE(model, nullptr);

    ModelGraphicsScene scene(0, 0, 2000, 2000);
    scene.setSimulator(&simulator);
    scene.setGuiExtensionManager(nullptr);

    auto* resource = new Resource(model, "Machine_1");
    auto* seize = new Seize(model, "Seize_1");
    auto* station = new Station(model, "station1");
    auto* enter = new Enter(model, "Enter_1");
    seize->addRequest(new SeizableItem(resource));
    enter->setStation(station);

    scene.notifyEntityMovePluginAnimations(seize, nullptr);
    scene.notifyAfterProcessPluginAnimations(enter, nullptr);
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
