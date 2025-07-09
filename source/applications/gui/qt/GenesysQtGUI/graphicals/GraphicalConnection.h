#ifndef GRAPHICALCONNECTION_H
#define GRAPHICALCONNECTION_H

#include <QGraphicsObject>

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QPen>
#include <QBrush>
#include "GraphicalComponentPort.h"
//#include "../../../../kernel/simulator/ModelComponent.h"
//#include "../../../../kernel/simulator/Plugin.h"
#include "../../../../kernel/simulator/ConnectionManager.h"

class GraphicalConnection : public QGraphicsObject {
public:
	enum class ConnectionType : int {
		HORIZONTAL = 1, VERTICAL = 2, DIRECT = 3, USERDEFINED = 4
	};

public:
    GraphicalConnection(GraphicalComponentPort* sourceGraphicalPort, GraphicalComponentPort* destinationGraphicalPort, unsigned int portSourceConnection = 0, unsigned int portDestinationConnection = 0, QColor color = Qt::black, QGraphicsItem *parent = nullptr);
	GraphicalConnection(const GraphicalConnection& orig);
	virtual ~GraphicalConnection();
public:
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	Connection* getSource() const;
	Connection* getDestination() const;
    GraphicalComponentPort* getSourceGraphicalPort();
    GraphicalComponentPort* getDestinationGraphicalPort();
	void updateDimensionsAndPosition();
	GraphicalConnection::ConnectionType connectionType() const;
    unsigned int getPortSourceConnection() const;
    unsigned int getPortDestinationConnection() const;
	void setConnectionType(GraphicalConnection::ConnectionType newConnectionType);
    QList<QPointF> getPoints() const;

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
private:
	QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
	//TraitsGUI<GConnection>::foreColor
private:
	qreal _width;
	qreal _height;
	unsigned int _margin = TraitsGUI<GConnection>::margin;//2;
	unsigned int _selWidth = TraitsGUI<GConnection>::selectionWidth;//8;
	ConnectionType _connectionType = ConnectionType::HORIZONTAL;
	Connection* _sourceConnection;
	Connection* _destinationConnection;
	GraphicalComponentPort* _sourceGraphicalPort;
	GraphicalComponentPort* _destinationGraphicalPort;
    unsigned int _portSourceConnection;
    unsigned int _portDestinationConnection;
	QColor _color;
    QList<QPointF> _points;
};
#endif // GRAPHICALCONNECTION_H
