#ifndef GRAPHICALDATADEFINITIONLAYOUT_H
#define GRAPHICALDATADEFINITIONLAYOUT_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>

class GraphicalDataDefinitionLayout {
public:
    static QPointF arcPosition(const QRectF& anchorBounds,
                               const QSizeF& childSize,
                               int index,
                               int count,
                               bool upperArc,
                               int radialLayer = 0);

private:
    GraphicalDataDefinitionLayout() = default;
};

#endif // GRAPHICALDATADEFINITIONLAYOUT_H
