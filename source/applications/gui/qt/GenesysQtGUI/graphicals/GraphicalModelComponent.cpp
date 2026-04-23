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
#include "GraphicalConnection.h"
#include "ModelGraphicsScene.h"
#include "TraitsGUI.h"
#include "UtilGUI.h"
#include <QPainter>
#include <QRgba64>
#include <QSet>
#include <QDebug>

GraphicalModelComponent::GraphicalModelComponent(Plugin* plugin, ModelComponent* component, QPointF position,
                                                 QColor color, QGraphicsItem* parent) : GraphicalModelDataDefinition(
	plugin, component, position, color) {
	//  QGraphicsObject(parent) {
	_component = component;
	_color = color;
	_color.setAlpha(TraitsGUI<GModelComponent>::opacity);
	int colorIncrease = (TraitsGUI<GModelComponent>::colorIncrease);
	int lighter = (TraitsGUI<GModelComponent>::colorIncrease);
	// define shape
	if (plugin->getPluginInfo()->isSource()) {
		_stretchRigth = 0.3;
		_stretchPosTop = 0.75;
		_stretchPosBottom = 0.75;
		_color.setRed(std::min(255, _color.red() + colorIncrease));
	}
	else if (plugin->getPluginInfo()->isSink()) {
		_stretchLeft = 0.3;
		_stretchPosTop = 0.25;
		_stretchPosBottom = 0.25;
		_color.setRed(std::min(255, _color.red() + colorIncrease));
	}
	else {
		_color = _color.lighter(lighter);
		if (plugin->getPluginInfo()->isSendTransfer()) {
			_stretchTop = 0.2;
			_color.setRed(std::min(255, _color.red() + colorIncrease));
		}
		else if (plugin->getPluginInfo()->isReceiveTransfer()) {
			_stretchBottom = 0.2;
			_color.setRed(std::min(255, _color.red() + colorIncrease));
		}
		else if (plugin->getPluginInfo()->getMinimumInputs() > 1) {
			//_stretchRigth=0.45;
			//_stretchLeft=0.45;
			_stretchTopMidle = 0.1;
			_stretchBottomMidle = 0.1;
		}
		else if (plugin->getPluginInfo()->getMinimumOutputs() > 1) {
			_stretchRigth = 0.45;
			_stretchLeft = 0.45;
			_stretchTopMidle = -(_margin / (_width - _margin));
			_stretchBottomMidle = -(_margin / (_width - _margin));
			//_stretchLeft=0.2;
			//_stretchTopMidle=0.05;
			//_stretchBottomMidle=0.05;
		}
	}
	// position and flags
	setPos(position.x()/*-_width/2*/, position.y() - _height / 2);
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
	setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
	setToolTip(QString::fromStdString(component->show()));
	// create input output ports
	GraphicalComponentPort* port;
	qreal px, py = 0;
	unsigned int numInputPorts = std::max<unsigned int>(
		component->getConnectionManager()->getCurrentInputConnectionsSize(),
		plugin->getPluginInfo()->getMinimumInputs());
	qreal step = (double)_height / (double)(numInputPorts + 1);
	for (unsigned int i = 0; i < numInputPorts; i++) {
		port = new GraphicalComponentPort(this, true, i, parent);
		port->setX(0);
		py += step;
		port->setY(py - port->height() / 2);
		port->setParentItem(this);
		this->_graphicalInputPorts.append(port);
	}
	py = 0;
	unsigned int numOutputPorts = std::max<unsigned int>(
		component->getConnectionManager()->getCurrentOutputConnectionsSize(),
		plugin->getPluginInfo()->getMinimumOutputs());
	step = (double)_height / (double)(numOutputPorts + 1);
	for (unsigned int i = 0; i < numOutputPorts; i++) {
		port = new GraphicalComponentPort(this, false, i, parent);
		port->setX(this->_width - port->width());
		py += step;
		port->setY(py - port->height() / 2);
		port->setParentItem(this);
		this->_graphicalOutputPorts.append(port);
	}
}

GraphicalModelComponent::GraphicalModelComponent(const GraphicalModelComponent& orig) : GraphicalModelDataDefinition(
	orig) {
}

GraphicalModelComponent::~GraphicalModelComponent() {
}

QRectF GraphicalModelComponent::boundingRect() const {
	//qreal penWidth = _pen.width();
	//return QRectF(penWidth / 2, penWidth / 2, _width + penWidth, _height + penWidth);
	return QRectF(0, 0, _width, _height);
}

QColor GraphicalModelComponent::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color & 0xFF000000) >> 24;
	g = (color & 0x00FF0000) >> 16;
	b = (color & 0x0000FF00) >> 8;
	a = (color & 0x000000FF);
	return QColor(r, g, b, a);
}

void GraphicalModelComponent::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	(void) option;
	(void) widget;
	GraphicalModelItemRenderer::paint(painter, renderContext());
}

GraphicalModelItemRenderContext GraphicalModelComponent::renderContext() const {
	GraphicalModelItemRenderContext context;
	context.kind = GraphicalModelItemRenderContext::ItemKind::Component;
	context.bounds = boundingRect();
	context.fillColor = _color;
	context.primaryText = QString::fromStdString(_component->getName());
	context.selected = isSelected();
	context.breakpoint = _component != nullptr && _component->hasBreakpointAt();
	context.width = _width;
	context.height = _height;
	context.penWidth = TraitsGUI<GModelComponent>::penWidth;
	context.raise = TraitsGUI<GModelComponent>::raise;
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
	context.borderColor = TraitsGUI<GModelComponent>::borderColor;
	context.raisedColor = TraitsGUI<GModelComponent>::pathRaised;
	context.sunkenColor = TraitsGUI<GModelComponent>::pathStunken;
	context.textColor = TraitsGUI<GModelComponent>::textColor;
	context.textShadowColor = TraitsGUI<GModelComponent>::textShadowColor;
	context.selectionColor = TraitsGUI<GModelComponent>::selectionSquaresColor;
	context.breakpointColor = TraitsGUI<GModelComponent>::breakpointColor;
	return context;
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

void GraphicalModelComponent::setColor(const QColor& color) {
	_color = color;
	_color.setAlpha(TraitsGUI<GModelComponent>::opacity);
	update();
}

qreal GraphicalModelComponent::getHeight() const {
	return _height;
}

bool GraphicalModelComponent::sceneEvent(QEvent* event) {
	return QGraphicsObject::sceneEvent(event); // Unnecessary
}

QVariant GraphicalModelComponent::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
	QVariant result = GraphicalModelDataDefinition::itemChange(change, value);
	if (change != QGraphicsItem::ItemPositionHasChanged
		&& change != QGraphicsItem::ItemScenePositionHasChanged) {
		return result;
	}

	ModelGraphicsScene* modelScene = dynamic_cast<ModelGraphicsScene*>(scene());
	if (modelScene != nullptr
		&& (modelScene->areConnectionGeometryUpdatesBlocked()
			|| modelScene->isGraphicalDataDefinitionsSyncInProgress())) {
		return result;
	}

	QSet<GraphicalConnection*> uniqueConnections;
	auto collectConnections = [&uniqueConnections](const QList<GraphicalComponentPort*>& ports) {
		for (GraphicalComponentPort* port : ports) {
			if (port == nullptr || port->getConnections() == nullptr) {
				continue;
			}
			for (GraphicalConnection* connection : *port->getConnections()) {
				if (connection != nullptr) {
					uniqueConnections.insert(connection);
				}
			}
		}
	};
	collectConnections(_graphicalInputPorts);
	collectConnections(_graphicalOutputPorts);

	int updatedConnections = 0;
	for (GraphicalConnection* connection : uniqueConnections) {
		connection->updateDimensionsAndPosition();
		connection->update();
		++updatedConnections;
	}

	qInfo() << "GraphicalModelComponent::itemChange refreshed connections" << updatedConnections
		<< "for component"
		<< (_component != nullptr ? QString::fromStdString(_component->getName()) : QString("<null>"));

	return result;
}

QList<GraphicalComponentPort*> GraphicalModelComponent::getGraphicalOutputPorts() const {
	return _graphicalOutputPorts;
}

QList<GraphicalComponentPort*> GraphicalModelComponent::getGraphicalInputPorts() const {
	return _graphicalInputPorts;
}

QList<ModelDataDefinition*>* GraphicalModelComponent::getInternalData() const {
	return _internalData;
}

QList<ModelDataDefinition*>* GraphicalModelComponent::getAttachedData() const {
	return _attachedData;
}

EntityType* GraphicalModelComponent::getEntityType() const {
	return _entityType;
}

unsigned int GraphicalModelComponent::getOcupiedInputPorts() const {
	return _ocupiedInputPorts;
}

unsigned int GraphicalModelComponent::getOcupiedOutputPorts() const {
	return _ocupiedOutputPorts;
}

void GraphicalModelComponent::setEntityType(EntityType* entityType) {
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

QMap<Queue*, QPair<unsigned int, unsigned int>>* GraphicalModelComponent::getMapQueue() {
	return _mapQueue;
}

QList<QList<GraphicalImageAnimation*>*>* GraphicalModelComponent::getImagesQueue() {
	return _imagesQueue;
}

void GraphicalModelComponent::verifyQueue() {
	clearQueues();

	std::map<std::string, ModelDataDefinition*>* internalData = this->getComponent()->getInternalData();
	std::map<std::string, ModelDataDefinition*>* attachedData = this->getComponent()->getAttachedData();

	QList<ModelDataDefinition*> qListInternalData;
	QList<ModelDataDefinition*> qListAttachedData;
	QList<Queue*> queues;

	for (auto it = internalData->begin(); it != internalData->end(); ++it) {
		qListInternalData.append(it->second);
	}

	for (auto it = attachedData->begin(); it != attachedData->end(); ++it) {
		qListAttachedData.append(it->second);
	}

	for (ModelDataDefinition* internalData : qListInternalData) {
		if (internalData->getClassname() == "Queue") {
			Queue* queue = dynamic_cast<Queue*>(internalData);
			if (queue) {
				queues.append(queue);
				_imagesQueue->append(new QList<GraphicalImageAnimation*>);
			}
		}
	}

	for (ModelDataDefinition* attachedData : qListAttachedData) {
		if (attachedData->getClassname() == "Queue") {
			Queue* queue = dynamic_cast<Queue*>(attachedData);
			if (queue) {
				queues.append(queue);
				_imagesQueue->append(new QList<GraphicalImageAnimation*>);
			}
		}
	}

	if (_component->getClassname() == "PickStation") {
		_imagesQueue->append(new QList<GraphicalImageAnimation*>);
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

unsigned int GraphicalModelComponent::getIndexQueue(Queue* queue) {
	QPair<unsigned int, unsigned int> pairIndexSize = _mapQueue->value(queue);
	return pairIndexSize.first;
}

unsigned int GraphicalModelComponent::getSizeQueue(Queue* queue) {
	QPair<unsigned int, unsigned int> pairIndexSize = _mapQueue->value(queue);
	return pairIndexSize.second;
}

void GraphicalModelComponent::populateMapQueue(QList<Queue*> queues) {
	QList<Queue*> sortedQueues = queues;
	std::sort(sortedQueues.begin(), sortedQueues.end(), &compareQueuesById);

	for (Queue* queue : queues) {
		_mapQueue->insert(queue, qMakePair(sortedQueues.indexOf(queue), (unsigned int)queue->size()));
	}
}

bool GraphicalModelComponent::insertImageQueue(Queue* queue, GraphicalImageAnimation* image) {
	if ((unsigned int)_imagesQueue->size() == getIndexQueue(queue) + 1) {
		QList<GraphicalImageAnimation*>* imagesList = _imagesQueue->at(getIndexQueue(queue));
		imagesList->append(image);
		actualizeMapQueue(queue);

		return true;
	}

	return false;
}

bool GraphicalModelComponent::insertImageQueue(GraphicalImageAnimation* image) {
	if (!_imagesQueue->empty()) {
		QList<GraphicalImageAnimation*>* imagesList = _imagesQueue->at(0);
		imagesList->append(image);
		return true;
	}
	return false;
}

QList<GraphicalImageAnimation*>* GraphicalModelComponent::removeImageQueue(Queue* queue, unsigned int quantityRemoved) {
	if ((unsigned int)_imagesQueue->size() == getIndexQueue(queue) + 1) {
		QList<GraphicalImageAnimation*>* imagesList = _imagesQueue->at(getIndexQueue(queue));

		if (imagesList) {
			if (!imagesList->empty()) {
				QList<GraphicalImageAnimation*>* imagesRemoved = new QList<GraphicalImageAnimation*>();

				if (quantityRemoved > (unsigned int)imagesList->size()) {
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
		QList<GraphicalImageAnimation*>* imagesList = _imagesQueue->at(0);

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

void GraphicalModelComponent::actualizeMapQueue(Queue* queue) {
	_mapQueue->insert(queue, qMakePair(getIndexQueue(queue), queue->size()));
}

void GraphicalModelComponent::visivibleImageQueue(bool visivible) {
	for (QList<GraphicalImageAnimation*>* imagesList : *_imagesQueue) {
		for (GraphicalImageAnimation* image : *imagesList) {
			image->setVisible(visivible);
		}
	}
}

void GraphicalModelComponent::clearQueues() {
	for (QList<GraphicalImageAnimation*>* imagesList : *_imagesQueue) {
		for (GraphicalImageAnimation* image : *imagesList) {
			delete image;
		}
		imagesList->clear();
		delete imagesList;
	}
	_imagesQueue->clear();
	_mapQueue->clear();
	// Keep logical queue state synchronized with structural queue cleanup.
	_hasQueue = false;
}

/*
void ModelComponentGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){

}
void ModelComponentGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event){

}

void ModelComponentGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event){

}
 */
