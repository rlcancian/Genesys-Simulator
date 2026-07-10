#include "GraphicalDiagramConnection.h"
#include "GraphicalModelDataDefinition.h"
#include "GraphicalModelComponent.h"
#include <QPainter>
#include <QPainterPathStroker>


GraphicalDiagramConnection::GraphicalDiagramConnection(QGraphicsItem* dataDefinition, QGraphicsItem* linkedTo, ConnectionType type) : QGraphicsLineItem() {
    _item1 = dataDefinition;
    _item2 = linkedTo;
    _type = type;
    refreshGeometry();
    // Keep diagram links purely representational in this phase.
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptedMouseButtons(Qt::NoButton);
}

void GraphicalDiagramConnection::refreshGeometry() {
    if (_item1 == nullptr || _item2 == nullptr) {
        return;
    }
    if (_item1->scene() == nullptr || _item2->scene() == nullptr || _item1->scene() != _item2->scene()) {
        return;
    }
    QRectF startRect = _item1->sceneBoundingRect();
    QRectF endRect = _item2->sceneBoundingRect();

    QPointF startPoint;
    QPointF endPoint;

    if (startRect.bottom() < endRect.top()) { //seta para baixo
        startPoint = QPointF(startRect.center().x(), startRect.bottom() - 10);
        endPoint = QPointF(endRect.center().x(), endRect.top() + 10);
    } else if (startRect.top() > endRect.bottom()){ //seta para cima
        startPoint = QPointF(startRect.center().x(), startRect.top() + 10);
        endPoint = QPointF(endRect.center().x(), endRect.bottom() - 10);
    } else if (startRect.right() < endRect.left()){ //seta para direita
        startPoint = QPointF(startRect.right() - 10, startRect.center().y());
        endPoint = QPointF(endRect.left() + 10, endRect.center().y());
    } else { //seta para esquerda
        startPoint = QPointF(startRect.left() + 10, startRect.center().y());
        endPoint = QPointF(endRect.right() - 10, endRect.center().y());
    }

    setLine(QLineF(startPoint, endPoint));
}

QPainterPath GraphicalDiagramConnection::connectionPath() const {
    return GraphicalConnectionStyle::diagramConnectionPath(line().p1(),
                                                           line().p2(),
                                                           GraphicalConnectionStyle::usesModernCurves());
}

QRectF GraphicalDiagramConnection::boundingRect() const {
    const QRectF pathBounds = connectionPath().boundingRect();
    return pathBounds.adjusted(-18.0, -18.0, 18.0, 18.0);
}

QPainterPath GraphicalDiagramConnection::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(10.0);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(connectionPath());
}

void GraphicalDiagramConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Desenha a linha e o losango na extremidade da linha
    if (qFuzzyIsNull(line().length())) {
        return;
    }

    const bool modernCurve = GraphicalConnectionStyle::usesModernCurves();
    const QPainterPath path = connectionPath();
    const QPointF tip = path.pointAtPercent(1.0);
    const QPointF tangentStart = path.pointAtPercent(0.98);
    const QLineF tangent(tangentStart, tip);
    if (qFuzzyIsNull(tangent.length())) {
        return;
    }

    double angle = ::acos(tangent.dx() / tangent.length());
    if (tangent.dy() >= 0) {
        angle = (M_PI * 2) - angle;
    }

    const double diamondLength = 16.0;
    const double diamondHalfWidth = 7.0;

    const QPointF back = tip - QPointF(::cos(angle) * diamondLength, -::sin(angle) * diamondLength);
    const QPointF center = tip - QPointF(::cos(angle) * diamondLength / 2.0, -::sin(angle) * diamondLength / 2.0);
    const QPointF perpendicular(::sin(angle) * diamondHalfWidth, ::cos(angle) * diamondHalfWidth);
    const QPointF side1 = center + perpendicular;
    const QPointF side2 = center - perpendicular;

    const QColor connectionColor = modernCurve ? QColor(47, 128, 237, 210) : QColor(Qt::black);
    QPen pen_line(connectionColor, modernCurve ? 2.2 : 1.5, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    QPen pen_final(connectionColor, modernCurve ? 2.0 : 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setRenderHint(QPainter::Antialiasing, modernCurve);
    painter->setPen(pen_line);
    if (modernCurve) {
        QPainterPath bodyPath = path;
        const qreal backPercent = path.percentAtLength(qMax<qreal>(0.0, path.length() - diamondLength));
        bodyPath.setElementPositionAt(bodyPath.elementCount() - 1,
                                      path.pointAtPercent(backPercent).x(),
                                      path.pointAtPercent(backPercent).y());
        painter->drawPath(bodyPath);
    } else {
        painter->drawLine(QLineF(line().p1(), back));
    }

    QPolygonF diamond;
    diamond << tip << side1 << back << side2;

    painter->setPen(pen_final);
    if (_type == ConnectionType::INTERNAL) {
        painter->setBrush(connectionColor);
    } else {
        painter->setBrush(Qt::NoBrush);
    }
    painter->drawPolygon(diamond);
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
