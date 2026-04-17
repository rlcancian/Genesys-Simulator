#include "GraphicalConnection.h"
#include "GraphicalModelComponent.h"
#include "ModelGraphicsScene.h"
#include <QPainter>
#include <QPainterPathStroker>

GraphicalConnection::GraphicalConnection(GraphicalComponentPort* sourceGraphicalPort, GraphicalComponentPort* destinationGraphicalPort, unsigned int portSourceConnection, unsigned int portDestinationConnection, QColor color, QGraphicsItem *parent) : QGraphicsObject(parent) {
	/**
	 * Bloco 1: inicialização de metadados de conexão.
	 * Mantemos wrappers de Connection para facilitar sincronização com kernel.
	 */
	_sourceGraphicalPort = sourceGraphicalPort;
    _destinationGraphicalPort = destinationGraphicalPort;
    _sourceConnection = new Connection({sourceGraphicalPort->graphicalComponent()->getComponent(), {sourceGraphicalPort->portNum()}});
    _destinationConnection = new Connection({destinationGraphicalPort->graphicalComponent()->getComponent(), {destinationGraphicalPort->portNum()}});
    _portSourceConnection = portSourceConnection;
    _portDestinationConnection = portDestinationConnection;
    _color = color;
    /**
     * Bloco 2: configuração visual/interativa do item.
     */
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
    /**
     * Bloco 3: cálculo inicial de geometria da conexão.
     */
	updateDimensionsAndPosition();
	/**
	 * Bloco 4: registro bidirecional nas portas para atualização quando houver movimento.
	 *
	 * @todo Extrair para método dedicado `_bindPorts()` para legibilidade.
	 */
	sourceGraphicalPort->addGraphicalConnection(this); // to update connection on port position change
	destinationGraphicalPort->addGraphicalConnection(this);
}

GraphicalConnection::GraphicalConnection(const GraphicalConnection& orig) {

}

GraphicalConnection::~GraphicalConnection() {
	Connection* sourceConnection = _sourceConnection;
	Connection* destinationConnection = _destinationConnection;
	GraphicalComponentPort* sourceGraphicalPort = _sourceGraphicalPort;
	GraphicalComponentPort* destinationGraphicalPort = _destinationGraphicalPort;

	_sourceConnection = nullptr;
	_destinationConnection = nullptr;
	_sourceGraphicalPort = nullptr;
	_destinationGraphicalPort = nullptr;

	if (sourceGraphicalPort != nullptr) {
		sourceGraphicalPort->removeGraphicalConnection(this);
	}
	if (destinationGraphicalPort != nullptr) {
		destinationGraphicalPort->removeGraphicalConnection(this);
	}

	// Destination connection may already be destroyed by ConnectionManager::remove;
	// avoid double free during scene/model teardown.
	if (destinationConnection != nullptr) {
		ConnectionManager* manager = nullptr;
		if (sourceConnection != nullptr && sourceConnection->component != nullptr) {
			manager = sourceConnection->component->getConnectionManager();
		}
		bool isManagedConnection = false;
		if (manager != nullptr && manager->connections() != nullptr) {
			for (const std::pair<const unsigned int, Connection*>& connectionPair : *manager->connections()) {
				if (connectionPair.second == destinationConnection) {
					isManagedConnection = true;
					break;
				}
			}
		}
		if (isManagedConnection) {
			manager->remove(destinationConnection);
		} else {
			delete destinationConnection;
		}
	}

	delete sourceConnection;
}

GraphicalConnection::ConnectionType GraphicalConnection::connectionType() const
{
	return _connectionType;
}

void GraphicalConnection::setConnectionType(GraphicalConnection::ConnectionType newConnectionType)
{
	_connectionType = newConnectionType;
}

void GraphicalConnection::updateDimensionsAndPosition() {
	if (!canRefreshGeometry()) {
		return;
	}
	/**
	 * Bloco 1: coleta de geometria absoluta de portas.
	 */
	qreal x1, x2, y1, y2, w1, w2, h1, h2;
	x1 = _sourceGraphicalPort->scenePos().x();
	x2 = _destinationGraphicalPort->scenePos().x();
	y1 = _sourceGraphicalPort->scenePos().y();
	y2 = _destinationGraphicalPort->scenePos().y();
	w1 = _sourceGraphicalPort->width();
	h1 = _sourceGraphicalPort->height();
	w2 = _destinationGraphicalPort->width();
	h2 = _destinationGraphicalPort->height();

	const bool sourceIsLeft = x1 < x2;
	const bool sourceIsAbove = y1 < y2;
	const QPointF newPos((sourceIsLeft ? x1 + w1 : x2 + w2) - 2/*penwidth*/, sourceIsAbove ? y1 : y2);
	const qreal newWidth = abs(x2 - x1) - (sourceIsLeft ? w2 : w1);
	const qreal newHeight = abs(y2 - y1) + (sourceIsAbove ? h2 : h1);
	const QPointF sourceLocal(sourceIsLeft ? -w1 / 2.0 : newWidth + w1 / 2.0,
	                         sourceIsAbove ? h1 / 2.0 : newHeight - h1 / 2.0);
	const QPointF destinationLocal(sourceIsLeft ? newWidth + w2 / 2.0 : -w2 / 2.0,
	                              sourceIsAbove ? newHeight - h1 / 2.0 : h1 / 2.0);
	const bool geometryChanged = !qFuzzyCompare(_width + 1.0, newWidth + 1.0)
	        || !qFuzzyCompare(_height + 1.0, newHeight + 1.0)
	        || !qFuzzyCompare(pos().x() + 1.0, newPos.x() + 1.0)
	        || !qFuzzyCompare(pos().y() + 1.0, newPos.y() + 1.0);
	if (geometryChanged) {
		prepareGeometryChange();
	}
	/**
	 * Bloco 2: projeta esta conexão para um sistema de coordenadas local mínimo.
	 */
	setPos(newPos);
	_width = newWidth;
	_height = newHeight;
	_sourcePointLocal = sourceLocal;
	_destinationPointLocal = destinationLocal;
	_points.clear();
	_points.append(mapToScene(_sourcePointLocal));
	if (_connectionType == ConnectionType::HORIZONTAL) {
		_points.append(mapToScene(QPointF((_sourcePointLocal.x() + _destinationPointLocal.x()) / 2.0, _sourcePointLocal.y())));
		_points.append(mapToScene(QPointF((_sourcePointLocal.x() + _destinationPointLocal.x()) / 2.0, _destinationPointLocal.y())));
	}
	_points.append(mapToScene(_destinationPointLocal));
	/**
	 * Bloco 3: mantém apenas atualização geométrica local.
	 */
}

bool GraphicalConnection::canRefreshGeometry() const {
	// Skip geometric work while endpoints/scenes are unstable or during data-definition sync teardown.
	if (_sourceGraphicalPort == nullptr || _destinationGraphicalPort == nullptr) {
		return false;
	}
	QGraphicsScene* sourceScene = _sourceGraphicalPort->scene();
	QGraphicsScene* destinationScene = _destinationGraphicalPort->scene();
	if (sourceScene == nullptr || destinationScene == nullptr || sourceScene != destinationScene) {
		return false;
	}
	ModelGraphicsScene* modelScene = dynamic_cast<ModelGraphicsScene*>(sourceScene);
	if (modelScene != nullptr && (modelScene->areConnectionGeometryUpdatesBlocked()
	                              || modelScene->isGraphicalDataDefinitionsSyncInProgress())) {
		return false;
	}
	return _sourceGraphicalPort->graphicalComponent() != nullptr && _destinationGraphicalPort->graphicalComponent() != nullptr;
}

QRectF GraphicalConnection::boundingRect() const {
	/**
	 * Bloco 1: calcula margem extra baseada na porta para reduzir clipping visual.
	 */
	int portWidth = _sourceGraphicalPort != nullptr ? _sourceGraphicalPort->width() : 0;
	int portHeight = _sourceGraphicalPort != nullptr ? _sourceGraphicalPort->height() : 0;
	/**
	 * Bloco 2: retorna retângulo local com margem.
	 */
	return QRectF(0 - portWidth, 0 - portHeight, _width + portWidth, _height + portHeight);
}

QPainterPath GraphicalConnection::connectionPath() const {
	if (!canRefreshGeometry()) {
		return QPainterPath();
	}

	return GraphicalConnectionStyle::modelConnectionPath(_sourcePointLocal,
	                                                     _destinationPointLocal,
	                                                     routeTypeForStyle(),
	                                                     usesCurvedStyle());
}

GraphicalConnectionStyle::RouteType GraphicalConnection::routeTypeForStyle() const {
	switch (_connectionType) {
	case ConnectionType::HORIZONTAL:
		return GraphicalConnectionStyle::RouteType::Horizontal;
	case ConnectionType::VERTICAL:
		return GraphicalConnectionStyle::RouteType::Vertical;
	case ConnectionType::DIRECT:
		return GraphicalConnectionStyle::RouteType::Direct;
	case ConnectionType::USERDEFINED:
	default:
		return GraphicalConnectionStyle::RouteType::UserDefined;
	}
}

QPainterPath GraphicalConnection::shape() const {
	// Keep hit testing close to the visible line instead of using the large bounding rectangle.
	QPainterPathStroker stroker;
	stroker.setWidth(qMax<qreal>(8.0, static_cast<qreal>(_selWidth)));
	stroker.setCapStyle(Qt::RoundCap);
	stroker.setJoinStyle(Qt::RoundJoin);
	return stroker.createStroke(connectionPath());
}

void GraphicalConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (!canRefreshGeometry()) {
		return;
	}
	QPen pen = QPen(_color);
	const bool curvedStyle = usesCurvedStyle();
	if (curvedStyle) {
		painter->setRenderHint(QPainter::Antialiasing, true);
		pen.setColor(QColor(47, 128, 237, 220));
		pen.setWidth(isSelected() ? 4 : 3);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
	} else {
		pen.setWidth(2);
	}
	painter->setPen(pen);

	/**
	 * Bloco 2: normaliza coordenadas locais considerando orientação relativa.
	 */
	const qreal x1 = _sourcePointLocal.x();
	const qreal y1 = _sourcePointLocal.y();
	const qreal x2 = _destinationPointLocal.x();
	const qreal y2 = _destinationPointLocal.y();
	/**
	 * Bloco 3: gera path de desenho com base no tipo de roteamento.
	 */
	const QPainterPath path = connectionPath();
	/**
	 * Bloco 4: renderiza o path final da conexão.
	 */
	painter->drawPath(path);
	//
	/**
	 * Bloco 5: desenha “handles” quando selecionado.
	 */
	if (isSelected()) {
		pen = QPen(curvedStyle ? QColor(47, 128, 237) : Qt::black);
		pen.setWidth(1);
		painter->setPen(pen);
		QBrush brush = QBrush(Qt::SolidPattern);
		brush.setColor(curvedStyle ? QColor(47, 128, 237) : Qt::black);
		painter->setBrush(brush);
		if (curvedStyle) {
			const QRectF startHandle(x1 - _selWidth / 2.0, y1 - _selWidth / 2.0, _selWidth, _selWidth);
			const QRectF midHandle(path.pointAtPercent(0.5).x() - _selWidth / 2.0,
			                       path.pointAtPercent(0.5).y() - _selWidth / 2.0,
			                       _selWidth,
			                       _selWidth);
			const QRectF endHandle(x2 - _selWidth / 2.0, y2 - _selWidth / 2.0, _selWidth, _selWidth);
			painter->drawEllipse(startHandle);
			painter->drawEllipse(midHandle);
			painter->drawEllipse(endHandle);
		} else {
			//@TODO Check this out to see if it solves the move redraw issue
			painter->drawRect(QRectF(x1 < x2 ? x1 : x1 - _selWidth, y1 - _selWidth / 2, _selWidth, _selWidth));
			painter->drawRect(QRectF(x2 < x1 ? x2 : x2 - _selWidth, y2 - _selWidth / 2, _selWidth, _selWidth));
			painter->drawRect(QRectF((x1 + x2) / 2 - _selWidth / 2, y1 - _selWidth / 2, _selWidth, _selWidth));
			painter->drawRect(QRectF((x1 + x2) / 2 - _selWidth / 2, y2 - _selWidth / 2, _selWidth, _selWidth));
		}
	}
	//
	//pen = QPen(Qt::yellow);
	//pen.setWidth(1);
	//painter->setPen(pen);
	//painter->drawRect(QRectF(0,0,_width-1,_height-1));
}

QList<QPointF> GraphicalConnection::getPoints() const {
    return _points;
}

bool GraphicalConnection::usesCurvedStyle() const {
	return GraphicalConnectionStyle::usesModernCurves();
}

QPainterPath GraphicalConnection::animationPathForImage(qreal imageWidth, qreal imageHeight) const {
	if (!canRefreshGeometry()) {
		return QPainterPath();
	}
	return mapToScene(connectionPath()).translated(-imageWidth / 2.0, -imageHeight / 2.0);
}

Connection* GraphicalConnection::getSource() const {
	return _sourceConnection;
}

Connection* GraphicalConnection::getDestination() const {
	return _destinationConnection;
}

bool GraphicalConnection::sceneEvent(QEvent *event) {
    return QGraphicsObject::sceneEvent(event);
}

unsigned int GraphicalConnection::getPortSourceConnection() const {
    return _portSourceConnection;
}
unsigned int GraphicalConnection::getPortDestinationConnection() const {
    return _portDestinationConnection;
}
GraphicalComponentPort* GraphicalConnection::getSourceGraphicalPort() {
	return _sourceGraphicalPort;
}
GraphicalComponentPort* GraphicalConnection::getDestinationGraphicalPort(){
    return _destinationGraphicalPort;
}
