#ifndef GRAPHICALDATADEFINITIONLAYOUT_H
#define GRAPHICALDATADEFINITIONLAYOUT_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>

/**
 * @brief Pure helper that computes arc placement for graphical data definitions.
 *
 * This helper keeps the geometry math independent from the GUI controllers and scene code so
 * layout policies can be reused in both live rebuilds and persisted-state restoration.
 */
class GraphicalDataDefinitionLayout {
public:
    /**
     * @brief Computes a child position along the upper or lower arc around an anchor item.
     *
     * @param anchorBounds Bounds of the anchor graphical item.
     * @param childSize Size of the child item to place.
     * @param index Zero-based child index.
     * @param count Total number of children in the arc.
     * @param upperArc True for the upper arc, false for the lower arc.
     * @param radialLayer Additional radial offset used for nested attachment rings.
     * @return Scene position for the requested child.
     */
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
