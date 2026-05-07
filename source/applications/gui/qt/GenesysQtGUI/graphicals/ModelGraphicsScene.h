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
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Plugin.h"
#include "animations/AnimationTransition.h"
#include "animations/AnimationVariable.h"
#include "animations/AnimationCounter.h"
#include "animations/AnimationTimer.h"
#include "animations/AnimationPlaceholder.h"
#include "kernel/simulator/Counter.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/PropertyGenesys.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "../../../../../plugins/data/Logic/Variable.h"

/**
 * @brief Lightweight event envelope used to notify GUI changes in the graphics scene.
 */
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

    /** @brief Creates one event payload for the given item and event categories. */
    GraphicalModelEvent(GraphicalModelEvent::EventType eventType, GraphicalModelEvent::EventObjectType eventObjectType, QGraphicsItem* item) {
        this->eventType = eventType;
        this->eventObjectType = eventObjectType;
        this->item = item;
    }
    GraphicalModelEvent::EventType eventType;
    GraphicalModelEvent::EventObjectType eventObjectType;
    QGraphicsItem* item;
};

/**
 * @brief Scene that owns and manipulates the graphical representation of a model.
 *
 * The scene manages components, ports, connections, drawings, animations, and the auxiliary
 * data-definition structures used by the GUI. It is the central editing surface for the
 * graphical model and the main consumer of the view/controller services documented elsewhere.
 */
class ModelGraphicsScene : public QGraphicsScene {
public:
    ModelGraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = nullptr);
    // Disable copy construction to keep scene ownership state unique.
    ModelGraphicsScene(const ModelGraphicsScene& orig) = delete;
    // Disable copy assignment to prevent shallow copies of GUI-owned resources.
    ModelGraphicsScene& operator=(const ModelGraphicsScene& other) = delete;
    virtual ~ModelGraphicsScene();
public: // editing graphic model
    enum DrawingMode{
        NONE, LINE, TEXT, RECTANGLE, ELLIPSE, POLYGON,  POLYGON_POINTS, POLYGON_FINISHED,
        COUNTER, VARIABLE, TIMER,
        ATTRIBUTE, ENTITY, EVENT, EXPRESSION, PLOT, QUEUE_PLACEHOLDER, RESOURCE, STATION, STATISTICS
    };
    /** @brief Creates and inserts one graphical component into the scene. */
    GraphicalModelComponent* addGraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color = Qt::blue, bool notify = false, GraphicalModelComponent* autoConnectSource = nullptr);
    /** @brief Creates and inserts one graphical connection between two ports. */
    GraphicalConnection* addGraphicalConnection(GraphicalComponentPort* sourcePort, GraphicalComponentPort* destinationPort, unsigned int portSourceConnection, unsigned int portDestinationConnection, bool notify = false);
    /** @brief Creates and inserts one graphical data definition into the scene. */
    GraphicalModelDataDefinition* addGraphicalModelDataDefinition(Plugin* plugin, ModelDataDefinition* element, QPointF position, QColor color = Qt::blue);
    /** @brief Creates or updates a diagram-level connection between data definitions. */
    GraphicalDiagramConnection* addGraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, GraphicalDiagramConnection::ConnectionType type);
    /** @brief Starts the interactive drawing workflow for animation-related items. */
    void initializeAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    /** @brief Continues the interactive drawing workflow. */
    void continueAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    /** @brief Finalizes the interactive drawing workflow. */
    void finishAnimationDrawing(QGraphicsSceneMouseEvent *mouseEvent);
    /** @brief Opens inline text editing for the current text item. */
    void startTextEditing();
    /** @brief Removes one component and optionally emits the graphical change notification. */
    void removeComponent(GraphicalModelComponent* gmc, bool notify = false);
    /** @brief Clears input and output connections attached to one component. */
    void clearConnectionsComponent(GraphicalModelComponent* gmc);
    /** @brief Removes all incoming connections from one component. */
    void clearInputConnectionsComponent(GraphicalModelComponent* graphicalComponent);
    /** @brief Removes all outgoing connections from one component. */
    void clearOutputConnectionsComponent(GraphicalModelComponent* graphicalComponent);
    /** @brief Clears the connection ports associated with one connection before removal. */
    void clearPorts(GraphicalConnection* connection, GraphicalModelComponent *source, GraphicalModelComponent *destination);
    /** @brief Connects one prepared connection to its source and destination items. */
    void connectComponents(GraphicalConnection* connection, GraphicalModelComponent *source = nullptr, GraphicalModelComponent *destination = nullptr, bool notify = false);
    /** @brief Binds a source component to a connection while checking port constraints. */
    bool connectSource(GraphicalConnection* connection, GraphicalModelComponent *source = nullptr);
    /** @brief Binds a destination component to a connection while checking port constraints. */
    bool connectDestination(GraphicalConnection* connection, GraphicalModelComponent *destination = nullptr);
    /** @brief Recreates all connection links for one graphical component. */
    void redoConnections(GraphicalModelComponent *graphicalComponent, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections);
    /** @brief Removes one component from the kernel model as part of scene deletion. */
    void removeComponentInModel(GraphicalModelComponent* gmc);
    /** @brief Inserts an existing component back into the scene and restores its connections. */
    void insertComponent(GraphicalModelComponent* gmc, QList<GraphicalConnection *> *inputConnections, QList<GraphicalConnection *> *outputConnections, bool addGMC = true, bool addAllGMC = true, bool notify = false);
    /** @brief Removes a graphical connection and updates the source/destination items. */
    void removeGraphicalConnection(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source, GraphicalModelComponent *destination, bool notify = false);
    /** @brief Removes the connection from the kernel model while keeping scene state consistent. */
    void removeConnectionInModel(GraphicalConnection* graphicalConnection, GraphicalModelComponent *source);
    /** @brief Adds one freeform drawing geometry item to the scene. */
    void addGeometry(QPointF endPoint, bool moving);
    /** @brief Adds one drawing item to the scene and optionally notifies listeners. */
    void addDrawing(QGraphicsItem * item, bool notify = false);
    /** @brief Attempts to register one geometry drawing item. */
    bool addDrawingGeometry(QGraphicsItem * item);
    /** @brief Attempts to register one animation drawing item. */
    bool addDrawingAnimation(QGraphicsItem * item);
    /** @brief Removes one graphical data definition from the scene. */
    void removeGraphicalModelDataDefinition(GraphicalModelDataDefinition* gmdd);
    /** @brief Detaches one graphical data definition without deleting the underlying model data. */
    void detachGraphicalModelDataDefinition(GraphicalModelDataDefinition* gmdd);
    /** @brief Restores one previously detached graphical data definition. */
    void restoreGraphicalModelDataDefinition(GraphicalModelDataDefinition* gmdd);
    /** @brief Removes one diagram connection from the scene. */
    void removeGraphicalDiagramConnection(GraphicalDiagramConnection* connection);
    /** @brief Clears all graphical data definitions from the scene. */
    void clearGraphicalModelDataDefinitions();
    /** @brief Clears all diagram connections from the scene. */
    void clearGraphicalDiagramConnections();
    /** @brief Repairs bookkeeping used to track graphical data definitions. */
    void sanitizeGraphicalDataDefinitionsBookkeeping();
    /** @brief Shows or hides the diagram layer and its creation state. */
    void setDiagramLayerState(bool diagramCreated, bool visible);
    // Return only items that can be directly manipulated by user edit commands.
    /** @brief Filters out scene items that the user cannot directly manipulate. */
    QList<QGraphicsItem*> userOperableItems(const QList<QGraphicsItem*>& items) const;
    // Filter out non-deletable items from user-triggered delete flows.
    /** @brief Filters the selected items down to items that can be deleted. */
    QList<QGraphicsItem*> userDeletableItems(const QList<QGraphicsItem*>& items) const;
    // Keep internal data-definition initial grouping opt-in during model rebuild only.
    /** @brief Optionally groups newly created internal data definitions under their component. */
    void ensureInitialInternalDataDefinitionGrouping(GraphicalModelDataDefinition* dataDefinition, GraphicalModelComponent* component);
    // Allow serializer to disable automatic initial grouping while persisted GUI state is being restored.
    /** @brief Marks that persisted GUI state is being restored. */
    void setPersistedGuiRestoreInProgress(bool restoring);
    /** @brief Returns whether persisted GUI state restoration is in progress. */
    bool isPersistedGuiRestoreInProgress() const;
    /** @brief Removes one freeform drawing item from the scene. */
    void removeDrawing(QGraphicsItem * item, bool notify = false);
    bool removeDrawingGeometry(QGraphicsItem * item);
    bool removeDrawingAnimation(QGraphicsItem * item);
    /** @brief Removes one graphical group item from the scene. */
    void removeGroup(QGraphicsItemGroup* group, bool notify = false);
    /** @brief Clears all graphical component items from the scene. */
    void clearGraphicalModelComponents();
    /** @brief Clears all graphical connection items from the scene. */
    void clearGraphicalModelConnections();
    /** @brief Groups the current selection into a scene group when possible. */
    void groupComponents(bool notify = false); // tenta agrupar (verifica se sao ModelGraphicalComponents)
    /** @brief Groups the supplied graphical components into the provided group item. */
    void groupModelComponents(QList<GraphicalModelComponent *> *graphicalComponents, QGraphicsItemGroup *group); // agrupa componentes
    /** @brief Ungroups the current selection when it contains a valid group item. */
    void ungroupComponents(bool notify = false);
    /** @brief Ungroups one group item and restores its child components. */
    void ungroupModelComponents(QGraphicsItemGroup *group);
    /** @brief Emits a scene-level change notification used by higher GUI layers. */
    void notifyGraphicalModelChange(GraphicalModelEvent::EventType eventType, GraphicalModelEvent::EventObjectType eventObjectType, QGraphicsItem *item);
    // Return model component items by value to avoid heap ownership transfer.
    /** @brief Returns the list of graphical component items currently in the scene. */
    QList<GraphicalModelComponent*> graphicalModelComponentItems();
    /** @brief Finds one graphical component by kernel identification. */
    GraphicalModelComponent* findGraphicalModelComponent(Util::identification id);
    /** @brief Finds one graphical data definition by kernel pointer. */
    GraphicalModelDataDefinition* findGraphicalModelDataDefinition(ModelDataDefinition* dataDefinition);
    /** @brief Resolves the source component associated with one graphical connection. */
    GraphicalModelComponent* resolveSourceComponent(GraphicalConnection* connection) const;
    /** @brief Resolves the destination component associated with one graphical connection. */
    GraphicalModelComponent* resolveDestinationComponent(GraphicalConnection* connection) const;
public:
    struct GRID {
        unsigned int interval;
        QPen pen;
        std::list<QGraphicsLineItem *> *lines;
        bool visible;
        void clear();
    };
    /** @brief Returns the current grid state owned by the scene. */
    GRID *grid();
    /** @brief Shows the grid using the current grid configuration. */
    void showGrid();
    // Aplica o estado visual do grid de forma determinística sem alternância implícita.
    /** @brief Sets grid visibility without toggling the state implicitly. */
    void setGridVisible(bool visible);
    // Informa o estado visual atual do grid para sincronização com QAction.
    /** @brief Returns whether the grid is currently visible. */
    bool isGridVisible() const;
    /** @brief Snaps movable items to the active grid spacing. */
    void snapItemsToGrid();
    /** @brief Recomputes the direction arrows used by diagram connections. */
    void actualizeDiagramArrows();
    /** @brief Shows all diagram-layer items. */
    void showDiagrams();
    /** @brief Hides all diagram-layer items. */
    void hideDiagrams();
    /** @brief Returns whether diagram-layer items exist. */
    bool existDiagram();
    /** @brief Returns whether diagram-layer items are currently visible. */
    bool visibleDiagram();
    /** @brief Destroys the diagram layer and its cached items. */
    void destroyDiagram();
    /** @brief Creates the diagram layer and its cached items. */
    void createDiagrams();
    /** @brief Returns the undo stack used by the scene. */
    QUndoStack* getUndoStack();
    /** @brief Returns the simulator bound to this scene. */
    Simulator* getSimulator();
    /** @brief Replaces the undo stack used by the scene. */
    void setUndoStack(QUndoStack* undo);
    /** @brief Starts a new component connection gesture. */
    void beginConnection();
    /** @brief Sets the simulator dependency used by scene logic. */
    void setSimulator(Simulator *simulator);
    /** @brief Sets the property editor dependency used by scene logic. */
    void setPropertyEditor(PropertyEditorGenesys *propEditor);
    /** @brief Sets the property list dependency used by scene logic. */
    void setPropertyList(std::map<SimulationControl*, DataComponentProperty*>* propList);
    /** @brief Sets the property-editor widget map used by scene logic. */
    void setPropertyEditorUI(std::map<SimulationControl*, DataComponentEditor*>* propEditorUI);
    /** @brief Sets the enum combo-box map used by scene logic. */
    void setComboBox(std::map<SimulationControl*, ComboBoxEnum*>* propCombo);
    /** @brief Records which tree item is currently being dragged into the scene. */
    void setObjectBeingDragged(QTreeWidgetItem* objectBeingDragged);
    /** @brief Marks whether the scene is restoring persisted GUI layout. */
    void setRestoringPersistedGuiLayout(bool restoring);
    /** @brief Returns whether the scene is restoring persisted GUI layout. */
    bool isRestoringPersistedGuiLayout() const;
    /** @brief Sets the parent widget used by transient UI interactions. */
    void setParentWidget(QWidget *parentWidget);
    /** @brief Returns the current step in the connection gesture. */
    unsigned short connectingStep() const;
    /** @brief Updates the current step in the connection gesture. */
    void setConnectingStep(unsigned short connectingStep);
    /** @brief Enables or disables grid snapping. */
    void setSnapToGrid(bool activated);
    /** @brief Returns whether grid snapping is enabled. */
    bool getSnapToGrid();
    /** @brief Rearranges model items using the specified layout direction. */
    void arranjeModels(int direction);
    /** @brief Sets the current drawing mode used by mouse gestures. */
    void setDrawingMode(DrawingMode drawingMode);
    /** @brief Clears the current drawing mode. */
    void clearDrawingMode();
    /** @brief Sets the current port used by connection gestures. */
    void setGraphicalComponentPort(GraphicalComponentPort * in);
    /** @brief Returns the current drawing mode. */
    DrawingMode getDrawingMode();
    /** @brief Sets the QAction that tracks the current scene tool. */
    void setAction(QAction* action);
    /** @brief Returns all component items owned by the scene. */
    QList<GraphicalModelComponent*> *getAllComponents();
    /** @brief Returns all connection items owned by the scene. */
    QList<GraphicalConnection*> *getAllConnections();
    /** @brief Returns all data-definition items owned by the scene. */
    QList<GraphicalModelDataDefinition*> *getAllDataDefinitions();
    /** @brief Returns all diagram-connection items owned by the scene. */
    QList<GraphicalDiagramConnection*> *getAllGraphicalDiagramsConnections();
    /** @brief Returns the mapping used to restore grouped components. */
    QMap<QGraphicsItemGroup *, QList<GraphicalModelComponent *>> getListComponentsGroup();
    /** @brief Stores the component list associated with one graphical group. */
    void insertComponentGroup(QGraphicsItemGroup *group, QList<GraphicalModelComponent *> componentsGroup);
    /** @brief Stores the previous scene position for one item. */
    void insertOldPositionItem(QGraphicsItem *item, QPointF position);
    /** @brief Returns the previous scene position stored for one item. */
    QPointF getOldPositionItem(QGraphicsItem *item) const;
    /** @brief Returns the active transition animations tracked by the scene. */
    QList<AnimationTransition *> *getAnimationsTransition();
    /** @brief Returns the active counter animations tracked by the scene. */
    QList<AnimationCounter *> *getAnimationsCounter();
    /** @brief Returns the active variable animations tracked by the scene. */
    QList<AnimationVariable *> *getAnimationsVariable();
    /** @brief Returns the active timer animations tracked by the scene. */
    QList<AnimationTimer *> *getAnimationsTimer();
    /** @brief Returns the placeholder animation items tracked by the scene. */
    QList<AnimationPlaceholder *> *getAnimationsPlaceholder();
    /** @brief Returns paused animations indexed by their simulation event. */
    QMap<Event *, QList<AnimationTransition *> *> *getAnimationPaused();
    /** @brief Returns true when the scene should suppress animation-driven event handling. */
    bool checkIgnoreEvent();
    /** @brief Clears all animation categories tracked by the scene. */
    void clearAnimations();
    /** @brief Clears transition animations tracked by the scene. */
    void clearAnimationsTransition();
    /** @brief Clears counter animations tracked by the scene. */
    void clearAnimationsCounter();
    /** @brief Clears variable animations tracked by the scene. */
    void clearAnimationsVariable();
    /** @brief Clears timer animations tracked by the scene. */
    void clearAnimationsTimer();
    /** @brief Clears placeholder animations tracked by the scene. */
    void clearAnimationsPlaceholder();
    /** @brief Clears queue animation overlays tracked by the scene. */
    void clearAnimationsQueue();
    /** @brief Refreshes plot animation overlays for the current scene state. */
    void animatePlot();
    /** @brief Returns the list of animation image identifiers used by the scene. */
    QList<QString> *getImagesAnimation();
    /** @brief Creates counter overlay geometry. */
    void drawingCounter();
    /** @brief Creates variable overlay geometry. */
    void drawingVariable();
    /** @brief Creates timer overlay geometry. */
    void drawingTimer();
    /** @brief Creates attribute overlay geometry. */
    void drawingAttribute();
    /** @brief Creates entity overlay geometry. */
    void drawingEntity();
    /** @brief Creates event overlay geometry. */
    void drawingEvent();
    /** @brief Creates expression overlay geometry. */
    void drawingExpression();
    /** @brief Creates plot overlay geometry. */
    void drawingPlot();
    /** @brief Creates queue overlay geometry. */
    void drawingQueue();
    /** @brief Creates resource overlay geometry. */
    void drawingResource();
    /** @brief Creates station overlay geometry. */
    void drawingStation();
    /** @brief Creates statistics overlay geometry. */
    void drawingStatistics();
    /** @brief Clears animation-specific cached values. */
    void clearAnimationsValues();
    /** @brief Synchronizes the component counters used by animation overlays. */
    void setCounters();
    /** @brief Synchronizes the variable counters used by animation overlays. */
    void setVariables();
    // TODO: Funções abaixo são usadas para reaver os dataDefinitions do componente nos dataDefinitions do modelo dos componentes deletados, "checados" e reinseridos (Control Z de um delete, por exemplo).
    // O kernel não trata este caso, ale acusa erro, pois não encontra os dataDefinitions do componente nos dataDefinitions do modelo, pois ele os remove como "órfãos" e não os reinsere quando voltados ao modelo.
    /** @brief Restores data definitions removed by previous delete/check cycles. */
    void insertRestoredDataDefinitions(bool loaded);
    /** @brief Caches data definitions before destructive scene operations. */
    void saveDataDefinitions();
    // --------------------------------- //
    /** @brief Requests an immediate data-definition synchronization pass. */
    void requestGraphicalDataDefinitionsSync();
    /** @brief Schedules a deferred data-definition synchronization pass. */
    void scheduleGraphicalDataDefinitionsSync();
    /** @brief Returns whether a data-definition sync is currently in progress. */
    bool isGraphicalDataDefinitionsSyncInProgress() const;
    /** @brief Shows or hides statistics-related data definitions. */
    void setShowStatisticsDataDefinitions(bool show);
    /** @brief Returns whether statistics-related data definitions are visible. */
    bool showStatisticsDataDefinitions() const;
    /** @brief Shows or hides editable data definitions. */
    void setShowEditableDataDefinitions(bool show);
    /** @brief Returns whether editable data definitions are visible. */
    bool showEditableDataDefinitions() const;
    /** @brief Shows or hides shared data definitions. */
    void setShowSharedDataDefinitions(bool show);
    /** @brief Returns whether shared data definitions are visible. */
    bool showSharedDataDefinitions() const;
    /** @brief Shows or hides recursive data-definition expansion. */
    void setShowRecursiveDataDefinitions(bool show);
    /** @brief Returns whether recursive data-definition expansion is enabled. */
    bool showRecursiveDataDefinitions() const;
    /**
     * @brief Restricts graphical rebuild/synchronization services to one kernel model level.
     *
     * ModelGraphicsScene keeps this rendering context because some synchronization entry points
     * are static services called from the property editor. Storing the active level in the scene
     * lets those calls preserve the same root/submodel scope as the visible tab.
     */
    void setModelLevelFilter(unsigned int modelLevel);
    /** @brief Clears any level restriction and lets rebuild services render all model levels. */
    void clearModelLevelFilter();
    /** @brief Returns true when this scene is scoped to a specific ModelDataDefinition level. */
    bool hasModelLevelFilter() const;
    /** @brief Returns the active level used by rebuild services when the filter is enabled. */
    unsigned int modelLevelFilter() const;
    /** @brief Shows or hides internal data definitions. */
    void setShowInternalDataDefinitions(bool show);
    /** @brief Returns whether internal data definitions are visible. */
    bool showInternalDataDefinitions() const;
    /** @brief Shows or hides attached data definitions. */
    void setShowAttachedDataDefinitions(bool show);
    /** @brief Returns whether attached data definitions are visible. */
    bool showAttachedDataDefinitions() const;
    /** @brief Temporarily blocks connection-geometry updates. */
    void setConnectionGeometryUpdatesBlocked(bool blocked);
    /** @brief Returns whether connection-geometry updates are blocked. */
    bool areConnectionGeometryUpdatesBlocked() const;

public:
    /** @brief Returns the scene items classified as model data definitions. */
    QList<QGraphicsItem*>*getGraphicalModelDataDefinitions() const;
    /** @brief Returns the scene items classified as model components. */
    QList<QGraphicsItem*>*getGraphicalModelComponents() const;
    /** @brief Returns the scene items classified as connections. */
    QList<QGraphicsItem*>*getGraphicalConnections() const;
    /** @brief Returns the scene items classified as geometry drawings. */
    QList<QGraphicsItem*>*getGraphicalGeometries() const;
    /** @brief Returns the scene items classified as animations. */
    QList<QGraphicsItem*>*getGraphicalAnimations() const;
    /** @brief Returns the scene items classified as entities. */
    QList<QGraphicsItem*>*getGraphicalEntities() const;
    /** @brief Returns the scene items classified as diagram connections. */
    QList<QGraphicsItem*>*getGraphicalDiagramsConnections() const;
    /** @brief Returns the groups currently tracked by the scene. */
    QList<QGraphicsItemGroup*>*getGraphicalGroups() const;

public:
    /** @brief Starts a transition animation between two model components. */
    void animateTransition(ModelComponent *source, ModelComponent *destination, bool viewSimulation, Event *event);
    /** @brief Runs one transition animation instance until completion. */
    void runAnimateTransition(AnimationTransition *animationTransition, Event *event, bool restart = false);
    /** @brief Handles animation state changes for transition-driven event loops. */
    void handleAnimationStateChanged(QAbstractAnimation::State newState, QEventLoop* loop, Event* event, AnimationTransition* animationTransition);
    /** @brief Creates the queue-insert animation overlay for one component. */
    void animateQueueInsert(ModelComponent *component, bool visivible);
    /** @brief Creates the queue-remove animation overlay for one component. */
    void animateQueueRemove(ModelComponent *component);
    /** @brief Updates the counter animation overlay. */
    void animateCounter();
    /** @brief Updates the variable animation overlay. */
    void animateVariable(Entity* entity = nullptr);
    /** @brief Updates the timer animation overlay. */
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
    bool tryCreateConnection(GraphicalComponentPort* source, GraphicalComponentPort* destination, bool notify = true);
    GraphicalComponentPort* firstAvailableOutputPort(GraphicalModelComponent* component) const;
    GraphicalComponentPort* firstInputPort(GraphicalModelComponent* component) const;
    void resetConnectingState();
private:
    GRID _grid;
    Simulator* _simulator = nullptr;
    PropertyEditorGenesys* _propertyEditor = nullptr;
    std::map<SimulationControl*, DataComponentProperty*>* _propertyList = nullptr;
    std::map<SimulationControl*, DataComponentEditor*>* _propertyEditorUI = nullptr;
    std::map<SimulationControl*, ComboBoxEnum*>* _propertyBox = nullptr;
    QTreeWidgetItem* _objectBeingDragged = nullptr;
    // Initialize the parent widget pointer to a known null state.
    QWidget* _parentWidget = nullptr;
    QList<GraphicalModelComponent*> _allGraphicalModelComponents;
    QList<GraphicalConnection*> _allGraphicalConnections;
    QList<GraphicalModelDataDefinition*> _allGraphicalModelDataDefinitions;
    QList<GraphicalDiagramConnection*> _allGraphicalDiagramConnections;
    QUndoStack *_undoStack = nullptr;
    QMap<QGraphicsItemGroup *, QList<GraphicalModelComponent *> > _listComponentsGroup;
    QMap<QGraphicsItem *, QPointF> _oldPositionsItems;
    QList<Counter *> *_counters = new QList<Counter *>();
    QList<ModelDataDefinition *> *_variables = new QList<ModelDataDefinition *>();
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
    bool _persistedGuiRestoreInProgress = false;
    unsigned short _connectingStep = 0; //0:nothing, 1:waiting click on source or destination, 2: click on source, 3: click on destination
    bool _controlIsPressed = false;
    bool _shiftSelectionInProgress = false;
    QGraphicsItem* _shiftClickedSelectableItem = nullptr;
    QList<QGraphicsItem*> _shiftSelectionBeforeClick;
    bool _snapToGrid = false;
    // Initialize the source port pointer before connection drawing starts.
    GraphicalComponentPort* _sourceGraphicalComponentPort = nullptr;
    // Initialize the destination port pointer before connection drawing starts.
    GraphicalComponentPort* _destinationGraphicalComponentPort = nullptr;
    // Initialize the current counter drawing pointer to avoid indeterminate access.
    AnimationCounter *_currentCounter = nullptr;
    // Initialize the current variable drawing pointer to avoid indeterminate access.
    AnimationVariable *_currentVariable = nullptr;
    // Initialize the current timer drawing pointer to avoid indeterminate access.
    AnimationTimer *_currentTimer = nullptr;
    AnimationPlaceholder *_currentPlaceholderAnimation = nullptr;
    QMap<Event *, QList<AnimationTransition *> *> *_animationPaused = new QMap<Event *, QList<AnimationTransition *> *>();

private:
    QList<QString> *_imagesAnimation = new QList<QString>;
    QList<AnimationTransition *> *_animationsTransition = new QList<AnimationTransition*>();
    QList<AnimationCounter *> *_animationsCounter = new QList<AnimationCounter*>();
    QList<AnimationVariable *> *_animationsVariable = new QList<AnimationVariable*>();
    QList<AnimationTimer *> *_animationsTimer = new QList<AnimationTimer*>();
    QList<AnimationPlaceholder *> *_animationsPlaceholder = new QList<AnimationPlaceholder*>();
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
    bool _restoringPersistedGuiLayout = false;
    bool _graphicalDataDefinitionsSyncPending = false;
    bool _graphicalDataDefinitionsSyncInProgress = false;
    bool _graphicalDataDefinitionsSyncDeferredDuringRestore = false;
    bool _connectionGeometryUpdatesBlocked = false;
    bool _showStatisticsDataDefinitions = true;
    bool _showEditableDataDefinitions = true;
    bool _showSharedDataDefinitions = true;
    bool _showRecursiveDataDefinitions = true;
    // Rendering scope used by root/submodel tabs. The filter is intentionally scene-local so
    // static graphical synchronization calls can keep the same scope as the active view.
    bool _hasModelLevelFilter = false;
    unsigned int _modelLevelFilter = 0;
};

#endif /* MODELGRAPHICSSCENE_H */
