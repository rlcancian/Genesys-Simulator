#ifndef GRAPHICALDIAGRAMCONNECTION_H
#define GRAPHICALDIAGRAMCONNECTION_H

#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <cmath>
#include <QGraphicsLineItem>
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/ModelDataDefinition.h"

#include "GraphicalModelComponent.h"
#include "GraphicalModelDataDefinition.h"
#include "TraitsGUI.h"
#include "UtilGUI.h"

class GraphicalDiagramConnection : public QGraphicsLineItem {
public:
    enum class ConnectionType{
        ATTACHED, INTERNAL
    };

public:
    GraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, ConnectionType type);
    void refreshGeometry();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    QGraphicsItem* getDataDefinition();
    QGraphicsItem* getLinkedDataDefinition();
    ConnectionType getConnectionType();

private:
    ConnectionType _type;
    QGraphicsItem* _item1;
    QGraphicsItem* _item2;
};


#endif // GRAPHICALDIAGRAMCONNECTION_H
