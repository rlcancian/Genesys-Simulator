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

/**
 * @brief Graphical edge that represents model-level component connections.
 *
 * `GraphicalConnection` links two `GraphicalComponentPort` objects and keeps
 * auxiliary `Connection` metadata synchronized with the simulator model.
 *
 * @todo Evaluate decoupling model metadata from this item to keep this class purely visual.
 */
class GraphicalConnection : public QGraphicsObject {
public:
	enum class ConnectionType : int {
		HORIZONTAL = 1, VERTICAL = 2, DIRECT = 3, USERDEFINED = 4
	};

public:
    /** @brief Creates a graphical connection between source and destination ports. */
    GraphicalConnection(GraphicalComponentPort* sourceGraphicalPort, GraphicalComponentPort* destinationGraphicalPort, unsigned int portSourceConnection = 0, unsigned int portDestinationConnection = 0, QColor color = Qt::black, QGraphicsItem *parent = nullptr);
	GraphicalConnection(const GraphicalConnection& orig);
	virtual ~GraphicalConnection();
public:
	/** @brief Returns item bounding rectangle for Qt painting system. */
	QRectF boundingRect() const override;
    /** @brief Paints connection path and selection handles. */
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /** @brief Returns auxiliary source Connection metadata. */
	Connection* getSource() const;
    /** @brief Returns auxiliary destination Connection metadata. */
	Connection* getDestination() const;
    /** @brief Returns source graphical port. */
    GraphicalComponentPort* getSourceGraphicalPort();
    /** @brief Returns destination graphical port. */
    GraphicalComponentPort* getDestinationGraphicalPort();
    /** @brief Recomputes local geometry based on current port positions. */
	void updateDimensionsAndPosition();
	/** @brief Returns configured connection routing type. */
	GraphicalConnection::ConnectionType connectionType() const;
    /** @brief Returns source output port index associated with model connection. */
    unsigned int getPortSourceConnection() const;
    /** @brief Returns destination input port index associated with model connection. */
    unsigned int getPortDestinationConnection() const;
	/** @brief Sets routing type used during painting. */
	void setConnectionType(GraphicalConnection::ConnectionType newConnectionType);
    /** @brief Returns sampled scene points used for serialization/inspection. */
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
