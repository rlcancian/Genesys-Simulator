#include "GraphicalContextMenuController.h"

#include "ui_mainwindow.h"
#include "../graphicals/GraphicalComponentPort.h"
#include "../graphicals/GraphicalConnection.h"
#include "../graphicals/GraphicalDiagramConnection.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/ModelGraphicsView.h"

#include <QContextMenuEvent>
#include <QGraphicsItemGroup>
#include <QMenu>

#include <utility>

GraphicalContextMenuController::GraphicalContextMenuController(ModelGraphicsView* graphicsView,
                                                               Ui::MainWindow* ui,
                                                               std::function<ModelGraphicsScene*()> currentScene,
                                                               std::function<void()> actualizeActions)
    : _graphicsView(graphicsView),
      _ui(ui),
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
        populateItemMenu(&menu, contextKind);
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

void GraphicalContextMenuController::populateItemMenu(QMenu* menu, ContextKind contextKind) const {
    if (menu == nullptr) {
        return;
    }

    // Item menus prioritize commands that operate on the active graphical selection.
    addEditActions(menu);

    switch (contextKind) {
        case ContextKind::SingleComponent:
            menu->addSeparator();
            menu->addAction(_ui->actionGModelShowConnect);
            menu->addAction(_ui->actionGModelComponentBreakpoint);
            menu->addSeparator();
            addShowMenu(menu);
            addZoomMenu(menu);
            break;

        case ContextKind::MultiSelection:
            menu->addSeparator();
            menu->addAction(_ui->actionEditGroup);
            menu->addAction(_ui->actionEditUngroup);
            addAlignMenu(menu);
            break;

        case ContextKind::SingleGroup:
            menu->addSeparator();
            menu->addAction(_ui->actionEditUngroup);
            addAlignMenu(menu);
            break;

        case ContextKind::SingleDataDefinition:
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
    showMenu->addAction(_ui->actionShowAttachedElements);
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
