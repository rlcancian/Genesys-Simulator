/*
 * The MIT License
 *
 * Copyright 2022 rlcancian.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * File:   ModelGraphicsScene.cpp
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 09:52
 */

//#include <qt5/QtWidgets/qgraphicssceneevent.h>
#include <QtWidgets/qgraphicssceneevent.h>
#include <QTreeWidget>
#include <QMessageBox>
#include <QUndoCommand>
#include <string>
#include <list>
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/ModelGraphicsView.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalComponentPort.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalDiagramConnection.h"
#include "actions/AddUndoCommand.h"
#include "actions/DeleteUndoCommand.h"
#include "actions/MoveUndoCommand.h"
#include "actions/GroupUndoCommand.h"
#include "actions/UngroupUndoCommand.h"
#include "dialogs/DialogSelectCounter.h"
#include "dialogs/DialogSelectVariable.h"
#include "dialogs/DialogTimerConfigure.h"
#include "animations/AnimationQueue.h"
#include <QCoreApplication>
#include <QThread>

ModelGraphicsScene::ModelGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent) : QGraphicsScene(x, y, width, height, parent) {
    // grid
    _grid.interval = TraitsGUI<GScene>::gridInterval; // 20;
    _grid.pen = QPen(TraitsGUI<GScene>::gridColor);	  // QPen(Qt::gray); //TODO: To use TraitsGUI<GScene>::gridColor must solve myrgba first
    _grid.lines = new std::list<QGraphicsLineItem *>();
    _grid.visible = false;

    _grid.pen.setWidth(TraitsGUI<GScene>::gridPenWidth);
    _grid.pen.setStyle(Qt::DotLine);

    _currentCounter = nullptr;
    _currentVariable = nullptr;
    _currentTimer = nullptr;

    // Imagens pré-salvas para animação
    // Recomendável imagens com mesma altura e largura de no mínimo 50x50 px e não muito além disso
    _imagesAnimation->append("default.png");
}

ModelGraphicsScene::ModelGraphicsScene(const ModelGraphicsScene& orig) { // : QGraphicsScene(orig) {
}

ModelGraphicsScene::~ModelGraphicsScene() {}


//-----------------------------------------------------------------------

void ModelGraphicsScene::notifyGraphicalModelChange(GraphicalModelEvent::EventType eventType, GraphicalModelEvent::EventObjectType eventObjectType, QGraphicsItem *item) {
    GraphicalModelEvent* modelGraphicsEvent = new GraphicalModelEvent(eventType, eventObjectType, item);
    dynamic_cast<ModelGraphicsView*> (views().at(0))->notifySceneGraphicalModelEventHandler(modelGraphicsEvent);
}

GraphicalModelComponent* ModelGraphicsScene::addGraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color, bool notify) {
    _propertyEditor->addElement(component);
    for (auto prop : *component->getProperties()->list()) {
        if (prop->getIsList()) {
            (*(_propertyList))[prop] = new DataComponentProperty(_propertyEditor, prop, false);
        }

        if (prop->getIsClass()) {
            (*(_propertyEditorUI))[prop] = new DataComponentEditor(_propertyEditor, prop);
        }

        if (prop->getIsEnum()) {
            (*(_propertyBox))[prop] = new ComboBoxEnum(_propertyEditor, prop);
        }
    }
    
    // cria o componente gráfico
    GraphicalModelComponent* graphComp = new GraphicalModelComponent(plugin, component, position, color);

    // cria as conexoes
    // verifica se tenho um componente selecionado
    if (selectedItems().size() == 1 && plugin->getPluginInfo()->getMinimumInputs() > 0) { // check if there is selected component and crate a connection between them
        GraphicalModelComponent* otherGraphComp = dynamic_cast<GraphicalModelComponent*> (selectedItems().at(0));

        // verifica se conseguiu converter o item selecionado para GraphicalModelComponent
        if (otherGraphComp != nullptr) { // a component is selected
            // pega o componente selecionado
            ModelComponent* otherComp = otherGraphComp->getComponent();

            // numero maximo de possiveis conexoes pela porta de saida
            unsigned int maxOutputsOtherComp = otherGraphComp->getGraphicalOutputPorts().size();

            // verifica se ainda posso criar conexoes com aquele componente
            if (otherGraphComp->getComponent()->getConnectionManager()->connections()->size() < maxOutputsOtherComp) {
                // caso tenha portas disponíveis, busca qual delas é
                for (unsigned int numPort = 0; numPort < maxOutputsOtherComp; numPort++) {
                    // caso seja um ponteiro vazio, ele esta livre
                    if (otherComp->getConnectionManager()->getConnectionAtPort(numPort) == nullptr) {
                        // create connection (both model and grapically, since model is being built
                        // model
                        otherGraphComp->getComponent()->getConnectionManager()->insertAtPort(numPort, new Connection({component, 0}));

                        //graphically
                        GraphicalComponentPort* srcport = ((GraphicalModelComponent*) selectedItems().at(0))->getGraphicalOutputPorts().at(numPort);
                        GraphicalComponentPort* destport = graphComp->getGraphicalInputPorts().at(0);
                        addGraphicalConnection(srcport, destport, numPort, 0);

                        otherGraphComp->setOcupiedOutputPorts(otherGraphComp->getOcupiedOutputPorts() + 1);
                        graphComp->setOcupiedInputPorts(graphComp->getOcupiedInputPorts() + 1);
                        break;
                    }
                }
            }
        // caso seja uma porta que esteja selecionada
        } else {
            GraphicalComponentPort* sourceGraphPort = dynamic_cast<GraphicalComponentPort*> (selectedItems().at(0));
            if (sourceGraphPort != nullptr) { // a specific output port of a component is selected.
                if (sourceGraphPort->getConnections()->size() == 0) {
                    // create connection (both model and grapically, since model is being built (ALMOST REPEATED CODE -- REFACTOR)
                    otherGraphComp = sourceGraphPort->graphicalComponent();
                    // create connection (both model and grapically, since model is being built (ALMOST REPEATED CODE -- REFACTOR)
                    // model
                    otherGraphComp->getComponent()->getConnectionManager()->insertAtPort(sourceGraphPort->portNum(), new Connection({component, 0}));
                    //graphically
                    GraphicalComponentPort* destport = graphComp->getGraphicalInputPorts().at(0);
                    addGraphicalConnection(sourceGraphPort, destport, sourceGraphPort->portNum(), 0);

                    otherGraphComp->setOcupiedOutputPorts(otherGraphComp->getOcupiedOutputPorts() + 1);
                    graphComp->setOcupiedInputPorts(graphComp->getOcupiedInputPorts() + 1);
                }
            }
        }
    }
    // adiciona na lista de componentes do modelo;
    _graphicalModelComponents->append(graphComp);

    // adiciona o objeto criado na lista de componentes graficos para nao perder a referencia   // TODO: WHY??
    _allGraphicalModelComponents.append(graphComp);

    // Seta o EntityType
    SourceModelComponent *isSrc = dynamic_cast<SourceModelComponent *>(graphComp->getComponent());

    if (isSrc) {
        EntityType* entityType = isSrc->getEntityType();
        graphComp->setEntityType(entityType);
    }

    // cria um objeto para undo e redo do add
    // ele propriamente adiciona o objeto na tela
    QUndoCommand *addUndoCommand = new AddUndoCommand(graphComp, this);
    _undoStack->push(addUndoCommand);

    //notify graphical model change
    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::COMPONENT;

        notifyGraphicalModelChange(eventType, eventObjectType, graphComp);
    }

    return graphComp;
}

GraphicalConnection* ModelGraphicsScene::addGraphicalConnection(GraphicalComponentPort* sourcePort, GraphicalComponentPort* destinationPort, unsigned int portSourceConnection, unsigned int portDestinationConnection, bool notify) {
    GraphicalConnection* graphicconnection = new GraphicalConnection(sourcePort, destinationPort, portSourceConnection, portDestinationConnection);

    graphicconnection->getDestination();

    addItem(graphicconnection);

    _graphicalConnections->append(graphicconnection);

    //para limpar referencias das conexoes no final  // TODO: Quem fez isso??
    _allGraphicalConnections.append(graphicconnection);

    //notify graphical model change // TODO: Quem fez isso?
    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::CONNECTION;

        notifyGraphicalModelChange(eventType, eventObjectType, graphicconnection);
    }

    return graphicconnection;
}

GraphicalModelDataDefinition* ModelGraphicsScene::addGraphicalModelDataDefinition(Plugin* plugin, ModelDataDefinition* element, QPointF position, QColor color) {
    // cria o componente gráfico
    GraphicalModelDataDefinition* graphDataDef = new GraphicalModelDataDefinition(plugin, element, position, color);

    // adiciona o objeto criado na lista de componentes graficos para nao perder a referencia
    getAllDataDefinitions()->append(graphDataDef);
    getGraphicalModelDataDefinitions()->append(graphDataDef);

    addItem(graphDataDef);

    return graphDataDef;
}

GraphicalDiagramConnection* ModelGraphicsScene::addGraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, GraphicalDiagramConnection::ConnectionType type) {
    GraphicalDiagramConnection* connection = new GraphicalDiagramConnection(dataDefinition,linkedTo,type);

    // adiciona o objeto criado na lista de componentes graficos para nao perder a referencia
    getAllGraphicalDiagramsConnections()->append(connection);
    getGraphicalDiagramsConnections()->append(connection);

    addItem(connection);

    return connection;
}


void ModelGraphicsScene::addGeometry(QPointF endPoint, bool moving) {
    QGraphicsItem *drawingItem = nullptr;

    if (_drawingMode == LINE) {
        //verifica se a linha é muito pequena antes de desenhar
        if (abs(_drawingStartPoint.x() - endPoint.x()) > _grid.interval || abs(_drawingStartPoint.y() - endPoint.y()) > _grid.interval) {

            if (_currentLine != nullptr) {
                removeItem(_currentLine);
                delete _currentLine;
            }

            // Crie e adicione um nova linha à cena
            if (moving) {
                _currentLine = new QGraphicsLineItem(_drawingStartPoint.x(), _drawingStartPoint.y(), endPoint.x(), endPoint.y());
                addItem(_currentLine);
            } else {
                QGraphicsLineItem* line = new QGraphicsLineItem(_drawingStartPoint.x(), _drawingStartPoint.y(), endPoint.x(), endPoint.y());
                line->setFlag(QGraphicsItem::ItemIsSelectable, true);
                line->setFlag(QGraphicsItem::ItemIsMovable, true);
                drawingItem = line;
            }
        }

    } else if (_drawingMode == TEXT) {

    } else if (_drawingMode == RECTANGLE) {
        if (abs(_drawingStartPoint.x() - endPoint.x()) > _grid.interval && abs(_drawingStartPoint.y() - endPoint.y()) > _grid.interval) {
            // Remova o retângulo anterior, se houver
            qreal width = endPoint.x() - _drawingStartPoint.x();
            qreal height = endPoint.y() - _drawingStartPoint.y();
            if (_currentRectangle != nullptr) {
                removeItem(_currentRectangle);
                delete _currentRectangle;
            }

            // Crie e adicione um novo retângulo à cena
            if (moving) {
                _currentRectangle = new QGraphicsRectItem(_drawingStartPoint.x(), _drawingStartPoint.y(), width, height);
                addItem(_currentRectangle);
            } else {
                QGraphicsRectItem* rectangle = new QGraphicsRectItem(0, 0, width, height);
                rectangle->setFlag(QGraphicsItem::ItemIsSelectable, true);
                rectangle->setFlag(QGraphicsItem::ItemIsMovable, true);
                rectangle->setPos(_drawingStartPoint.x(), _drawingStartPoint.y());
                drawingItem = rectangle;
            }
        }
    } else if (_drawingMode == ELLIPSE) {
        if (abs(_drawingStartPoint.x() - endPoint.x()) > _grid.interval && abs(_drawingStartPoint.y() - endPoint.y()) > _grid.interval) {
            // Remova a ellipse anterior, se houver
            qreal width = endPoint.x() - _drawingStartPoint.x();
            qreal height = endPoint.y() - _drawingStartPoint.y();
            if (_currentEllipse != nullptr) {
                removeItem(_currentEllipse);
                delete _currentEllipse;
            }

            // Crie e adicione uma nova ellipse à cena
            if (moving) {
                _currentEllipse = new QGraphicsEllipseItem(_drawingStartPoint.x(), _drawingStartPoint.y(), width, height);
                addItem(_currentEllipse);
            } else {
                QGraphicsEllipseItem* ellipse = new QGraphicsEllipseItem(0, 0, width, height);
                ellipse->setFlag(QGraphicsItem::ItemIsSelectable, true);
                ellipse->setFlag(QGraphicsItem::ItemIsMovable, true);
                ellipse->setPos(_drawingStartPoint.x(), _drawingStartPoint.y());
                drawingItem = ellipse;
            }
        }
    } else if (_drawingMode == POLYGON) {
        // Adiciona o primeiro ponto do poligono
        _drawingMode = POLYGON_POINTS;
        _currentPolygonPoints.clear();
        _currentPolygonPoints << endPoint;
        _currentPolygon = new QGraphicsPolygonItem(QPolygonF(_currentPolygonPoints));
        _currentPolygon->setVisible(true);
        addItem(_currentPolygon);
    } else if (_drawingMode == POLYGON_POINTS) {
        removeItem(_currentPolygon);
        _currentPolygonPoints << endPoint;
        _currentPolygon = new QGraphicsPolygonItem(QPolygonF(_currentPolygonPoints));
        _currentPolygon->setVisible(true);
        addItem(_currentPolygon);
    } else if (_drawingMode == POLYGON_FINISHED) {
        _currentPolygon->setFlag(QGraphicsItem::ItemIsSelectable, true);
        _currentPolygon->setFlag(QGraphicsItem::ItemIsMovable, true);
        drawingItem = _currentPolygon;
    }
    // Redefina o estado de desenho
    if (!moving && !(_drawingMode == POLYGON) && !(_drawingMode == POLYGON_POINTS)) {
        _drawingMode = NONE;
        _currentRectangle = nullptr;
        _currentLine = nullptr;
        _currentEllipse = nullptr;
        _currentPolygon = nullptr;
        _currentPolygonPoints.clear();
        _drawing = false;

        if (_currentAction != nullptr)
            _currentAction->setChecked(false);

        if (drawingItem) {
            QUndoCommand *addUndoCommand = new AddUndoCommand(drawingItem, this);
            _undoStack->push(addUndoCommand);
        }
    }
}

void ModelGraphicsScene::startTextEditing() {
    QGraphicsItem* selectedItem = focusItem();
    if (selectedItem) {
        QGraphicsTextItem* textItem = dynamic_cast<QGraphicsTextItem*>(selectedItem);
        if (textItem) {
            textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
            textItem->setFocus(); // Dê foco ao item para iniciar a edição de texto
        }
    }
}


// limpa todo o modelo
void ModelGraphicsScene::clearGraphicalModelComponents() {
    QList<GraphicalModelComponent*> *componentsInModel = this->graphicalModelComponentItems();
    GraphicalModelComponent *source;
    GraphicalModelComponent *destination;

    for (unsigned int x = 0; x < (unsigned int) componentsInModel->size(); x++){
        GraphicalModelComponent *gmc = componentsInModel->at(x);
        removeComponentInModel(gmc);
    }

    unsigned int size = (unsigned int) _allGraphicalModelComponents.size();

    // limpa todos os componentes no final, desfazendo as conexoes
    for (unsigned int x = 0; x < size; x++){
        GraphicalModelComponent *gmc = _allGraphicalModelComponents.at(0);

        if (gmc) {
            for (unsigned int i = 0; i < (unsigned int) gmc->getGraphicalInputPorts().size(); i++){
                GraphicalComponentPort* port = gmc->getGraphicalInputPorts().at(i);
                for (unsigned int j = 0; j < (unsigned int) port->getConnections()->size(); j++){
                    GraphicalConnection* graphConn = port->getConnections()->at(j);
                    source = findGraphicalModelComponent(graphConn->getSource()->component->getId());
                    removeGraphicalConnection(graphConn, source, gmc);
                }
            }

            for (unsigned int i = 0; i < (unsigned int) gmc->getGraphicalOutputPorts().size(); i++){
                GraphicalComponentPort* port = gmc->getGraphicalOutputPorts().at(i);
                for (unsigned int j = 0; j < (unsigned int) port->getConnections()->size(); j++){
                    GraphicalConnection* graphConn = port->getConnections()->at(j);
                    destination = findGraphicalModelComponent(graphConn->getDestination()->component->getId());
                    removeGraphicalConnection(graphConn, gmc, destination);
                }
            }

            // remove da lista de componentes graficos
            _allGraphicalModelComponents.removeOne(gmc);
            _graphicalModelComponents->removeOne(gmc);
        }
    }
}

// limpa todas as referencias das conexoes no final
void ModelGraphicsScene::clearGraphicalModelConnections() {
    while (! _allGraphicalConnections.isEmpty()) {
        GraphicalConnection *gmc = _allGraphicalConnections.at(0);
        if (gmc) {
            // remove da lista de conexões graficas
            _allGraphicalConnections.removeOne(gmc);

            // libera o ponteiro alocado
            delete gmc;
        }
    }
}

// remove um componente e suas conexoes
void ModelGraphicsScene::removeComponent(GraphicalModelComponent* gmc, bool notify) {
    //graphically
    removeItem(gmc);
    getGraphicalModelComponents()->removeOne(gmc);

    //remove in model
    removeComponentInModel(gmc);

    //limpa as conexoes
    clearConnectionsComponent(gmc);

    // limpa o contador
    std::map<std::string, ModelDataDefinition*>* internalData = gmc->getComponent()->getInternalData();

    for (const auto& pair : *internalData) {
        ModelDataDefinition* data = pair.second;

        if (Counter *counter = dynamic_cast<Counter *>(data)) {
            counter->clear();
        }
    }

    //notify graphical model change
    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::REMOVE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::COMPONENT;

        notifyGraphicalModelChange(eventType, eventObjectType, gmc);
    }
}

// remove o componente do modelo
void ModelGraphicsScene::removeComponentInModel(GraphicalModelComponent* gmc) {
    // pega o componente do modelo grafico
    ModelComponent* component = gmc->getComponent();
    // pega o modelo corrente
    Model* model = _simulator->getModelManager()->current();
    // remove o componente do modelo
    model->getComponentManager()->remove(component);

    gmc->setEntityType(nullptr);
}

void ModelGraphicsScene::insertComponent(GraphicalModelComponent* gmc, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections, bool addGMC, bool addAllGMC, bool notify) {
    // Adiciona na tela
    addItem(gmc);
    // pega o componente do modelo grafico
    ModelComponent* component = gmc->getComponent();
    // pega o modelo corrente
    Model* model = _simulator->getModelManager()->current();
    // adiciona o componente do modelo
    model->insert(component);
    // adiciona na lista de componentes do modelo
    if (addGMC) {
        this->getGraphicalModelComponents()->append(gmc);
    }
    // adiciona na lista de todos os componentes
    if (addAllGMC) {
        this->getAllComponents()->append(gmc);
    }
    // refaz as conexões
    redoConnections(gmc, inputConnections, outputConnections);

    // Seta o EntityType
    SourceModelComponent *isSrc = dynamic_cast<SourceModelComponent *>(component);

    if (isSrc) {
        EntityType* entityType = isSrc->getEntityType();
        gmc->setEntityType(entityType);
    }

    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::COMPONENT;

        notifyGraphicalModelChange(eventType, eventObjectType, gmc);
    }
}

void ModelGraphicsScene::removeGraphicalModelDataDefinition(GraphicalModelDataDefinition* gmdd) {
    //graphically
    removeItem(gmdd);
    getGraphicalModelComponents()->removeOne(gmdd);
    getAllDataDefinitions()->removeOne(gmdd);
    delete(gmdd);
}

void ModelGraphicsScene::removeGraphicalDiagramConnection(GraphicalDiagramConnection* connection ) {
    //graphically
    removeItem(connection);
    getGraphicalDiagramsConnections()->removeOne(connection);
    getAllGraphicalDiagramsConnections()->removeOne(connection);
    delete(connection);
}

// trata da remocao das conexoes de um componente
void ModelGraphicsScene::clearConnectionsComponent(GraphicalModelComponent* gmc) {
    ModelGraphicsScene::clearInputConnectionsComponent(gmc);
    ModelGraphicsScene::clearOutputConnectionsComponent(gmc);
}

// trata da remocao das conexoes de entrada de um componente
void ModelGraphicsScene::clearInputConnectionsComponent(GraphicalModelComponent* graphicalComponent) {
    GraphicalModelComponent *source; // origiem da conexao

    // varre todas as portas de entrada do componente a ser removido
    for (GraphicalComponentPort* port : graphicalComponent->getGraphicalInputPorts()) {
        if (port->getConnections()->size() > 0) {
            // pega a conexao dessa porta
            for (GraphicalConnection* graphConn : *port->getConnections()) {
                // se ha conexao nessa porta
                if (graphConn != nullptr) {
                    // pega a origem da conexao
                    source = ModelGraphicsScene::findGraphicalModelComponent(graphConn->getSource()->component->getId());

                    // se o componente de origem esta no modelo
                    if (source != nullptr) {
                        // remove a conexao entre eles
                        ModelGraphicsScene::removeGraphicalConnection(graphConn, source, graphicalComponent);
                    }
                }
            }
        }
    }
}

// trata da remocao das conexoes de saida de um componente
void ModelGraphicsScene::clearOutputConnectionsComponent(GraphicalModelComponent* graphicalComponent) {
    GraphicalConnection *graphConn; // conexao grafica
    GraphicalModelComponent *destination; // destino da conexao

    // varre todas as portas de saida do componente a ser removido
    for (GraphicalComponentPort* port : graphicalComponent->getGraphicalOutputPorts()) {
        if (port->getConnections()->size() > 0) {
            // pega a conexao dessa porta
            graphConn = port->getConnections()->at(0);

            // se ha conexao nessa porta
            if (graphConn != nullptr) {
                // pega o destino da conexao
                destination = this->findGraphicalModelComponent(graphConn->getDestination()->component->getId());

                // se o componente de destino esta no modelo
                if (destination != nullptr) {
                    // remove a conexao entre eles
                    ModelGraphicsScene::removeGraphicalConnection(graphConn, graphicalComponent, destination);
                }
            }
        }
    }
}

// remove uma conexao grafica (e consequentemente, faz a limpeza no modelo)
void ModelGraphicsScene::removeGraphicalConnection(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source, GraphicalModelComponent *destination, bool notify) {
    unsigned int sourcePortNumber = graphicalConnection->getSource()->channel.portNumber;
    unsigned int destinationPortNumber = graphicalConnection->getDestination()->channel.portNumber;

    // remove in model
    removeConnectionInModel(graphicalConnection, source);

    // remove a conexao do componente origem (conexao grafica)
    source->getGraphicalOutputPorts().at(sourcePortNumber)->removeGraphicalConnection(graphicalConnection);
    // uma porta de saida a menos esta sendo ocupada no componente de origem da conexao
    source->setOcupiedOutputPorts(source->getOcupiedOutputPorts() - 1);

    // remove a conexao do componente de destino (conexao grafica)
    destination->getGraphicalInputPorts().at(destinationPortNumber)->removeGraphicalConnection(graphicalConnection);
    // uma porta de entrada a menos esta sendo ocupada no componente de destino da conexao
    destination->setOcupiedInputPorts(destination->getOcupiedInputPorts() - 1);

    // remove graphically
    removeItem(graphicalConnection);
    _graphicalConnections->removeOne(graphicalConnection);

    //notify graphical model change
    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::REMOVE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::CONNECTION;

        notifyGraphicalModelChange(eventType, eventObjectType, graphicalConnection);
    }
}

// remove uma conexao do modelo
void ModelGraphicsScene::removeConnectionInModel(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source) {
    unsigned int portNumber = graphicalConnection->getSource()->channel.portNumber;

    source->getComponent()->getConnectionManager()->removeAtPort(portNumber);
}

void ModelGraphicsScene::clearPorts(GraphicalConnection* connection, GraphicalModelComponent *source, GraphicalModelComponent *destination) {
    if (!source->getGraphicalOutputPorts().empty()) {
        GraphicalComponentPort *outputPort = source->getGraphicalOutputPorts().at(connection->getSource()->channel.portNumber);
        if (outputPort) {
            if (!outputPort->getConnections()->empty()) {
                outputPort->removeGraphicalConnection(connection);
            }
        }
    }

    if (!destination->getGraphicalInputPorts().empty()) {
        GraphicalComponentPort *inputPort = destination->getGraphicalInputPorts().at(connection->getDestination()->channel.portNumber);
        if (inputPort) {
            if (!inputPort->getConnections()->empty()) {
                inputPort->removeGraphicalConnection(connection);
            }
        }
    }
}

// trata da conexao dos componentes (necessario que ambos estejam no modelo)
void ModelGraphicsScene::connectComponents(GraphicalConnection* connection, GraphicalModelComponent *source, GraphicalModelComponent *destination, bool notify) {
    // faz as conexoes

    ModelGraphicsScene::connectSource(connection, source);
    ModelGraphicsScene::connectDestination(connection, destination);

    // adiciona a conexao na lista de conexoes da cena
    _graphicalConnections->append(connection);

    //notify graphical model change
    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::CONNECTION;

        notifyGraphicalModelChange(eventType, eventObjectType, connection);
    }
}

// esta funcao trata da conexao com o componente de origem
bool ModelGraphicsScene::connectSource(GraphicalConnection* connection, GraphicalModelComponent *source) {
    GraphicalModelComponent *src;

    // necessario GraphicalModelComponent do componente, entao...
    // se enviarem o GraphicalModelComponent de origem nao precisa buscar pelo Id componente
    if (source != nullptr)
        src = source;
    // se nao enviarem o GraphicalModelComponent de origem, busca pelo Id do componente
    else
        src = findGraphicalModelComponent(connection->getSource()->component->getId());

    if (src != nullptr) {
        // varre todas as portas de saida do componente de origem, ate encontrar a porta correta
        for (GraphicalComponentPort* port : src->getGraphicalOutputPorts()) {
            // se encontrar a porta correta
            if (port->portNum() == connection->getSource()->channel.portNumber && port->getConnections()->isEmpty()) {
                // adiciona o componente grafico nessa porta
                port->addGraphicalConnection(connection);
                // adiciona a conexao no modelo do componente de origem
                src->getComponent()->getConnectionManager()->insertAtPort(port->portNum(), connection->getDestination());
                break;
            }
        }

        // diz que o componente de origem tem mais uma porta de saida ocupada
        src->setOcupiedOutputPorts(src->getOcupiedOutputPorts() + 1);

        return true;
    }

    return false;
}

// esta funcao trata da conexao com o componente de destino
bool ModelGraphicsScene::connectDestination(GraphicalConnection* connection, GraphicalModelComponent *destination) {
    GraphicalModelComponent *dst;

    // necessario GraphicalModelComponent do componente, entao...
    // se enviarem o GraphicalModelComponent de destina nao precisa buscar pelo Id componente
    if (destination != nullptr)
        dst = destination;
    // se nao enviarem o GraphicalModelComponent de destino, busca pelo Id do componente
    else
        dst = this->findGraphicalModelComponent(connection->getDestination()->component->getId());

    if (dst != nullptr) {
        // varre todas as portas de entrada do componente de destino, ate encontrar a porta correta
        for (GraphicalComponentPort* port : dst->getGraphicalInputPorts()) {
            // se encontrar a porta correta
            if (port->portNum() == connection->getDestination()->channel.portNumber && !port->getConnections()->contains(connection)) {
                // adiciona o componente grafico nessa porta
                port->addGraphicalConnection(connection);
                // nao e necessario adicionar a conexao no modelo do componente de destino, pois apenas os componentes de origem a possui
                break;
            }
        }

        // diz que o componente de destino tem mais uma porta de saida ocupada
        dst->setOcupiedInputPorts(dst->getOcupiedOutputPorts() + 1);

        return true;
    }

    return false;
}

// Refaz as conexões depois de voltar pra tela
void ModelGraphicsScene::redoConnections(GraphicalModelComponent *graphicalComponent, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections) {
    for (int j = 0; j < inputConnections->size(); ++j) {
        GraphicalConnection *connection = inputConnections->at(j);
        GraphicalModelComponent *source = findGraphicalModelComponent(connection->getSource()->component->getId());

        // so refaz a conexao se ambos estiverem no modelo, se nao, quando o outro for adicionado, ele faz a conexao
        if (source != nullptr)
            connectComponents(inputConnections->at(j), source, graphicalComponent);
    }

    for (int j = 0; j < outputConnections->size(); ++j) {
        GraphicalConnection *connection = outputConnections->at(j);
        GraphicalModelComponent *destination = findGraphicalModelComponent(connection->getDestination()->component->getId());

        // so refaz a conexao se ambos estiverem no modelo, se nao, quando o outro for adicionado, ele faz a conexao
        if (destination != nullptr)
            connectComponents(outputConnections->at(j), graphicalComponent, destination);
    }
}


void ModelGraphicsScene::saveDataDefinitions() {
    QList<GraphicalModelComponent*> *components = this->graphicalModelComponentItems();

    for (GraphicalModelComponent* component : *components) {
        component->verifyQueue();

        if (component->getInternalData()->empty() || component->getAttachedData()->empty()) {
            std::map<std::string, ModelDataDefinition*>* internalsData = component->getComponent()->getInternalData();
            std::map<std::string, ModelDataDefinition*>* attachedData = component->getComponent()->getAttachedData();

            if (component->getInternalData()->empty()) {
                for (auto it = internalsData->begin(); it != internalsData->end(); ++it) {
                    ModelDataDefinition* dataDefinition = it->second;

                    component->getInternalData()->append(dataDefinition);
                }
            }

            if (component->getAttachedData()->empty()) {
                for (auto it = attachedData->begin(); it != attachedData->end(); ++it) {
                    ModelDataDefinition* dataDefinition = it->second;

                    if (dataDefinition->getClassname() != "EntityType")
                        component->getAttachedData()->append(dataDefinition);
                }
            }

            SourceModelComponent *isSrc = dynamic_cast<SourceModelComponent *>(component->getComponent());

            if (isSrc) {
                EntityType* entityType = isSrc->getEntityType();
                component->setEntityType(entityType);
            }
        }
    }
}

void ModelGraphicsScene::insertRestoredDataDefinitions(bool loaded) {
    QList<GraphicalModelComponent*> *components = this->graphicalModelComponentItems();
    QList<GraphicalModelComponent*> *allComponentes = this->getAllComponents();

    if (!allComponentes->empty()) {
        for (GraphicalModelComponent* component : *allComponentes) {
            if (!components->contains(component)) {
                if (component->getEntityType() == nullptr) {
                    SourceModelComponent *isSrc = dynamic_cast<SourceModelComponent *>(component->getComponent());

                    if (isSrc) {
                        EntityType* entityType = isSrc->getEntityType();
                        if (entityType != nullptr)
                            _simulator->getModelManager()->current()->getDataManager()->remove(entityType);
                    }
                }
            }
        }
    }

    if (!components->empty()) {
        for (GraphicalModelComponent* component : *components) {
            for (ModelDataDefinition* dataInternal : *component->getInternalData()) {
                _simulator->getModelManager()->current()->getDataManager()->insert(dataInternal);
            }

            for (ModelDataDefinition* dataAttached : *component->getAttachedData()) {
                _simulator->getModelManager()->current()->getDataManager()->insert(dataAttached);
            }

            if (component->getEntityType() != nullptr) {
                unsigned int size = _simulator->getModelManager()->current()->getDataManager()->getNumberOfDataDefinitions("EntityType");

                if (size == 0) {
                    _simulator->getModelManager()->current()->getDataManager()->insert(component->getEntityType());
                }
            }
        }
    }

    if (loaded) {
        std::list<ModelDataDefinition*>* entityTypes = _simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
        entityTypes->clear();
    }
}

void ModelGraphicsScene::addDrawing(QGraphicsItem * item, bool notify) {
    bool isGeometry = false;
    bool isAnimation = false;

    isAnimation = addDrawingAnimation(item);

    if (!isAnimation)
        isGeometry = removeDrawingGeometry(item);

    if (isAnimation || isGeometry) {
        addItem(item);

        if (notify) {
            //notify graphical model change
            GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
            GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::DRAWING;

            notifyGraphicalModelChange(eventType, eventObjectType, item);
        }
    }
}

bool ModelGraphicsScene::addDrawingAnimation(QGraphicsItem * item) {
    if (AnimationCounter *animationCounter = dynamic_cast<AnimationCounter *>(item)) {
        _animationsCounter->append(animationCounter);
        return true;
    } else if (AnimationVariable *animationVariable = dynamic_cast<AnimationVariable *>(item)) {
        _animationsVariable->append(animationVariable);
        return true;
    } else if (AnimationTimer *animationTimer = dynamic_cast<AnimationTimer *>(item)) {
        _animationsTimer->append(animationTimer);
        return true;
    }

    return false;
}

bool ModelGraphicsScene::addDrawingGeometry(QGraphicsItem * item) {
    _graphicalGeometries->append(item);

    return true;
}

void ModelGraphicsScene::removeDrawing(QGraphicsItem * item, bool notify) {
    bool isGeometry = false;
    bool isAnimation = false;

    isAnimation = removeDrawingAnimation(item);

    if (!isAnimation)
        isGeometry = removeDrawingGeometry(item);

    if (isAnimation || isGeometry) {
        removeItem(item);

        if (notify) {
            //notify graphical model change
            GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::REMOVE;
            GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::DRAWING;

            notifyGraphicalModelChange(eventType, eventObjectType, item);
        }
    }
}

bool ModelGraphicsScene::removeDrawingAnimation(QGraphicsItem * item) {
    if (AnimationCounter *animationCounter = dynamic_cast<AnimationCounter *>(item))
        return _animationsCounter->removeOne(animationCounter);
    else if (AnimationVariable *animationVariable = dynamic_cast<AnimationVariable *>(item))
        return _animationsVariable->removeOne(animationVariable);
    else if (AnimationTimer *animationTimer = dynamic_cast<AnimationTimer *>(item))
        return _animationsTimer->removeOne(animationTimer);

    return false;
}

bool ModelGraphicsScene::removeDrawingGeometry(QGraphicsItem * item) {
    bool removed = _graphicalGeometries->removeOne(item);

    return removed;
}
//------------------------------------------------------------------------


// retorna o elemento _grid que é privado
ModelGraphicsScene::GRID *ModelGraphicsScene::grid() {
    return &_grid;
}

// implementação da função clear() da estrutura GRID
void ModelGraphicsScene::GRID::clear() {
    // limpa e libera a memória da lista de linhas
    for (QGraphicsLineItem *line : *lines) {
        delete line;
    }
    lines->clear();

    // volta a visibilidade pra false
    visible = false;
}

void ModelGraphicsScene::showGrid()
{
    // pego a informação se o grid está visível
    // obs.: o grid é criado uma única vez para a cena e habilitado como visível ou não. =

    // se eu quero que o grid fique visível, verifico se o grid já está desenhado ou não
    if (_grid.visible) {
        // se não tenho linhas no grid, eu as desenho
        if (_grid.lines->size() <= 0) {
            // add new grid
            for (int i = sceneRect().left(); i < sceneRect().right(); i += _grid.interval) {
                QGraphicsLineItem *line = addLine(i, sceneRect().top(), i, sceneRect().bottom(), _grid.pen);
                line->setZValue(-1.0);
                line->setVisible(true);
                _grid.lines->insert(_grid.lines->end(), line);
            }
            for (int j = sceneRect().top(); j < sceneRect().bottom(); j += _grid.interval) {
                QGraphicsLineItem *line = addLine(sceneRect().left(), j, sceneRect().right(), j, _grid.pen);
                line->setZValue(-1.0);
                line->setVisible(true);
                _grid.lines->insert(_grid.lines->end(), line);
            }
        }
        // se eu já tenho meu grid desenhado eu apenas o torno visível
        else {
            for (QGraphicsLineItem *line : *_grid.lines) {
                line->setVisible(true);
            }
        }
    }
    // se eu quero esconder o grid eu tiro a visibilidade das linhas
    else {
        for (QGraphicsLineItem *line : *_grid.lines) {
            line->setVisible(false);
        }
    }

    // troco o valor de visible
    _grid.visible = !_grid.visible;
}

void ModelGraphicsScene::createDiagrams()
{
    Model * m = _simulator->getModelManager()->current();
    ModelDataManager* dataManager = m->getDataManager();

    QColor purple(128,0,128);
    QColor grey(220,220,220);
    //creating graphicalModelDataDefinitions
    for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
        std::list<ModelDataDefinition*>* listDataDefinitions = dataManager->getDataDefinitionList(dataTypename)->list();

        for (auto it = listDataDefinitions->begin(); it != listDataDefinitions->end(); ++it) {
            ModelDataDefinition* datadef = *it;
            std::string pluginName = datadef->getName();
            Plugin* plugin = _simulator->getPluginManager()->find(pluginName);
            addGraphicalModelDataDefinition(plugin, datadef, QPointF(0, 0), grey);
        }
    }
    //organizing diagram
    QList<GraphicalModelDataDefinition*>* datadef_visited = new QList<GraphicalModelDataDefinition*>();
    QList<GraphicalModelComponent*>* gmcs = getAllComponents();
    //internal and attached data of the modelComponents
    for (int i = 0; i < gmcs->size(); i++) {
        GraphicalModelComponent* gmc = gmcs->at(i);
        std::map<std::string, ModelDataDefinition*>* internalData = gmc->getComponent()->getInternalData();
        std::map<std::string, ModelDataDefinition*>* attachedData = gmc->getComponent()->getAttachedData();

        QPointF component_pos = gmc->getOldPosition();
        qreal y_internal = component_pos.y();
        qreal y_attached = component_pos.y();

        //attached Data
        for (auto it = attachedData->begin(); it != attachedData->end(); ++it) {
            ModelDataDefinition* dataDefinition = it->second; // Obtém o valor (ModelDataDefinition*)

            QList<GraphicalModelDataDefinition*>* graphicalDataDefinitions = getAllDataDefinitions();

            for (int j = 0; j < graphicalDataDefinitions->size(); j++) {
                GraphicalModelDataDefinition* gdd = graphicalDataDefinitions->at(j);
                std::string name = gdd->getDataDefinition()->getName();
                if (name == dataDefinition->getName()) {
                    if (getGraphicalModelComponents()->contains(gmc)) {
                        if (datadef_visited->contains(gdd)) {
                            qreal x = (gdd->x() + component_pos.x()) / 2;
                            gdd->setPos(x, y_attached -150);
                            gdd->setOldPosition(x, y_attached -150);

                            //criar conexao
                            GraphicalDiagramConnection* arrowLine = new GraphicalDiagramConnection(gdd, gmc, GraphicalDiagramConnection::ConnectionType::ATTACHED);
                            addItem(arrowLine);
                            _allGraphicalDiagramConnections.append(arrowLine);
                            getGraphicalDiagramsConnections()->append(arrowLine);

                        } else {

                            datadef_visited->append(gdd);
                            y_attached = y_attached - 150;

                            gdd->setPos(component_pos.x(), y_attached);
                            gdd->setOldPosition(component_pos.x(), y_attached);
                            gdd->setColor(purple);

                            //criar conexao
                            GraphicalDiagramConnection* arrowLine = new GraphicalDiagramConnection(gdd, gmc, GraphicalDiagramConnection::ConnectionType::ATTACHED);
                            addItem(arrowLine);
                            _allGraphicalDiagramConnections.append(arrowLine);
                            getGraphicalDiagramsConnections()->append(arrowLine);
                        }
                    }
                }
            }
        }
        //internal Data
        for (auto it = internalData->begin(); it != internalData->end(); ++it) {
            ModelDataDefinition* dataDefinition = it->second; // Obtém o valor (ModelDataDefinition*)

            QList<GraphicalModelDataDefinition*>* graphicalDataDefinitions = getAllDataDefinitions();

            for (int j = 0; j < graphicalDataDefinitions->size(); j++) {
                GraphicalModelDataDefinition* gdd = graphicalDataDefinitions->at(j);
                std::string name = gdd->getDataDefinition()->getName();
                if (name == dataDefinition->getName()) {
                    datadef_visited->append(gdd);
                    y_internal = y_internal + 150;

                    gdd->setPos(component_pos.x(), y_internal);
                    gdd->setOldPosition(component_pos.x(), y_internal);
                    GraphicalDiagramConnection* arrowLine = new GraphicalDiagramConnection(gdd, gmc, GraphicalDiagramConnection::ConnectionType::INTERNAL);
                    addItem(arrowLine);
                    _allGraphicalDiagramConnections.append(arrowLine);
                    getGraphicalDiagramsConnections()->append(arrowLine);
                }
            }
        }
    }
    //internalData of the DataDefinitions
    for (int i = 0; i < datadef_visited->size(); i++) {
        GraphicalModelDataDefinition* parentDataDefinition = datadef_visited->at(i);
        std::map<std::string, ModelDataDefinition*>* internalData = parentDataDefinition->getDataDefinition()->getInternalData();

        QPointF dataDefinition_pos = parentDataDefinition->getOldPosition();
        qreal x = dataDefinition_pos.x();

        for (auto it = internalData->begin(); it != internalData->end(); ++it) {
            ModelDataDefinition* dataDefinition = it->second; // Obtém o valor (ModelDataDefinition*)
            QList<GraphicalModelDataDefinition*>* graphicalDataDefinitions = getAllDataDefinitions();

            for (int j = 0; j < graphicalDataDefinitions->size(); j++) {
                GraphicalModelDataDefinition* gdd = graphicalDataDefinitions->at(j);
                std::string name = gdd->getDataDefinition()->getName();
                if (name == dataDefinition->getName()) {
                    datadef_visited->append(gdd);
                    x = x - 200;

                    gdd->setPos(x, dataDefinition_pos.y());
                    gdd->setOldPosition(x, dataDefinition_pos.y());
                    GraphicalDiagramConnection* arrowLine = new GraphicalDiagramConnection(gdd, parentDataDefinition, GraphicalDiagramConnection::ConnectionType::INTERNAL);
                    addItem(arrowLine);
                    _allGraphicalDiagramConnections.append(arrowLine);
                    getGraphicalDiagramsConnections()->append(arrowLine);
                }
            }
        }
    }
    _diagram = true;
    actualizeDiagramArrows();
    delete datadef_visited;
}

void ModelGraphicsScene::actualizeDiagramArrows() {

    if (existDiagram()) {
        QList<GraphicalDiagramConnection*>* connections = getAllGraphicalDiagramsConnections();
        int size_connections = connections->size();
        for (int i = 0; i < size_connections; i++) {

            GraphicalDiagramConnection* itemConnection = connections->first();

            QGraphicsItem * item_1 = itemConnection->getDataDefinition();
            QGraphicsItem * item_2 = itemConnection->getLinkedDataDefinition();
            addGraphicalDiagramConnection(item_1, item_2, itemConnection->getConnectionType());

            removeGraphicalDiagramConnection(itemConnection);
        }
        if (!visibleDiagram()) hideDiagrams();
    }
}

bool ModelGraphicsScene::existDiagram() {
    return _diagram;
}

bool ModelGraphicsScene::visibleDiagram() {
    return _visibleDiagram;
}

void ModelGraphicsScene::destroyDiagram() {
    QList<GraphicalModelDataDefinition*>* gmdds = getAllDataDefinitions();
    int size_gmdds = gmdds->size();
    for (int i = 0; i < size_gmdds; i++) {
        GraphicalModelDataDefinition* gmdd = gmdds->first();
        removeGraphicalModelDataDefinition(gmdd);
    }

    QList<GraphicalDiagramConnection*>* connections = getAllGraphicalDiagramsConnections();
    int size_connections = connections->size();
    for (int i = 0; i < size_connections; i++) {
        GraphicalDiagramConnection* itemConnection = connections->first();
        removeGraphicalDiagramConnection(itemConnection);
    }

    _diagram = false;
}

void ModelGraphicsScene::hideDiagrams() {
    QList<GraphicalModelDataDefinition*>* dataDefinitions = getAllDataDefinitions();
    for (int i = 0; i < dataDefinitions->size(); i++) {
        dataDefinitions->at(i)->hide();
        dataDefinitions->at(i)->setFlag(QGraphicsItem::ItemIsSelectable, false);
        dataDefinitions->at(i)->setFlag(QGraphicsItem::ItemIsMovable, false);
    }

    QList<GraphicalDiagramConnection*>* connections = getAllGraphicalDiagramsConnections();
    for (int i = 0; i < connections->size(); i++) {
        connections->at(i)->hide();
    }
    _visibleDiagram = false;
}

void ModelGraphicsScene::showDiagrams() {
    QList<GraphicalModelDataDefinition*>* dataDefinitions = getAllDataDefinitions();
    if (dataDefinitions->size() > 0) {
        for (int i = 0; i < dataDefinitions->size(); i++) {

            dataDefinitions->at(i)->show();
            dataDefinitions->at(i)->setFlag(QGraphicsItem::ItemIsSelectable, true);
            dataDefinitions->at(i)->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    QList<GraphicalDiagramConnection*>* connections = getAllGraphicalDiagramsConnections();
    if (connections->size() > 0) {
        for (int i = 0; i < connections->size(); i++) {
            connections->at(i)->show();
        }
    }
    _visibleDiagram = true;
}

void ModelGraphicsScene::setSnapToGrid(bool activated)
{
    _snapToGrid = activated;
}

bool ModelGraphicsScene::getSnapToGrid() {
    return _snapToGrid;
}

void ModelGraphicsScene::animateTransition(ModelComponent *source, ModelComponent *destination, bool viewSimulation, Event *event) {
    // Cria a animação
    AnimationTransition *animationTransition = new AnimationTransition(this, source, destination, viewSimulation);

    if (animationTransition->getGraphicalStartComponent() != nullptr && animationTransition->getGraphicalEndComponent() != nullptr && viewSimulation) {
        runAnimateTransition(animationTransition, event);
    } else {
        animationTransition->stopAnimation();
    }
}

void ModelGraphicsScene::runAnimateTransition(AnimationTransition *animationTransition, Event *event, bool restart) {
    _animationsTransition->append(animationTransition);

    // Inicia ou reinicia a animação
    if (restart)
        animationTransition->restartAnimation();
    else
        animationTransition->startAnimation();

    // Cria um loop de eventos para aguardar a conclusão da animação
    QEventLoop loop;
    connect(animationTransition, &AnimationTransition::finished, &loop, &QEventLoop::quit);

    // Conecta o sinal de stateChanged para sair do loop quando a animação for pausada
    connect(animationTransition, &QAbstractAnimation::stateChanged, [this, &loop, event, animationTransition](QAbstractAnimation::State newState, QAbstractAnimation::State oldState) {
        handleAnimationStateChanged(newState, &loop, event, animationTransition);
    });

    // Aguarda a conclusão da animação sem bloquear o restante do código
    loop.exec();

    _animationsTransition->removeOne(animationTransition);
}

void ModelGraphicsScene::handleAnimationStateChanged(QAbstractAnimation::State newState, QEventLoop* loop, Event* event, AnimationTransition* animationTransition) {
    if (newState == QAbstractAnimation::Paused) {
        if (!_animationPaused->contains(event)) {
            QList<AnimationTransition *> *newList = new  QList<AnimationTransition *>();
            _animationPaused->insert(event, newList);
        }
        _animationPaused->value(event)->append(animationTransition);
        if (loop) loop->quit();
    }
}

void ModelGraphicsScene::animateQueueInsert(ModelComponent *component, bool visivible) {
    // Cria a animação
    AnimationQueue *animationQueue = new AnimationQueue(this, component);

    // Adiciona uma imagem na fila
    animationQueue->addAnimationQueue(visivible);

    // Libera o espaço de memória alocado pela animação
    delete animationQueue;
}

void ModelGraphicsScene::animateQueueRemove(ModelComponent *component) {
    // Cria a animação
    AnimationQueue *animationQueue = new AnimationQueue(this, component);

    // Remove uma imagem da fila
    animationQueue->removeAnimationQueue();

    // Libera o espaço de memória alocado pela animação
    delete animationQueue;
}


void ModelGraphicsScene::animateCounter() {
    for (unsigned int i = 0; i < (unsigned int) _animationsCounter->size(); i++) {
        AnimationCounter *animationCounter = _animationsCounter->at(i);
        double currentValue = 0.0;

        if (animationCounter->getCounter()) {
            currentValue = animationCounter->getCounter()->getCountValue();
        }
        animationCounter->setValue(currentValue);
    }
}

void ModelGraphicsScene::animateVariable() {
    for (unsigned int i = 0; i < (unsigned int) _animationsVariable->size(); i++) {
        AnimationVariable *animationVariable = _animationsVariable->at(i);
        double currentValue = 0.0;
        if (animationVariable->getVariable()) {
            currentValue = animationVariable->getVariable()->getValue();
        }
        animationVariable->setValue(currentValue);
    }
}

void ModelGraphicsScene::animateTimer(double time) {
    for (unsigned int i = 0; i < (unsigned int) _animationsTimer->size(); i++) {
        _animationsTimer->at(i)->setTime(time);
    }
}

QList<QString>* ModelGraphicsScene::getImagesAnimation() {
    return _imagesAnimation;
}

QMap<QGraphicsItemGroup *, QList<GraphicalModelComponent *>> ModelGraphicsScene::getListComponentsGroup() {
    return _listComponentsGroup;
}

void ModelGraphicsScene::clearAnimations() {
    this->clearAnimationsQueue();
    this->clearAnimationsTransition();
    this->clearAnimationsCounter();
    this->clearAnimationsVariable();
    this->clearAnimationsTimer();
}

void ModelGraphicsScene::clearAnimationsTransition() {
    // Limpa lista de animações de transição
    if (_animationsTransition) {
        for (unsigned int i = 0; i < (unsigned int) _animationsTransition->size(); i++) {
            delete _animationsTransition->at(i);
        }
        _animationsTransition->clear();
    }
}

void ModelGraphicsScene::clearAnimationsCounter() {
    // Limpa lista de animações de contadores
    if (_animationsCounter)
        _animationsCounter->clear();

    if (_counters)
        _counters->clear();

    _currentCounter = nullptr;
}

void ModelGraphicsScene::clearAnimationsVariable() {
    // Limpa lista de animações de variáveis
    if (_animationsVariable)
        _animationsVariable->clear();

    if (_variables)
        _variables->clear();

    _currentVariable = nullptr;
}

void ModelGraphicsScene::clearAnimationsTimer() {
    // Limpa lista de animações de tempo
    if (_animationsTimer)
        _animationsTimer->clear();

    _currentTimer = nullptr;
}

void ModelGraphicsScene::clearAnimationsQueue() {
    // Limpa as animações de fila dos componentes
    QList<GraphicalModelComponent *> *componentes = getAllComponents();

    for (GraphicalModelComponent* componente : *componentes) {
        componente->clearQueues();
    }
}

void ModelGraphicsScene::insertComponentGroup(QGraphicsItemGroup *group, QList<GraphicalModelComponent *> componentsGroup) {
    _listComponentsGroup.insert(group, componentsGroup);
}

void ModelGraphicsScene::insertOldPositionItem(QGraphicsItem *item, QPointF position) {
    _oldPositionsItems.insert(item, position);
}

QPointF ModelGraphicsScene::getOldPositionItem(QGraphicsItem *item) const {
   return _oldPositionsItems[item];
}

void ModelGraphicsScene::snapItemsToGrid()
{
    if (_snapToGrid) {
        // Obtenha a lista de visualizações associadas a esta cena

        QList<QGraphicsItem*>* items = getGraphicalModelComponents();
        int num_items = items->size();

        for (int i = 0; i < num_items; i++) {
            QGraphicsItem* item = items->at(i);

            GraphicalModelComponent* modelItem = dynamic_cast<GraphicalModelComponent*>(item);
            if (modelItem) {
                // Obtenha a posição atual do item
                QPointF itemPos = modelItem->pos();

                // Calcule a nova posição ajustada ao grid
                qreal x = qRound(itemPos.x() / _grid.interval) * _grid.interval;
                qreal y = qRound(itemPos.y() / _grid.interval) * _grid.interval;


                // Verifique se a nova posição está dentro dos limites da cena
                if (x < sceneRect().left()) {
                    x = sceneRect().left();
                }
                else if (x > sceneRect().right()) {
                    x = sceneRect().right();
                }
                if (y < sceneRect().top()) {
                    y = sceneRect().top();
                }
                else if (y > sceneRect().bottom()) {
                    y = sceneRect().bottom();
                }

                //Defina a nova posição ajustada ao grid
                modelItem->setPos(x, y);
            }
        }
    }
}

QUndoStack* ModelGraphicsScene::getUndoStack() {
    return _undoStack;
}

Simulator* ModelGraphicsScene::getSimulator() {
    return _simulator;
}

void ModelGraphicsScene::setUndoStack(QUndoStack* undo) {
    _undoStack = undo;
}


void ModelGraphicsScene::beginConnection() {
    _connectingStep = 1;
    ((QGraphicsView*)this->parent())->setCursor(Qt::CrossCursor);
}

void ModelGraphicsScene::groupComponents(bool notify) {
    int size = selectedItems().size();
    int num_groups = getGraphicalGroups()->size();
    //verifica se algum item selecionado já faz parte de um grupo
    bool isItemGroup = false;
    if (num_groups > 0) {
        for (int i = 0; (i < size) && !isItemGroup; i++) {  //percorrer todos os itens selecionados
            QGraphicsItem* c = selectedItems().at(i);
            QGraphicsItemGroup* isGroup = dynamic_cast<QGraphicsItemGroup*>(c);
            if (isGroup) {
                isItemGroup = true;
            }
        }
    }
    if (!isItemGroup) {

        QList<QGraphicsItem *> group = selectedItems();
        QList<GraphicalModelComponent *> graphicalComponents;

        for (int i = 0; i < group.size(); i++) {
            QGraphicsItem* c = group.at(i);
            GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(c);
            if (gmc) {
                graphicalComponents.append(gmc);
            }
        }

        if (!graphicalComponents.empty()) {
            QUndoCommand *groupCommand = new GroupUndoCommand(graphicalComponents , this);
            _undoStack->push(groupCommand);

            //notify graphical model change (colocar aqui um ponteiro)
            if (notify) {
                GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
                GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

                notifyGraphicalModelChange(eventType, eventObjectType, nullptr);
            }
        }
    }
}

void ModelGraphicsScene::groupModelComponents(QList<GraphicalModelComponent *> *graphicalComponents,  QGraphicsItemGroup *group) {
    // cria um grupo auxiliar
    QGraphicsItemGroup *newGroup = new QGraphicsItemGroup();

    // atualiza os grupos
    group->update();
    newGroup->update();

    // copia os grupos pra um novo grupo
    for (int i = 0; i < graphicalComponents->size(); i++) {
        if (ModelComponent *component = dynamic_cast<ModelComponent *>(graphicalComponents->at(i)->getComponent())) {
            newGroup->addToGroup(graphicalComponents->at(i));
        }
    }

    // pega as coordenadas do retangulo do grupo
    // isso e para contornar o caso que a posicao do grupo sempre comeca com (0, 0)
    // porem a posicao do retangulo do grupo tem as coordenadas corretas na cena
    qreal x = newGroup->boundingRect().x();
    qreal y = newGroup->boundingRect().y();

    // seta a posicao do grupo
    group->setX(x);
    group->setY(y);

    // remove os itens adicionados no grupo reserva
    for (int i = 0; i < graphicalComponents->size(); i++) {
        newGroup->removeFromGroup(graphicalComponents->at(i));
    }

    // adiciona os itens do grupo ao grupo correto
    // como a posicao do grupo foi setada, os valores xp e xy de boundingRect fica 0, pois o Qt faz uma "compensacao"
    for (int i = 0; i < graphicalComponents->size(); i++) {
        group->addToGroup(graphicalComponents->at(i));
    }

    // adiciona o grupo na cena
    addItem(group);

    // flags do grupo
    group->setFlag(QGraphicsItem::ItemIsSelectable, true);
    group->setFlag(QGraphicsItem::ItemIsMovable, true);

    // deixa todos os itens de dentro do grupo selecionado
    for (QGraphicsItem *item : group->childItems()) {
        item->setSelected(false);
    }
    // deixa o grupo selecionado
    group->setSelected(true);

    // adicione o grupo a sua lista de grupos
    getGraphicalGroups()->append(group);

    // atualiza o grupo
    group->update();

    // remove as referencias do grupo auxiliar
    destroyItemGroup(newGroup);
}


void ModelGraphicsScene::ungroupComponents(bool notify) {
    int size = selectedItems().size();
    if (size == 1) {
        QGraphicsItem* item = selectedItems().at(0);
        QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup*>(item);

        /*if (group) {
            // Recupere os itens individuais no grupo
            QList<QGraphicsItem*> itemsInGroup = group->childItems();


            // Adicione novamente os itens individuais à cena
            for (int i = 0; i < itemsInGroup.size(); i++) {
                QGraphicsItem * item = itemsInGroup.at(i);
                //remova item por item do grupo
                group->removeFromGroup(item);
                //adicionar novamente a cena
                addItem(item);
                item->setFlag(QGraphicsItem::ItemIsSelectable, true);
                item->setFlag(QGraphicsItem::ItemIsMovable, true);
            }
            // Remova o grupo da cena

            getGraphicalGroups()->removeOne(group);
            removeItem(group);
        }*/
        if (group) {
            QUndoCommand *ungroupCommand = new UngroupUndoCommand(group , this);
            _undoStack->push(ungroupCommand);
        }

        //notify graphical model change (colocar aqui um ponteiro)
        if (notify) {
            GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::CREATE;
            GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

            notifyGraphicalModelChange(eventType, eventObjectType, nullptr);
        }
    }
}

void ModelGraphicsScene::ungroupModelComponents(QGraphicsItemGroup *group) {
    QList<QGraphicsItem*> itemsInGroup = group->childItems();

    // Adicione novamente os itens individuais à cena
    for (int i = 0; i < itemsInGroup.size(); i++) {
        QGraphicsItem * item = itemsInGroup.at(i);
        //remova item por item do grupo
        group->removeFromGroup(item);
        //adicionar novamente a cena
        addItem(item);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setFlag(QGraphicsItem::ItemIsMovable, true);

        //item->setActive(true);
        item->setSelected(false);
    }
    // Remova o grupo da cena

    QList<QGraphicsItem *> selecteds = selectedItems();

    getGraphicalGroups()->removeOne(group);
    removeItem(group);
}

void ModelGraphicsScene::removeGroup(QGraphicsItemGroup* group, bool notify) {
    //Recupere os itens individuais no grupo
    QList<QGraphicsItem*> itemsInGroup = group->childItems();

    //remover todos os componentes do grupo
    // caso seja necessario desfazer o grupo

    unsigned int size = itemsInGroup.size();
    for (unsigned int i = 0; i < size; i++) {
        GraphicalModelComponent * gmc = dynamic_cast<GraphicalModelComponent *> (itemsInGroup.at(i));

        group->removeFromGroup(gmc);
        removeComponent(gmc);
    }
    _graphicalGroups->removeOne(group);
    group->update();
    removeItem(group);

    if (notify) {
        GraphicalModelEvent::EventType eventType = GraphicalModelEvent::EventType::REMOVE;
        GraphicalModelEvent::EventObjectType eventObjectType = GraphicalModelEvent::EventObjectType::OTHER;

        notifyGraphicalModelChange(eventType, eventObjectType, group);
    }
}

void ModelGraphicsScene::arranjeModels(int direction) {
    QList<QGraphicsItem *> items;
    QList<QPointF> newPositions;
    QList<QPointF> oldPositions;

    for (unsigned int i = 0; i < (unsigned int) selectedItems().size(); i++) {
        if (GraphicalModelComponent *component = dynamic_cast<GraphicalModelComponent *> (selectedItems().at(i))) {
            items.append(component);
        } else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (selectedItems().at(i))) {
            items.append(group);
        } else if (AnimationVariable *animationVariable = dynamic_cast<AnimationVariable *> (selectedItems().at(i))) {
            items.append(animationVariable);
        }
    }

    int size = items.size();
    qreal most_direction;
    qreal most_up;
    qreal most_down;
    qreal most_left;
    qreal most_right;
    qreal middle;
    qreal center;

    if (size >= 2) {
        switch (direction) {
        case 0: //left
            most_direction = sceneRect().right();
            break;
        case 1: //right
            most_direction = sceneRect().left();
            break;
        case 2: //top
            most_direction = sceneRect().bottom();
            break;
        case 3: //bottom
            most_direction = sceneRect().top();
            break;
        case 4: //center
            most_left = sceneRect().right();
            most_right = sceneRect().left();
            for (int i =0; i < size; i++) {
                QGraphicsItem* item = items.at(i);
                if (!dynamic_cast<GraphicalConnection *>(item) && !dynamic_cast<QGraphicsItemGroup *>(item)) {
                    qreal item_posX = item->x();
                    if (item_posX < most_left) {
                        most_left = item_posX;
                    }
                    if (item_posX > most_right) {
                        most_right = item_posX;
                    }
                }
            }
            center = (most_right + most_left) / 2;
            for (int i =0; i < size; i++) {
                QGraphicsItem* item = selectedItems().at(i);
                if (!dynamic_cast<GraphicalConnection *>(item)) {
                    if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item)) {
                        insertOldPositionItem(item, item->pos());
                        oldPositions.append(item->pos());
                    } else {
                        oldPositions.append(item->pos());
                    }
                    item->setX(center);
                    newPositions.append(item->pos());
                }
            }
            break;
        case 5: //middle
            most_up = sceneRect().bottom();
            most_down = sceneRect().top();
            for (int i =0; i < size; i++) {
                QGraphicsItem* item = items.at(i);
                if (!dynamic_cast<GraphicalConnection *>(item) && !dynamic_cast<QGraphicsItemGroup *>(item)) {
                    qreal item_posY = item->y();
                    if (item_posY < most_up) {
                        most_up = item_posY;
                    }
                    if (item_posY > most_down) {
                        most_down = item_posY;
                    }
                }
            }
            middle = (most_up + most_down) / 2;
            for (int i =0; i < size; i++) {
                QGraphicsItem* item = selectedItems().at(i);
                if (!dynamic_cast<GraphicalConnection *>(item)) {
                    if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item)) {
                        insertOldPositionItem(item, item->pos());
                        oldPositions.append(item->pos());
                    } else {
                        oldPositions.append(item->pos());
                    }
                    item->setX(middle);
                    newPositions.append(item->pos());
                }
            }
            break;
        }
        if (direction < 4) {
            for (int i =0; i < size; i++) {
                QGraphicsItem* item = selectedItems().at(i);
                if (!dynamic_cast<GraphicalConnection *>(item) && !dynamic_cast<GraphicalConnection *>(item)) {
                    if (direction < 2) {
                        qreal item_posX = item->x();
                        if ((item_posX < most_direction && direction == 0) || (item_posX > most_direction && direction == 1) ) {
                            most_direction = item_posX;
                        }
                    } else {
                        qreal item_posY = item->y();
                        if ((item_posY < most_direction && direction == 2) || (item_posY > most_direction && direction == 3) ) {
                            most_direction = item_posY;
                        }
                    }
                }
            }
            if (direction < 2) {
                for (int i =0; i < size; i++) {
                    QGraphicsItem* item = selectedItems().at(i);
                    if (!dynamic_cast<GraphicalConnection *>(item)) {
                        if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item)) {
                            insertOldPositionItem(item, item->pos());
                            oldPositions.append(item->pos());
                        } else {
                            oldPositions.append(item->pos());
                        }
                        item->setX(most_direction);
                        newPositions.append(item->pos());
                    }
                }
            } else {
                for (int i =0; i < size; i++) {
                    QGraphicsItem* item = selectedItems().at(i);
                    if (!dynamic_cast<GraphicalConnection *>(item)) {
                        if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item)) {
                            insertOldPositionItem(item, item->pos());
                            oldPositions.append(item->pos());
                        } else {
                            oldPositions.append(item->pos());
                        }
                        item->setY(most_direction);
                        newPositions.append(item->pos());
                    }
                }
            }
        }
        QUndoCommand *moveUndoCommand = new MoveUndoCommand(items, this, oldPositions, newPositions);
        _undoStack->push(moveUndoCommand);
    }
}


//-------------------------
// PROTECTED VIRTUAL FUNCTIONS
//-------------------------

bool ModelGraphicsScene::checkIgnoreEvent() {
    if (_simulator->getModelManager()->current()->getSimulation()) {
        if (_simulator->getModelManager()->current()->getSimulation()->isRunning()) {
            return true;
        } else {
            return false;
        }
    }
}
void ModelGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (checkIgnoreEvent()) {
        mouseEvent->ignore();
        return;
    }

    QGraphicsScene::mousePressEvent(mouseEvent);

    if (mouseEvent->button() == Qt::LeftButton) {

        QGraphicsItem* item = this->itemAt(mouseEvent->scenePos(), QTransform());

        if (GraphicalModelComponent *component = dynamic_cast<GraphicalModelComponent *> (item)) {
            component->setOldPosition(component->scenePos());
        } else {
            GraphicalComponentPort *port = dynamic_cast<GraphicalComponentPort *> (item);
            GraphicalConnection *conn = dynamic_cast<GraphicalConnection *> (item);

            if ((!conn) && (!port) && item) {
                insertOldPositionItem(item, item->pos());
            }
        }

        if (_connectingStep > 0) {
            if (item != nullptr) {
                GraphicalComponentPort* port = dynamic_cast<GraphicalComponentPort*> (item);
                if (port != nullptr) {
                    GraphicalComponentPort* src = dynamic_cast<GraphicalComponentPort*> (_sourceGraphicalComponentPort);
                    GraphicalComponentPort* dst = dynamic_cast<GraphicalComponentPort*> (_destinationGraphicalComponentPort);

                    if (_connectingStep == 1 && src == nullptr && dst == nullptr) {
                        if (!port->isInputPort() && port->getConnections()->empty()) {
                            _sourceGraphicalComponentPort = port;
                            _connectingStep = 2;
                            return;
                        } else if (port->isInputPort()) {
                            _destinationGraphicalComponentPort = port;
                            _connectingStep = 3;
                            return;
                        }
                    } else if (_connectingStep == 2 && port->isInputPort() && _sourceGraphicalComponentPort != nullptr) {
                        _destinationGraphicalComponentPort = port;
                        // create connection
                        // in the model
                        GraphicalConnection* graphicconnection = new GraphicalConnection(_sourceGraphicalComponentPort, _destinationGraphicalComponentPort);

                        // faz essa limpeza pois quando cria a conexao ela ja adiciona essa conexao nas portas
                        // porem o connectComponents ja faz isso pra quando há necessidade de fazer reconexao
                        QUndoCommand *addUndoCommand = new AddUndoCommand(graphicconnection, this);
                        _undoStack->push(addUndoCommand);

                        addItem(graphicconnection);

                        ((ModelGraphicsView *) (this->parent()))->unsetCursor();
                        _connectingStep = 0;

                        _sourceGraphicalComponentPort = nullptr;
                        _destinationGraphicalComponentPort = nullptr;
                        return;
                    } else if (_connectingStep == 3 && !port->isInputPort() && _destinationGraphicalComponentPort != nullptr && port->getConnections()->empty()) {
                        _sourceGraphicalComponentPort = port;
                        // create connection
                        // in the model
                        GraphicalConnection* graphicconnection = new GraphicalConnection(_sourceGraphicalComponentPort, _destinationGraphicalComponentPort);

                        // faz essa limpeza pois quando cria a conexao ela ja adiciona essa conexao nas portas
                        // porem o connectComponents ja faz isso pra quando há necessidade de fazer reconexao
                        QUndoCommand *addUndoCommand = new AddUndoCommand(graphicconnection, this);
                        _undoStack->push(addUndoCommand);

                        addItem(graphicconnection);

                        ((ModelGraphicsView *) (this->parent()))->unsetCursor();
                        _connectingStep = 0;

                        _sourceGraphicalComponentPort = nullptr;
                        _destinationGraphicalComponentPort = nullptr;
                        return;
                    }
                }
            } else {
                _connectingStep = 0;
                _sourceGraphicalComponentPort = nullptr;
                _destinationGraphicalComponentPort = nullptr;
                ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::ArrowCursor);
            }
        } else if (_drawingMode != NONE) {
            // Capturar o ponto de início do desenho
            _drawingStartPoint = mouseEvent->scenePos();
            _currentRectangle = nullptr;
            _currentLine = nullptr;
            _currentEllipse = nullptr;

            if (_drawingMode == POLYGON) {
                // Cria o poligono
                addGeometry(_drawingStartPoint, false);
            } else if (_drawingMode == POLYGON_POINTS) {
                // Continue a adicionar pontos ao polígono
                addGeometry(mouseEvent->scenePos(), false);
            } else if (_drawingMode == TEXT) {
                addGeometry(mouseEvent->scenePos(), false);
            } else {
                _drawing = true;
            }

            if (_drawingMode == COUNTER || _drawingMode == VARIABLE || _drawingMode == TIMER) {
                initializeAnimationDrawing(mouseEvent);
            }
        }
    }
}

void ModelGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (checkIgnoreEvent()) {
        mouseEvent->ignore();
        return;
    }

    QGraphicsScene::mouseReleaseEvent(mouseEvent);

    snapItemsToGrid();
    if (existDiagram()) {
        actualizeDiagramArrows();
    }

    QList<QGraphicsItem *> items;
    QList<QPointF> oldPositions;
    QList<QPointF> newPositions;
    QPointF myQPointF;

    foreach (QGraphicsItem* item, this->selectedItems()) {
        GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(item);
        QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup *> (item);
        GraphicalComponentPort *port = dynamic_cast<GraphicalComponentPort *> (item);
        GraphicalConnection* connection = dynamic_cast<GraphicalConnection*>(item);

        // As posições são diferentes fora da tolerância de 1.5

        if (component && component->getOldPosition() != component->scenePos()) {
            myQPointF = component->pos();
            QPointF oldPos = component->getOldPosition();

            if (!(qAbs(oldPos.x() - myQPointF.x()) <= 1.5 && qAbs(oldPos.y() - myQPointF.y()) <= 1.5)) {
                items.append(component);
                oldPositions.append(component->getOldPosition());
                newPositions.append(component->scenePos());
            }
        } else if (group) {
            myQPointF = group->pos();
            QPointF oldPos = _oldPositionsItems[group];

            if (!(qAbs(oldPos.x() - myQPointF.x()) <= 1.5 && qAbs(oldPos.y() - myQPointF.y()) <= 1.5)) {
                items.append(group);
                oldPositions.append(_oldPositionsItems[group]);
                newPositions.append(group->pos());
            }
        } else if (component == nullptr && group == nullptr && port == nullptr && connection == nullptr){
            QRectF rectScenePost = item->sceneBoundingRect();

            qreal rectX = rectScenePost.bottomLeft().x();
            qreal rectY = rectScenePost.topRight().y();

            myQPointF.setX(rectX);
            myQPointF.setY(rectY);

            QPointF oldPos = _oldPositionsItems[item];

            if (!(qAbs(oldPos.x() - myQPointF.x()) <= 1.5 && qAbs(oldPos.y() - myQPointF.y()) <= 1.5)) {
                items.append(item);
                oldPositions.append(_oldPositionsItems[item]);
                newPositions.append(item->pos());
            }
        }
    }

    if (items.size() >= 1) {
        QUndoCommand *moveUndoCommand = new MoveUndoCommand(items, this, oldPositions, newPositions);
        _undoStack->push(moveUndoCommand);
    }

    foreach (QGraphicsItem* item, items) {
        if (GraphicalModelComponent *component = dynamic_cast<GraphicalModelComponent *> (item)) {
            component->setOldPosition(item->scenePos());
        } else {
            insertOldPositionItem(item, item->pos());
        }
    }


    if (mouseEvent->button() == Qt::LeftButton && _drawingMode != NONE) {
        if (_drawingMode == COUNTER || _drawingMode == VARIABLE || _drawingMode == TIMER) {
            finishAnimationDrawing(mouseEvent);
        } else {
            // Capturar o ponto final da linha
            QPointF drawingEndPoint = mouseEvent->scenePos();
            //Adicionar desenho a tela
            addGeometry(drawingEndPoint, false);
        }
        ((ModelGraphicsView *) (this->parent()))->unsetCursor();
    } else if (_drawingMode == NONE && _currentPolygon != nullptr) {
        removeItem(_currentPolygon);
        _currentPolygon = nullptr;
        _currentPolygonPoints.clear();
    }
}

void ModelGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (checkIgnoreEvent()) {
        mouseEvent->ignore();
        return;
    }

    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);

    QGraphicsItem* item = this->itemAt(mouseEvent->scenePos(), QTransform());

    if (_connectingStep == 0) {
        _connectingStep = 1;

        GraphicalComponentPort* port = dynamic_cast<GraphicalComponentPort*> (item);

        if (port != nullptr) { // if doubleclick on a port, then start connecting
            if (!port->isInputPort() && this->_connectingStep == 1 && port->getConnections()->empty()) {
                _sourceGraphicalComponentPort = port;
                _connectingStep = 2;
            } else if (port->isInputPort() && this->_connectingStep == 1) {
                _destinationGraphicalComponentPort = port;
                _connectingStep = 3;
            } else if (!port->isInputPort() && !port->getConnections()->empty()) {
                _connectingStep = 0;
            }
        } else {
            _connectingStep = 0;
        }
    }

    if (AnimationCounter *animationCounter = dynamic_cast<AnimationCounter *>(item)) {
        DialogSelectCounter dialog;

        dialog.setCounterItems(_counters, animationCounter->getCounter());

        if (dialog.exec() == QDialog::Accepted) {
            Counter *counterSelected = dialog.selectedIndex();

            if (counterSelected) {
                animationCounter->setCounter(counterSelected);
            } else {
                animationCounter->setCounter(nullptr);
            }
        }
    }

    if (AnimationVariable *animationVariable = dynamic_cast<AnimationVariable *>(item)) {
        DialogSelectVariable dialog;

        dialog.setVariableItems(_variables, animationVariable->getVariable());

        if (dialog.exec() == QDialog::Accepted) {
            Variable *variableSelected = dialog.selectedIndex();

            if (variableSelected) {
                animationVariable->setVariable(variableSelected);
            } else {
                animationVariable->setVariable(nullptr);
            }
        }
    }


    if (AnimationTimer *animationTimer = dynamic_cast<AnimationTimer *>(item)) {
        DialogTimerConfigure dialog;

        dialog.setAnimationTimer(animationTimer);
        dialog.exec();
    }

    if (_drawingMode == POLYGON_POINTS)
        _drawingMode = POLYGON_FINISHED;
}

void ModelGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent) {
    QGraphicsScene::wheelEvent(wheelEvent);
    if (_controlIsPressed){
        if (wheelEvent->delta() > 0){
            ((ModelGraphicsView *)(this->parent()))->notifySceneWheelInEventHandler();
        }
        else{
            ((ModelGraphicsView *)(this->parent()))->notifySceneWheelOutEventHandler();
        }
        wheelEvent->accept();
    }
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalEntities() const {
    return _graphicalEntities;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalAnimations() const {
    return _graphicalAnimations;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalGeometries() const {
    return _graphicalGeometries;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalConnections() const {
    return _graphicalConnections;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalModelComponents() const {
    return _graphicalModelComponents;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalModelDataDefinitions() const {
    return _graphicalModelDataDefinitions;
}

QList<QGraphicsItem*>*ModelGraphicsScene::getGraphicalDiagramsConnections() const {
    return _graphicalDiagramConnections;
}

QList<QGraphicsItemGroup*>*ModelGraphicsScene::getGraphicalGroups() const {
    return _graphicalGroups;
}

void ModelGraphicsScene::initializeAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent) {
    if (_drawingMode == COUNTER) {
        _currentCounter = new AnimationCounter();
        _currentCounter->startDrawing(mouseEvent);
        addItem(_currentCounter);
    }

    if (_drawingMode == VARIABLE) {
        _currentVariable = new AnimationVariable();
        _currentVariable->startDrawing(mouseEvent);
        addItem(_currentVariable);
    }

    if (_drawingMode == TIMER) {
        _currentTimer = new AnimationTimer(this);
        _currentTimer->startDrawing(mouseEvent);
        addItem(_currentTimer);
    }
}

void ModelGraphicsScene::continueAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent) {
    if (_drawingMode == COUNTER) {
        if (_currentCounter) {
            if (_currentCounter->isDrawingInicialized() && !_currentCounter->isDrawingFinalized()) {
                removeItem(_currentCounter);
                _currentCounter->continueDrawing(mouseEvent);
                addItem(_currentCounter);
            }
        } else {
            _currentCounter = new AnimationCounter();
        }
    }

    if (_drawingMode == VARIABLE) {
        if (_currentVariable) {
            if (_currentVariable->isDrawingInicialized() && !_currentVariable->isDrawingFinalized()) {
                removeItem(_currentVariable);
                _currentVariable->continueDrawing(mouseEvent);
                addItem(_currentVariable);
            }
        } else {
            _currentVariable = new AnimationVariable();
        }
    }

    if (_drawingMode == TIMER) {
        if (_currentTimer) {
            if (_currentTimer->isDrawingInicialized() && !_currentTimer->isDrawingFinalized()) {
                removeItem(_currentTimer);
                _currentTimer->continueDrawing(mouseEvent);
                addItem(_currentTimer);
            } else {
                _currentTimer = new AnimationTimer(this);
            }
        }
    }
}

void ModelGraphicsScene::finishAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent) {
    QGraphicsItem *animatedItem = nullptr;

    if (_drawingMode == COUNTER) {
        if (_currentCounter) {
            if (_currentCounter->isDrawingInicialized() && !_currentCounter->isDrawingFinalized()) {
                removeItem(_currentCounter);
                _currentCounter->stopDrawing(mouseEvent);
                addItem(_currentCounter);
                animatedItem = _currentCounter;
                _currentCounter = nullptr;
            }
        }
    }

    if (_drawingMode == VARIABLE) {
        if (_currentVariable) {
            if (_currentVariable->isDrawingInicialized() && !_currentVariable->isDrawingFinalized()) {
                removeItem(_currentVariable);
                _currentVariable->stopDrawing(mouseEvent);
                addItem(_currentVariable);
                animatedItem = _currentVariable;
                _currentVariable = nullptr;
            }
        }
    }

    if (_drawingMode == TIMER) {
        if (_currentTimer) {
            if (_currentTimer->isDrawingInicialized() && !_currentTimer->isDrawingFinalized()) {
                removeItem(_currentTimer);
                _currentTimer->stopDrawing(mouseEvent);
                addItem(_currentTimer);
                animatedItem = _currentTimer;
                _currentTimer = nullptr;
            }
        }
    }

    if (animatedItem != nullptr) {
        if (_currentAction != nullptr)
            _currentAction->setChecked(false);

        QUndoCommand *addUndoCommand = new AddUndoCommand(animatedItem , this);
        _undoStack->push(addUndoCommand);

        _drawingMode = DrawingMode::NONE;
        _drawing = false;
    }
}
void ModelGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if (checkIgnoreEvent()) {
        mouseEvent->ignore();
        return;
    }

    QGraphicsScene::mouseMoveEvent(mouseEvent);

    ((ModelGraphicsView *) (this->parent()))->notifySceneMouseEventHandler(mouseEvent); // to show coords
    if (_connectingStep > 0) {
        QGraphicsItem* item = this->itemAt(mouseEvent->scenePos(), QTransform());
        if (item != nullptr) {
            GraphicalComponentPort* port = dynamic_cast<GraphicalComponentPort*> (item);
            if (port != nullptr) {
                if (_connectingStep == 1 && !port->getConnections()->empty()) {
                    port->getConnections();
                    GraphicalModelComponent *teste = port->graphicalComponent();
                    teste->getComponent();
                }
                if (_connectingStep == 1 && port->isInputPort()) {
                    ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::PointingHandCursor);
                } else if (_connectingStep == 1 && !port->isInputPort() && port->getConnections()->empty()) {
                    ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::PointingHandCursor);
                } else if (_connectingStep == 2 && port->isInputPort()) {
                    ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::PointingHandCursor);
                } else if (_connectingStep == 3 && !port->isInputPort() && port->getConnections()->empty()) {
                    ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::PointingHandCursor);
                }
                return;
            }
        }
        if (_connectingStep > 1) {
            ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::ClosedHandCursor);
        } else if (_connectingStep == 1){
            ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::CrossCursor);
        }
    }  else if (_drawingMode != NONE && _drawing){
        if (_drawingMode == COUNTER || _drawingMode == VARIABLE || _drawingMode == TIMER) {
            continueAnimationDrawing(mouseEvent);
            ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::CrossCursor);
        } else {
            //mostrar desenho se formando
            QPointF currentPoint = mouseEvent->scenePos();
            addGeometry(currentPoint, true);

            if (_drawingMode == LINE) {
                ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::SizeHorCursor);
            } else if (_drawingMode == POLYGON || _drawingMode == POLYGON_POINTS) {
                ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::ArrowCursor);
            } else {
                ((ModelGraphicsView *) (this->parent()))->setCursor(Qt::CrossCursor);
            }
        }

    }

    update();
}

void ModelGraphicsScene::focusInEvent(QFocusEvent *focusEvent) {
    QGraphicsScene::focusInEvent(focusEvent);
}

void ModelGraphicsScene::focusOutEvent(QFocusEvent *focusEvent) {
    QGraphicsScene::focusOutEvent(focusEvent);
}

void ModelGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event) {
    if (checkIgnoreEvent()) {
        event->ignore();
        return;
    }

    QGraphicsScene::dropEvent(event);
    if (this->_objectBeingDragged != nullptr) {
        QTreeWidgetItem*    treeItem = /*dynamic_cast<QTreeWidgetItem*>*/(_objectBeingDragged);
        if (treeItem != nullptr) {
            QColor color = treeItem->foreground(0).color(); // treeItem->textColor(0);
            QString pluginname = treeItem->whatsThis(0);
            Plugin* plugin = _simulator->getPluginManager()->find(pluginname.toStdString());
            if (plugin != nullptr) {
                if (plugin->getPluginInfo()->isComponent()) {
                    destroyDiagram();

                    event->setDropAction(Qt::IgnoreAction);
                    event->accept();
                    // create component in the model
                    ModelComponent* component = (ModelComponent*) plugin->newInstance(_simulator->getModelManager()->current());
                    // create graphically
                    addGraphicalModelComponent(plugin, component, event->scenePos(), color, true);
                    return;
                }
            }
        }
    }
    event->setAccepted(false);
}

void ModelGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent) {
    QGraphicsScene::contextMenuEvent(contextMenuEvent);
}

void ModelGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
    QGraphicsScene::dragEnterEvent(event);
    //QString name;
    //Plugin* plugin = _simulator->getPlugins()->find(name.toStdString());
    //if(true) {//(plugin != nullptr) {
    event->setDropAction(Qt::CopyAction);
    event->accept();
    //}
}

void ModelGraphicsScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) {
    QGraphicsScene::dragLeaveEvent(event);
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void ModelGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
    QGraphicsScene::dragMoveEvent(event);
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void ModelGraphicsScene::keyPressEvent(QKeyEvent *keyEvent) {
    QGraphicsScene::keyPressEvent(keyEvent);
    QList<QGraphicsItem*> selected = this->selectedItems();
    if (keyEvent->key() == Qt::Key_Delete && selected.size() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(this->_parentWidget, "Delete Component", "Are you sure you want to delete the selected components?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }

        destroyDiagram();
        QUndoCommand *deleteUndoCommand = new DeleteUndoCommand(selected, this);
        _undoStack->push(deleteUndoCommand);
    }
    _controlIsPressed = (keyEvent->key() == Qt::Key_Control);
}

void ModelGraphicsScene::keyReleaseEvent(QKeyEvent *keyEvent) {
    QGraphicsScene::keyReleaseEvent(keyEvent);
    if (_controlIsPressed)
        _controlIsPressed = (keyEvent->key() != Qt::Key_Control);
}

//--------------------------
//
//--------------------------

void ModelGraphicsScene::drawingCounter() {
    _drawingMode = DrawingMode::COUNTER;
}

void ModelGraphicsScene::drawingVariable() {
    _drawingMode = DrawingMode::VARIABLE;
}

void ModelGraphicsScene::drawingTimer() {
    _drawingMode = DrawingMode::TIMER;
}

void ModelGraphicsScene::setObjectBeingDragged(QTreeWidgetItem* objectBeingDragged) {
    _objectBeingDragged = objectBeingDragged;
}

void ModelGraphicsScene::setSimulator(Simulator *simulator) {
    _simulator = simulator;
}

void ModelGraphicsScene::setPropertyEditor(PropertyEditorGenesys *propEditor) {
	_propertyEditor = propEditor;
}

void ModelGraphicsScene::setPropertyList(std::map<SimulationControl*, DataComponentProperty*>* propList) {
    _propertyList = propList;
}

void ModelGraphicsScene::setPropertyEditorUI(std::map<SimulationControl*, DataComponentEditor*>* propEditorUI) {
    _propertyEditorUI = propEditorUI;
}

void ModelGraphicsScene::setComboBox(std::map<SimulationControl*, ComboBoxEnum*>* propCombo) {
    _propertyBox = propCombo;
}

unsigned short ModelGraphicsScene::connectingStep() const {
    return _connectingStep;
}

void ModelGraphicsScene::setConnectingStep(unsigned short connectingStep) {
    _connectingStep = connectingStep;
}

void ModelGraphicsScene::setParentWidget(QWidget *parentWidget) {
    _parentWidget = parentWidget;
}

void ModelGraphicsScene::setDrawingMode(DrawingMode drawingMode) {
    _drawingMode = drawingMode;
}

ModelGraphicsScene::DrawingMode ModelGraphicsScene::getDrawingMode() {
    return _drawingMode;
}

void ModelGraphicsScene::setAction(QAction *action) {
    _currentAction = action;
}

void ModelGraphicsScene::setGraphicalComponentPort(GraphicalComponentPort * in) {
    _sourceGraphicalComponentPort = in;

}

QList<GraphicalModelComponent*> *ModelGraphicsScene::getAllComponents() {
    return &_allGraphicalModelComponents;
}

QList<GraphicalModelDataDefinition*> *ModelGraphicsScene::getAllDataDefinitions() {
    return &_allGraphicalModelDataDefinitions;
}

QList<GraphicalDiagramConnection*> *ModelGraphicsScene::getAllGraphicalDiagramsConnections() {
    return &_allGraphicalDiagramConnections;
}
QList<AnimationTransition *> *ModelGraphicsScene::getAnimationsTransition() {
    return _animationsTransition;
}

QList<AnimationCounter *> *ModelGraphicsScene::getAnimationsCounter() {
    return _animationsCounter;
}
QList<AnimationVariable *> *ModelGraphicsScene::getAnimationsVariable() {
    return _animationsVariable;
}
QList<AnimationTimer *> *ModelGraphicsScene::getAnimationsTimer() {
    return _animationsTimer;
}

QMap<Event *, QList<AnimationTransition *> *>* ModelGraphicsScene::getAnimationPaused() {
    return _animationPaused;
}

QList<GraphicalConnection*> *ModelGraphicsScene::getAllConnections() {
    return &_allGraphicalConnections;
}

void ModelGraphicsScene::clearDrawingMode() {
    if (_drawingMode == ModelGraphicsScene::POLYGON_POINTS) {
        removeItem(_currentPolygon);
        _currentPolygonPoints.clear();
        _currentPolygon = nullptr;
    }

    _drawingMode = ModelGraphicsScene::NONE;
    ((QGraphicsView*)this->parent())->setCursor(Qt::ArrowCursor);
}
QList<GraphicalModelComponent*>* ModelGraphicsScene::graphicalModelComponentItems(){
    QList<GraphicalModelComponent*>* list = new QList<GraphicalModelComponent*>();
    for(QGraphicsItem* item: this->items()) {
        GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item);
        if (gmc != nullptr) {
            list->append(gmc);
        }
    }
    return list;
}

GraphicalModelComponent* ModelGraphicsScene::findGraphicalModelComponent(Util::identification id){
    QList<GraphicalModelComponent*> allComponents = ModelGraphicsScene::_allGraphicalModelComponents;

    for(GraphicalModelComponent* item: allComponents) {
        if (item->getComponent()->getId() == id) {
            return item;
        }
    }
    return nullptr;
}

void ModelGraphicsScene::clearAnimationsValues() {
    for (AnimationCounter* counter : *_animationsCounter) {
        counter->setValue(0.0);
    }

    for (AnimationVariable* variable : *_animationsVariable) {
        variable->setValue(0.0);
    }

    for (AnimationTimer* timer : *_animationsTimer) {
        timer->setTime(0.0);
    }
}



void ModelGraphicsScene::setCounters() {
    Model* currentModel = _simulator->getModelManager()->current();

    QList<ModelDataDefinition *> *counters = nullptr;

    if (currentModel) {
        _counters->clear();

        List<ModelDataDefinition *> *countersList = currentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());

        counters = new QList<ModelDataDefinition *>(countersList->list()->begin(), countersList->list()->end());

        foreach(ModelDataDefinition *counter, *counters) {
            Counter *newCounter = dynamic_cast<Counter *>(counter);

            if (newCounter) {
                _counters->append(newCounter);
            }
        }
    }
}

void ModelGraphicsScene::setVariables() {
    Model* currentModel = _simulator->getModelManager()->current();

    QList<ModelDataDefinition *> *variables = nullptr;

    if (currentModel) {
        _variables->clear();

        List<ModelDataDefinition *> *variablesList = currentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Variable>());

        variables = new QList<ModelDataDefinition *>(variablesList->list()->begin(), variablesList->list()->end());

        foreach(ModelDataDefinition *variable, *variables) {
            Variable *newVariable = dynamic_cast<Variable *>(variable);

            if (newVariable) {
                _variables->append(newVariable);
            }
        }
    }
}
//------------------------
// Private
//------------------------

