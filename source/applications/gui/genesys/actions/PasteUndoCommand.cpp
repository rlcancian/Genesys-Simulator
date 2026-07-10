#include "PasteUndoCommand.h"

PasteUndoCommand::PasteUndoCommand(QList<GraphicalModelComponent *> *graphicalComponents, QList<GraphicalConnection *> *connections, QList<QGraphicsItemGroup *> *groups, QList<QGraphicsItem *> *drawing, ModelGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent), _myComponentItems(new QList<ComponentItem>()), _myConnectionItems(new QList<GraphicalConnection *>()), _myDrawingItems(new QList<DrawingItem>()), _myGroupItems(new QList<GroupItem>()), _myGraphicsScene(scene) {

    const auto captureComponentConnections = [](GraphicalModelComponent* component, ComponentItem* componentItem) {
        if (component == nullptr || componentItem == nullptr) {
            return;
        }

        if (!component->getGraphicalInputPorts().empty()) {
            GraphicalComponentPort* inputPort = component->getGraphicalInputPorts().at(0);
            if (inputPort != nullptr && inputPort->getConnections() != nullptr) {
                for (int j = 0; j < inputPort->getConnections()->size(); ++j) {
                    componentItem->inputConnections.append(inputPort->getConnections()->at(j));
                }
            }
        }

        for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
            GraphicalComponentPort* port = component->getGraphicalOutputPorts().at(j);
            if (port == nullptr || port->getConnections() == nullptr || port->getConnections()->empty()) {
                continue;
            }
            componentItem->outputConnections.append(port->getConnections()->at(0));
        }
    };

    // filtra os tipos de item
    for (int i = 0; i < connections->size(); i++) {
        _myConnectionItems->append(connections->at(i));
    }

    for (int i = 0; i < drawing->size(); i++) {
        // pega o desenho
        QGraphicsItem *draw = drawing->at(i);

        // posicao que vai ficar o desenho colado
        draw->setX(draw->pos().x() + draw->boundingRect().width()/2);
        draw->setY(draw->pos().y() - draw->boundingRect().height()/2);

        // adiciona essa posicao como sua posicao antiga para ModeUndoCommand
        _myGraphicsScene->insertOldPositionItem(draw, draw->pos());

        // seta um DrawingItem
        DrawingItem drawingItem;

        drawingItem.myDrawingItem = draw;
        drawingItem.initialPosition = drawingItem.myDrawingItem->pos();

        _myDrawingItems->append(drawingItem);
    }

    for (int i = 0; i < graphicalComponents->size(); i++) {
        // pega o componente
        GraphicalModelComponent *component = graphicalComponents->at(i);

        // posicao que vai ficar o componente colado
        component->setX(component->pos().x() + component->boundingRect().width()/2);
        component->setY(component->pos().y() - component->boundingRect().height()/2);

        // adiciona essa posicao como sua posicao antiga para ModeUndoCommand
        component->setOldPosition(component->pos());

        // seta um ComponentItem
        ComponentItem componentItem;

        componentItem.graphicalComponent = component;
        componentItem.initialPosition = component->pos();
        captureComponentConnections(component, &componentItem);

        _myComponentItems->append(componentItem);
    }

    if (groups != nullptr) {
        for (int i = 0; i < groups->size(); i++) {
            // pega o grupo
            QGraphicsItemGroup *group = groups->at(i);

            // cria um GroupItem
            GroupItem groupItem;
            groupItem.group = group;

            // pega os componentes desse grupo
            QList<GraphicalModelComponent *> components = _myGraphicsScene->getListComponentsGroup()[group];
            for (int j = 0; j < components.size(); j++) {
                // pega o componente
                GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(components.at(j));

                // seta um ComponentItem
                ComponentItem componentItem;

                componentItem.graphicalComponent = component;
                componentItem.initialPosition = component->pos();
                captureComponentConnections(component, &componentItem);

                // coloca esse componentItem na lista de componentes do grupo
                groupItem.myComponentItems.append(componentItem);
            }
            _myGroupItems->append(groupItem);
        }
    }

    setText(QObject::tr("Paste"));
}

PasteUndoCommand::~PasteUndoCommand() {}

void PasteUndoCommand::undo() {
    // remove tudo que é gráfico

    // remove as conexões selecionadas individualmente
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);
        _myGraphicsScene->removeItem(connection);
    }

    // remove os componentes e suas conexões
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        if (!componentItem.graphicalComponent->group())
            _myGraphicsScene->removeItem(componentItem.graphicalComponent);

        for (int j = 0; j < _myComponentItems->at(i).inputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.inputConnections.at(j);
            _myGraphicsScene->removeItem(connection);
        }

        for (int j = 0; j < _myComponentItems->at(i).outputConnections.size(); ++j) {
            GraphicalConnection *connection = componentItem.outputConnections.at(j);
            _myGraphicsScene->removeItem(connection);
        }
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

        if (!componentItem.graphicalComponent->group())
            _myGraphicsScene->removeComponent(componentItem.graphicalComponent);
    }

    // remove as conexões
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);
        GraphicalModelComponent *source = _myGraphicsScene->resolveSourceComponent(connection);
        GraphicalModelComponent *destination = _myGraphicsScene->resolveDestinationComponent(connection);
        if (source != nullptr && destination != nullptr) {
            _myGraphicsScene->removeGraphicalConnection(connection, source, destination);
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
    _myGraphicsScene->update();
}

void PasteUndoCommand::redo() {
    // remove todos os itens selecionados antes disso
    for (int i = 0; i < _myGraphicsScene->items().size(); ++i) {
        QGraphicsItem *item = _myGraphicsScene->items().at(i);

        item->setSelected(false);
    }

    // adiciona tudo que é gráfico

    // adiciona os componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        componentItem.graphicalComponent->setPos(componentItem.initialPosition);
        _myGraphicsScene->addItem(componentItem.graphicalComponent);

        componentItem.graphicalComponent->setSelected(true);
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
        connection->setSelected(true);
    }

    // coloca os componentes do grupo no modelo
    for (int i = 0; i < _myGroupItems->size(); ++i) {
        GroupItem groupItem = _myGroupItems->at(i);

        for (int j = 0; j < groupItem.myComponentItems.size(); ++j) {
            _myGraphicsScene->addItem(groupItem.myComponentItems.at(j).graphicalComponent);
        }
    }

    // adiciona os outros itens do tipo QGraphicsItem na tela
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);

        drawingItem.myDrawingItem->setPos(drawingItem.initialPosition);
        _myGraphicsScene->addItem(drawingItem.myDrawingItem);

        drawingItem.myDrawingItem->setSelected(true);
    }

    // agora adiciona o que se deve no modelo

    // adiciona os componentes
    for (int i = 0; i < _myComponentItems->size(); ++i) {
        ComponentItem componentItem = _myComponentItems->at(i);

        _myGraphicsScene->insertComponent(componentItem.graphicalComponent, &componentItem.inputConnections, &componentItem.outputConnections);
    }

    // adiciona as conexões
    for (int i = 0; i < _myConnectionItems->size(); ++i) {
        GraphicalConnection *connection = _myConnectionItems->at(i);

        _myGraphicsScene->connectComponents(connection, nullptr, nullptr);

    }

    // volta os grupos no modelo
    for (int i = 0; i < _myGroupItems->size(); ++i) {
        GroupItem groupItem = _myGroupItems->at(i);
        QList<GraphicalModelComponent *> *componentsGroup = new QList<GraphicalModelComponent *>();

        for (int j = 0; j < groupItem.myComponentItems.size(); ++j) {
            ComponentItem componentItem = groupItem.myComponentItems.at(j);

            _myGraphicsScene->addItem(componentItem.graphicalComponent);
            
            if (componentItem.graphicalComponent->group())
                groupItem.group->removeFromGroup(componentItem.graphicalComponent);
            componentsGroup->append(componentItem.graphicalComponent);
        }

        _myGraphicsScene->groupModelComponents(componentsGroup, groupItem.group);
        groupItem.group->update();

        _myGraphicsScene->insertOldPositionItem(groupItem.group, groupItem.group->pos());

        delete componentsGroup;
    }

    // adiciona os outros itens do tipo QGraphicsItem
    for (int i = 0; i < _myDrawingItems->size(); ++i) {
        DrawingItem drawingItem = _myDrawingItems->at(i);

        _myGraphicsScene->addDrawing(drawingItem.myDrawingItem);
    }

    GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
    GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

    _myGraphicsScene->notifyGraphicalModelChange(eventType, eventObjectType, nullptr);


    // atualiza a cena
    _myGraphicsScene->update();
}
