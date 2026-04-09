#include "SimulationEventController.h"

#include "mainwindow.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "animations/AnimationTimer.h"
#include "animations/AnimationTransition.h"

#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/util/Util.h"

#include <QAction>
#include <QCoreApplication>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>

#include <string>
#include <utility>

namespace {
// Select and detach the paused-animation list that can be safely resumed.
static QList<AnimationTransition*>* takePausedAnimationListForResume(
    QMap<Event*, QList<AnimationTransition*>*>* pausedAnimationsMap,
    Event* currentEvent) {
    if (!pausedAnimationsMap || pausedAnimationsMap->empty()) {
        return nullptr;
    }

    if (currentEvent && pausedAnimationsMap->contains(currentEvent)) {
        return pausedAnimationsMap->take(currentEvent);
    }

    if (pausedAnimationsMap->size() == 1) {
        auto it = pausedAnimationsMap->begin();
        Event* onlyKey = it.key();
        return pausedAnimationsMap->take(onlyKey);
    }

    return nullptr;
}

// Release one paused-animation list and optionally destroy its animations.
static void cleanupPausedAnimationList(QList<AnimationTransition*>* pausedAnimations, bool destroyAnimations) {
    if (!pausedAnimations) {
        return;
    }

    if (destroyAnimations) {
        // Stop each paused transition and schedule Qt-safe destruction.
        for (AnimationTransition* animation : *pausedAnimations) {
            if (animation) {
                animation->stopAnimation();
                animation->deleteLater();
            }
        }
    }

    pausedAnimations->clear();
    delete pausedAnimations;
}

// Release all paused-animation lists in the map and then clear the map keys.
static void cleanupPausedAnimationMap(QMap<Event*, QList<AnimationTransition*>*>* pausedAnimationsMap,
                                      bool destroyAnimations) {
    if (!pausedAnimationsMap) {
        return;
    }

    for (auto it = pausedAnimationsMap->begin(); it != pausedAnimationsMap->end(); ++it) {
        cleanupPausedAnimationList(it.value(), destroyAnimations);
    }

    pausedAnimationsMap->clear();
}
} // namespace

// Build the simulation-event controller with narrow simulator/view/widget dependencies.
SimulationEventController::SimulationEventController(Simulator* simulator,
                                                     ModelGraphicsScene* scene,
                                                     ModelGraphicsView* graphicsView,
                                                     QLabel* replicationLabel,
                                                     QProgressBar* simulationProgressBar,
                                                     QTableWidget* simulationEventsTable,
                                                     QTableWidget* entitiesTable,
                                                     QTableWidget* variablesTable,
                                                     QTextEdit* simulationText,
                                                     QTextEdit* reportsText,
                                                     QTabWidget* centralTabWidget,
                                                     QAction* activateGraphicalSimulation,
                                                     bool* modelChecked,
                                                     int tabCentralReportsIndex,
                                                     Callbacks callbacks)
    : _simulator(simulator),
      _scene(scene),
      _graphicsView(graphicsView),
      _replicationLabel(replicationLabel),
      _simulationProgressBar(simulationProgressBar),
      _simulationEventsTable(simulationEventsTable),
      _entitiesTable(entitiesTable),
      _variablesTable(variablesTable),
      _simulationText(simulationText),
      _reportsText(reportsText),
      _centralTabWidget(centralTabWidget),
      _activateGraphicalSimulation(activateGraphicalSimulation),
      _modelChecked(modelChecked),
      _tabCentralReportsIndex(tabCentralReportsIndex),
      _callbacks(std::move(callbacks)) {
}

// Preserve legacy model-check success behavior without broad MainWindow coupling.
void SimulationEventController::onModelCheckSuccessHandler(ModelEvent* re) const {
    Model* model = _simulator->getModelManager()->current();
    if (_simulator->getModelManager()->current() == re->getModel()) {
        ModelDataManager* dm = model->getDataManager();
        ModelGraphicsView* modelGraphView = _graphicsView;
        Q_UNUSED(modelGraphView)
        for (auto elemclassname : *dm->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* elem : *dm->getDataDefinitionList(elemclassname)->list()) {
                Util::identification id = elem->getId();
                Q_UNUSED(id)
            }
        }
    }
}

// Preserve replication-start UI updates and event-table append behavior.
void SimulationEventController::onReplicationStartHandler(SimulationEvent* re) const {
    ModelSimulation* sim = _simulator->getModelManager()->current()->getSimulation();
    QString text = QString::fromStdString(std::to_string(sim->getCurrentReplicationNumber())) + "/" +
                   QString::fromStdString(std::to_string(sim->getNumberOfReplications()));
    _replicationLabel->setText(text);
    int row = _simulationEventsTable->rowCount();
    _simulationEventsTable->setRowCount(row + 1);
    QTableWidgetItem* newItem = new QTableWidgetItem(QString::fromStdString(
        "Replication " + std::to_string(re->getCurrentReplicationNumber())));
    _simulationEventsTable->setItem(row, 2, newItem);

    QCoreApplication::processEvents();
}

// Preserve simulation-start setup, clears and conversion factor update behavior.
void SimulationEventController::onSimulationStartHandler(SimulationEvent* re) const {
    Q_UNUSED(re)
    _callbacks.actualizeActions();
    _simulationProgressBar->setMaximum(
        _simulator->getModelManager()->current()->getSimulation()->getReplicationLength());
    _simulationEventsTable->setRowCount(0);
    _entitiesTable->setRowCount(0);
    _variablesTable->setRowCount(0);
    _simulationText->clear();
    _reportsText->clear();

    Util::TimeUnit replicationBaseTimeUnit =
        _simulator->getModelManager()->current()->getSimulation()->getReplicationBaseTimeUnit();
    double conversionFactorToSeconds = Util::TimeUnitConvert(replicationBaseTimeUnit, Util::TimeUnit(5));
    AnimationTimer::setConversionFactorToSeconds(conversionFactorToSeconds);

    QCoreApplication::processEvents();
}

// Preserve simulation-paused UI reaction behavior.
void SimulationEventController::onSimulationPausedHandler(SimulationEvent* re) const {
    Q_UNUSED(re)
    _callbacks.actualizeActions();
    QCoreApplication::processEvents();
}

// Preserve simulation-resume animation continuation behavior.
void SimulationEventController::onSimulationResumeHandler(SimulationEvent* re) const {
    _callbacks.actualizeActions();

    // Resume only a deterministically selected paused list detached from the map.
    QMap<Event*, QList<AnimationTransition*>*>* pausedAnimationsMap = _scene->getAnimationPaused();
    Event* currentEvent = re ? re->getCurrentEvent() : nullptr;
    QList<AnimationTransition*>* pausedAnimations =
        takePausedAnimationListForResume(pausedAnimationsMap, currentEvent);
    if (pausedAnimations) {
        // Replay each paused transition with the current resume event context.
        for (AnimationTransition* animation : *pausedAnimations) {
            if (animation) {
                _scene->runAnimateTransition(animation, currentEvent, true);
            }
        }
        cleanupPausedAnimationList(pausedAnimations, false);
    }

    QCoreApplication::processEvents();
}

// Preserve simulation-end cleanup, tab switch and model-check flag reset behavior.
void SimulationEventController::onSimulationEndHandler(SimulationEvent* re) const {
    Q_UNUSED(re)
    // Destroy all paused-animation lists and remaining paused animations before ending.
    cleanupPausedAnimationMap(_scene->getAnimationPaused(), true);
    _callbacks.actualizeActions();
    _centralTabWidget->setCurrentIndex(_tabCentralReportsIndex);
    for (unsigned int i = 0; i < 50; i++) {
        QCoreApplication::processEvents();
    }

    _scene->clearAnimationsQueue();

    *_modelChecked = false;
}

// Preserve process-event UI updates and delegated debug/graphical refresh behavior.
void SimulationEventController::onProcessEventHandler(SimulationEvent* re) const {
    _simulationProgressBar->setValue(_simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
    _callbacks.actualizeSimulationEvents(re);
    _callbacks.actualizeDebugEntities(false);
    _callbacks.actualizeDebugVariables(false);
    _callbacks.actualizeGraphicalModel(re);
    QCoreApplication::processEvents();
}

// Keep entity-create handler behavior unchanged as an intentional no-op.
void SimulationEventController::onEntityCreateHandler(SimulationEvent* re) const {
    Q_UNUSED(re)
}

// Keep entity-remove handler behavior unchanged as an intentional no-op.
void SimulationEventController::onEntityRemoveHandler(SimulationEvent* re) const {
    Q_UNUSED(re)
}

// Preserve entity-move animation pipeline behavior.
void SimulationEventController::onMoveEntityEvent(SimulationEvent* re) const {
    _scene->animateCounter();
    _scene->animateVariable();

    if (re) {
        if (re->getCurrentEvent()) {
            if (re->getCurrentEvent()->getComponent()) {
                ModelComponent* source = re->getCurrentEvent()->getComponent();
                ModelComponent* destination = re->getDestinationComponent();

                _scene->animateQueueRemove(source);

                _scene->animateTransition(
                    source,
                    destination,
                    _activateGraphicalSimulation->isChecked(),
                    re->getCurrentEvent());
            }
        }
    }
}

// Preserve after-process animation update behavior.
void SimulationEventController::onAfterProcessEvent(SimulationEvent* re) const {
    Q_UNUSED(re)
    _scene->animateCounter();
    _scene->animateVariable();
    _scene->animateTimer(_simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
}

// Preserve event registration semantics by registering existing MainWindow wrapper handlers.
void SimulationEventController::setOnEventHandlers(MainWindow* owner) const {
    OnEventManager* eventManager = _simulator->getModelManager()->current()->getOnEventManager();
    eventManager->addOnAfterProcessEventHandler(owner, &MainWindow::_onAfterProcessEvent);
    eventManager->addOnEntityCreateHandler(owner, &MainWindow::_onEntityCreateHandler);
    eventManager->addOnEntityRemoveHandler(owner, &MainWindow::_onEntityRemoveHandler);
    eventManager->addOnEntityMoveHandler(owner, &MainWindow::_onMoveEntityEvent);
    eventManager->addOnProcessEventHandler(owner, &MainWindow::_onProcessEventHandler);
    eventManager->addOnReplicationStartHandler(owner, &MainWindow::_onReplicationStartHandler);
    eventManager->addOnSimulationStartHandler(owner, &MainWindow::_onSimulationStartHandler);
    eventManager->addOnSimulationPausedHandler(owner, &MainWindow::_onSimulationPausedHandler);
    eventManager->addOnSimulationResumeHandler(owner, &MainWindow::_onSimulationResumeHandler);
    eventManager->addOnSimulationEndHandler(owner, &MainWindow::_onSimulationEndHandler);
}
