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
 * File:   ModelComponentGraphicItem.cpp
 * Author: rlcancian
 *
 * Created on 16 de fevereiro de 2022, 11:41
 */

#include "GraphicalModelComponent.h"
#include "GraphicalComponentPort.h"
#include "TraitsGUI.h"
#include "UtilGUI.h"
#include <QPainter>
#include <QRgba64>

GraphicalModelComponent::GraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position, QColor color, QGraphicsItem *parent) :  GraphicalModelDataDefinition(plugin, component, position, color) { //  QGraphicsObject(parent) {
	_component = component;
	_color = color;
	_color.setAlpha(TraitsGUI<GModelComponent>::opacity);
	// define shape
	if (plugin->getPluginInfo()->isSource()) {
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
	}
	// position and flags
	setPos(position.x()/*-_width/2*/, position.y() - _height / 2);
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
	setToolTip(QString::fromStdString(component->show()));
	// create input output ports
	GraphicalComponentPort* port;
	qreal px, py = 0;
	unsigned int numInputPorts = std::max<unsigned int>(component->getConnectionManager()->getCurrentInputConnectionsSize(), plugin->getPluginInfo()->getMinimumInputs());
	qreal step = (double) _height / (double) (numInputPorts+1);
	for (unsigned int i = 0; i < numInputPorts; i++) {
		port = new GraphicalComponentPort(this, true, i, parent);
		port->setX(0);
		py += step;
		port->setY(py - port->height() / 2);
		port->setParentItem(this);
		this->_graphicalInputPorts.append(port);
	}
	py = 0;
	unsigned int numOutputPorts = std::max<unsigned int>(component->getConnectionManager()->getCurrentOutputConnectionsSize(), plugin->getPluginInfo()->getMinimumOutputs());
	step = (double) _height / (double) (numOutputPorts+1);
	for (unsigned int i = 0; i < numOutputPorts; i++) {
		port = new GraphicalComponentPort(this, false, i, parent);
		port->setX(this->_width - port->width());
		py += step;
		port->setY(py - port->height() / 2);
		port->setParentItem(this);
		this->_graphicalOutputPorts.append(port);
	}
}

GraphicalModelComponent::GraphicalModelComponent(const GraphicalModelComponent& orig): GraphicalModelDataDefinition(orig) {
}

GraphicalModelComponent::~GraphicalModelComponent() {}

QRectF GraphicalModelComponent::boundingRect() const {
	//qreal penWidth = _pen.width();
	//return QRectF(penWidth / 2, penWidth / 2, _width + penWidth, _height + penWidth);
	return QRectF(0, 0, _width, _height);
}

QColor GraphicalModelComponent::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
}

void GraphicalModelComponent::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	int _penWidth = TraitsGUI<GModelComponent>::penWidth; //1;
	int _raise = TraitsGUI<GModelComponent>::raise; // 5;
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
	pen = QPen(myrgba(TraitsGUI<GModelComponent>::borderColor));
	//pen = QPen(Qt::black);
	pen.setWidth(_penWidth);
	painter->setPen(pen);
	// Path raised
	brush = QBrush(Qt::SolidPattern);
	brush.setColor(myrgba(TraitsGUI<GModelComponent>::pathRaised)); // default white
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
	brush.setColor(myrgba(TraitsGUI<GModelComponent>::pathStunken)); //Qt::darkGray);
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
	// text
	QString text = QString::fromStdString(_component->getName());
	QRect rect2 = QRect(_margin + _raise + 1, _margin + _raise + 1, _margin + wi - 2 * _raise - _margin, _margin + hi - 2 * _raise - _margin);
	brush = QBrush(Qt::NoBrush);
	painter->setBrush(brush);
	pen = QPen(myrgba(TraitsGUI<GModelComponent>::textColor));
	pen.setWidth(2);
	pen.setCosmetic(true);
	painter->setPen(pen);
	painter->drawText(rect2, Qt::AlignCenter, text);
	pen.setColor(myrgba(TraitsGUI<GModelComponent>::textShadowColor));
	painter->setPen(pen);
	painter->drawText(rect2.adjusted(0,2,2,0), Qt::AlignCenter, text);
	// text shadow
	//
	if (isSelected()) { //draw squares on corners
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GModelComponent>::selectionSquaresColor));
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
	ModelComponent* mc = getComponent();
	if (mc->hasBreakpointAt()) {
		brush = QBrush(Qt::NoBrush);
		painter->setBrush(brush);
		pen = QPen(myrgba(TraitsGUI<GModelComponent>::breakpointColor));
		pen.setWidth(1);
		painter->setPen(pen);
		rect = QRect(0, 0, _width, _height);
		painter->drawRect(rect);
	}
}

ModelComponent* GraphicalModelComponent::getComponent() const {
	return _component;
}

QPointF GraphicalModelComponent::getOldPosition() const {
    return _oldPosition;
}


void GraphicalModelComponent::setOldPosition(QPointF oldPosition) {
    _oldPosition = oldPosition;
}


QColor GraphicalModelComponent::getColor() const {
    return _color;
}

qreal GraphicalModelComponent::getHeight() const {
    return _height;
}

bool GraphicalModelComponent::sceneEvent(QEvent *event) {
    return QGraphicsObject::sceneEvent(event); // Unnecessary
}

QList<GraphicalComponentPort *> GraphicalModelComponent::getGraphicalOutputPorts() const {
    return _graphicalOutputPorts;
}

QList<GraphicalComponentPort *> GraphicalModelComponent::getGraphicalInputPorts() const {
	return _graphicalInputPorts;
}

QList<ModelDataDefinition *> *GraphicalModelComponent::getInternalData() const {
    return _internalData;
}

QList<ModelDataDefinition *> *GraphicalModelComponent::getAttachedData() const {
    return _attachedData;
}

EntityType * GraphicalModelComponent::getEntityType() const{
    return _entityType;
}

unsigned int GraphicalModelComponent::getOcupiedInputPorts() const {
    return _ocupiedInputPorts;
}
unsigned int GraphicalModelComponent::getOcupiedOutputPorts() const {
    return _ocupiedOutputPorts;
}

void GraphicalModelComponent::setEntityType(EntityType *entityType) {
    _entityType = entityType;
}

void GraphicalModelComponent::setOcupiedInputPorts(unsigned int value) {
    _ocupiedInputPorts = value;
}

void GraphicalModelComponent::setOcupiedOutputPorts(unsigned int value) {
    _ocupiedOutputPorts = value;
}

QString GraphicalModelComponent::getAnimationImageName() {
    return _animationImageName;
}
void GraphicalModelComponent::setAnimationImageName(QString name) {
    _animationImageName = name;
}


// Em caso de possuir Queue

QMap<Queue *, QPair<unsigned int, unsigned int>>* GraphicalModelComponent::getMapQueue() {
    return _mapQueue;
}

QList<QList<GraphicalImageAnimation *> *>* GraphicalModelComponent::getImagesQueue() {
    return _imagesQueue;
}

void GraphicalModelComponent::verifyQueue() {
    clearQueues();

    std::map<std::string, ModelDataDefinition*>* internalData = this->getComponent()->getInternalData();
    std::map<std::string, ModelDataDefinition*>* attachedData = this->getComponent()->getAttachedData();

    QList<ModelDataDefinition*> qListInternalData;
    QList<ModelDataDefinition*> qListAttachedData;
    QList<Queue *> queues;

    for (auto it = internalData->begin(); it != internalData->end(); ++it) {
        qListInternalData.append(it->second);
    }

    for (auto it = attachedData->begin(); it != attachedData->end(); ++it) {
        qListAttachedData.append(it->second);
    }

    for (ModelDataDefinition* internalData : qListInternalData) {
        if (internalData->getClassname() == "Queue") {
            Queue *queue = dynamic_cast<Queue*>(internalData);
            if (queue) {
                queues.append(queue);
                _imagesQueue->append(new QList<GraphicalImageAnimation *>);
            }
        }
    }

    for (ModelDataDefinition* attachedData : qListAttachedData) {
        if (attachedData->getClassname() == "Queue") {
            Queue *queue = dynamic_cast<Queue*>(attachedData);
            if (queue) {
                queues.append(queue);
                _imagesQueue->append(new QList<GraphicalImageAnimation *>);
            }
        }
    }

    if (_component->getClassname() == "PickStation") {
        _imagesQueue->append(new QList<GraphicalImageAnimation *>);
        _hasQueue = true;
    }

    if (!queues.empty()) {
        populateMapQueue(queues);
        _hasQueue = true;
    }
}

bool GraphicalModelComponent::hasQueue() {
    return _hasQueue;
}

bool GraphicalModelComponent::compareQueuesById(const Queue* a, const Queue* b) {
    return a->getId() < b->getId();
}

unsigned int GraphicalModelComponent::getIndexQueue(Queue *queue) {
    QPair<unsigned int, unsigned int> pairIndexSize = _mapQueue->value(queue);
    return pairIndexSize.first;
}

unsigned int GraphicalModelComponent::getSizeQueue(Queue *queue) {
    QPair<unsigned int, unsigned int> pairIndexSize = _mapQueue->value(queue);
    return pairIndexSize.second;
}

void GraphicalModelComponent::populateMapQueue(QList<Queue *> queues) {
    QList<Queue*> sortedQueues = queues;
    std::sort(sortedQueues.begin(), sortedQueues.end(), &compareQueuesById);

    for (Queue* queue : queues) {
        _mapQueue->insert(queue, qMakePair(sortedQueues.indexOf(queue), (unsigned int) queue->size()));
    }
}

bool GraphicalModelComponent::insertImageQueue(Queue *queue, GraphicalImageAnimation *image) {
    if ((unsigned int) _imagesQueue->size() == getIndexQueue(queue) + 1) {
        QList<GraphicalImageAnimation *> *imagesList = _imagesQueue->at(getIndexQueue(queue));
        imagesList->append(image);
        actualizeMapQueue(queue);

        return true;
    }

    return false;
}

bool GraphicalModelComponent::insertImageQueue(GraphicalImageAnimation *image) {
    if (!_imagesQueue->empty()) {
        QList<GraphicalImageAnimation *> *imagesList = _imagesQueue->at(0);
        imagesList->append(image);
        return true;
    }
    return false;
}

QList<GraphicalImageAnimation *>* GraphicalModelComponent::removeImageQueue(Queue *queue, unsigned int quantityRemoved) {
    if ((unsigned int) _imagesQueue->size() == getIndexQueue(queue) + 1) {
        QList<GraphicalImageAnimation *> *imagesList = _imagesQueue->at(getIndexQueue(queue));

        if (imagesList) {
            if (!imagesList->empty()) {
                QList<GraphicalImageAnimation *> *imagesRemoved = new QList<GraphicalImageAnimation *>();

                if (quantityRemoved > (unsigned int) imagesList->size()) {
                    quantityRemoved = imagesList->size();
                }

                for (unsigned int i = 0; i < quantityRemoved; i++) {
                    imagesRemoved->append(imagesList->last());
                    imagesList->removeLast();
                }

                actualizeMapQueue(queue);

                if (!imagesRemoved->empty())
                    return imagesRemoved;
            }
        }
    }
    return nullptr;
}

GraphicalImageAnimation* GraphicalModelComponent::removeImageQueue() {
    if (!_imagesQueue->empty()) {
        QList<GraphicalImageAnimation *> *imagesList = _imagesQueue->at(0);

        if (imagesList) {
            if (!imagesList->empty()) {
                GraphicalImageAnimation* imageRemoved = imagesList->last();

                if (imageRemoved) {
                    imagesList->removeLast();
                    return imageRemoved;
                }
            }
        }
    }

    return nullptr;
}

void GraphicalModelComponent::actualizeMapQueue(Queue *queue) {
    _mapQueue->insert(queue, qMakePair(getIndexQueue(queue), queue->size()));
}

void GraphicalModelComponent::visivibleImageQueue(bool visivible) {
    for (QList<GraphicalImageAnimation*> *imagesList : *_imagesQueue) {
        for (GraphicalImageAnimation* image : *imagesList) {
            image->setVisible(visivible);
        }
    }
}

void GraphicalModelComponent::clearQueues() {
    for (QList<GraphicalImageAnimation*> *imagesList : *_imagesQueue) {
        for (GraphicalImageAnimation *image : *imagesList) {
            delete image;
        }
        imagesList->clear();
        delete imagesList;
    }
    _imagesQueue->clear();
    _mapQueue->clear();
}

/*
void ModelComponentGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){

}
void ModelComponentGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event){

}

void ModelComponentGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event){

}
 */
