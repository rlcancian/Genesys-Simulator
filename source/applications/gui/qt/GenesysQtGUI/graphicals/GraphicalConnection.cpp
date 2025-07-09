#include "GraphicalConnection.h"
#include "GraphicalModelComponent.h"
#include <QPainter>

GraphicalConnection::GraphicalConnection(GraphicalComponentPort* sourceGraphicalPort, GraphicalComponentPort* destinationGraphicalPort, unsigned int portSourceConnection, unsigned int portDestinationConnection, QColor color, QGraphicsItem *parent) : QGraphicsObject(parent) {
	//// connect in the model
	//ModelComponent* sourceComponent = sourceGraphicalPort->graphicalComponent()->getComponent();
	//ModelComponent* destComponent=destinationGraphicalPort->graphicalComponent()->getComponent();
	//_sourceConnection = new Connection({sourceComponent,sourceGraphicalPort->portNum()});
	//_destinationConnection = new Connection({destComponent,destinationGraphicalPort->portNum()});
	//sourceComponent->getConnections()->insertAtRank(sourceGraphicalPort->portNum(), _destinationConnection);
	// connect graphically
	_sourceGraphicalPort = sourceGraphicalPort;
    _destinationGraphicalPort = destinationGraphicalPort;
    _sourceConnection = new Connection({sourceGraphicalPort->graphicalComponent()->getComponent(), {sourceGraphicalPort->portNum()}});
    _destinationConnection = new Connection({destinationGraphicalPort->graphicalComponent()->getComponent(), {destinationGraphicalPort->portNum()}});
    _portSourceConnection = portSourceConnection;
    _portDestinationConnection = portDestinationConnection;
    _color = color;
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
	updateDimensionsAndPosition();
	//update source and dest PORTS
	sourceGraphicalPort->addGraphicalConnection(this); // to update connection on port position change
	destinationGraphicalPort->addGraphicalConnection(this);
}

GraphicalConnection::GraphicalConnection(const GraphicalConnection& orig) {

}

GraphicalConnection::~GraphicalConnection() {
    _sourceConnection->component->getConnectionManager()->remove(_destinationConnection);
	_sourceGraphicalPort->removeGraphicalConnection(this);
	_destinationGraphicalPort->removeGraphicalConnection(this);
}

QColor GraphicalConnection::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
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
	qreal x1, x2, y1, y2, w1, w2, h1, h2;
	x1 = _sourceGraphicalPort->scenePos().x();
	x2 = _destinationGraphicalPort->scenePos().x();
	y1 = _sourceGraphicalPort->scenePos().y();
	y2 = _destinationGraphicalPort->scenePos().y();
	w1 = _sourceGraphicalPort->width();
	h1 = _sourceGraphicalPort->height();
	w2 = _destinationGraphicalPort->width();
	h2 = _destinationGraphicalPort->height();
	setPos((x1 < x2 ? x1 + w1 : x2 + w2) - 2/*penwidth*/, y1 < y2 ? y1 : y2);
	//setPos((x1 < x2 ? x1 + w1 : x2 + w2) - 2/*penwidth*/, y1 < y2 ? y1 : y2);
	_width = abs(x2 - x1)-(x1 < x2 ? w2 : w1);
	_height = abs(y2 - y1)+(y1 < y2 ? h2 : h1);
	update(); //@TODO SHould not call it here
}

QRectF GraphicalConnection::boundingRect() const {
	int portWidth = _sourceGraphicalPort->width(); //@TODO REMOVE. Did not solve redraw issue
	int portHeight = _sourceGraphicalPort->height();
	return QRectF(0-portWidth, 0-portHeight, _width+portWidth, _height+portHeight); //@TODO add port dimensions
	return QRectF(0, 0, _width, _height); //@TODO add port dimensions
}

void GraphicalConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	updateDimensionsAndPosition();
	QPen pen = QPen(_color);
	pen.setWidth(2);
	painter->setPen(pen);
	QPainterPath path;
	QPointF inipos;
	QPointF endpos;

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
	path.lineTo(endpos);
	painter->drawPath(path);
	//
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
    return QGraphicsObject::sceneEvent(event); // TODO: CRASH!
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
