#include "GraphicalDataDefinitionLayout.h"

#include <algorithm>
#include <cmath>

namespace {

qreal normalizedSiblingPosition(int index, int count) {
    if (count <= 1) {
        return 0.0;
    }
    return (2.0 * static_cast<qreal>(index) / static_cast<qreal>(count - 1)) - 1.0;
}

} // namespace

QPointF GraphicalDataDefinitionLayout::arcPosition(const QRectF& anchorBounds,
                                                   const QSizeF& childSize,
                                                   int index,
                                                   int count,
                                                   bool upperArc) {
    if (!childSize.isValid() || childSize.isEmpty()) {
        return QPointF();
    }

    const qreal childWidth = childSize.width();
    const qreal childHeight = childSize.height();
    const qreal spacing = std::max<qreal>(childWidth + 34.0, 214.0);
    const qreal normalized = normalizedSiblingPosition(index, count);
    const qreal horizontalOffset = (static_cast<qreal>(index) - (static_cast<qreal>(count - 1) / 2.0)) * spacing;
    const qreal verticalGap = std::max<qreal>(52.0, childHeight * 0.72);
    const qreal arcLift = std::min<qreal>(60.0, std::abs(normalized) < 0.001 ? 44.0 : 18.0);

    const qreal centerX = anchorBounds.center().x() + horizontalOffset;
    const qreal centerY = upperArc
                              ? anchorBounds.top() - verticalGap - childHeight / 2.0 - arcLift
                              : anchorBounds.bottom() + verticalGap + childHeight / 2.0 + arcLift;

    return QPointF(centerX - childWidth / 2.0, centerY - childHeight / 2.0);
}
