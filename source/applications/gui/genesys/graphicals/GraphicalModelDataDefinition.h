#ifndef GRAPHICALMODELDATADEFINITION_H
#define GRAPHICALMODELDATADEFINITION_H

/*
 * File:   ModelDataDefinitionGraphicItem.h
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 11:41
 */

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include "kernel/simulator/Plugin.h"
#include "graphicals/GraphicalModelItemRenderStrategy.h"
#include "TraitsGUI.h"

/**
 * @brief Visual representation of a model data definition in the scene.
 *
 * This is the base graphical item used for non-component data definitions. Derived classes
 * such as `GraphicalModelComponent` extend it with ports, internal data, and richer layout.
 */
class GraphicalModelDataDefinition : public QGraphicsObject {
public:
    /** @brief Creates one visual item bound to a kernel data definition. */
	GraphicalModelDataDefinition(Plugin* plugin, ModelDataDefinition* element, QPointF position, QColor color = Qt::blue, QGraphicsItem *parent = nullptr);
    /** @brief Copy constructor used by scene-copy and restore flows. */
	GraphicalModelDataDefinition(const GraphicalModelDataDefinition& orig);
    /** @brief Releases the graphical item and any owned helper state. */
	virtual ~GraphicalModelDataDefinition();
public:
    /** @brief Returns the item bounds used by Qt for painting and hit testing. */
	QRectF boundingRect() const override;
    /** @brief Returns the precise shape used for selection and collision checks. */
	QPainterPath shape() const override;
    /** @brief Paints the data-definition body, label, and selection frame. */
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /** @brief Returns the kernel data definition represented by this item. */
    ModelDataDefinition* getDataDefinition() const;
    /** @brief Returns whether the property editor may edit this item in place. */
    bool isEditableInPropertyEditor() const;
    /** @brief Marks the item as editable or read-only in the property editor. */
    void setEditableInPropertyEditor(bool editable);
    /** @brief Returns the last persisted scene position before a move operation. */
    QPointF getOldPosition() const;
    /** @brief Stores the old scene position used by undo and restoration flows. */
    void setOldPosition(qreal x, qreal y);
    /** @brief Returns the color used to paint the item. */
    QColor getColor() const;
    /** @brief Sets the color used to paint the item. */
    void setColor(QColor newColor);
    /** @brief Returns the height used by the current render strategy. */
    qreal getHeight() const;

protected:
	QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
	virtual GraphicalModelItemRenderContext renderContext() const;
protected: // virtual
	virtual bool sceneEvent(QEvent *event) override;
	//virtual void	hoverEnterEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	hoverMoveEvent(QGraphicsSceneHoverEvent * event)
	//virtual void	inputMethodEvent(QInputMethodEvent * event)
	//virtual QVariant	inputMethodQuery(Qt::InputMethodQuery query) const
	//virtual QVariant	itemChange(GraphicsItemChange change, const QVariant & value)
	//virtual void	keyPressEvent(QKeyEvent * event)
	//virtual void	keyReleaseEvent(QKeyEvent * event)
	//virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
	//virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	//virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	//virtual void	mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
protected:
	qreal _width = TraitsGUI<GModelDataDefinition>::width; //150;
	qreal _height = _width * TraitsGUI<GModelDataDefinition>::heightProportion; //0.67;
	unsigned int _margin = TraitsGUI<GModelDataDefinition>::margin;//8;
	unsigned int _selWidth = TraitsGUI<GModelDataDefinition>::selectionWidth;//8;
	ModelDataDefinition* _element;
    bool _editableInPropertyEditor = false;
	QColor _color;
    QPointF _oldPosition;
	qreal _stretchPosTop = TraitsGUI<GModelDataDefinition>::stretchPos;//0.5;
	qreal _stretchPosBottom = TraitsGUI<GModelDataDefinition>::stretchPos;//0.5;
	qreal _stretchPosLeft = TraitsGUI<GModelDataDefinition>::stretchPos;//0.5;
	qreal _stretchPosRigth = TraitsGUI<GModelDataDefinition>::stretchPos;//0.5;
	qreal _stretchRigth = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchLeft = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchRigthMidle = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchLeftMidle = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchTop = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchBottom = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchTopMidle = TraitsGUI<GModelDataDefinition>::stretch;//0;
	qreal _stretchBottomMidle = TraitsGUI<GModelDataDefinition>::stretch;//0;
private:
};

#endif /* GRAPHICALMODELDATADEFINITION_H */
