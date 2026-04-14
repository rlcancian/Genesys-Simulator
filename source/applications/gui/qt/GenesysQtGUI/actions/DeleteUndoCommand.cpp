#include "DeleteUndoCommand.h"
#include "graphicals/GraphicalModelDataDefinition.h"

DeleteUndoCommand::DeleteUndoCommand(QList<QGraphicsItem *> items, ModelGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent), _myComponentItems(new QList<ComponentItem>()), _myConnectionItems(new QList<GraphicalConnection *>()), _myDrawingItems(new QList<DrawingItem>()), _myGroupItems(new QList<GroupItem>()), _myInternalDataDefinitionItems(new QList<DataDefinitionItem>()), _myInternalDataDefinitions(new QList<ModelDataDefinition *>()), _myGraphicsScene(scene) {

    // filtra cada tipo de item possível em sua respectiva lista
    for (QGraphicsItem *item : items) {
        // Keep data definitions managed by model reconstruction instead of direct delete actions.
        const bool isDataDefinition = dynamic_cast<GraphicalModelDataDefinition *>(item) != nullptr;
        const bool isComponent = dynamic_cast<GraphicalModelComponent *>(item) != nullptr;
        if (isDataDefinition && !isComponent) {
            continue;
        }
        if (GraphicalModelComponent *component = isComponent ? dynamic_cast<GraphicalModelComponent *>(item) : nullptr) {
            ComponentItem componentItem;

            componentItem.graphicalComponent = component;
            componentItem.initialPosition = component->pos();


            if (!component->getGraphicalInputPorts().empty() && !component->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                for (int j = 0; j < component->getGraphicalInputPorts().at(0)->getConnections()->size(); ++j) {
                    componentItem.inputConnections.append(component->getGraphicalInputPorts().at(0)->getConnections()->at(j));
                }
            }

            for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
                GraphicalComponentPort *port = component->getGraphicalOutputPorts().at(j);

                if (!port->getConnections()->empty())
                    componentItem.outputConnections.append(port->getConnections()->at(0));
            }

            _myComponentItems->append(componentItem);
            captureInternalDataDefinitions(component);
        } else if (GraphicalConnection *connection = dynamic_cast<GraphicalConnection *>(item)) {
            _myConnectionItems->append(connection);
        } else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item)){
            GroupItem groupItem;

            groupItem.group = group;
            groupItem.initialPosition = group->pos();

            for (int i = 0; i < group->childItems().size(); i++) {
                GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(groupItem.group->childItems().at(i));
                if (component == nullptr) {
                    continue;
                }

                ComponentItem componentItem;

                componentItem.graphicalComponent = component;
                componentItem.initialPosition = component->pos();

                if (!component->getGraphicalInputPorts().empty() && !component->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                    for (int j = 0; j < component->getGraphicalInputPorts().at(0)->getConnections()->size(); ++j) {
                        componentItem.inputConnections.append(component->getGraphicalInputPorts().at(0)->getConnections()->at(j));
                    }
                }

                for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
                    GraphicalComponentPort *port = component->getGraphicalOutputPorts().at(j);

                    if (!port->getConnections()->empty())
                        componentItem.outputConnections.append(port->getConnections()->at(0));
                }

                groupItem.myComponentItems.append(componentItem);
                captureInternalDataDefinitions(component);
            }

            _myGroupItems->append(groupItem);
        } else {
            DrawingItem drawingItem;

            drawingItem.myDrawingItem = item;
            drawingItem.initialPosition = item->pos();

            _myDrawingItems->append(drawingItem);
        }
    }

    setText(QObject::tr("Delete"));
}

DeleteUndoCommand::~DeleteUndoCommand() {
    delete _myComponentItems;
    delete _myConnectionItems;
    delete _myDrawingItems;
    delete _myGroupItems;
    delete _myInternalDataDefinitionItems;
    delete _myInternalDataDefinitions;
}

void DeleteUndoCommand::undo() {
    // adiciona tudo que é gráfico

    // adiciona os componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        _myGraphicsScene->addItem(componentItem.graphicalComponent);
        componentItem.graphicalComponent->setPos(componentItem.initialPosition);
    }

    // adiciona as conexões dos componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        for (int j = 0; j < _myComponentItems->at(i).inputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.inputConnections.at(j);
            _myGraphicsScene->addItem(connection);
        }

        for (int j = 0; j < _myComponentItems->at(i).outputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.outputConnections.at(j);
            _myGraphicsScene->addItem(connection);
        }
    }

    // adiciona as conexões selecionadas individualmente
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);
        _myGraphicsScene->addItem(connection);
    }

    // volta os itens do grupo e o grupo na tela
    for (int i = 0; i < _myGroupItems->size(); ++i) {
        GroupItem groupItem = _myGroupItems->at(i);

        // adiciona os componentes
        for (int j = 0; j < groupItem.myComponentItems.size(); ++j) {
            ComponentItem componentItem = groupItem.myComponentItems.at(j);
            componentItem.graphicalComponent->setPos(componentItem.initialPosition);

            _myGraphicsScene->addItem(componentItem.graphicalComponent);
        }

        // adiciona as conexões dos componentes
        for (int j = 0; j < groupItem.myComponentItems.size(); ++j) {
            ComponentItem componentItem = groupItem.myComponentItems.at(j);

            for (int j = 0; j < componentItem.inputConnections.size(); ++j) {
                GraphicalConnection *connection = componentItem.inputConnections.at(j);
                _myGraphicsScene->addItem(connection);
            }

            for (int j = 0; j < componentItem.outputConnections.size(); ++j) {
                GraphicalConnection *connection = componentItem.outputConnections.at(j);
                _myGraphicsScene->addItem(connection);
            }
        }
    }

    // adiciona os outros itens do tipo QGraphicsItem na tela
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);
        drawingItem.myDrawingItem->setPos(drawingItem.initialPosition);

        _myGraphicsScene->addItem(drawingItem.myDrawingItem);
    }

    // agora adiciona o que se deve no modelo

    // adiciona os componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        _myGraphicsScene->insertComponent(componentItem.graphicalComponent, &componentItem.inputConnections, &componentItem.outputConnections, true);
    }

    // realiza as conexõo entre os componentes da conexão
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);

        // verifico se a conexão ainda não existe (ela pode ser uma conexão que foi refeita no connectComponents)
        // por exemplo, pode ter uma conexão selecionada que foi eliminada anteriormente pois ela fazia parte de um componente
        // então ao refazer as conexões do componente, ela já foi religada
        if (!_myGraphicsScene->getGraphicalConnections()->contains(connection)) {
            _myGraphicsScene->connectComponents(connection, nullptr, nullptr);
        }
    }

    // volta os grupos no modelo
    for (int i = 0; i < _myGroupItems->size(); ++i) {
        GroupItem groupItem = _myGroupItems->at(i);
        QList<GraphicalModelComponent *> *componentsGroup = new QList<GraphicalModelComponent *>();

        for (int j = 0; j < groupItem.myComponentItems.size(); ++j) {
            ComponentItem componentItem = groupItem.myComponentItems.at(j);

            _myGraphicsScene->insertComponent(componentItem.graphicalComponent, &componentItem.inputConnections, &componentItem.outputConnections, true);

            componentsGroup->append(componentItem.graphicalComponent);
        }

        groupItem.group->update();

        _myGraphicsScene->groupModelComponents(componentsGroup, groupItem.group);

        groupItem.group->setPos(groupItem.initialPosition);

        delete componentsGroup;
    }

    // adiciona os outros itens do tipo QGraphicsItem
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);
        drawingItem.myDrawingItem->setPos(drawingItem.initialPosition);

        _myGraphicsScene->addDrawing(drawingItem.myDrawingItem);
    }

    restoreInternalDataDefinitionsToModel();
    restoreInternalDataDefinitionItems();

    GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
    GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

    _myGraphicsScene->notifyGraphicalModelChange(eventType, eventObjectType, nullptr);

    // atualiza a cena
    _myGraphicsScene->requestGraphicalDataDefinitionsSync();
    _myGraphicsScene->update();
}

void DeleteUndoCommand::redo() {
    // remove tudo que é gráfico

    detachInternalDataDefinitionItems();
    removeInternalDataDefinitionsFromModel();

    // remove as conexões selecionadas individualmente
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);
        _myGraphicsScene->removeItem(connection);
    }

    // remove os componentes e suas conexões
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        componentItem.graphicalComponent->setOldPosition(componentItem.graphicalComponent->pos());

        for (int j = 0; j < _myComponentItems->at(i).inputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.inputConnections.at(j);
            _myGraphicsScene->removeItem(connection);
        }

        for (int j = 0; j < _myComponentItems->at(i).outputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.outputConnections.at(j);
            _myGraphicsScene->removeItem(connection);
        }

        _myGraphicsScene->removeItem(componentItem.graphicalComponent);
    }

    // remove os outros itens do tipo QGraphicsItem da tela
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);

        _myGraphicsScene->removeItem(drawingItem.myDrawingItem);
    }

    // agora remove o que deve ser removido do modelo

    // remove os componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        _myGraphicsScene->removeComponent(componentItem.graphicalComponent);

    }

    // remove as conexões
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);
        GraphicalModelComponent *source = _myGraphicsScene->findGraphicalModelComponent(connection->getSource()->component->getId());
        GraphicalModelComponent *destination = _myGraphicsScene->findGraphicalModelComponent(connection->getDestination()->component->getId());

        // verifica se a conexão ainda existe, pois ela pode ja ter sido removida caso fizesse parte de um componente que foi removido
        if (_myGraphicsScene->getGraphicalConnections()->contains(connection)) {
            // se ela existe, a remove
            _myGraphicsScene->removeGraphicalConnection(connection, source, destination);
        } else {
            // se não existe, quer dizer que a conexão faz parte de um componente que foi removido, e portando ela já foi removida
            // então a remove da lista
            _myConnectionItems->removeOne(connection);
        }
    }

    // remove os grupos
    for (int i = 0; i < _myGroupItems->size(); ++i) {
        QGraphicsItemGroup *group = _myGroupItems->at(i).group;

        _myGraphicsScene->removeGroup(group);
    }

    // remove os outros itens do tipo QGraphicsItem
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);

        _myGraphicsScene->removeDrawing(drawingItem.myDrawingItem);
    }

    GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::REMOVE;
    GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

    _myGraphicsScene->notifyGraphicalModelChange(eventType, eventObjectType, nullptr);

    // atualiza a cena
    _myGraphicsScene->requestGraphicalDataDefinitionsSync();
    _myGraphicsScene->update();
}

void DeleteUndoCommand::captureInternalDataDefinitions(GraphicalModelComponent *component) {
    if (component == nullptr || component->getComponent() == nullptr) {
        return;
    }

    QSet<ModelDataDefinition *> internalDataDefinitions;
    collectInternalDataDefinitions(component->getComponent(), &internalDataDefinitions);

    for (ModelDataDefinition *dataDefinition : internalDataDefinitions) {
        if (dataDefinition == nullptr || _myInternalDataDefinitions->contains(dataDefinition)) {
            continue;
        }
        _myInternalDataDefinitions->append(dataDefinition);

        GraphicalModelDataDefinition *graphicalDataDefinition = nullptr;
        for (GraphicalModelDataDefinition *candidate : *_myGraphicsScene->getAllDataDefinitions()) {
            if (candidate != nullptr && candidate->getDataDefinition() == dataDefinition) {
                graphicalDataDefinition = candidate;
                break;
            }
        }
        if (graphicalDataDefinition == nullptr) {
            continue;
        }

        DataDefinitionItem dataDefinitionItem;
        dataDefinitionItem.graphicalDataDefinition = graphicalDataDefinition;
        dataDefinitionItem.initialPosition = graphicalDataDefinition->pos();
        dataDefinitionItem.initiallySelected = graphicalDataDefinition->isSelected();
        _myInternalDataDefinitionItems->append(dataDefinitionItem);
    }
}

void DeleteUndoCommand::collectInternalDataDefinitions(ModelDataDefinition *owner,
                                                       QSet<ModelDataDefinition *> *visited) {
    if (owner == nullptr || visited == nullptr || owner->getInternalData() == nullptr) {
        return;
    }

    for (const auto &internalData : *owner->getInternalData()) {
        ModelDataDefinition *dataDefinition = internalData.second;
        if (dataDefinition == nullptr || visited->contains(dataDefinition)) {
            continue;
        }
        visited->insert(dataDefinition);
        collectInternalDataDefinitions(dataDefinition, visited);
    }
}

void DeleteUndoCommand::removeInternalDataDefinitionsFromModel() {
    if (_myGraphicsScene == nullptr || _myGraphicsScene->getSimulator() == nullptr ||
        _myGraphicsScene->getSimulator()->getModelManager() == nullptr ||
        _myGraphicsScene->getSimulator()->getModelManager()->current() == nullptr) {
        return;
    }

    Model *model = _myGraphicsScene->getSimulator()->getModelManager()->current();
    for (ModelDataDefinition *dataDefinition : *_myInternalDataDefinitions) {
        if (dataDefinition != nullptr) {
            model->getDataManager()->remove(dataDefinition);
        }
    }
}

void DeleteUndoCommand::restoreInternalDataDefinitionsToModel() {
    if (_myGraphicsScene == nullptr || _myGraphicsScene->getSimulator() == nullptr ||
        _myGraphicsScene->getSimulator()->getModelManager() == nullptr ||
        _myGraphicsScene->getSimulator()->getModelManager()->current() == nullptr) {
        return;
    }

    Model *model = _myGraphicsScene->getSimulator()->getModelManager()->current();
    for (ModelDataDefinition *dataDefinition : *_myInternalDataDefinitions) {
        if (dataDefinition != nullptr) {
            model->getDataManager()->insert(dataDefinition);
        }
    }
}

void DeleteUndoCommand::detachInternalDataDefinitionItems() {
    if (_myGraphicsScene == nullptr) {
        return;
    }

    // Diagram connections are derived from the live model graph, so they can be rebuilt after the compound edit.
    _myGraphicsScene->clearGraphicalDiagramConnections();
    for (DataDefinitionItem &dataDefinitionItem : *_myInternalDataDefinitionItems) {
        _myGraphicsScene->detachGraphicalModelDataDefinition(dataDefinitionItem.graphicalDataDefinition);
    }
}

void DeleteUndoCommand::restoreInternalDataDefinitionItems() {
    if (_myGraphicsScene == nullptr) {
        return;
    }

    for (DataDefinitionItem &dataDefinitionItem : *_myInternalDataDefinitionItems) {
        GraphicalModelDataDefinition *graphicalDataDefinition = dataDefinitionItem.graphicalDataDefinition;
        if (graphicalDataDefinition == nullptr) {
            continue;
        }
        graphicalDataDefinition->setPos(dataDefinitionItem.initialPosition);
        graphicalDataDefinition->setOldPosition(dataDefinitionItem.initialPosition.x(),
                                                dataDefinitionItem.initialPosition.y());
        graphicalDataDefinition->setSelected(dataDefinitionItem.initiallySelected);
        _myGraphicsScene->restoreGraphicalModelDataDefinition(graphicalDataDefinition);
    }
}
