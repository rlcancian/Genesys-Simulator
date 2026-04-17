#include "GraphicalModelItemRenderStrategy.h"

#include "systempreferences.h"

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QRadialGradient>

namespace {

QColor colorFromRgba(uint64_t color) {
    const uint8_t r = (color & 0xFF000000) >> 24;
    const uint8_t g = (color & 0x00FF0000) >> 16;
    const uint8_t b = (color & 0x0000FF00) >> 8;
    const uint8_t a = (color & 0x000000FF);
    return QColor(r, g, b, a);
}

QRectF innerRect(const GraphicalModelItemRenderContext& context) {
    return QRectF(
        context.margin,
        context.margin,
        context.width - 2 * context.margin - context.penWidth,
        context.height - 2 * context.margin - context.penWidth);
}

void drawSelectionHandles(QPainter* painter, const GraphicalModelItemRenderContext& context, bool organic) {
    if (!context.selected) {
        return;
    }

    QBrush brush(Qt::SolidPattern);
    brush.setColor(colorFromRgba(context.selectionColor));
    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);

    const qreal size = context.selectionWidth;
    const QRectF handles[] = {
        QRectF(0, 0, size, size),
        QRectF(context.width - size, 0, size, size),
        QRectF(0, context.height - size, size, size),
        QRectF(context.width - size, context.height - size, size, size)
    };

    for (const QRectF& handle : handles) {
        if (organic) {
            painter->drawEllipse(handle);
        } else {
            painter->drawRect(handle);
        }
    }
}

void drawCenteredTextLine(QPainter* painter,
                          const QRectF& rect,
                          const QString& text,
                          const QColor& textColor,
                          const QColor& shadowColor,
                          bool shadowFirst) {
    QPen pen(textColor);
    pen.setWidth(2);
    pen.setCosmetic(true);

    if (shadowFirst) {
        painter->setPen(QPen(shadowColor, 2));
        painter->drawText(rect.translated(0, 2), Qt::AlignCenter, text);
        painter->setPen(pen);
        painter->drawText(rect, Qt::AlignCenter, text);
        return;
    }

    painter->setPen(pen);
    painter->drawText(rect, Qt::AlignCenter, text);
    painter->setPen(QPen(shadowColor, 2));
    painter->drawText(rect.adjusted(0, 2, 2, 0), Qt::AlignCenter, text);
}

void drawItemText(QPainter* painter,
                  const GraphicalModelItemRenderContext& context,
                  const QRectF& contentRect,
                  bool shadowFirstForSingleLine) {
    painter->setBrush(Qt::NoBrush);
    const QColor textColor = colorFromRgba(context.textColor);
    const QColor shadowColor = colorFromRgba(context.textShadowColor);

    if (context.secondaryText.isEmpty()) {
        drawCenteredTextLine(painter, contentRect, context.primaryText, textColor, shadowColor, shadowFirstForSingleLine);
        return;
    }

    const QFontMetrics fm = painter->fontMetrics();
    const int lineHeight = fm.height();
    const int lineSpacing = 3;
    const int totalHeight = lineHeight * 2 + lineSpacing;
    const qreal blockTop = contentRect.top() + (contentRect.height() - totalHeight) / 2.0;
    const QRectF line1Rect(contentRect.left(), blockTop, contentRect.width(), lineHeight);
    const QRectF line2Rect(contentRect.left(), blockTop + lineHeight + lineSpacing, contentRect.width(), lineHeight);

    drawCenteredTextLine(painter, line1Rect, context.primaryText, textColor, shadowColor, true);
    drawCenteredTextLine(painter, line2Rect, context.secondaryText, textColor, shadowColor, true);
}

class ClassicGraphicalModelItemRenderStrategy final : public GraphicalModelItemRenderStrategy {
public:
    const char* name() const override {
        return "classic";
    }

    QPainterPath shape(const GraphicalModelItemRenderContext& context) const override {
        QPainterPath path;
        path.addRect(context.bounds);
        return path;
    }

    void paint(QPainter* painter, const GraphicalModelItemRenderContext& context) const override {
        painter->save();

        const int wi = context.width - 2 * context.margin - context.penWidth;
        const int hi = context.height - 2 * context.margin - context.penWidth;
        const int wt2 = context.width * context.stretchPosTop;
        const int wb2 = context.width * context.stretchPosBottom;
        const int hl2 = context.height * context.stretchPosLeft;
        const int hr2 = context.height * context.stretchPosRight;
        const qreal sfr = hi * context.stretchRight;
        const qreal sfl = hi * context.stretchLeft;
        const qreal sfrm = hi * context.stretchRightMiddle;
        const qreal sflm = hi * context.stretchLeftMiddle;
        const qreal sft = wi * context.stretchTop;
        const qreal sfb = wi * context.stretchBottom;
        const qreal sftm = wi * context.stretchTopMiddle;
        const qreal sfbm = wi * context.stretchBottomMiddle;
        const int shiftt = (context.stretchRight == 0 && context.stretchLeft == 0)
                           || (context.stretchRight > 0 && context.stretchLeft > 0)
                               ? 0
                               : (context.stretchRight > 0 ? context.raise : -context.raise);
        const int shiftb = shiftt;
        const int shiftr = (context.stretchTop == 0 && context.stretchBottom == 0)
                           || (context.stretchTop > 0 && context.stretchBottom > 0)
                               ? 0
                               : (context.stretchBottom > 0 ? context.raise : -context.raise);
        const int shiftl = shiftr;

        const QPointF pp1(context.margin + sft, context.margin + sfl);
        const QPointF pp2(context.margin + sflm, hl2 + shiftl);
        const QPointF pp3(context.margin + sfb, context.margin + hi - sfl);
        const QPointF pp4(context.margin + context.raise + sfb, context.margin + hi - context.raise - sfl);
        const QPointF pp5(context.margin + context.raise + sflm, hl2);
        const QPointF pp6(context.margin + context.raise + sft, context.margin + context.raise + sfl);
        const QPointF pp7(wt2, context.margin + context.raise + sftm);
        const QPointF pp8(context.margin + wi - context.raise - sft, context.margin + context.raise + sfr);
        const QPointF pp9(context.margin + wi - sft, context.margin + sfr);
        const QPointF pp10(wt2 + shiftt, context.margin + sftm);
        const QPointF pp11(context.margin + wi - sfb, context.margin + hi - sfr);
        const QPointF pp12(context.margin + wi - sfrm, hr2 + shiftr);
        const QPointF pp13(context.margin + wi - context.raise - sfrm, hr2);
        const QPointF pp14(context.margin + wi - context.raise - sfb, context.margin + hi - context.raise - sfr);
        const QPointF pp15(wb2, context.margin + hi - context.raise - sfbm);
        const QPointF pp16(wb2 + shiftb, context.margin + hi - sfbm);

        QPen pen(colorFromRgba(context.borderColor));
        pen.setWidth(context.penWidth);
        painter->setPen(pen);

        QBrush brush(Qt::SolidPattern);
        brush.setColor(colorFromRgba(context.raisedColor));
        painter->setBrush(brush);
        QPainterPath raisedPath;
        raisedPath.moveTo(pp1);
        raisedPath.lineTo(pp2);
        if (context.stretchTop > 0 || context.stretchBottom > 0) {
            raisedPath.lineTo(pp5);
            raisedPath.lineTo(pp2);
        }
        raisedPath.lineTo(pp3);
        raisedPath.lineTo(pp4);
        raisedPath.lineTo(pp5);
        raisedPath.lineTo(pp6);
        raisedPath.lineTo(pp7);
        raisedPath.lineTo(pp8);
        raisedPath.lineTo(pp9);
        raisedPath.lineTo(pp10);
        if (context.stretchRight > 0 || context.stretchLeft > 0) {
            raisedPath.lineTo(pp7);
            raisedPath.lineTo(pp10);
        }
        raisedPath.lineTo(pp1);
        raisedPath.lineTo(pp6);
        painter->drawPath(raisedPath);

        brush.setColor(colorFromRgba(context.sunkenColor));
        painter->setBrush(brush);
        QPainterPath sunkenPath;
        sunkenPath.moveTo(pp11);
        sunkenPath.lineTo(pp12);
        sunkenPath.lineTo(pp9);
        sunkenPath.lineTo(pp8);
        sunkenPath.lineTo(pp13);
        if (context.stretchTop > 0 || context.stretchBottom > 0) {
            sunkenPath.lineTo(pp12);
            sunkenPath.lineTo(pp13);
        }
        sunkenPath.lineTo(pp14);
        sunkenPath.lineTo(pp15);
        if (context.stretchRight > 0 || context.stretchLeft > 0) {
            sunkenPath.lineTo(pp16);
            sunkenPath.lineTo(pp15);
        }
        sunkenPath.lineTo(pp4);
        sunkenPath.lineTo(pp3);
        sunkenPath.lineTo(pp16);
        sunkenPath.lineTo(pp11);
        sunkenPath.lineTo(pp14);
        painter->drawPath(sunkenPath);

        QLinearGradient gradient(0, 0, context.width, context.height);
        gradient.setColorAt(0.0, Qt::white);
        gradient.setColorAt(0.33, context.fillColor.lighter());
        gradient.setColorAt(0.67, context.fillColor);
        gradient.setColorAt(1.0, context.fillColor.darker());
        painter->setBrush(QBrush(gradient));
        QPainterPath fillPath;
        fillPath.moveTo(pp6);
        fillPath.lineTo(pp7);
        fillPath.lineTo(pp8);
        fillPath.lineTo(pp13);
        fillPath.lineTo(pp14);
        fillPath.lineTo(pp15);
        fillPath.lineTo(pp4);
        fillPath.lineTo(pp5);
        fillPath.lineTo(pp6);
        painter->drawPath(fillPath);

        const QRectF textRect(
            context.margin + context.raise + 1,
            context.margin + context.raise + 1,
            context.kind == GraphicalModelItemRenderContext::ItemKind::Component
                ? context.margin + wi - 2 * context.raise - context.margin
                : wi - 2 * context.raise - 2,
            context.kind == GraphicalModelItemRenderContext::ItemKind::Component
                ? context.margin + hi - 2 * context.raise - context.margin
                : hi - 2 * context.raise - 2);
        drawItemText(painter, context, textRect, context.kind != GraphicalModelItemRenderContext::ItemKind::Component);

        drawSelectionHandles(painter, context, false);

        if (context.breakpoint) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(colorFromRgba(context.breakpointColor), 1));
            painter->drawRect(QRectF(0, 0, context.width, context.height));
        }

        painter->restore();
    }
};

class OrganicGraphicalModelItemRenderStrategy final : public GraphicalModelItemRenderStrategy {
public:
    const char* name() const override {
        return "organic";
    }

    QPainterPath shape(const GraphicalModelItemRenderContext& context) const override {
        QPainterPath path;
        if (context.kind == GraphicalModelItemRenderContext::ItemKind::Component) {
            const QRectF body = bodyRect(context);
            path.addRoundedRect(body, body.height() / 2.0, body.height() / 2.0);
        } else {
            path.addEllipse(bodyRect(context));
        }
        return path;
    }

    void paint(QPainter* painter, const GraphicalModelItemRenderContext& context) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QRectF body = bodyRect(context);
        const QPainterPath bodyPath = shape(context);
        const QColor border = colorFromRgba(context.borderColor);
        QColor accent = context.kind == GraphicalModelItemRenderContext::ItemKind::Component
                            ? QColor(47, 128, 237)
                            : QColor(16, 185, 129);
        accent.setAlpha(context.selected ? 230 : 180);
        QColor shadow = border;
        shadow.setAlpha(context.selected ? 120 : 60);

        painter->setPen(Qt::NoPen);
        painter->setBrush(shadow);
        painter->drawPath(bodyPath.translated(0, context.selected ? 3 : 2));

        QRadialGradient gradient(body.center() - QPointF(body.width() * 0.16, body.height() * 0.22),
                                 qMax(body.width(), body.height()) * 0.72);
        gradient.setColorAt(0.0, context.fillColor.lighter(170));
        gradient.setColorAt(0.58, context.fillColor);
        gradient.setColorAt(1.0, context.fillColor.darker(135));
        painter->setBrush(QBrush(gradient));

        QPen bodyPen(accent);
        bodyPen.setWidth(context.selected ? context.penWidth + 3 : context.penWidth + 1);
        bodyPen.setCosmetic(true);
        painter->setPen(bodyPen);
        painter->drawPath(bodyPath);

        QPainterPath highlight;
        const QRectF highlightRect = body.adjusted(body.width() * 0.13,
                                                   body.height() * 0.13,
                                                   -body.width() * 0.38,
                                                   -body.height() * 0.54);
        highlight.addEllipse(highlightRect);
        QColor highlightColor = colorFromRgba(context.raisedColor);
        highlightColor.setAlpha(context.kind == GraphicalModelItemRenderContext::ItemKind::Component ? 120 : 90);
        painter->setPen(Qt::NoPen);
        painter->setBrush(highlightColor);
        painter->drawPath(highlight);

        QPainterPath accentPath;
        if (context.kind == GraphicalModelItemRenderContext::ItemKind::Component) {
            const QRectF accentRect = body.adjusted(body.width() * 0.08,
                                                    body.height() * 0.18,
                                                    -body.width() * 0.58,
                                                    -body.height() * 0.62);
            accentPath.addRoundedRect(accentRect, accentRect.height() / 2.0, accentRect.height() / 2.0);
        } else {
            const QRectF accentRect = body.adjusted(body.width() * 0.15,
                                                    body.height() * 0.16,
                                                    -body.width() * 0.15,
                                                    -body.height() * 0.16);
            accentPath.addEllipse(accentRect);
        }
        QPen accentPen(accent.lighter(135), 2);
        accentPen.setCosmetic(true);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(accentPen);
        painter->drawPath(accentPath);

        const QRectF textRect = body.adjusted(context.raise + 2,
                                              context.raise + 2,
                                              -(context.raise + 2),
                                              -(context.raise + 2));
        drawItemText(painter, context, textRect, true);

        drawSelectionHandles(painter, context, true);

        if (context.breakpoint) {
            painter->setBrush(Qt::NoBrush);
            QPen breakpointPen(colorFromRgba(context.breakpointColor), 2);
            breakpointPen.setCosmetic(true);
            painter->setPen(breakpointPen);
            painter->drawPath(bodyPath);
        }

        painter->restore();
    }

private:
    static QRectF bodyRect(const GraphicalModelItemRenderContext& context) {
        QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
        if (context.kind == GraphicalModelItemRenderContext::ItemKind::Component) {
            return body.adjusted(-context.raise, 0, context.raise, 0);
        }
        return body;
    }
};

const GraphicalModelItemRenderStrategy& classicStrategy() {
    static const ClassicGraphicalModelItemRenderStrategy strategy;
    return strategy;
}

const GraphicalModelItemRenderStrategy& organicStrategy() {
    static const OrganicGraphicalModelItemRenderStrategy strategy;
    return strategy;
}

} // namespace

void GraphicalModelItemRenderer::paint(QPainter* painter, const GraphicalModelItemRenderContext& context) {
    currentStrategy().paint(painter, context);
}

QPainterPath GraphicalModelItemRenderer::shape(const GraphicalModelItemRenderContext& context) {
    return currentStrategy().shape(context);
}

const GraphicalModelItemRenderStrategy& GraphicalModelItemRenderer::currentStrategy() {
    return SystemPreferences::interfaceStyle() == SystemPreferences::InterfaceStyle::Modern
               ? organicStrategy()
               : classicStrategy();
}
