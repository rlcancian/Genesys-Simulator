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
#include "../../../../kernel/simulator/model/ModelComponent.h"
#include "../../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../../../kernel/simulator/essentialPlugins/Counter.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/data/Logic/Variable.h"
#include "kernel/simulator/Plugin.h"
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
	void editOutputPort(unsigned int index, bool inserted);
	void moveOutputPort(unsigned int from, unsigned int to);
	void synchronizeDynamicPorts();
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
    QMap<ModelDataDefinition *, QPair<unsigned int, unsigned int>> *_animationTrackMap = new QMap<ModelDataDefinition *, QPair<unsigned int, unsigned int>>();
    QList<QList<GraphicalImageAnimation *> *> *_animationTracks = new QList<QList<GraphicalImageAnimation *> *>;
    bool _hasAnimationTracks = false;

public:
    /** @brief Returns the animation image identifier used for queue visualization. */
    QString getAnimationImageName();
    /** @brief Updates the animation image identifier used by the renderer. */
    void setAnimationImageName(QString name);
    QMap<ModelDataDefinition *, QPair<unsigned int, unsigned int>>* getAnimationTrackMap();
    QList<QList<GraphicalImageAnimation *> *>* getAnimationTracks();
    void resetAnimationTracks();
    void addAnimationTrack(ModelDataDefinition* source, unsigned int index, unsigned int currentSize);
    void addUnmappedAnimationTrack();
    bool hasAnimationTracks() const;
    unsigned int getAnimationTrackIndex(ModelDataDefinition* source) const;
    unsigned int getAnimationTrackSize(ModelDataDefinition* source) const;
    bool insertAnimationImage(ModelDataDefinition* source, GraphicalImageAnimation* image, unsigned int newSize);
    QList<GraphicalImageAnimation*>* removeAnimationImages(ModelDataDefinition* source, unsigned int quantityRemoved, unsigned int newSize);
    bool insertAnimationImage(GraphicalImageAnimation* image);
    GraphicalImageAnimation* removeAnimationImage();
    void setAnimationTracksVisible(bool visible);
    void clearAnimationTracks();
};

#endif /* MODELCOMPONENTGRAPHICITEM_H */
