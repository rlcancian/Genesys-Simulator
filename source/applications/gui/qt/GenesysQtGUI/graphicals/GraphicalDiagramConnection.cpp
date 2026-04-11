#include "GraphicalDiagramConnection.h"
#include "GraphicalModelDataDefinition.h"
#include "GraphicalModelComponent.h"
#include <QPainter>


GraphicalDiagramConnection::GraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, ConnectionType type) : QGraphicsLineItem() {
    _item1 = dataDefinition;
    _item2 = linkedTo;
    _type = type;

    QRectF startRect = dataDefinition->sceneBoundingRect();
    QRectF endRect = linkedTo->sceneBoundingRect();

    QPointF startPoint;
    QPointF endPoint;

    if (startRect.bottom() < endRect.top()) { //seta para baixo

        startPoint.setX(startRect.center().x());
        startPoint.setY(startRect.bottom() - 10);

        endPoint.setX(endRect.center().x());
        endPoint.setY(endRect.top() + 10);

    } else if (startRect.top() > endRect.bottom()){ //seta para cima

        startPoint.setX(startRect.center().x());
        startPoint.setY(startRect.top() + 10);

        endPoint.setX(endRect.center().x());
        endPoint.setY(endRect.bottom() - 10);

    } else if (startRect.right() < endRect.left()){ //seta para direita

        startPoint.setX(startRect.right() - 10);
        startPoint.setY(startRect.center().y());

        endPoint.setX(endRect.left() + 10);
        endPoint.setY(endRect.center().y());

    } else { //seta para esquerda

        startPoint.setX(startRect.left() + 10);
        startPoint.setY(startRect.center().y());

        endPoint.setX(endRect.right() - 10);
        endPoint.setY(endRect.center().y());
    }

    setLine(QLineF(startPoint, endPoint));
    // Keep diagram links purely representational in this phase.
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptedMouseButtons(Qt::NoButton);
}


void GraphicalDiagramConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Desenha a linha
    QPen pen_line(Qt::black, 1.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    QPen pen_final(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen_line);
    painter->drawLine(line());

    // Desenha a seta na extremidade da linha
    double angle = ::acos(line().dx() / line().length());
    if (line().dy() >= 0)
        angle = (M_PI * 2) - angle;

    double arrowSize = 15.0;

    QPointF arrowP1 = line().p2() - QPointF(sin(angle + M_PI / 3) * arrowSize, cos(angle + M_PI / 3) * arrowSize);
    QPointF arrowP2 = line().p2() - QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize, cos(angle + M_PI - M_PI / 3) * arrowSize);

    QPolygonF arrowHead;
    arrowHead << line().p2() << arrowP1 << arrowP2;

    painter->setPen(pen_final);
    if (_type == ConnectionType::INTERNAL) {
        painter->setBrush(Qt::black);
    }
    painter->drawPolygon(arrowHead);
}


QGraphicsItem* GraphicalDiagramConnection::getDataDefinition() {
    return _item1;
}

QGraphicsItem* GraphicalDiagramConnection::getLinkedDataDefinition() {
    return _item2;
}

GraphicalDiagramConnection::ConnectionType GraphicalDiagramConnection::getConnectionType() {
    return _type;
}
