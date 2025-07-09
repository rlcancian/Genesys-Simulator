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
 * File:   ModelGraphicsScene.h
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 09:52
 */

#ifndef MODELGRAPHICSSCENE_H
#define MODELGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QTreeWidgetItem>
#include <QUndoStack>
#include <QAction>
#include <QEventLoop>
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalComponentPort.h"
#include "graphicals/GraphicalDiagramConnection.h"
#include "propertyeditor/DataComponentProperty.h"
#include "propertyeditor/DataComponentEditor.h"
#include "propertyeditor/ComboBoxEnum.h"
#include "../../../../kernel/simulator/ModelComponent.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Plugin.h"
#include "animations/AnimationTransition.h"
#include "animations/AnimationVariable.h"
#include "animations/AnimationCounter.h"
#include "animations/AnimationTimer.h"
#include "../../../../kernel/simulator/Counter.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"
#include "../../../../plugins/data/Variable.h"

class GraphicalModelEvent {
public:

    enum class EventType : int {
        CREATE = 1,
        REMOVE = 2,
        EDIT = 3,
        CLONE = 4,
        OTHER = 9
    };

    enum class EventObjectType : int {
        COMPONENT = 1,
        DATADEFINITION = 2,
        CONNECTION = 3,
        DRAWING = 4,
        ANIMATION = 5,
        PLOT = 6,
        OTHER = 9
    };

public:

    GraphicalModelEvent(GraphicalModelEvent::EventType eventType, GraphicalModelEvent::EventObjectType eventObjectType, QGraphicsItem* item) {
        this->eventType = eventType;
        this->eventObjectType = eventObjectType;
        this->item = item;
    }
    GraphicalModelEvent::EventType eventType;
    GraphicalModelEvent::EventObjectType eventObjectType;
    QGraphicsItem* item;
};

class ModelGraphicsScene : public QGraphicsScene {
public:
    ModelGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    ModelGraphicsScene(const ModelGraphicsScene& orig);
    virtual ~ModelGraphicsScene();
public: // editing graphic model
    enum DrawingMode{
        NONE, LINE, TEXT, RECTANGLE, ELLIPSE, POLYGON,  POLYGON_POINTS, POLYGON_FINISHED, COUNTER, VARIABLE, TIMER
    };
    GraphicalModelComponent* addGraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color = Qt::blue, bool notify = false);
    GraphicalConnection* addGraphicalConnection(GraphicalComponentPort* sourcePort, GraphicalComponentPort* destinationPort, unsigned int portSourceConnection, unsigned int portDestinationConnection, bool notify = false);
    GraphicalModelDataDefinition* addGraphicalModelDataDefinition(Plugin* plugin, ModelDataDefinition* element, QPointF position, QColor color = Qt::blue);
    GraphicalDiagramConnection* addGraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, GraphicalDiagramConnection::ConnectionType type);
    void initializeAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    void continueAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    void finishAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    void startTextEditing();
    void removeComponent(GraphicalModelComponent* gmc, bool notify = false);
    void clearConnectionsComponent(GraphicalModelComponent* gmc);
    void clearInputConnectionsComponent(GraphicalModelComponent* graphicalComponent);
    void clearOutputConnectionsComponent(GraphicalModelComponent* graphicalComponent);
    void clearPorts(GraphicalConnection* connection, GraphicalModelComponent *source, GraphicalModelComponent *destination);
    void connectComponents(GraphicalConnection* connection, GraphicalModelComponent *source = nullptr, GraphicalModelComponent *destination = nullptr, bool notify = false);
    bool connectSource(GraphicalConnection* connection, GraphicalModelComponent *source = nullptr);
    bool connectDestination(GraphicalConnection* connection, GraphicalModelComponent *destination = nullptr);
    void redoConnections(GraphicalModelComponent *graphicalComponent, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections);
    void removeComponentInModel(GraphicalModelComponent* gmc);
    void insertComponent(GraphicalModelComponent* gmc, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections, bool addGMC = true, bool addAllGMC = true, bool notify = false);
    void removeGraphicalConnection(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source, GraphicalModelComponent *destination, bool notify = false);
    void removeConnectionInModel(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source);
    void addGeometry(QPointF endPoint, bool moving);
    void addDrawing(QGraphicsItem * item, bool notify = false);
    bool addDrawingGeometry(QGraphicsItem * item);
    bool addDrawingAnimation(QGraphicsItem * item);
    void removeGraphicalModelDataDefinition(GraphicalModelDataDefinition* gmdd);
    void removeGraphicalDiagramConnection(GraphicalDiagramConnection* connection);
    void removeDrawing(QGraphicsItem * item, bool notify = false);
    bool removeDrawingGeometry(QGraphicsItem * item);
    bool removeDrawingAnimation(QGraphicsItem * item);
    void removeGroup(QGraphicsItemGroup* group, bool notify = false);
    void clearGraphicalModelComponents();
    void clearGraphicalModelConnections();
    void groupComponents(bool notify = false); // tenta agrupar (verifica se sao ModelGraphicalComponents)
    void groupModelComponents(QList<GraphicalModelComponent *> *graphicalComponents, QGraphicsItemGroup *group); // agrupa componentes
    void ungroupComponents(bool notify = false);
    void ungroupModelComponents(QGraphicsItemGroup *group);
    void notifyGraphicalModelChange(GraphicalModelEvent::EventType eventType, GraphicalModelEvent::EventObjectType eventObjectType, QGraphicsItem *item);
    QList<GraphicalModelComponent*>* graphicalModelComponentItems();
    GraphicalModelComponent* findGraphicalModelComponent(Util::identification id);
public:
    struct GRID {
        unsigned int interval;
        QPen pen;
        std::list<QGraphicsLineItem *> *lines;
        bool visible;
        void clear();
    };
    GRID *grid();
    void showGrid();
    void snapItemsToGrid();
    void actualizeDiagramArrows();
    void showDiagrams();
    void hideDiagrams();
    bool existDiagram();
    bool visibleDiagram();
    void destroyDiagram();
    void createDiagrams();
    QUndoStack* getUndoStack();
    Simulator* getSimulator();
    void setUndoStack(QUndoStack* undo);
    void beginConnection();
    void setSimulator(Simulator *simulator);
    void setPropertyEditor(PropertyEditorGenesys *propEditor);
    void setPropertyList(std::map<SimulationControl*, DataComponentProperty*>* propList);
    void setPropertyEditorUI(std::map<SimulationControl*, DataComponentEditor*>* propEditorUI);
    void setComboBox(std::map<SimulationControl*, ComboBoxEnum*>* propCombo);
    void setObjectBeingDragged(QTreeWidgetItem* objectBeingDragged);
    void setParentWidget(QWidget *parentWidget);
    unsigned short connectingStep() const;
    void setConnectingStep(unsigned short connectingStep);
    void setSnapToGrid(bool activated);
    bool getSnapToGrid();
    void arranjeModels(int direction);
    void setDrawingMode(DrawingMode drawingMode);
    void clearDrawingMode();
    void setGraphicalComponentPort(GraphicalComponentPort * in);
    DrawingMode getDrawingMode();
    void setAction(QAction* action);
    QList<GraphicalModelComponent*> *getAllComponents();
    QList<GraphicalConnection*> *getAllConnections();
    QList<GraphicalModelDataDefinition*> *getAllDataDefinitions();
    QList<GraphicalDiagramConnection*> *getAllGraphicalDiagramsConnections();
    QMap<QGraphicsItemGroup *, QList<GraphicalModelComponent *>> getListComponentsGroup();
    void insertComponentGroup(QGraphicsItemGroup *group, QList<GraphicalModelComponent *> componentsGroup);
    void insertOldPositionItem(QGraphicsItem *item, QPointF position);
    QPointF getOldPositionItem(QGraphicsItem *item) const;
    QList<AnimationTransition *> *getAnimationsTransition();
    QList<AnimationCounter *> *getAnimationsCounter();
    QList<AnimationVariable *> *getAnimationsVariable();
    QList<AnimationTimer *> *getAnimationsTimer();
    QMap<Event *, QList<AnimationTransition *> *> *getAnimationPaused();
    bool checkIgnoreEvent();
    void clearAnimations();
    void clearAnimationsTransition();
    void clearAnimationsCounter();
    void clearAnimationsVariable();
    void clearAnimationsTimer();
    void clearAnimationsQueue();
    QList<QString> *getImagesAnimation();
    void drawingCounter();
    void drawingVariable();
    void drawingTimer();
    void clearAnimationsValues();
    void setCounters();
    void setVariables();
    // TODO: Funções abaixo são usadas para reaver os dataDefinitions do componente nos dataDefinitions do modelo dos componentes deletados, "checados" e reinseridos (Control Z de um delete, por exemplo).
    // O kernel não trata este caso, ale acusa erro, pois não encontra os dataDefinitions do componente nos dataDefinitions do modelo, pois ele os remove como "órfãos" e não os reinsere quando voltados ao modelo.
    void insertRestoredDataDefinitions(bool loaded);
    void saveDataDefinitions();
    // --------------------------------- //

public:
    QList<QGraphicsItem*>*getGraphicalModelDataDefinitions() const;
    QList<QGraphicsItem*>*getGraphicalModelComponents() const;
    QList<QGraphicsItem*>*getGraphicalConnections() const;
    QList<QGraphicsItem*>*getGraphicalGeometries() const;
    QList<QGraphicsItem*>*getGraphicalAnimations() const;
    QList<QGraphicsItem*>*getGraphicalEntities() const;
    QList<QGraphicsItem*>*getGraphicalDiagramsConnections() const;
    QList<QGraphicsItemGroup*>*getGraphicalGroups() const;

public:
    void animateTransition(ModelComponent *source, ModelComponent *destination, bool viewSimulation, Event *event);
    void runAnimateTransition(AnimationTransition *animationTransition, Event *event, bool restart = false);
    void handleAnimationStateChanged(QAbstractAnimation::State newState, QEventLoop* loop, Event* event, AnimationTransition* animationTransition);
    void animateQueueInsert(ModelComponent *component, bool visivible);
    void animateQueueRemove(ModelComponent *component);
    void animateCounter();
    void animateVariable();
    void animateTimer(double time);

protected: // virtual functions
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    //virtual void	drawBackground(QPainter *painter, const QRectF &rect);
    //virtual void	drawForeground(QPainter *painter, const QRectF &rect);
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
    virtual void focusInEvent(QFocusEvent *focusEvent);
    virtual void focusOutEvent(QFocusEvent *focusEvent);
    //virtual void	helpEvent(QGraphicsSceneHelpEvent *helpEvent);
    //virtual void	inputMethodEvent(QInputMethodEvent *event);
    virtual void keyPressEvent(QKeyEvent *keyEvent);
    virtual void keyReleaseEvent(QKeyEvent *keyEvent);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *wheelEvent);

private:
    GRID _grid;
    Simulator* _simulator = nullptr;
    PropertyEditorGenesys* _propertyEditor = nullptr;
    std::map<SimulationControl*, DataComponentProperty*>* _propertyList = nullptr;
    std::map<SimulationControl*, DataComponentEditor*>* _propertyEditorUI = nullptr;
    std::map<SimulationControl*, ComboBoxEnum*>* _propertyBox = nullptr;
    QTreeWidgetItem* _objectBeingDragged = nullptr;
    QWidget* _parentWidget;
    QList<GraphicalModelComponent*> _allGraphicalModelComponents;
    QList<GraphicalConnection*> _allGraphicalConnections;
    QList<GraphicalModelDataDefinition*> _allGraphicalModelDataDefinitions;
    QList<GraphicalDiagramConnection*> _allGraphicalDiagramConnections;
    QUndoStack *_undoStack = nullptr;
    QMap<QGraphicsItemGroup *, QList<GraphicalModelComponent *> > _listComponentsGroup;
    QMap<QGraphicsItem *, QPointF> _oldPositionsItems;
    QList<Counter *> *_counters = new QList<Counter *>();
    QList<Variable *> *_variables = new QList<Variable *>();
private:
    DrawingMode _drawingMode = NONE;
    QGraphicsRectItem* _currentRectangle = nullptr;
    QGraphicsLineItem* _currentLine = nullptr;
    QGraphicsPolygonItem* _currentPolygon = nullptr;
    QGraphicsEllipseItem* _currentEllipse = nullptr;
    QPolygonF _currentPolygonPoints;
    QPointF _drawingStartPoint;
    QAction* _currentAction = nullptr;
    bool _drawing = false;
    bool _diagram = false;
    bool _visibleDiagram = false;
    unsigned short _connectingStep = 0; //0:nothing, 1:waiting click on source or destination, 2: click on source, 3: click on destination
    bool _controlIsPressed = false;
    bool _snapToGrid = false;
    GraphicalComponentPort* _sourceGraphicalComponentPort;
    GraphicalComponentPort* _destinationGraphicalComponentPort;
    AnimationCounter *_currentCounter;
    AnimationVariable *_currentVariable;
    AnimationTimer *_currentTimer;
    QMap<Event *, QList<AnimationTransition *> *> *_animationPaused = new QMap<Event *, QList<AnimationTransition *> *>();

private:
    QList<QString> *_imagesAnimation = new QList<QString>;
    QList<AnimationTransition *> *_animationsTransition = new QList<AnimationTransition*>();
    QList<AnimationCounter *> *_animationsCounter = new QList<AnimationCounter*>();
    QList<AnimationVariable *> *_animationsVariable = new QList<AnimationVariable*>();
    QList<AnimationTimer *> *_animationsTimer = new QList<AnimationTimer*>();
private:
    // IMPORTANT. MUST BE CONSISTENT WITH SIMULATOR->MODEL
    QList<QGraphicsItem*>* _graphicalModelComponents = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalModelDataDefinitions = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalConnections = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalDiagramConnections = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalAssociations = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalGeometries = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalAnimations = new QList<QGraphicsItem*>();
    QList<QGraphicsItem*>* _graphicalEntities = new QList<QGraphicsItem*>();
    QList<QGraphicsItemGroup*>* _graphicalGroups = new QList<QGraphicsItemGroup*>();
};

#endif /* MODELGRAPHICSSCENE_H */

