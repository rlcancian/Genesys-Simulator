#ifndef GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H
#define GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <memory>

class GraphicalAutomaticPositioningStrategy {
public:
    virtual ~GraphicalAutomaticPositioningStrategy() = default;

    virtual QPointF initialComponentLayoutOrigin() const = 0;

    virtual QPointF dataDefinitionPosition(const QRectF& anchorBounds,
                                           const QSizeF& childSize,
                                           int index,
                                           int count,
                                           bool upperArc,
                                           int radialLayer) const = 0;
};

class GraphicalAutomaticPositioningStrategyFactory {
public:
    static std::unique_ptr<GraphicalAutomaticPositioningStrategy> createFromPreferences();
};

#endif // GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H
