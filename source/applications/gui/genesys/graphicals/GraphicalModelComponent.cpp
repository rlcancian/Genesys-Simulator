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
#include "plugins/components/Decisions/Decide.h"
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
	setToolTip(QString::fromStdString(component->getLabel().empty() ? component->getName() : component->getLabel()));
	// create input output ports
	GraphicalComponentPort* port;
	qreal px, py = 0;
	unsigned int numInputPorts = std::max<unsigned int>(
		component->getConnectionManager()->getCurrentInputConnectionsSize(),
		plugin->getPluginInfo()->getMinimumInputs());
	numInputPorts = std::max(numInputPorts, component->getConnectionManager()->getMinInputConnections());
	if (component->getDynamicInputPortCount() >= 0) {
		numInputPorts = std::max(numInputPorts, static_cast<unsigned int>(component->getDynamicInputPortCount()));
	}
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
	unsigned int declaredOutputPorts = plugin->getPluginInfo()->getMinimumOutputs();
	if (auto* decide = dynamic_cast<Decide*>(component); decide != nullptr) {
		declaredOutputPorts = decide->getConditions()->size() + 1;
	}
	unsigned int numOutputPorts = std::max<unsigned int>(
		component->getConnectionManager()->getCurrentOutputConnectionsSize(), declaredOutputPorts);
	numOutputPorts = std::max(numOutputPorts, component->getConnectionManager()->getMinOutputConnections());
	if (component->getDynamicOutputPortCount() >= 0) {
		numOutputPorts = std::max(numOutputPorts, static_cast<unsigned int>(component->getDynamicOutputPortCount()));
	}
	if (dynamic_cast<Decide*>(component) != nullptr) {
		// A Decide's semantic ports are defined by its ordered conditions, even if
		// a malformed/stale connection map contains an out-of-range key.
		numOutputPorts = declaredOutputPorts;
	}
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

void GraphicalModelComponent::editOutputPort(unsigned int index, bool inserted) {
	if (index > static_cast<unsigned int>(_graphicalOutputPorts.size())) return;
	if (inserted) {
		auto* port = new GraphicalComponentPort(this, false, index, nullptr);
		port->setParentItem(this);
		port->setX(_width - port->width());
		_graphicalOutputPorts.insert(static_cast<int>(index), port);
	} else {
		if (index >= static_cast<unsigned int>(_graphicalOutputPorts.size())) return;
		GraphicalComponentPort* port = _graphicalOutputPorts.at(static_cast<int>(index));
		const QList<GraphicalConnection*> attached = *port->getConnections();
		for (GraphicalConnection* connection : attached) {
			if (connection == nullptr) continue;
			if (auto* modelScene = dynamic_cast<ModelGraphicsScene*>(connection->scene())) {
				modelScene->removeGraphicalConnectionVisual(connection);
			} else {
				if (connection->scene() != nullptr) connection->scene()->removeItem(connection);
				delete connection;
			}
		}
		_graphicalOutputPorts.removeAt(static_cast<int>(index));
		delete port;
	}

	const unsigned int count = static_cast<unsigned int>(_graphicalOutputPorts.size());
	const qreal step = static_cast<qreal>(_height) / static_cast<qreal>(count + 1);
	for (unsigned int portIndex = 0; portIndex < count; ++portIndex) {
		GraphicalComponentPort* port = _graphicalOutputPorts.at(static_cast<int>(portIndex));
		port->setPortNum(portIndex);
		port->setY(step * static_cast<qreal>(portIndex + 1) - port->height() / 2);
		for (GraphicalConnection* connection : *port->getConnections()) {
			if (connection != nullptr && connection->getSourceGraphicalPort() == port) {
				connection->setSourcePortNumber(portIndex);
				connection->updateDimensionsAndPosition();
			}
		}
	}
}

void GraphicalModelComponent::moveOutputPort(unsigned int from, unsigned int to) {
	if (_graphicalOutputPorts.isEmpty() || from >= static_cast<unsigned int>(_graphicalOutputPorts.size() - 1)
		|| to >= static_cast<unsigned int>(_graphicalOutputPorts.size() - 1) || from == to) return;
	_graphicalOutputPorts.move(static_cast<int>(from), static_cast<int>(to));
	const unsigned int count = static_cast<unsigned int>(_graphicalOutputPorts.size());
	const qreal step = static_cast<qreal>(_height) / static_cast<qreal>(count + 1);
	for (unsigned int index = 0; index < count; ++index) {
		GraphicalComponentPort* port = _graphicalOutputPorts.at(static_cast<int>(index));
		port->setPortNum(index);
		port->setY(step * static_cast<qreal>(index + 1) - port->height() / 2);
		for (GraphicalConnection* connection : *port->getConnections()) {
			if (connection != nullptr && connection->getSourceGraphicalPort() == port) {
				connection->setSourcePortNumber(index);
				connection->updateDimensionsAndPosition();
			}
		}
	}
}

void GraphicalModelComponent::synchronizeDynamicPorts() {
	if (_component == nullptr) return;
	const int dynamicInputs = _component->getDynamicInputPortCount();
	const int dynamicOutputs = _component->getDynamicOutputPortCount();
	if (dynamicInputs < 0 && dynamicOutputs < 0) return;

	auto synchronize = [this](QList<GraphicalComponentPort*>& ports, unsigned int desiredCount, bool input) {
		for (unsigned int index = static_cast<unsigned int>(ports.size()); index > desiredCount; --index) {
			GraphicalComponentPort* port = ports.at(static_cast<int>(index - 1));
			if (port != nullptr && !port->getConnections()->isEmpty()) break;
			ports.removeAt(static_cast<int>(index - 1));
			delete port;
		}
		while (static_cast<unsigned int>(ports.size()) < desiredCount) {
			const unsigned int index = static_cast<unsigned int>(ports.size());
			auto* port = new GraphicalComponentPort(this, input, index, nullptr);
			port->setParentItem(this);
			port->setX(input ? 0 : _width - port->width());
			ports.append(port);
		}
		const unsigned int count = static_cast<unsigned int>(ports.size());
		const qreal step = static_cast<qreal>(_height) / static_cast<qreal>(count + 1);
		for (unsigned int index = 0; index < count; ++index) {
			GraphicalComponentPort* port = ports.at(static_cast<int>(index));
			port->setPortNum(index);
			port->setY(step * static_cast<qreal>(index + 1) - port->height() / 2);
			for (GraphicalConnection* connection : *port->getConnections()) {
				if (connection == nullptr) continue;
				if (input && connection->getDestinationGraphicalPort() == port) {
					connection->setDestinationPortNumber(index);
				} else if (!input && connection->getSourceGraphicalPort() == port) {
					connection->setSourcePortNumber(index);
				}
				connection->updateDimensionsAndPosition();
			}
		}
	};

	if (dynamicInputs >= 0) {
		unsigned int required = static_cast<unsigned int>(dynamicInputs);
		for (GraphicalComponentPort* port : _graphicalInputPorts) {
			if (port != nullptr && !port->getConnections()->isEmpty()) {
				required = std::max(required, port->portNum() + 1);
			}
		}
		synchronize(_graphicalInputPorts, required, true);
	}
	if (dynamicOutputs >= 0) {
		unsigned int required = static_cast<unsigned int>(dynamicOutputs);
		for (GraphicalComponentPort* port : _graphicalOutputPorts) {
			if (port != nullptr && !port->getConnections()->isEmpty()) {
				required = std::max(required, port->portNum() + 1);
			}
		}
		synchronize(_graphicalOutputPorts, required, false);
	}
	update();
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
	context.secondaryText = QString::fromStdString(_component->getLabel());
	context.tertiaryText = QString(); //QString::fromStdString(_component->getClassname());
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


QMap<ModelDataDefinition*, QPair<unsigned int, unsigned int>>* GraphicalModelComponent::getAnimationTrackMap() { return _animationTrackMap; }
QList<QList<GraphicalImageAnimation*>*>* GraphicalModelComponent::getAnimationTracks() { return _animationTracks; }

void GraphicalModelComponent::resetAnimationTracks() {
	clearAnimationTracks();
	_hasAnimationTracks = false;
}

void GraphicalModelComponent::addAnimationTrack(ModelDataDefinition* source, unsigned int index, unsigned int currentSize) {
	_animationTracks->append(new QList<GraphicalImageAnimation*>);
	if (source != nullptr) {
		_animationTrackMap->insert(source, qMakePair(index, currentSize));
		_hasAnimationTracks = true;
	}
}

void GraphicalModelComponent::addUnmappedAnimationTrack() {
	_animationTracks->append(new QList<GraphicalImageAnimation*>);
	_hasAnimationTracks = true;
}

bool GraphicalModelComponent::hasAnimationTracks() const { return _hasAnimationTracks; }

unsigned int GraphicalModelComponent::getAnimationTrackIndex(ModelDataDefinition* source) const {
	return _animationTrackMap->value(source).first;
}

unsigned int GraphicalModelComponent::getAnimationTrackSize(ModelDataDefinition* source) const {
	return _animationTrackMap->value(source).second;
}

bool GraphicalModelComponent::insertAnimationImage(ModelDataDefinition* source, GraphicalImageAnimation* image, unsigned int newSize) {
	if (!_animationTrackMap->contains(source)) return false;
	const unsigned int index = getAnimationTrackIndex(source);
	if (index >= static_cast<unsigned int>(_animationTracks->size())) return false;
	_animationTracks->at(static_cast<int>(index))->append(image);
	_animationTrackMap->insert(source, qMakePair(index, newSize));
	return true;
}

QList<GraphicalImageAnimation*>* GraphicalModelComponent::removeAnimationImages(ModelDataDefinition* source, unsigned int quantityRemoved, unsigned int newSize) {
	if (!_animationTrackMap->contains(source)) return nullptr;
	const unsigned int index = getAnimationTrackIndex(source);
	if (index >= static_cast<unsigned int>(_animationTracks->size())) return nullptr;
	auto* images = _animationTracks->at(static_cast<int>(index));
	if (images == nullptr || images->isEmpty()) return nullptr;
	auto* removed = new QList<GraphicalImageAnimation*>;
	quantityRemoved = std::min(quantityRemoved, static_cast<unsigned int>(images->size()));
	for (unsigned int i = 0; i < quantityRemoved; ++i) removed->append(images->takeLast());
	_animationTrackMap->insert(source, qMakePair(index, newSize));
	return removed->isEmpty() ? (delete removed, nullptr) : removed;
}

bool GraphicalModelComponent::insertAnimationImage(GraphicalImageAnimation* image) {
	if (_animationTracks->isEmpty() || _animationTracks->first() == nullptr) return false;
	_animationTracks->first()->append(image);
	return true;
}

GraphicalImageAnimation* GraphicalModelComponent::removeAnimationImage() {
	if (_animationTracks->isEmpty() || _animationTracks->first() == nullptr || _animationTracks->first()->isEmpty()) return nullptr;
	return _animationTracks->first()->takeLast();
}

void GraphicalModelComponent::setAnimationTracksVisible(bool visible) {
	for (auto* track : *_animationTracks) if (track != nullptr) for (auto* image : *track) if (image != nullptr) image->setVisible(visible);
}

void GraphicalModelComponent::clearAnimationTracks() {
	for (auto* track : *_animationTracks) {
		if (track == nullptr) continue;
		for (auto* image : *track) delete image;
		track->clear();
		delete track;
	}
	_animationTracks->clear();
	_animationTrackMap->clear();
	_hasAnimationTracks = false;
}

/*
void ModelComponentGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){

}
void ModelComponentGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event){

}

void ModelComponentGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event){

}
 */
