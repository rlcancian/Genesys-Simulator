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

class GraphicalModelComponent : public GraphicalModelDataDefinition {
public:
	GraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color = Qt::blue, QGraphicsItem *parent = nullptr);
	GraphicalModelComponent(const GraphicalModelComponent& orig);
	virtual ~GraphicalModelComponent();
public:
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	ModelComponent* getComponent() const;
    qreal getHeight() const;
    QPointF getOldPosition() const;
    void setOldPosition(QPointF oldPosition);
	QList<GraphicalComponentPort *> getGraphicalInputPorts() const;
	QList<GraphicalComponentPort *> getGraphicalOutputPorts() const;
    QColor getColor() const;
    QList<ModelDataDefinition *> *getInternalData() const;
    QList<ModelDataDefinition *> *getAttachedData() const;
    EntityType* getEntityType() const;
    void setEntityType(EntityType *entityType);
    unsigned int getOcupiedInputPorts() const;
    unsigned int getOcupiedOutputPorts() const;
    void setOcupiedInputPorts(unsigned int value);
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
    QString getAnimationImageName();
    void setAnimationImageName(QString name);
    QMap<Queue *, QPair<unsigned int, unsigned int>>* getMapQueue();
    QList<QList<GraphicalImageAnimation *> *>* getImagesQueue();

    void verifyQueue();
    bool hasQueue();
    static bool compareQueuesById(const Queue* a, const Queue* b);
    unsigned int getIndexQueue(Queue *queue);
    unsigned int getSizeQueue(Queue *queue);
    void populateMapQueue(QList<Queue *> queues);
    bool insertImageQueue(Queue *queue, GraphicalImageAnimation *image);
    QList<GraphicalImageAnimation *> *removeImageQueue(Queue *queue, unsigned int quantityRemoved);
    bool insertImageQueue(GraphicalImageAnimation *image);
    GraphicalImageAnimation *removeImageQueue();
    void actualizeMapQueue(Queue *queue);
    void visivibleImageQueue(bool visivible);
    void clearQueues();
};

#endif /* MODELCOMPONENTGRAPHICITEM_H */
