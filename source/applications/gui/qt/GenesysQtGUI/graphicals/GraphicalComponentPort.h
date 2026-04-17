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
	GraphicalComponentPort(GraphicalModelComponent* componentGraph, bool isInputPort, unsigned int portNum = 0, QGraphicsItem *parent = nullptr);

protected: // virtual functions
	//virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	//virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
	//virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
public: //overrides
	QRectF boundingRect() const override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	qreal width() const;
	qreal height() const;
public:
	void addGraphicalConnection(GraphicalConnection* connection);
	void removeGraphicalConnection(GraphicalConnection* connection);

public: // sets and gets
	unsigned int portNum() const;
	bool isInputPort() const;
	GraphicalModelComponent *graphicalComponent() const;
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
