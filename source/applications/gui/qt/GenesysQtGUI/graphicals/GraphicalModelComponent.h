#ifndef GRAPHICALMODELCOMPONENT_H
#define GRAPHICALMODELCOMPONENT_H

/*
 * File:   ModelComponentGraphicItem.h
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 11:41
 */

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QPen>
#include <QBrush>
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "graphicals/GraphicalImageAnimation.h"
#include "GraphicalComponentPort.h"
#include "GraphicalModelDataDefinition.h"
#include "TraitsGUI.h"

/**
 * @brief Visual component item that extends a data definition with ports and internal data.
 *
 * The scene uses this class for kernel model components that need both the visual body and the
 * connection ports used to route edges between components.
 */
class GraphicalModelComponent : public GraphicalModelDataDefinition {
public:
	/** @brief Creates a graphical component bound to a kernel component and plugin metadata. */
	GraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color = Qt::blue, QGraphicsItem *parent = nullptr);
	/** @brief Copy constructor used by clipboard and restoration flows. */
	GraphicalModelComponent(const GraphicalModelComponent& orig);
	/** @brief Releases the component item, including its generated port items and queues. */
	virtual ~GraphicalModelComponent();
public:
    /** @brief Returns the rectangle used for painting and hit testing the component body. */
	QRectF boundingRect() const override;
    /** @brief Paints the component body, ports, and selection indicators. */
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /** @brief Returns the kernel component represented by this item. */
	ModelComponent* getComponent() const;
    /** @brief Returns the current item height derived from the render strategy. */
    qreal getHeight() const;
    /** @brief Returns the previous scene position used by undo and restoration flows. */
    QPointF getOldPosition() const;
    /** @brief Stores the previous scene position used by undo and restoration flows. */
    void setOldPosition(QPointF oldPosition);
    /** @brief Returns the input ports generated for this component. */
    QList<GraphicalComponentPort *> getGraphicalInputPorts() const;
	/** @brief Returns the output ports generated for this component. */
	QList<GraphicalComponentPort *> getGraphicalOutputPorts() const;
    /** @brief Returns the component color used by the renderer. */
    QColor getColor() const;
    /** @brief Updates the component color used by the renderer. */
    void setColor(const QColor& color);
    /** @brief Returns the list of internal data definitions attached to the component. */
    QList<ModelDataDefinition *> *getInternalData() const;
    /** @brief Returns the list of externally attached data definitions. */
    QList<ModelDataDefinition *> *getAttachedData() const;
    /** @brief Returns the entity type synthesized or associated with the component. */
    EntityType* getEntityType() const;
    /** @brief Sets the entity type synthesized or associated with the component. */
    void setEntityType(EntityType *entityType);
    /** @brief Returns how many input ports are currently occupied. */
    unsigned int getOcupiedInputPorts() const;
    /** @brief Returns how many output ports are currently occupied. */
    unsigned int getOcupiedOutputPorts() const;
    /** @brief Stores the number of occupied input ports tracked by the layout code. */
    void setOcupiedInputPorts(unsigned int value);
    /** @brief Stores the number of occupied output ports tracked by the layout code. */
    void setOcupiedOutputPorts(unsigned int value);
private:
	QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
protected: // virtual
	virtual bool sceneEvent(QEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    GraphicalModelItemRenderContext renderContext() const override;
	//virtual void	hoverEnterEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	hoverMoveEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	inputMethodEvent(QInputMethodEvent * event)
	//virtual QVariant	inputMethodQuery(Qt::InputMethodQuery query) const
	//virtual QVariant	itemChange(GraphicsItemChange change, const QVariant & value)
	//virtual void	keyPressEvent(QKeyEvent * event)
	//virtual void	keyReleaseEvent(QKeyEvent * event)
	//   virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
	//   virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	//   virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	//virtual void	mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
protected:
	qreal _width = TraitsGUI<GModelComponent>::width; //150;
	qreal _height = _width * TraitsGUI<GModelComponent>::heightProportion; //0.67;
	unsigned int _margin = TraitsGUI<GModelComponent>::margin;//8;
	unsigned int _selWidth = TraitsGUI<GModelComponent>::selectionWidth;//8;
	ModelComponent* _component;
	QColor _color;
    QPointF _oldPosition;
	qreal _stretchPosTop = TraitsGUI<GModelComponent>::stretchPos;//0.5;
	qreal _stretchPosBottom = TraitsGUI<GModelComponent>::stretchPos;//0.5;
	qreal _stretchPosLeft = TraitsGUI<GModelComponent>::stretchPos;//0.5;
	qreal _stretchPosRigth = TraitsGUI<GModelComponent>::stretchPos;//0.5;
	qreal _stretchRigth = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchLeft = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchRigthMidle = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchLeftMidle = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchTop = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchBottom = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchTopMidle = TraitsGUI<GModelComponent>::stretch;//0;
	qreal _stretchBottomMidle = TraitsGUI<GModelComponent>::stretch;//0;
private:
	QList<GraphicalComponentPort*> _graphicalInputPorts = QList<GraphicalComponentPort*>();
	QList<GraphicalComponentPort*> _graphicalOutputPorts = QList<GraphicalComponentPort*>();
    QList<ModelDataDefinition *> *_internalData = new QList<ModelDataDefinition*>();
    QList<ModelDataDefinition *> *_attachedData = new QList<ModelDataDefinition*>();
    EntityType* _entityType = nullptr;
    unsigned int _ocupiedInputPorts = 0;
    unsigned int _ocupiedOutputPorts = 0;

private:
    QString _animationImageName = "default.png";
    // Map que mapeia o ponteiro da fila do componente para seu índice (caso tiver mais de uma) e tamanho (usado para animação)
    QMap<Queue *, QPair<unsigned int, unsigned int>> *_mapQueue = new QMap<Queue *, QPair<unsigned int, unsigned int>>();
    // Irá conter referência para as imagens usadas para mostrar a fila do componente
    QList<QList<GraphicalImageAnimation *> *> *_imagesQueue = new QList<QList<GraphicalImageAnimation *> *>;
    bool _hasQueue = false;

public:
    /** @brief Returns the animation image identifier used for queue visualization. */
    QString getAnimationImageName();
    /** @brief Updates the animation image identifier used for queue visualization. */
    void setAnimationImageName(QString name);
    /** @brief Returns the queue-to-metadata map used by the queue overlay renderer. */
    QMap<Queue *, QPair<unsigned int, unsigned int>>* getMapQueue();
    /** @brief Returns the grouped queue image lists used by the renderer. */
    QList<QList<GraphicalImageAnimation *> *>* getImagesQueue();

    /** @brief Verifies whether the component currently references a queue animation context. */
    void verifyQueue();
    /** @brief Returns true when a queue animation context is available. */
    bool hasQueue();
    /** @brief Compares queues by identifier so queue overlays can be ordered deterministically. */
    static bool compareQueuesById(const Queue* a, const Queue* b);
    /** @brief Returns the queue index assigned during queue-overlay population. */
    unsigned int getIndexQueue(Queue *queue);
    /** @brief Returns the queue size recorded during queue-overlay population. */
    unsigned int getSizeQueue(Queue *queue);
    /** @brief Populates the queue overlay metadata from the supplied queue list. */
    void populateMapQueue(QList<Queue *> queues);
    /** @brief Inserts one animation image entry for the given queue. */
    bool insertImageQueue(Queue *queue, GraphicalImageAnimation *image);
    /** @brief Removes a batch of animation images for the given queue. */
    QList<GraphicalImageAnimation *> *removeImageQueue(Queue *queue, unsigned int quantityRemoved);
    /** @brief Inserts one animation image using the current queue context. */
    bool insertImageQueue(GraphicalImageAnimation *image);
    /** @brief Removes the most recently inserted queue animation image. */
    GraphicalImageAnimation *removeImageQueue();
    /** @brief Refreshes the queue metadata after the queue model changes. */
    void actualizeMapQueue(Queue *queue);
    /** @brief Shows or hides the queue image overlay. */
    void visivibleImageQueue(bool visivible);
    /** @brief Clears the cached queue overlay state. */
    void clearQueues();
};

#endif /* MODELCOMPONENTGRAPHICITEM_H */
