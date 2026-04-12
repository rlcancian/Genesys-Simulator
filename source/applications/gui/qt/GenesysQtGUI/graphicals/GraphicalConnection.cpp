#include "GraphicalConnection.h"
#include "GraphicalModelComponent.h"
#include "ModelGraphicsScene.h"
#include <QPainter>
#include <QtMath>

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
    /**
     * @brief Desfaz vínculo modelo<->gráfico da conexão.
     *
     * Ordem adotada:
     * 1) remove relação no ConnectionManager do componente de origem;
     * 2) remove ponteiros desta conexão nas portas gráficas;
     * 3) libera objetos Connection auxiliares alocados neste item gráfico.
     *
     * @todo Avaliar migração de `_sourceConnection/_destinationConnection` para smart pointers.
     */
    if (_sourceConnection != nullptr && _sourceConnection->component != nullptr) {
        _sourceConnection->component->getConnectionManager()->remove(_destinationConnection);
    }
    if (_sourceGraphicalPort != nullptr) {
	    _sourceGraphicalPort->removeGraphicalConnection(this);
    }
    if (_destinationGraphicalPort != nullptr) {
	    _destinationGraphicalPort->removeGraphicalConnection(this);
    }
    delete _destinationConnection;
    delete _sourceConnection;
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
	// Skip geometric work while endpoints/scenes are unstable or during data-definition sync teardown.
	if (_sourceGraphicalPort == nullptr || _destinationGraphicalPort == nullptr) {
		return;
	}
	QGraphicsScene* sourceScene = _sourceGraphicalPort->scene();
	QGraphicsScene* destinationScene = _destinationGraphicalPort->scene();
	if (sourceScene == nullptr || destinationScene == nullptr || sourceScene != destinationScene) {
		return;
	}
	ModelGraphicsScene* modelScene = dynamic_cast<ModelGraphicsScene*>(sourceScene);
	if (modelScene != nullptr && (modelScene->areConnectionGeometryUpdatesBlocked()
	                              || modelScene->isGraphicalDataDefinitionsSyncInProgress())) {
		return;
	}
	if (_sourceGraphicalPort->graphicalComponent() == nullptr || _destinationGraphicalPort->graphicalComponent() == nullptr) {
		return;
	}
	const QPointF sourceScenePos = _sourceGraphicalPort->scenePos();
	const QPointF destinationScenePos = _destinationGraphicalPort->scenePos();
	const qreal w1 = _sourceGraphicalPort->width();
	const qreal h1 = _sourceGraphicalPort->height();
	const qreal w2 = _destinationGraphicalPort->width();
	const qreal h2 = _destinationGraphicalPort->height();

	const qreal newX = (sourceScenePos.x() < destinationScenePos.x() ? sourceScenePos.x() + w1 : destinationScenePos.x() + w2) - 2;
	const qreal newY = sourceScenePos.y() < destinationScenePos.y() ? sourceScenePos.y() : destinationScenePos.y();
	const qreal newWidth = qAbs(destinationScenePos.x() - sourceScenePos.x()) - (sourceScenePos.x() < destinationScenePos.x() ? w2 : w1);
	const qreal newHeight = qAbs(destinationScenePos.y() - sourceScenePos.y()) + (sourceScenePos.y() < destinationScenePos.y() ? h2 : h1);

	const bool positionChanged = !qFuzzyCompare(pos().x() + 1.0, newX + 1.0) || !qFuzzyCompare(pos().y() + 1.0, newY + 1.0);
	const bool geometryChanged = !qFuzzyCompare(_width + 1.0, newWidth + 1.0) || !qFuzzyCompare(_height + 1.0, newHeight + 1.0);
	if (!positionChanged && !geometryChanged) {
		return;
	}
	if (geometryChanged) {
		prepareGeometryChange();
	}
	_width = newWidth;
	_height = newHeight;
	setPos(newX, newY);
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

void GraphicalConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (_sourceGraphicalPort == nullptr || _destinationGraphicalPort == nullptr) {
		return;
	}
	QGraphicsScene* sourceScene = _sourceGraphicalPort->scene();
	QGraphicsScene* destinationScene = _destinationGraphicalPort->scene();
	if (sourceScene == nullptr || destinationScene == nullptr || sourceScene != destinationScene || scene() != sourceScene) {
		return;
	}
	if (_sourceGraphicalPort->graphicalComponent() == nullptr || _destinationGraphicalPort->graphicalComponent() == nullptr) {
		return;
	}
	QPen pen = QPen(_color);
	pen.setWidth(2);
	painter->setPen(pen);
	QPainterPath path;
	QPointF inipos;
	QPointF endpos;

	/**
	 * Bloco 2: normaliza coordenadas locais considerando orientação relativa.
	 */
	qreal x1, x2, y1, y2; // x1 < x2
	if (_sourceGraphicalPort->scenePos().x() < _destinationGraphicalPort->scenePos().x()) {
		x1 = 0-_sourceGraphicalPort->width()/2;
		x2 = _width+_destinationGraphicalPort->width()/2;
	} else {
		x2 = 0-_destinationGraphicalPort->width()/2;
		x1 = _width+_sourceGraphicalPort->width()/2;
	}
	// y1 < y2
	if (_sourceGraphicalPort->scenePos().y() < _destinationGraphicalPort->scenePos().y()) {
		y1 = _sourceGraphicalPort->height() / 2.0;
		y2 = _height - _sourceGraphicalPort->height() / 2.0;
	} else {
		y2 = _sourceGraphicalPort->height() / 2.0;
		y1 = _height - _sourceGraphicalPort->height() / 2.0;
	}
	inipos = QPointF(x1, y1); //QPointF(_sourceGraphicalPort->pos());//_sourceGraphicalPort->pos().x()+_sourceGraphicalPort->width()/2.0, _sourceGraphicalPort->pos().y()+_sourceGraphicalPort->height()/2.0
	endpos = QPointF(x2, y2); //QPointF(_destinationGraphicalPort->pos());// _destinationGraphicalPort->pos().x()+_destinationGraphicalPort->width()/2.0, _destinationGraphicalPort->pos().y()+_destinationGraphicalPort->height()/2.0
	/**
	 * Bloco 3: gera path de desenho com base no tipo de roteamento.
	 */
	path.moveTo(inipos);
	switch (_connectionType) {
		case ConnectionType::HORIZONTAL:
			path.lineTo((x1 + x2) / 2, y1);
            path.lineTo((x1 + x2) / 2, y2);
            _points.clear();
            _points.append(mapToScene(inipos));
            _points.append(mapToScene(QPointF(inipos.x() + ((x1 + x2) / 2), y1)));
            _points.append(mapToScene(QPointF(inipos.x() + ((x1 + x2) / 2), y2)));
            _points.append(mapToScene(endpos));
			break;
		case ConnectionType::VERTICAL:
            path.lineTo(x1, (y1 + y2) / 2);
            path.lineTo(x2, (y1 + y2) / 2);
			break;
		case ConnectionType::DIRECT:
			break;
		case ConnectionType::USERDEFINED:
			//@TODO: draw intermediate points
			break;
	}
	/**
	 * Bloco 4: renderiza o path final da conexão.
	 */
	path.lineTo(endpos);
	painter->drawPath(path);
	//
	/**
	 * Bloco 5: desenha “handles” quando selecionado.
	 */
	if (isSelected()) {
		pen = QPen(Qt::black);
		pen.setWidth(1);
		painter->setPen(pen);
		QBrush brush = QBrush(Qt::SolidPattern);
		brush.setColor(Qt::black);
		painter->setBrush(brush);
		//@TODO Check this out to see if it solves the move redraw issue
		painter->drawRect(QRectF(x1 < x2 ? x1 : x1 - _selWidth, y1 - _selWidth / 2, _selWidth, _selWidth));
		painter->drawRect(QRectF(x2 < x1 ? x2 : x2 - _selWidth, y2 - _selWidth / 2, _selWidth, _selWidth));
		painter->drawRect(QRectF((x1 + x2) / 2 - _selWidth / 2, y1 - _selWidth / 2, _selWidth, _selWidth));
		painter->drawRect(QRectF((x1 + x2) / 2 - _selWidth / 2, y2 - _selWidth / 2, _selWidth, _selWidth));
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
