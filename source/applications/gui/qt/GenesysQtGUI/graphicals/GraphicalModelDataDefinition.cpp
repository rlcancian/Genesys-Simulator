/*
 * File:   GraphicalModelDataDefinition.cpp
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 11:44
 */

#include "GraphicalModelDataDefinition.h"

#include "kernel/simulator/ModelDataDefinition.h"

#include <QRgba64>

namespace {
    constexpr int nonEditableFillAlpha = 58;
    constexpr int selectedNonEditableFillAlpha = 72;
}

GraphicalModelDataDefinition::GraphicalModelDataDefinition(Plugin* plugin, ModelDataDefinition* element, QPointF position, QColor color, QGraphicsItem *parent)
	:  QGraphicsObject(parent) {
	_element = element;
	_color = color;
	_color.setAlpha(TraitsGUI<GModelDataDefinition>::opacity);
	// define shape

    /*if (plugin->getPluginInfo()->isSource()) {
		_stretchRigth = 0.3;
		_stretchPosTop = 0.75;
		_stretchPosBottom = 0.75;
	} else if (plugin->getPluginInfo()->isSink()) {
		_stretchLeft = 0.3;
		_stretchPosTop = 0.25;
		_stretchPosBottom = 0.25;
	} else if (plugin->getPluginInfo()->isSendTransfer()) {
		_stretchTop = 0.2;
	} else if (plugin->getPluginInfo()->isReceiveTransfer()) {
		_stretchBottom = 0.2;
	} else if (plugin->getPluginInfo()->getMinimumInputs() > 1) {
		//_stretchRigth=0.45;
		//_stretchLeft=0.45;
		_stretchTopMidle = 0.1;
		_stretchBottomMidle = 0.1;
	} else if (plugin->getPluginInfo()->getMinimumOutputs() > 1) {
		_stretchRigth = 0.45;
		_stretchLeft = 0.45;
		_stretchTopMidle = -(_margin / (_width - _margin));
		_stretchBottomMidle = -(_margin / (_width - _margin));
		//_stretchLeft=0.2;
		//_stretchTopMidle=0.05;
		//_stretchBottomMidle=0.05;
    } */

	// position and flags
	setPos(position.x()/*-_width/2*/, position.y() - _height / 2);
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
	setToolTip(QString::fromStdString(element->getName()));
	// create associstions
	//@TODO
}

GraphicalModelDataDefinition::GraphicalModelDataDefinition(const GraphicalModelDataDefinition& orig)
	{//: QGraphicsObject(orig) {
}

GraphicalModelDataDefinition::~GraphicalModelDataDefinition() {
	//_component->~ModelDataDefinition();
}

QRectF GraphicalModelDataDefinition::boundingRect() const {
	//qreal penWidth = _pen.width();
	//return QRectF(penWidth / 2, penWidth / 2, _width + penWidth, _height + penWidth);
	return QRectF(0, 0, _width, _height);
}

QPainterPath GraphicalModelDataDefinition::shape() const {
	return GraphicalModelItemRenderer::shape(renderContext());
}

QColor GraphicalModelDataDefinition::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
}

void GraphicalModelDataDefinition::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	(void) option;
	(void) widget;
	GraphicalModelItemRenderer::paint(painter, renderContext());
}

GraphicalModelItemRenderContext GraphicalModelDataDefinition::renderContext() const {
	GraphicalModelItemRenderContext context;
	context.kind = GraphicalModelItemRenderContext::ItemKind::DataDefinition;
	context.bounds = boundingRect();
	context.fillColor = _color;
	if (!_editableInPropertyEditor) {
		context.fillColor.setAlpha(isSelected() ? selectedNonEditableFillAlpha : nonEditableFillAlpha);
	}
	context.primaryText = QString::fromStdString(_element->getClassname());
	context.secondaryText = QString::fromStdString(_element->getName());
	context.selected = isSelected();
	context.breakpoint = false;
	context.width = _width;
	context.height = _height;
	context.penWidth = TraitsGUI<GModelDataDefinition>::penWidth;
	context.raise = TraitsGUI<GModelDataDefinition>::raise;
	context.margin = _margin;
	context.selectionWidth = _selWidth;
	context.stretchPosTop = _stretchPosTop;
	context.stretchPosBottom = _stretchPosBottom;
	context.stretchPosLeft = _stretchPosLeft;
	context.stretchPosRight = _stretchPosRigth;
	context.stretchRight = _stretchRigth;
	context.stretchLeft = _stretchLeft;
	context.stretchRightMiddle = _stretchRigthMidle;
	context.stretchLeftMiddle = _stretchLeftMidle;
	context.stretchTop = _stretchTop;
	context.stretchBottom = _stretchBottom;
	context.stretchTopMiddle = _stretchTopMidle;
	context.stretchBottomMiddle = _stretchBottomMidle;
	context.borderColor = TraitsGUI<GModelDataDefinition>::borderColor;
	context.raisedColor = TraitsGUI<GModelDataDefinition>::pathRaised;
	context.sunkenColor = TraitsGUI<GModelDataDefinition>::pathStunken;
	context.textColor = TraitsGUI<GModelDataDefinition>::textColor;
	context.textShadowColor = TraitsGUI<GModelDataDefinition>::textShadowColor;
	context.selectionColor = TraitsGUI<GModelDataDefinition>::selectionSquaresColor;
	context.breakpointColor = TraitsGUI<GModelDataDefinition>::breakpointColor;
	return context;
}

ModelDataDefinition* GraphicalModelDataDefinition::getDataDefinition() const {
    return _element;
}

bool GraphicalModelDataDefinition::isEditableInPropertyEditor() const {
    return _editableInPropertyEditor;
}

void GraphicalModelDataDefinition::setEditableInPropertyEditor(bool editable) {
    _editableInPropertyEditor = editable;
    update();
}

QPointF GraphicalModelDataDefinition::getOldPosition() const {
    return _oldPosition;
}

void GraphicalModelDataDefinition::setOldPosition(qreal x, qreal y) {
    QPointF newPosition;
    newPosition.setX(x);
    newPosition.setY(y);
    _oldPosition = newPosition;
}

QColor GraphicalModelDataDefinition::getColor() const {
    return _color;
}

void GraphicalModelDataDefinition::setColor(QColor newColor) {
    _color = newColor;
    _color.setAlpha(TraitsGUI<GModelDataDefinition>::opacity);
}

qreal GraphicalModelDataDefinition::getHeight() const {
    return _height;
}

bool GraphicalModelDataDefinition::sceneEvent(QEvent *event) {
	return QGraphicsObject::sceneEvent(event);
}
