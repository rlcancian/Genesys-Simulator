#ifndef GRAPHICALCOMPONENTPORT_H
#define GRAPHICALCOMPONENTPORT_H

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QPen>
#include <QBrush>
#include "kernel/simulator/ModelComponent.h"
#include "TraitsGUI.h"
//#include "GraphicalConnection.h"

class GraphicalModelComponent;
class GraphicalConnection;

class GraphicalComponentPort : public QGraphicsObject {
public:
    /**
     * @brief Visual port attached to a graphical component for connection routing.
     *
     * Ports are the connection endpoints used by the scene to attach `GraphicalConnection`
     * items to a component. The port item keeps track of connected edges and exposes enough
     * geometry for the scene to render and route links consistently.
     */
	GraphicalComponentPort(GraphicalModelComponent* componentGraph, bool isInputPort, unsigned int portNum = 0, QGraphicsItem *parent = nullptr);

protected: // virtual functions
	//virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	//virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
	//virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    /** @brief Keeps the port aligned when its parent component moves or is reparented. */
	virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
public: //overrides
    /** @brief Returns the local bounding rectangle used for hit testing and painting. */
	QRectF boundingRect() const override;
    /** @brief Paints the port handle using the component and selection styling. */
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    /** @brief Returns the logical width used by the port geometry. */
	qreal width() const;
    /** @brief Returns the logical height used by the port geometry. */
	qreal height() const;
public:
    /** @brief Registers one connection attached to this port. */
	void addGraphicalConnection(GraphicalConnection* connection);
    /** @brief Unregisters one connection attached to this port. */
	void removeGraphicalConnection(GraphicalConnection* connection);

public: // sets and gets
    /** @brief Returns the port number within the owning component. */
	unsigned int portNum() const;
    /** @brief Returns true when this is an input port. */
	bool isInputPort() const;
    /** @brief Returns the owning graphical component. */
	GraphicalModelComponent *graphicalComponent() const;
    /** @brief Returns the list of connections currently attached to this port. */
	QList<GraphicalConnection*>*getConnections() const;
private:
	QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
private:
	qreal _width = TraitsGUI<GComponentPort>::width;//20;
	qreal _height = _width;
	qreal _margin = TraitsGUI<GComponentPort>::margin; //2
	int _raise = TraitsGUI<GComponentPort>::raise; //3;
	bool _isInputPort;
	unsigned int _portNum;
	GraphicalModelComponent* _componentGraph;
	QList<GraphicalConnection*>* _connections = new QList<GraphicalConnection*>();
};

#endif // GRAPHICALCOMPONENTPORT_H
