#include "GraphicalComponentPort.h"
#include "GraphicalConnection.h"
#include "GraphicalModelComponent.h"
#include "ModelGraphicsScene.h"
#include <QPainter>
#include "TraitsGUI.h"
#include "UtilGUI.h"

GraphicalComponentPort::GraphicalComponentPort(GraphicalModelComponent* componentGraph, bool isInputPort, unsigned int portNum, QGraphicsItem *parent) : QGraphicsObject(parent) {
	//_component = component;
	//_color = color;
	//_color.setAlpha(64);
	_isInputPort = isInputPort;
	_portNum = portNum;
	_componentGraph = componentGraph;
	if (!isInputPort) {
		_width *= 1.15;
		_height *= 1.15;
	}
	//setPos(0,0);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
	setAcceptHoverEvents(true);
	setAcceptTouchEvents(true);
	setActive(true);
	setSelected(false);
	QString text = QString::fromStdString(componentGraph->getComponent()->getName()) + "\n";
	if (isInputPort)
		text += "Input Port";
	else
		text += "Output Port";
	text += " " + QString::fromStdString(std::to_string(portNum));
	setToolTip(text);
}

//-------------------------
// PROTECTED VIRTUAL FUNCTIONS
//-------------------------

/*
void GraphicalComponentPort::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
	QGraphicsObject::mousePressEvent(mouseEvent);
}

void GraphicalComponentPort::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
	QGraphicsObject::mouseReleaseEvent(mouseEvent);
}

void GraphicalComponentPort::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
	QGraphicsObject::mouseMoveEvent(mouseEvent);
}
 */

QVariant GraphicalComponentPort::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
	QVariant res = QGraphicsObject::itemChange(change, value);
	// Avoid connection geometry updates while scene membership or parentage is changing.
	if (change == QGraphicsItem::ItemSceneChange
	        || change == QGraphicsItem::ItemSceneHasChanged
	        || change == QGraphicsItem::ItemParentChange
	        || change == QGraphicsItem::ItemParentHasChanged) {
		return res;
	}

	// Update connected edges only when scene/component are stable and final position is committed.
	if (change == QGraphicsItem::ItemPositionHasChanged || change == QGraphicsItem::ItemScenePositionHasChanged) {
		QGraphicsScene* ownerScene = scene();
		ModelGraphicsScene* modelScene = dynamic_cast<ModelGraphicsScene*>(ownerScene);
		if (ownerScene == nullptr || _componentGraph == nullptr
		        || (modelScene != nullptr && (modelScene->isGraphicalDataDefinitionsSyncInProgress()
		                                       || modelScene->areConnectionGeometryUpdatesBlocked()))) {
			return res;
		}
		for (GraphicalConnection* graphconnection : *_connections) {
			if (graphconnection == nullptr) {
				continue;
			}
			if (graphconnection->scene() != nullptr && graphconnection->scene() != ownerScene) {
				continue;
			}
			graphconnection->updateDimensionsAndPosition();
			//graphconnection->update();//updateDimensions();
		}
	}
	return res;
}


//----------------------
// overrides
//----------------------

QRectF GraphicalComponentPort::boundingRect() const {
	//qreal penWidth = _pen.width();
	//return QRectF(penWidth / 2, penWidth / 2, _width + penWidth, _height + penWidth);
	return QRectF(0, 0, _width, _height);
}

QColor GraphicalComponentPort::myrgba(uint64_t color) {
	uint8_t r, g, b, a;
	r = (color&0xFF000000)>>24;
	g = (color&0x00FF0000)>>16;
	b = (color&0x0000FF00)>>8;
	a = (color&0x000000FF);
	return QColor(r, g, b, a);
}

void GraphicalComponentPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	int _penWidth = TraitsGUI<GComponentPort>::penWidth;
	int _iwidth = _width - 2 * _margin - _penWidth;
	int _iheight = _height - 2 * _margin - _penWidth;
	QPen pen;
	QBrush brush;
	QRect rect;
	QPainterPath path;
	QPainterPath path2;
	// pen border
	pen = QPen(myrgba(TraitsGUI<GComponentPort>::borderColor));//Qt::black);
	pen.setWidth(_penWidth);
	pen.setJoinStyle(Qt::RoundJoin);
	painter->setPen(pen);
	if (_isInputPort) {
		// Path raised
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GComponentPort>::pathRaised));//Qt::darkGray);
		painter->setBrush(brush);
		path.moveTo(_margin, _margin);
		path.lineTo(_margin, _margin + _iheight);
		path.lineTo(_margin + _raise, _margin + _iheight - _raise);
		path.lineTo(_margin + _raise, _margin + _raise);
		path.lineTo(_margin, _margin);
		path.lineTo(_margin + _iwidth, _margin);
		path.lineTo(_margin + _iwidth - _raise, _margin + _raise);
		path.lineTo(_margin + _raise, _margin + _raise);
		painter->drawPath(path);
		// path stunken
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GComponentPort>::pathStunken));//Qt::white);
		painter->setBrush(brush);
		path2.moveTo(_margin + _iwidth, _margin + _iheight);
		path2.lineTo(_margin + _iwidth, _margin);
		path2.lineTo(_margin + _iwidth - _raise, _margin + _raise);
		path2.lineTo(_margin + _iwidth - _raise, _margin + _iheight - _raise);
		path2.lineTo(_margin + _raise, _margin + _iheight - _raise);
		path2.lineTo(_margin, _margin + _iheight);
		path2.lineTo(_margin + _iwidth, _margin + _iheight);
		path2.lineTo(_margin + _iwidth - _raise, _margin + _iheight - _raise);
		painter->drawPath(path2);
		// fill
		brush = QBrush(Qt::SolidPattern);
		if (_connections->size() > 0) { // connected port
			brush.setColor(myrgba(TraitsGUI<GComponentPort>::connectedBackgroundColor));
		} else {
			brush.setColor(myrgba(TraitsGUI<GComponentPort>::unconnectedBackgroundColor));
		}
		painter->setBrush(brush);
		rect = QRect(_margin + _raise, _margin + _raise, _margin + _iwidth - 2 * _raise - _margin, _margin + _iheight - 2 * _raise - _margin);
		painter->drawRect(rect);
	} else {
		// output port
		// Path raised
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GComponentPort>::pathRaised));//Qt::darkGray);
		painter->setBrush(brush);
		path.moveTo(_margin, _margin);
		path.lineTo(_margin, _margin + _iheight);
		path.lineTo(_margin + _raise, _margin + _iheight - 2 * _raise);
		path.lineTo(_margin + _raise, _margin + 2 * _raise);
		path.lineTo(_margin, _margin);
		path.lineTo(_margin + _iwidth, _margin + _iheight / 2.0);
		path.lineTo(_margin + _iwidth - 2 * _raise, _margin + _iheight / 2.0);
		path.lineTo(_margin + _raise, _margin + 2 * _raise);
		painter->drawPath(path);
		// path stunken
		brush = QBrush(Qt::SolidPattern);
		brush.setColor(myrgba(TraitsGUI<GComponentPort>::pathStunken));//Qt::white);
		painter->setBrush(brush);
		path2.moveTo(_margin + _iwidth, _margin + _iheight / 2.0);
		path2.lineTo(_margin + _iwidth - 1 - _raise, _margin + _iheight / 2.0);
		path2.lineTo(_margin + _raise, _margin + _iheight - 1 - _raise);
		path2.lineTo(_margin, _margin + _iheight);
		path2.lineTo(_margin + _iwidth, _margin + _iheight / 2.0);
		painter->drawPath(path2);
		// fill
		brush = QBrush(Qt::SolidPattern);
		if (_connections->size() > 0) { // connected port
			brush.setColor(myrgba(TraitsGUI<GComponentPort>::connectedBackgroundColor));
		} else {
			brush.setColor(myrgba(TraitsGUI<GComponentPort>::unconnectedBackgroundColor));
		}
		painter->setBrush(brush);
		QPainterPath path3;
		path3.moveTo(_margin + _raise, _margin + 1 + _raise);
		path3.lineTo(_margin + _iwidth - 1 - _raise, _margin + _iheight / 2.0);
		path3.lineTo(_margin + _raise, _margin + _iheight - 1 - _raise);
		painter->drawPath(path3);
	}
	if (this->isSelected()) { //draw squares on corners
		brush = QBrush(Qt::NoBrush);
		brush.setColor(myrgba(TraitsGUI<GComponentPort>::selectionSquaresColor));//QColor(0, 0, 0, 255));
		painter->setBrush(brush);
		rect = QRect(0, 0, _width - _penWidth, _height - _penWidth);
		painter->drawRect(rect);
	}
}

qreal GraphicalComponentPort::width() const {
	return _width;
}

qreal GraphicalComponentPort::height() const {
	return _height;
}


//---------------------

void GraphicalComponentPort::addGraphicalConnection(GraphicalConnection* connection) {
	_connections->append(connection);
}

void GraphicalComponentPort::removeGraphicalConnection(GraphicalConnection* connection) {
    if (connection && _connections) {
        if (!_connections->empty()) {
            _connections->removeOne(connection);
        }
    }
}


//---------------------

unsigned int GraphicalComponentPort::portNum() const {
	return _portNum;
}

bool GraphicalComponentPort::isInputPort() const {
	return _isInputPort;
}

GraphicalModelComponent *GraphicalComponentPort::graphicalComponent() const {
	return _componentGraph;
}

QList<GraphicalConnection*>*GraphicalComponentPort::getConnections() const {
	return _connections;
}
