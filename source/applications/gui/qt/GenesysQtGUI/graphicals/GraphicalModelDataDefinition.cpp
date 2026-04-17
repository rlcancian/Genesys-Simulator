
/*
 * File:   GraphicalModelDataDefinition.cpp
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 11:44
 */

#include "GraphicalModelDataDefinition.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include <QPainter>
#include <QRgba64>

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

QColor GraphicalModelDataDefinition::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
}

void GraphicalModelDataDefinition::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	int _penWidth = TraitsGUI<GModelDataDefinition>::penWidth; //1;
	int _raise = TraitsGUI<GModelDataDefinition>::raise; // 5;
	int wi = _width - 2 * _margin - _penWidth;
	int hi = _height - 2 * _margin - _penWidth;
	int wt2 = _width*_stretchPosTop;
	int wb2 = _width*_stretchPosBottom;
	int hl2 = _height*_stretchPosLeft;
	int hr2 = _height*_stretchPosRigth;
	qreal sfr = hi*_stretchRigth;
	qreal sfl = hi*_stretchLeft;
	qreal sfrm = hi*_stretchRigthMidle;
	qreal sflm = hi*_stretchLeftMidle;
	qreal sft = wi*_stretchTop;
	qreal sfb = wi*_stretchBottom;
	qreal sftm = wi*_stretchTopMidle;
	qreal sfbm = wi*_stretchBottomMidle;
	int shiftt = (_stretchRigth == 0 && _stretchLeft == 0) || (_stretchRigth > 0 && _stretchLeft > 0) ? 0 : (_stretchRigth > 0 ? _raise : -_raise);
	int shiftb = shiftt;
	int shiftr = (_stretchTop == 0 && _stretchBottom == 0) || (_stretchTop > 0 && _stretchBottom > 0) ? 0 : (_stretchBottom > 0 ? _raise : -_raise);
	int shiftl = shiftr;
	QPen pen;
	QBrush brush;
	QRect rect;
	QPointF pp1, pp2, pp3, pp4, pp5, pp6, pp7, pp8, pp9, pp10, pp11, pp12, pp13, pp14, pp15, pp16;
	// points
	pp1 = QPointF(_margin + sft, _margin + sfl);
	pp2 = QPointF(_margin + sflm, hl2 + shiftl);
	pp3 = QPointF(_margin + sfb, _margin + hi - sfl);
	pp4 = QPointF(_margin + _raise + sfb, _margin + hi - _raise - sfl);
	pp5 = QPointF(_margin + _raise + sflm, hl2);
	pp6 = QPointF(_margin + _raise + sft, _margin + _raise + sfl);
	pp7 = QPointF(wt2, _margin + _raise + sftm);
	pp8 = QPointF(_margin + wi - _raise - sft, _margin + _raise + sfr);
	pp9 = QPointF(_margin + wi - sft, _margin + sfr);
	pp10 = QPointF(wt2 + shiftt, _margin + sftm);
	pp11 = QPointF(_margin + wi - sfb, _margin + hi - sfr);
	pp12 = QPointF(_margin + wi - sfrm, hr2 + shiftr);
	pp13 = QPointF(_margin + wi - _raise - sfrm, hr2);
	pp14 = QPointF(_margin + wi - _raise - sfb, _margin + hi - _raise - sfr);
	pp15 = QPointF(wb2, _margin + hi - _raise - sfbm);
	pp16 = QPointF(wb2 + shiftb, _margin + hi - sfbm);
	// pen
	pen = QPen(myrgba(TraitsGUI<GModelDataDefinition>::borderColor));
	//pen = QPen(Qt::black);
	pen.setWidth(_penWidth);
	painter->setPen(pen);
	// Path raised
	brush = QBrush(Qt::SolidPattern);
	brush.setColor(myrgba(TraitsGUI<GModelDataDefinition>::pathRaised)); // default white
	painter->setBrush(brush);
	QPainterPath path;
	path.moveTo(pp1);
	path.lineTo(pp2);
	if (_stretchTop > 0 || _stretchBottom > 0) {
		path.lineTo(pp5);
		path.lineTo(pp2);
	}
	path.lineTo(pp3);
	path.lineTo(pp4);
	path.lineTo(pp5);
	path.lineTo(pp6);
	path.lineTo(pp7);
	path.lineTo(pp8);
	path.lineTo(pp9);
	path.lineTo(pp10);
	if (_stretchRigth > 0 || _stretchLeft > 0) {
		path.lineTo(pp7);
		path.lineTo(pp10);
	}
	path.lineTo(pp1);
	path.lineTo(pp6);
	painter->drawPath(path);
	// path stunken
	brush = QBrush(Qt::SolidPattern);
	brush.setColor(myrgba(TraitsGUI<GModelDataDefinition>::pathStunken)); //Qt::darkGray);
	painter->setBrush(brush);
	QPainterPath path2;
	path2.moveTo(pp11);
	path2.lineTo(pp12);
	path2.lineTo(pp9);
	path2.lineTo(pp8);
	path2.lineTo(pp13);
	if (_stretchTop > 0 || _stretchBottom > 0) {
		path2.lineTo(pp12);
		path2.lineTo(pp13);
	}
	path2.lineTo(pp14);
	path2.lineTo(pp15);
	if (_stretchRigth > 0 || _stretchLeft > 0) {
		path2.lineTo(pp16);
		path2.lineTo(pp15);
	}
	path2.lineTo(pp4);
	path2.lineTo(pp3);
	path2.lineTo(pp16);
	path2.lineTo(pp11);
	path2.lineTo(pp14);
	painter->drawPath(path2);
	// fill
	QLinearGradient gradient(0,0,_width,_height);
	gradient.setColorAt(0.0, Qt::white);
	gradient.setColorAt(0.33, _color.lighter());
	gradient.setColorAt(0.67, _color);
	gradient.setColorAt(1.0, _color.darker());
	//brush.setColor(_color);
	brush = QBrush(gradient);//Qt::SolidPattern);//@TODO: Oportunity do improve LinearGradientPattern);
	painter->setBrush(brush);
	QPainterPath pathFill;
	pathFill.moveTo(pp6);
	pathFill.lineTo(pp7);
	pathFill.lineTo(pp8);
	pathFill.lineTo(pp13);
	pathFill.lineTo(pp14);
	pathFill.lineTo(pp15);
	pathFill.lineTo(pp4);
	pathFill.lineTo(pp5);
	pathFill.lineTo(pp6);
	painter->drawPath(pathFill);
	// text (two centered lines as a single vertically centered block)
	const QString text1 = QString::fromStdString(_element->getClassname());
	const QString text2 = QString::fromStdString(_element->getName());
	const QRect contentRect(
		_margin + _raise + 1,
		_margin + _raise + 1,
		wi - 2 * _raise - 2,
		hi - 2 * _raise - 2
	);
	const QFontMetrics fm = painter->fontMetrics();
	const int lineHeight = fm.height();
	const int lineSpacing = 3;
	const int totalHeight = lineHeight * 2 + lineSpacing;
	const int blockTop = contentRect.top() + (contentRect.height() - totalHeight) / 2;
	const QRect line1Rect(contentRect.left(), blockTop, contentRect.width(), lineHeight);
	const QRect line2Rect(contentRect.left(), blockTop + lineHeight + lineSpacing, contentRect.width(), lineHeight);
	const QPoint shadowOffset(0, 2);
	brush = QBrush(Qt::NoBrush);
	painter->setBrush(brush);
	pen = QPen(myrgba(TraitsGUI<GModelDataDefinition>::textColor));
	pen.setWidth(2);
	pen.setCosmetic(true);
	auto drawCenteredTextWithShadow = [&](const QRect& rect, const QString& textLine) {
		pen.setColor(myrgba(TraitsGUI<GModelDataDefinition>::textShadowColor));
		painter->setPen(pen);
		painter->drawText(rect.translated(shadowOffset), Qt::AlignCenter, textLine);
		pen.setColor(myrgba(TraitsGUI<GModelDataDefinition>::textColor));
		painter->setPen(pen);
		painter->drawText(rect, Qt::AlignCenter, textLine);
	};
	drawCenteredTextWithShadow(line1Rect, text1);
	drawCenteredTextWithShadow(line2Rect, text2);

	if (isSelected()) { //draw squares on corners
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GModelDataDefinition>::selectionSquaresColor));
		painter->setBrush(brush);
		rect = QRect(0, 0, _selWidth, _selWidth);
		painter->drawRect(rect);
		rect = QRect(_width - _selWidth, 0, _selWidth, _selWidth);
		painter->drawRect(rect);
		rect = QRect(0, _height - _selWidth, _selWidth, _selWidth);
		painter->drawRect(rect);
		rect = QRect(_width - _selWidth, _height - _selWidth, _selWidth, _selWidth);
		painter->drawRect(rect);
	}
}

ModelDataDefinition* GraphicalModelDataDefinition::getDataDefinition() const {
    return _element;
}

bool GraphicalModelDataDefinition::isEditableInPropertyEditor() const {
    return _editableInPropertyEditor;
}

void GraphicalModelDataDefinition::setEditableInPropertyEditor(bool editable) {
    _editableInPropertyEditor = editable;
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
