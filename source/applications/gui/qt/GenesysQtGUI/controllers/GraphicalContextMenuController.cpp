#include "GraphicalContextMenuController.h"

#include "kernel/simulator/Model.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "ui_mainwindow.h"
#include "../graphicals/GraphicalComponentPort.h"
#include "../graphicals/GraphicalConnection.h"
#include "../graphicals/GraphicalDiagramConnection.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/ModelGraphicsView.h"

#include <QActionGroup>
#include <QContextMenuEvent>
#include <QGraphicsItemGroup>
#include <QMenu>

#include <utility>

namespace {

struct TraceLevelMenuEntry {
    TraceManager::Level level;
    const char* label;
};

constexpr TraceLevelMenuEntry kTraceLevelEntries[] = {
    {TraceManager::Level::L0_noTraces, "L0_noTraces"},
    {TraceManager::Level::L1_errorFatal, "L1_errorFatal"},
    {TraceManager::Level::L2_results, "L2_results"},
    {TraceManager::Level::L3_errorRecover, "L3_errorRecover"},
    {TraceManager::Level::L4_warning, "L4_warning"},
    {TraceManager::Level::L5_event, "L5_event"},
    {TraceManager::Level::L6_arrival, "L6_arrival"},
    {TraceManager::Level::L7_internal, "L7_internal"},
    {TraceManager::Level::L8_detailed, "L8_detailed"},
    {TraceManager::Level::L9_mostDetailed, "L9_mostDetailed"}
};

bool collectBioReactionSelection(ModelGraphicsScene* scene,
                                 BioReaction*& reaction,
                                 std::vector<BioSpecies*>& speciesDefinitions) {
    reaction = nullptr;
    speciesDefinitions.clear();
    if (scene == nullptr) {
        return false;
    }

    const QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    if (selectedItems.size() < 2) {
        return false;
    }

    for (QGraphicsItem* item : selectedItems) {
        auto* graphicalDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphicalDefinition == nullptr) {
            return false;
        }

        ModelDataDefinition* dataDefinition = graphicalDefinition->getDataDefinition();
        if (auto* candidateReaction = dynamic_cast<BioReaction*>(dataDefinition)) {
            if (reaction != nullptr) {
                return false;
            }
            reaction = candidateReaction;
            continue;
        }

        auto* species = dynamic_cast<BioSpecies*>(dataDefinition);
        if (species == nullptr) {
            return false;
        }
        speciesDefinitions.push_back(species);
    }

    return reaction != nullptr && !speciesDefinitions.empty();
}

bool reactionAlreadyContainsReactant(BioReaction* reaction, const std::string& speciesName) {
    for (const auto& term : reaction->getReactants()) {
        if (term.speciesName == speciesName) {
            return true;
        }
    }
    return false;
}

bool reactionAlreadyContainsProduct(BioReaction* reaction, const std::string& speciesName) {
    for (const auto& term : reaction->getProducts()) {
        if (term.speciesName == speciesName) {
            return true;
        }
    }
    return false;
}

bool reactionAlreadyContainsModifier(BioReaction* reaction, const std::string& speciesName) {
    for (const std::string& modifierName : reaction->getModifiers()) {
        if (modifierName == speciesName) {
            return true;
        }
    }
    return false;
}

} // namespace

GraphicalContextMenuController::GraphicalContextMenuController(ModelGraphicsView* graphicsView,
                                                               Ui::MainWindow* ui,
                                                               QAction* openSubmodelAction,
                                                               std::function<ModelGraphicsScene*()> currentScene,
                                                               std::function<void()> actualizeActions)
    : _graphicsView(graphicsView),
      _ui(ui),
      _openSubmodelAction(openSubmodelAction),
      _currentScene(std::move(currentScene)),
      _actualizeActions(std::move(actualizeActions)) {
}

void GraphicalContextMenuController::handleGraphicsViewContextMenu(QContextMenuEvent* event) {
    if (event == nullptr || _graphicsView == nullptr || _ui == nullptr || !_currentScene) {
        return;
    }

    QGraphicsItem* clickedItem = semanticItemFor(_graphicsView->itemAt(event->pos()));
    synchronizeSelectionForContext(clickedItem);

    if (_actualizeActions) {
        // Refresh action enabled/checked state after the context click potentially changed selection.
        _actualizeActions();
    }

    QMenu menu(_graphicsView);
    const ContextKind contextKind = classifyContext(clickedItem);

    if (contextKind == ContextKind::Background) {
        populateBackgroundMenu(&menu);
    } else {
        populateItemMenu(&menu, contextKind, clickedItem);
    }

    if (!menu.actions().isEmpty()) {
        event->accept();
        menu.exec(event->globalPos());
    }
}

QGraphicsItem* GraphicalContextMenuController::semanticItemFor(QGraphicsItem* item) const {
    QGraphicsItem* current = item;
    while (current != nullptr) {
        // Components inherit from data definitions, so component checks must run first.
        if (dynamic_cast<GraphicalModelComponent*>(current) != nullptr) {
            return current;
        }
        if (dynamic_cast<GraphicalModelDataDefinition*>(current) != nullptr) {
            return current;
        }
        if (dynamic_cast<GraphicalConnection*>(current) != nullptr) {
            return current;
        }
        if (dynamic_cast<GraphicalDiagramConnection*>(current) != nullptr) {
            return current;
        }
        if (dynamic_cast<QGraphicsItemGroup*>(current) != nullptr) {
            return current;
        }
        if (dynamic_cast<GraphicalComponentPort*>(current) != nullptr && current->parentItem() != nullptr) {
            current = current->parentItem();
            continue;
        }
        current = current->parentItem();
    }
    return nullptr;
}

void GraphicalContextMenuController::synchronizeSelectionForContext(QGraphicsItem* item) const {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr || item == nullptr) {
        return;
    }

    if (!item->flags().testFlag(QGraphicsItem::ItemIsSelectable)) {
        return;
    }

    if (!item->isSelected()) {
        // Right-clicking a non-selected item makes commands target that item instead of stale selection.
        scene->clearSelection();
        item->setSelected(true);
    }
}

GraphicalContextMenuController::ContextKind GraphicalContextMenuController::classifyContext(QGraphicsItem* item) const {
    if (item == nullptr) {
        return ContextKind::Background;
    }

    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr && item->isSelected() && scene->selectedItems().size() > 1) {
        return ContextKind::MultiSelection;
    }

    if (dynamic_cast<GraphicalModelComponent*>(item) != nullptr) {
        return ContextKind::SingleComponent;
    }
    if (dynamic_cast<GraphicalModelDataDefinition*>(item) != nullptr) {
        return ContextKind::SingleDataDefinition;
    }
    if (dynamic_cast<GraphicalConnection*>(item) != nullptr) {
        return ContextKind::SingleConnection;
    }
    if (dynamic_cast<GraphicalDiagramConnection*>(item) != nullptr) {
        return ContextKind::SingleDiagramConnection;
    }
    if (dynamic_cast<QGraphicsItemGroup*>(item) != nullptr) {
        return ContextKind::SingleGroup;
    }

    return ContextKind::GenericItem;
}

ModelDataDefinition* GraphicalContextMenuController::modelDataDefinitionFor(QGraphicsItem* item) const {
    if (item == nullptr) {
        return nullptr;
    }

    if (auto* graphicalComponent = dynamic_cast<GraphicalModelComponent*>(item)) {
        return graphicalComponent->getComponent();
    }
    if (auto* graphicalDataDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
        return graphicalDataDefinition->getDataDefinition();
    }
    return nullptr;
}

void GraphicalContextMenuController::populateBackgroundMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    // Background menus prioritize scene-level commands because no specific item was targeted.
    menu->addAction(_ui->actionEditPaste);
    menu->addAction(_ui->actionSelectAll);
    menu->addSeparator();
    addShowMenu(menu);
    addZoomMenu(menu);
    menu->addAction(_ui->actionGModelShowConnect);
    menu->addSeparator();
    addDrawMenu(menu);
    addAnimateMenu(menu);
    menu->addSeparator();
    menu->addAction(_ui->actionViewConfigure);
}

void GraphicalContextMenuController::populateItemMenu(QMenu* menu, ContextKind contextKind, QGraphicsItem* clickedItem) const {
    if (menu == nullptr) {
        return;
    }

    // Item menus prioritize commands that operate on the active graphical selection.
    addEditActions(menu);

    switch (contextKind) {
        case ContextKind::SingleComponent:
            menu->addSeparator();
            addTraceLevelMenu(menu, clickedItem);
            menu->addAction(_ui->actionGModelShowConnect);
            menu->addAction(_ui->actionGModelComponentBreakpoint);
            if (_openSubmodelAction != nullptr) {
                menu->addAction(_openSubmodelAction);
            }
            menu->addSeparator();
            addShowMenu(menu);
            addZoomMenu(menu);
            break;

        case ContextKind::SingleDataDefinition:
            menu->addSeparator();
            addTraceLevelMenu(menu, clickedItem);
            addBioReactionLinkMenu(menu);
            if (_openSubmodelAction != nullptr) {
                menu->addAction(_openSubmodelAction);
            }
            break;

        case ContextKind::MultiSelection:
            menu->addSeparator();
            addBioReactionLinkMenu(menu);
            menu->addAction(_ui->actionEditGroup);
            menu->addAction(_ui->actionEditUngroup);
            addAlignMenu(menu);
            break;

        case ContextKind::SingleGroup:
            menu->addSeparator();
            menu->addAction(_ui->actionEditUngroup);
            addAlignMenu(menu);
            break;

        case ContextKind::SingleConnection:
        case ContextKind::SingleDiagramConnection:
        case ContextKind::GenericItem:
        case ContextKind::Background:
            break;
    }
}

void GraphicalContextMenuController::addEditActions(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    menu->addAction(_ui->actionEditUndo);
    menu->addAction(_ui->actionEditRedo);
    menu->addSeparator();
    menu->addAction(_ui->actionEditCut);
    menu->addAction(_ui->actionEditCopy);
    menu->addAction(_ui->actionEditPaste);
    menu->addAction(_ui->actionEditDelete);
}

void GraphicalContextMenuController::addShowMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    QMenu* showMenu = menu->addMenu("Show");
    // These actions are checkable scene toggles, so reusing them preserves checked state in the popup.
    showMenu->addAction(_ui->actionShowGrid);
    showMenu->addAction(_ui->actionShowRule);
    showMenu->addAction(_ui->actionShowSnap);
    showMenu->addAction(_ui->actionShowGuides);
    showMenu->addSeparator();
    showMenu->addAction(_ui->actionShowInternalElements);
    showMenu->addAction(_ui->actionShowEditableElements);
    showMenu->addAction(_ui->actionShowAttachedElements);
    showMenu->addAction(_ui->actionShowRecursiveElements);
}

void GraphicalContextMenuController::addZoomMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    QMenu* zoomMenu = menu->addMenu("Zoom");
    zoomMenu->addAction(_ui->actionZoom_In);
    zoomMenu->addAction(_ui->actionZoom_Out);
    zoomMenu->addAction(_ui->actionZoom_All);
}

void GraphicalContextMenuController::addTraceLevelMenu(QMenu* menu, QGraphicsItem* clickedItem) const {
    if (menu == nullptr) {
        return;
    }

    ModelDataDefinition* modelDataDefinition = modelDataDefinitionFor(clickedItem);
    if (modelDataDefinition == nullptr) {
        return;
    }

    // commented since Trace Level became a property (SistemControl) of every ModelDataDefinition
    /*
    QMenu* traceLevelMenu = menu->addMenu("Set Trace Level");
    auto* actionGroup = new QActionGroup(traceLevelMenu);
    actionGroup->setExclusive(true);

    const bool specificEnabled = modelDataDefinition->isTraceLevelSpecificEnabled();
    const TraceManager::Level activeLevel = modelDataDefinition->getTraceLevelSpecific();

    for (const TraceLevelMenuEntry& entry : kTraceLevelEntries) {
        QAction* action = traceLevelMenu->addAction(entry.label);
        action->setCheckable(true);
        action->setChecked(specificEnabled && activeLevel == entry.level);
        actionGroup->addAction(action);

        QObject::connect(action, &QAction::triggered, traceLevelMenu, [modelDataDefinition, level = entry.level]() {
            modelDataDefinition->defineTraceLevelSpecific(level, true);
        });
    }
    */
}

void GraphicalContextMenuController::addBioReactionLinkMenu(QMenu* menu) const {
    if (menu == nullptr || !_currentScene) {
        return;
    }

    ModelGraphicsScene* scene = _currentScene();
    BioReaction* reaction = nullptr;
    std::vector<BioSpecies*> selectedSpecies;
    if (!collectBioReactionSelection(scene, reaction, selectedSpecies)) {
        return;
    }

    QMenu* relationMenu = menu->addMenu("Biochemical Relation");
    QAction* addReactantsAction = relationMenu->addAction("Add Selected BioSpecies as Reactants");
    QAction* addProductsAction = relationMenu->addAction("Add Selected BioSpecies as Products");
    QAction* addModifiersAction = relationMenu->addAction("Add Selected BioSpecies as Modifiers");

    // All three actions mutate the same BioReaction and rely on the generic scene sync to rebuild diagram links.
    auto refreshAfterChange = [scene, reaction]() {
        reaction->setHasChanged(true);
        if (reaction->getParentModel() != nullptr) {
            reaction->getParentModel()->setHasChanged(true);
        }
        scene->requestGraphicalDataDefinitionsSync();
        scene->update();
    };

    QObject::connect(addReactantsAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsReactant(reaction, species->getName())) {
                continue;
            }
            reaction->addReactant(species->getName(), 1.0);
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });

    QObject::connect(addProductsAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsProduct(reaction, species->getName())) {
                continue;
            }
            reaction->addProduct(species->getName(), 1.0);
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });

    QObject::connect(addModifiersAction, &QAction::triggered, relationMenu, [reaction, selectedSpecies, refreshAfterChange]() {
        bool changed = false;
        for (BioSpecies* species : selectedSpecies) {
            if (species == nullptr || reactionAlreadyContainsModifier(reaction, species->getName())) {
                continue;
            }
            reaction->addModifier(species->getName());
            changed = true;
        }
        if (changed) {
            refreshAfterChange();
        }
    });
}

void GraphicalContextMenuController::addDrawMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    QMenu* drawMenu = menu->addMenu("Draw");
    // Drawing actions are canvas-started tools, so they only appear on background contexts.
    drawMenu->addAction(_ui->actionDrawLine);
    drawMenu->addAction(_ui->actionDrawRectangle);
    drawMenu->addAction(_ui->actionDrawEllipse);
    drawMenu->addAction(_ui->actionDrawPoligon);
    drawMenu->addAction(_ui->actionDrawText);
}

void GraphicalContextMenuController::addAnimateMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    QMenu* animateMenu = menu->addMenu("Animate");
    animateMenu->addAction(_ui->actionAnimateSimulatedTime);
    animateMenu->addAction(_ui->actionAnimateAttribute);
    animateMenu->addAction(_ui->actionAnimateEntity);
    animateMenu->addAction(_ui->actionAnimateEvent);
    animateMenu->addAction(_ui->actionAnimateCounter);
    animateMenu->addAction(_ui->actionAnimateStatistics);
    animateMenu->addAction(_ui->actionAnimatePlot);
    animateMenu->addSeparator();
    animateMenu->addAction(_ui->actionAnimateVariable);
    animateMenu->addAction(_ui->actionAnimateResource);
    animateMenu->addAction(_ui->actionAnimateQueue);
    animateMenu->addAction(_ui->actionAnimateExpression);
}

void GraphicalContextMenuController::addAlignMenu(QMenu* menu) const {
    if (menu == nullptr) {
        return;
    }

    QMenu* alignMenu = menu->addMenu("Align");
    // Alignment commands are useful whenever the active context is a group or multi-selection.
    alignMenu->addAction(_ui->actionArranjeLeft);
    alignMenu->addAction(_ui->actionArranjeCenter);
    alignMenu->addAction(_ui->actionArranjeRight);
    alignMenu->addSeparator();
    alignMenu->addAction(_ui->actionArranjeTop);
    alignMenu->addAction(_ui->actionArranjeMiddle);
    alignMenu->addAction(_ui->actionArranjeBototm);
}
