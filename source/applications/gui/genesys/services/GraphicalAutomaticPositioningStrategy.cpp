#include "GraphicalAutomaticPositioningStrategy.h"

#include "GraphicalDataDefinitionLayout.h"
#include "../systempreferences.h"
#include "../TraitsGUI.h"

#include <algorithm>

namespace {

class CenteredAutomaticPositioningStrategy final : public GraphicalAutomaticPositioningStrategy {
public:
    QPointF initialComponentLayoutOrigin() const override {
        const qreal center = static_cast<qreal>(TraitsGUI<GView>::sceneCenter);
        const qreal horizontalOffset = static_cast<qreal>(TraitsGUI<GModelComponent>::width) * 2.0;
        const qreal verticalOffset = static_cast<qreal>(TraitsGUI<GModelComponent>::width)
                                     * TraitsGUI<GModelComponent>::heightProportion
                                     * 1.8;
        return QPointF(center - horizontalOffset, center - verticalOffset);
    }

    QPointF dataDefinitionPosition(const QRectF& anchorBounds,
                                   const QSizeF& childSize,
                                   int index,
                                   int count,
                                   bool upperArc,
                                   int radialLayer) const override {
        return GraphicalDataDefinitionLayout::arcPosition(anchorBounds,
                                                          childSize,
                                                          index,
                                                          count,
                                                          upperArc,
                                                          radialLayer);
    }
};

class LegacyAutomaticPositioningStrategy final : public GraphicalAutomaticPositioningStrategy {
public:
    QPointF initialComponentLayoutOrigin() const override {
        const qreal center = static_cast<qreal>(TraitsGUI<GView>::sceneCenter);
        const qreal distance = static_cast<qreal>(TraitsGUI<GView>::sceneDistanceCenter);
        return QPointF(center - distance * 0.8, center - distance * 0.8);
    }

    QPointF dataDefinitionPosition(const QRectF& anchorBounds,
                                   const QSizeF& childSize,
                                   int index,
                                   int count,
                                   bool upperArc,
                                   int radialLayer) const override {
        if (!childSize.isValid() || childSize.isEmpty()) {
            return QPointF();
        }

        const int safeLayer = std::max(0, radialLayer);
        const qreal spacing = std::max<qreal>(childSize.width() + 34.0, 214.0) + safeLayer * 18.0;
        const qreal offset = (static_cast<qreal>(index) - (static_cast<qreal>(count - 1) / 2.0)) * spacing;
        const qreal verticalGap = std::max<qreal>(52.0, childSize.height() * 0.72);
        const qreal radialGap = safeLayer * (childSize.height() + 30.0);

        const qreal x = anchorBounds.center().x() + offset - childSize.width() / 2.0;
        const qreal y = upperArc
                            ? anchorBounds.top() - verticalGap - childSize.height() - radialGap
                            : anchorBounds.bottom() + verticalGap + radialGap;
        return QPointF(x, y);
    }
};

} // namespace

std::unique_ptr<GraphicalAutomaticPositioningStrategy> GraphicalAutomaticPositioningStrategyFactory::createFromPreferences() {
    if (SystemPreferences::automaticPositioningStrategy() == SystemPreferences::AutomaticPositioningStrategy::Legacy) {
        return std::make_unique<LegacyAutomaticPositioningStrategy>();
    }
    return std::make_unique<CenteredAutomaticPositioningStrategy>();
}
