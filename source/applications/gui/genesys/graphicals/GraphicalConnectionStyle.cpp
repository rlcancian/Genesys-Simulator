#include "GraphicalConnectionStyle.h"

#include "systempreferences.h"

#include <QLineF>

#include <algorithm>
#include <cmath>

bool GraphicalConnectionStyle::usesModernCurves() {
    return SystemPreferences::interfaceStyle() == SystemPreferences::InterfaceStyle::Modern;
}

QPainterPath GraphicalConnectionStyle::modelConnectionPath(const QPointF& source,
                                                           const QPointF& destination,
                                                           RouteType routeType,
                                                           bool modernCurves) {
    QPainterPath path;
    path.moveTo(source);

    if (modernCurves) {
        const QLineF line(source, destination);
        if (line.length() <= 0.0) {
            return path;
        }

        const qreal dx = destination.x() - source.x();
        const qreal dy = destination.y() - source.y();
        const qreal horizontalDirection = dx >= 0.0 ? 1.0 : -1.0;
        const qreal verticalDirection = dy >= 0.0 ? 1.0 : -1.0;
        const qreal horizontalDistance = std::abs(dx);
        const qreal verticalDistance = std::abs(dy);
        const qreal controlDistance = std::clamp(std::max(horizontalDistance, verticalDistance) * 0.45, 60.0, 180.0);
        const QPointF normal(-line.dy() / line.length(), line.dx() / line.length());
        const qreal bend = std::clamp(line.length() * 0.20, 24.0, 90.0);
        const QPointF curveBend = normal * bend;

        QPointF control1;
        QPointF control2;
        if (horizontalDistance >= verticalDistance) {
            control1 = QPointF(source.x() + horizontalDirection * controlDistance, source.y()) + curveBend;
            control2 = QPointF(destination.x() - horizontalDirection * controlDistance, destination.y()) + curveBend;
        } else {
            control1 = QPointF(source.x(), source.y() + verticalDirection * controlDistance) + curveBend;
            control2 = QPointF(destination.x(), destination.y() - verticalDirection * controlDistance) + curveBend;
        }
        path.cubicTo(control1, control2, destination);
        return path;
    }

    const qreal x1 = source.x();
    const qreal y1 = source.y();
    const qreal x2 = destination.x();
    const qreal y2 = destination.y();
    switch (routeType) {
    case RouteType::Horizontal:
        path.lineTo((x1 + x2) / 2.0, y1);
        path.lineTo((x1 + x2) / 2.0, y2);
        break;
    case RouteType::Vertical:
        path.lineTo(x1, (y1 + y2) / 2.0);
        path.lineTo(x2, (y1 + y2) / 2.0);
        break;
    case RouteType::Direct:
        break;
    case RouteType::UserDefined:
        break;
    }
    path.lineTo(destination);
    return path;
}

QPainterPath GraphicalConnectionStyle::diagramConnectionPath(const QPointF& source,
                                                             const QPointF& destination,
                                                             bool modernCurves) {
    QPainterPath path;
    path.moveTo(source);
    if (!modernCurves) {
        path.lineTo(destination);
        return path;
    }

    const QLineF line(source, destination);
    if (line.length() <= 0.0) {
        return path;
    }

    const QPointF midpoint = (source + destination) / 2.0;
    const QPointF normal(-line.dy() / line.length(), line.dx() / line.length());
    const qreal bend = std::clamp(line.length() * 0.22, 24.0, 90.0);
    path.quadTo(midpoint + normal * bend, destination);
    return path;
}

QPointF GraphicalConnectionStyle::pointAtProgress(const QPainterPath& path, qreal progress) {
    const qreal boundedProgress = std::clamp(progress, 0.0, 1.0);
    const qreal length = path.length();
    if (length <= 0.0) {
        return path.pointAtPercent(boundedProgress);
    }
    return path.pointAtPercent(path.percentAtLength(length * boundedProgress));
}
