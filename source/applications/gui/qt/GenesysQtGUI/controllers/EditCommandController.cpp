#include "EditCommandController.h"

#include "../actions/DeleteUndoCommand.h"
#include "../actions/PasteUndoCommand.h"
#include "../animations/AnimationCounter.h"
#include "../animations/AnimationVariable.h"
#include "../graphicals/GraphicalComponentPort.h"
#include "../graphicals/GraphicalConnection.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/Plugin.h"
#include "../../../../../kernel/simulator/Simulator.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QPair>
#include <QUndoCommand>
#include <QUndoStack>

// Bind Phase 9 dependencies required to preserve existing edit command behavior.
EditCommandController::EditCommandController(Simulator* simulator,
                                             ModelGraphicsView* graphicsView,
                                             std::function<void()> actualizeActions,
                                             bool* cut,
                                             QList<GraphicalModelComponent*>** gmcCopies,
                                             QList<GraphicalConnection*>** portsCopies,
                                             QList<QGraphicsItem*>** drawCopy,
                                             QList<QGraphicsItemGroup*>** groupCopy)
    : _simulator(simulator),
      _graphicsView(graphicsView),
      _actualizeActions(std::move(actualizeActions)),
      _cut(cut),
      _gmcCopies(gmcCopies),
      _portsCopies(portsCopies),
      _drawCopy(drawCopy),
      _groupCopy(groupCopy) {
}

// Resolve the shared model scene used by all delegated edit commands.
ModelGraphicsScene* EditCommandController::scene() const {
    return (_graphicsView != nullptr) ? _graphicsView->getScene() : nullptr;
}

// Preserve undo-stack behavior while moving orchestration out of MainWindow.
void EditCommandController::onActionEditUndoTriggered() const {
    ModelGraphicsScene* currentScene = scene();
    if (currentScene != nullptr && currentScene->getUndoStack() != nullptr) {
        currentScene->getUndoStack()->undo();
    }
}

// Preserve redo-stack behavior while moving orchestration out of MainWindow.
void EditCommandController::onActionEditRedoTriggered() const {
    ModelGraphicsScene* currentScene = scene();
    if (currentScene != nullptr && currentScene->getUndoStack() != nullptr) {
        currentScene->getUndoStack()->redo();
    }
}

// Preserve cut semantics and copy-buffer preparation exactly as before.
void EditCommandController::onActionEditCutTriggered() const {
    (*_gmcCopies)->clear();
    (*_portsCopies)->clear();
    (*_groupCopy)->clear();
    (*_drawCopy)->clear();

    ModelGraphicsScene* currentScene = scene();
    QList<QGraphicsItem*> selecteds = currentScene->userOperableItems(_graphicsView->scene()->selectedItems());
    if (selecteds.size() > 0) {
        *_cut = true;

        for (QGraphicsItem* item : selecteds) {
            QList<GraphicalModelComponent*> groupComponents;
            QList<GraphicalConnection*>* connGroup = new QList<GraphicalConnection*>();

            if (GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                (*_gmcCopies)->append(gmc);
            } else if (QGraphicsItemGroup* group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(group->childItems().at(i));
                    if (component == nullptr) {
                        continue;
                    }

                    if (!component->getGraphicalInputPorts().empty() && !component->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                        for (int j = 0; j < component->getGraphicalInputPorts().at(0)->getConnections()->size(); ++j) {
                            connGroup->append(component->getGraphicalInputPorts().at(0)->getConnections()->at(j));
                        }
                    }

                    for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
                        GraphicalComponentPort* port = component->getGraphicalOutputPorts().at(j);
                        if (!port->getConnections()->empty()) {
                            connGroup->append(port->getConnections()->at(0));
                        }
                    }

                    (*_gmcCopies)->append(component);
                    groupComponents.append(component);
                }

                if (!groupComponents.isEmpty()) {
                    saveItemForCopy(&groupComponents, connGroup);
                    (*_groupCopy)->append(group);
                    currentScene->insertComponentGroup(group, groupComponents);
                    for (unsigned int k = 0; k < static_cast<unsigned int>(connGroup->size()); k++) {
                        (*_portsCopies)->append(connGroup->at(k));
                    }
                }
            } else if (GraphicalConnection* port = dynamic_cast<GraphicalConnection*>(item)) {
                (*_portsCopies)->append(port);
            } else if (dynamic_cast<GraphicalModelDataDefinition*>(item) != nullptr) {
                continue;
            } else {
                (*_drawCopy)->append(item);
            }

            delete connGroup;
        }

        saveItemForCopy(*_gmcCopies, *_portsCopies);
        QUndoCommand* deleteUndoCommand = new DeleteUndoCommand(selecteds, currentScene);
        currentScene->getUndoStack()->push(deleteUndoCommand);
    }
}

// Preserve copy semantics and selection cleanup exactly as before.
void EditCommandController::onActionEditCopyTriggered() const {
    (*_gmcCopies)->clear();
    (*_portsCopies)->clear();
    (*_groupCopy)->clear();
    (*_drawCopy)->clear();

    ModelGraphicsScene* currentScene = scene();
    QList<QGraphicsItem*> selected = currentScene->userOperableItems(_graphicsView->scene()->selectedItems());
    QList<GraphicalModelComponent*> gmcCopiesCopy;

    if (selected.size() > 0) {
        *_cut = false;
        for (QGraphicsItem* item : selected) {
            if (GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                gmc->setSelected(false);
                (*_gmcCopies)->append(gmc);
                gmcCopiesCopy.append(gmc);
            } else if (GraphicalConnection* conn = dynamic_cast<GraphicalConnection*>(item)) {
                conn->setSelected(false);
                (*_portsCopies)->append(conn);
            } else if (QGraphicsItemGroup* group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                group->setSelected(false);
                QList<GraphicalModelComponent*> groupComponents;

                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(group->childItems().at(i));
                    if (component == nullptr) {
                        continue;
                    }
                    gmcCopiesCopy.append(component);
                    groupComponents.append(component);
                }

                if (!groupComponents.isEmpty()) {
                    (*_groupCopy)->append(group);
                }
            } else if (dynamic_cast<GraphicalModelDataDefinition*>(item) != nullptr) {
                item->setSelected(false);
                continue;
            } else {
                item->setSelected(false);
                (*_drawCopy)->append(item);
            }
        }

        saveItemForCopy(&gmcCopiesCopy, *_portsCopies);
        gmcCopiesCopy.clear();
    }
}

// Preserve paste behavior and post-paste buffer cleanup semantics.
void EditCommandController::onActionEditPasteTriggered() const {
    if ((*_gmcCopies)->size() > 0 || (*_drawCopy)->size() > 0 || (*_groupCopy)->size() > 0) {
        ModelGraphicsScene* currentScene = scene();
        if (!*_cut) {
            helpCopy();
        }

        QUndoCommand* pasteUndoCommand = new PasteUndoCommand(*_gmcCopies, *_portsCopies, *_groupCopy, *_drawCopy, currentScene);
        currentScene->getUndoStack()->push(pasteUndoCommand);

        (*_gmcCopies)->clear();
        (*_portsCopies)->clear();
        (*_drawCopy)->clear();
        (*_groupCopy)->clear();
        *_cut = false;
    }
}

// Preserve delete behavior and action-state refresh callback.
void EditCommandController::onActionEditDeleteTriggered() const {
    QList<QGraphicsItem*> selecteds = scene()->userDeletableItems(_graphicsView->scene()->selectedItems());
    if (selecteds.isEmpty()) {
        return;
    }

    ModelGraphicsScene* currentScene = scene();
    QUndoCommand* deleteUndoCommand = new DeleteUndoCommand(selecteds, currentScene);
    currentScene->getUndoStack()->push(deleteUndoCommand);
    if (_actualizeActions) {
        _actualizeActions();
    }
}

// Keep edit-group action delegating to the shared grouping implementation.
void EditCommandController::onActionEditGroupTriggered() const {
    onActionViewGroupTriggered();
}

// Keep edit-ungroup action delegating to the shared ungrouping implementation.
void EditCommandController::onActionEditUngroupTriggered() const {
    onActionViewUngroupTriggered();
}

// Preserve direct scene grouping behavior.
void EditCommandController::onActionViewGroupTriggered() const {
    scene()->groupComponents(false);
}

// Preserve direct scene ungrouping behavior.
void EditCommandController::onActionViewUngroupTriggered() const {
    scene()->ungroupComponents();
}

// Preserve copy-buffer connection filtering behavior used by cut/copy/group flows.
void EditCommandController::saveItemForCopy(QList<GraphicalModelComponent*>* gmcList, QList<GraphicalConnection*>* connList) const {
    for (GraphicalConnection* conn : *connList) {
        ModelComponent* source = conn->getSource()->component;
        ModelComponent* dst = conn->getDestination()->component;

        GraphicalModelComponent* sourceSelected = nullptr;
        GraphicalModelComponent* dstSelected = nullptr;
        for (GraphicalModelComponent* comp : *gmcList) {
            if (source != nullptr && comp->getComponent()->getId() == source->getId()) {
                sourceSelected = comp;
            }
            if (dst != nullptr && comp->getComponent()->getId() == dst->getId()) {
                dstSelected = comp;
            }
        }

        if (sourceSelected == nullptr || dstSelected == nullptr) {
            connList->removeOne(conn);
        }
    }
}

// Preserve deep copy of components, connections, drawings, and groups for paste support.
void EditCommandController::helpCopy() const {
    ModelGraphicsScene* currentScene = scene();

    QList<QPair<GraphicalModelComponent*, GraphicalModelComponent*>>* aux = new QList<QPair<GraphicalModelComponent*, GraphicalModelComponent*>>();
    QList<GraphicalModelComponent*>* gmcAux = new QList<GraphicalModelComponent*>();
    QList<GraphicalModelComponent*>* gmcOldGroupAux = new QList<GraphicalModelComponent*>();
    QList<GraphicalModelComponent*>* gmcNewGroupAux = new QList<GraphicalModelComponent*>();
    QList<GraphicalConnection*>* portsAux = new QList<GraphicalConnection*>();
    QList<QGraphicsItem*>* drawingAux = new QList<QGraphicsItem*>();
    QList<QGraphicsItemGroup*>* groupAux = new QList<QGraphicsItemGroup*>();

    for (GraphicalModelComponent* gmc : **_gmcCopies) {
        if (gmc->group()) {
            continue;
        }

        ModelComponent* previousComponent = gmc->getComponent();
        _simulator->getModelManager()->current()->getComponentManager()->insert(previousComponent);
        const std::string pluginname = previousComponent->getClassname();
        Plugin* plugin = _simulator->getPluginManager()->find(pluginname);
        QPointF position = gmc->pos();
        QColor color = gmc->getColor();
        ModelComponent* component = static_cast<ModelComponent*>(plugin->newInstance(_simulator->getModelManager()->current()));

        GraphicalModelComponent* newgmc = new GraphicalModelComponent(plugin, component, position, color);
        GraphicalModelComponent* oldgmc = currentScene->findGraphicalModelComponent(previousComponent->getId());
        aux->append(qMakePair(oldgmc, newgmc));
        gmcAux->append(newgmc);
    }

    for (QGraphicsItemGroup* group : **_groupCopy) {
        QList<GraphicalConnection*>* connGroup = new QList<GraphicalConnection*>();
        QList<QGraphicsItem*> groupChildren = group->childItems();

        for (QGraphicsItem* child : groupChildren) {
            GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(child);
            if (gmc == nullptr) {
                continue;
            }
            group->removeFromGroup(gmc);

            ModelComponent* previousComponent = gmc->getComponent();
            _simulator->getModelManager()->current()->getComponentManager()->insert(previousComponent);
            const std::string pluginname = previousComponent->getClassname();
            Plugin* plugin = _simulator->getPluginManager()->find(pluginname);
            QPointF position = gmc->pos();
            QColor color = gmc->getColor();
            ModelComponent* component = static_cast<ModelComponent*>(plugin->newInstance(_simulator->getModelManager()->current()));

            GraphicalModelComponent* newgmc = new GraphicalModelComponent(plugin, component, position, color);
            GraphicalModelComponent* oldgmc = currentScene->findGraphicalModelComponent(previousComponent->getId());
            if (oldgmc == nullptr) {
                continue;
            }

            if (!oldgmc->getGraphicalInputPorts().empty() && !oldgmc->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                connGroup->removeOne(oldgmc->getGraphicalInputPorts().at(0)->getConnections()->at(0));
                connGroup->append(oldgmc->getGraphicalInputPorts().at(0)->getConnections()->at(0));
            }

            for (int j = 0; j < oldgmc->getGraphicalOutputPorts().size(); ++j) {
                GraphicalComponentPort* port = oldgmc->getGraphicalOutputPorts().at(j);
                if (!port->getConnections()->empty()) {
                    connGroup->removeOne(port->getConnections()->at(0));
                    connGroup->append(port->getConnections()->at(0));
                }
            }

            gmcOldGroupAux->append(oldgmc);
            gmcNewGroupAux->append(newgmc);
            aux->append(qMakePair(oldgmc, newgmc));
            gmcAux->append(newgmc);
        }

        saveItemForCopy(gmcOldGroupAux, connGroup);
        for (unsigned int k = 0; k < static_cast<unsigned int>(gmcOldGroupAux->size()); k++) {
            group->addToGroup(gmcOldGroupAux->at(k));
        }

        for (unsigned int k = 0; k < static_cast<unsigned int>(connGroup->size()); k++) {
            (*_portsCopies)->removeOne(connGroup->at(k));
            (*_portsCopies)->append(connGroup->at(k));
        }

        if (!gmcNewGroupAux->isEmpty()) {
            QGraphicsItemGroup* newGroup = new QGraphicsItemGroup();
            _graphicsView->getScene()->insertComponentGroup(newGroup, *gmcNewGroupAux);
            groupAux->append(newGroup);
        }
        gmcOldGroupAux->clear();
        gmcNewGroupAux->clear();
        delete connGroup;
    }

    for (GraphicalConnection* conn : **_portsCopies) {
        ModelComponent* source = conn->getSource()->component;
        ModelComponent* dst = conn->getDestination()->component;

        GraphicalComponentPort* sourcePort = nullptr;
        GraphicalComponentPort* destinationPort = nullptr;
        unsigned int portSourceConnection = 0;
        unsigned int portDestinationConnection = 0;

        GraphicalModelComponent* gmcSource = currentScene->findGraphicalModelComponent(source->getId());
        GraphicalModelComponent* gmcDestination = currentScene->findGraphicalModelComponent(dst->getId());

        for (GraphicalModelComponent* comp : **_gmcCopies) {
            if (comp->getComponent()->getId() == source->getId()) {
                sourcePort = conn->getSourceGraphicalPort();
                portSourceConnection = conn->getPortSourceConnection();
            }
            if (comp->getComponent()->getId() == dst->getId()) {
                destinationPort = conn->getDestinationGraphicalPort();
                portDestinationConnection = conn->getPortDestinationConnection();
            }
        }

        GraphicalModelComponent* gmcNewSource = nullptr;
        GraphicalModelComponent* gmcNewDestination = nullptr;
        for (const auto& pair : *aux) {
            if (pair.first == gmcSource) {
                gmcNewSource = pair.second;
            }
            if (pair.first == gmcDestination) {
                gmcNewDestination = pair.second;
            }
        }

        if (gmcNewSource == nullptr || gmcNewDestination == nullptr) {
            continue;
        }
        sourcePort = gmcNewSource->getGraphicalOutputPorts().at(portSourceConnection);
        destinationPort = gmcNewDestination->getGraphicalInputPorts().at(portDestinationConnection);

        GraphicalConnection* newConn = currentScene->addGraphicalConnection(sourcePort, destinationPort, portSourceConnection, portDestinationConnection);
        _graphicsView->getScene()->clearPorts(newConn, gmcNewSource, gmcDestination);
        portsAux->append(newConn);
    }

    for (QGraphicsItem* draw : **_drawCopy) {
        if (AnimationCounter* animationCounter = dynamic_cast<AnimationCounter*>(draw)) {
            AnimationCounter* copiedItem = new AnimationCounter();
            copiedItem->setRect(0, 0, animationCounter->boundingRect().width(), animationCounter->boundingRect().height());
            copiedItem->setPos(animationCounter->pos());
            copiedItem->setCounter(animationCounter->getCounter());
            copiedItem->setValue(animationCounter->getValue());
            drawingAux->append(copiedItem);
            continue;
        }

        if (AnimationVariable* animationVariable = dynamic_cast<AnimationVariable*>(draw)) {
            AnimationVariable* copiedItem = new AnimationVariable();
            copiedItem->setRect(0, 0, animationVariable->boundingRect().width(), animationVariable->boundingRect().height());
            copiedItem->setPos(animationVariable->pos());
            copiedItem->setVariable(animationVariable->getVariable());
            copiedItem->setValue(animationVariable->getValue());
            drawingAux->append(copiedItem);
            continue;
        }

        if (QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(draw)) {
            QGraphicsRectItem* copiedItem = new QGraphicsRectItem(rectItem->rect());
            copiedItem->setPos(rectItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawingAux->append(copiedItem);
            continue;
        }

        if (QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(draw)) {
            QGraphicsEllipseItem* copiedItem = new QGraphicsEllipseItem(ellipseItem->rect());
            copiedItem->setPos(ellipseItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawingAux->append(copiedItem);
            continue;
        }

        if (QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(draw)) {
            QGraphicsPolygonItem* copiedItem = new QGraphicsPolygonItem(polygonItem->polygon());
            copiedItem->setPos(polygonItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawingAux->append(copiedItem);
            continue;
        }

        if (QGraphicsLineItem* lineItem = dynamic_cast<QGraphicsLineItem*>(draw)) {
            QGraphicsLineItem* copiedItem = new QGraphicsLineItem(lineItem->line());
            copiedItem->setPos(lineItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawingAux->append(copiedItem);
            continue;
        }
    }

    *_gmcCopies = gmcAux;
    *_portsCopies = portsAux;
    *_drawCopy = drawingAux;
    *_groupCopy = groupAux;
    delete aux;
    delete gmcOldGroupAux;
    delete gmcNewGroupAux;
}
