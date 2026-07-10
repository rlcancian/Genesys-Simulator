#ifndef GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H
#define GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H

#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include <memory>

class GraphicalAutomaticPositioningStrategy {
public:
    /**
     * @brief Strategy interface for automatic placement of graphical data definitions.
     *
     * Implementations decide how child graphical items are laid out around an anchor item.
     * The GUI uses this abstraction so layout policy can be selected from preferences without
     * changing the reconstruction code that consumes the strategy.
     */
    virtual ~GraphicalAutomaticPositioningStrategy() = default;

    /** @brief Returns the origin used when laying out the first graphical component. */
    virtual QPointF initialComponentLayoutOrigin() const = 0;

    /**
     * @brief Computes the position of one child item around an anchor rectangle.
     *
     * @param anchorBounds Bounds of the item being decorated.
     * @param childSize Size of the child item to place.
     * @param index Zero-based child index in the current arc.
     * @param count Number of children to place in the same arc.
     * @param upperArc True when the child should be positioned above the anchor.
     * @param radialLayer Additional radial layer for nested attachments.
     * @return Scene position where the child should be placed.
     */
    virtual QPointF dataDefinitionPosition(const QRectF& anchorBounds,
                                           const QSizeF& childSize,
                                           int index,
                                           int count,
                                           bool upperArc,
                                           int radialLayer) const = 0;
};

/**
 * @brief Factory for the automatic positioning strategy selected from GUI preferences.
 */
class GraphicalAutomaticPositioningStrategyFactory {
public:
    /** @brief Creates the preferred automatic positioning strategy instance. */
    static std::unique_ptr<GraphicalAutomaticPositioningStrategy> createFromPreferences();
};

#endif // GRAPHICALAUTOMATICPOSITIONINGSTRATEGY_H
