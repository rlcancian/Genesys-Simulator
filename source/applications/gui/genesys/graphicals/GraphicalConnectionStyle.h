#ifndef GRAPHICALCONNECTIONSTYLE_H
#define GRAPHICALCONNECTIONSTYLE_H

#include <QPainterPath>
#include <QPointF>

class GraphicalConnectionStyle {
public:
    enum class RouteType {
        Horizontal,
        Vertical,
        Direct,
        UserDefined
    };

    static bool usesModernCurves();
    static QPainterPath modelConnectionPath(const QPointF& source,
                                            const QPointF& destination,
                                            RouteType routeType,
                                            bool modernCurves);
    static QPainterPath diagramConnectionPath(const QPointF& source,
                                              const QPointF& destination,
                                              bool modernCurves);
    static QPointF pointAtProgress(const QPainterPath& path, qreal progress);

private:
    GraphicalConnectionStyle() = default;
};

#endif // GRAPHICALCONNECTIONSTYLE_H
