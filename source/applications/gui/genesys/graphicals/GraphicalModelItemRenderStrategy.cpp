#include "GraphicalModelItemRenderStrategy.h"

#include "systempreferences.h"

#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QRadialGradient>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

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

QStringList wrapTextToWidth(const QString& text, const QFontMetrics& fm, qreal maxWidth) {
    QStringList lines;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return lines;
    }

    const QStringList paragraphs = text.split('\n', Qt::KeepEmptyParts);
    for (const QString& paragraphText : paragraphs) {
        const QString paragraph = paragraphText.trimmed();
        if (paragraph.isEmpty()) {
            // Preserve explicit blank lines so primary and secondary text stay vertically separated.
            lines << QString();
            continue;
        }

        const QStringList words = paragraph.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        QString currentLine;
        auto flushCurrentLine = [&]() {
            if (!currentLine.isEmpty()) {
                lines << currentLine;
                currentLine.clear();
            }
        };

        for (const QString& word : words) {
            if (word.isEmpty()) {
                continue;
            }

            const QString candidate = currentLine.isEmpty() ? word : currentLine + " " + word;
            if (fm.horizontalAdvance(candidate) <= maxWidth) {
                currentLine = candidate;
                continue;
            }

            flushCurrentLine();

            if (fm.horizontalAdvance(word) <= maxWidth) {
                currentLine = word;
                continue;
            }

            QString chunk;
            for (const QChar ch : word) {
                const QString chunkCandidate = chunk + ch;
                if (!chunk.isEmpty() && fm.horizontalAdvance(chunkCandidate) > maxWidth) {
                    lines << chunk;
                    chunk = ch;
                } else {
                    chunk = chunkCandidate;
                }
            }
            if (!chunk.isEmpty()) {
                currentLine = chunk;
            }
        }

        flushCurrentLine();
    }
    return lines;
}

void drawWrappedText(QPainter* painter,
                     const GraphicalModelItemRenderContext& context,
                     const QRectF& contentRect,
                     const QString& text,
                     bool alignLeft,
                     bool alignTop,
                     bool shadowFirst) {
    const QColor textColor = colorFromRgba(context.textColor);
    const QColor shadowColor = colorFromRgba(context.textShadowColor);
    const QFontMetrics fm = painter->fontMetrics();
    const qreal maxWidth = qMax<qreal>(1.0, contentRect.width()); // contentRect.width()*0.9
    QStringList lines = wrapTextToWidth(text, fm, maxWidth);
    if (lines.isEmpty()) {
        return;
    }

    const int lineHeight = fm.height();
    const int lineSpacing = 2; // 2
    const int totalHeight = lines.size() * lineHeight + qMax(0, lines.size() - 1) * lineSpacing;
    const qreal top = alignTop
                          ? contentRect.top()
                          : contentRect.top() + (contentRect.height() - totalHeight) / 2.0;
    const qreal left = alignLeft ? contentRect.left() : contentRect.left();

    QPen pen(textColor);
    pen.setWidth(2);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    qreal y = top;
    for (const QString& line : lines) {
        QRectF lineRect(left, y, contentRect.width(), lineHeight);
        if (shadowFirst) {
            painter->setPen(QPen(shadowColor, 2));
            painter->drawText(lineRect.translated(0, 2), alignLeft ? Qt::AlignLeft : Qt::AlignHCenter, line);
            painter->setPen(pen);
            painter->drawText(lineRect, alignLeft ? Qt::AlignLeft : Qt::AlignHCenter, line);
        } else {
            painter->drawText(lineRect, alignLeft ? Qt::AlignLeft : Qt::AlignHCenter, line);
            painter->setPen(QPen(shadowColor, 2));
            painter->drawText(lineRect.adjusted(0, 2, 2, 0), alignLeft ? Qt::AlignLeft : Qt::AlignHCenter, line);
            painter->setPen(pen);
        }
        y += lineHeight + lineSpacing;
    }
}

void drawItemText(QPainter* painter,
                  const GraphicalModelItemRenderContext& context,
                  const QRectF& contentRect,
                  bool shadowFirstForSingleLine) {
    const QString primaryText = context.primaryText.trimmed();
    const QString secondaryText = context.secondaryText.trimmed();
    const QString tertiaryText = context.tertiaryText.trimmed();
    if (primaryText.isEmpty() && secondaryText.isEmpty() && tertiaryText.isEmpty()) {
        return;
    }

    const QColor textColor = colorFromRgba(context.textColor);
    const QColor shadowColor = colorFromRgba(context.textShadowColor);
    const qreal maxWidth = qMax<qreal>(1.0, contentRect.width());
    const qreal left = contentRect.left();

    QFont baseFont = painter->font();
    QFont primaryFont = baseFont;
    primaryFont.setBold(true);
    QFont secondaryFont = baseFont;
    if (secondaryFont.pointSizeF() > 0.0) {
        secondaryFont.setPointSizeF(qMax<qreal>(1.0, secondaryFont.pointSizeF() - 1.0));
    } else if (secondaryFont.pixelSize() > 0) {
        secondaryFont.setPixelSize(qMax(1, secondaryFont.pixelSize() - 1));
    }

    QFontMetrics primaryFm(primaryFont);
    QFontMetrics secondaryFm(secondaryFont);
    const QStringList secondaryLines = secondaryText.isEmpty() ? QStringList() : wrapTextToWidth(secondaryText, secondaryFm, maxWidth);
    const QStringList tertiaryLines = tertiaryText.isEmpty() ? QStringList() : wrapTextToWidth(tertiaryText, secondaryFm, maxWidth);
    const int secondaryLineSpacing = secondaryLines.size() >= 4 ? 0 : 1;

    struct TextBlock {
        QStringList lines;
        QFont font;
        Qt::Alignment alignment;
        int lineSpacing = 0;
        int penWidth = 1;
    };

    const QVector<TextBlock> blocks = {
        {QStringList{primaryText}, primaryFont, Qt::AlignHCenter, 0, 2},
        {secondaryLines, secondaryFont, Qt::AlignLeft, secondaryLineSpacing, 1},
        {tertiaryLines, secondaryFont, Qt::AlignRight, 1, 1}
    };

    struct MeasuredBlock {
        QStringList lines;
        QFont font;
        Qt::Alignment alignment;
        int lineSpacing = 0;
        int penWidth = 1;
        int height = 0;
        bool isEmpty() const {
            return lines.isEmpty() || (lines.size() == 1 && lines.first().trimmed().isEmpty());
        }
    };

    QVector<MeasuredBlock> measuredBlocks;
    measuredBlocks.reserve(blocks.size());
    int totalHeight = 0;
    const int blockSpacing = 1;
    for (const TextBlock& block : blocks) {
        MeasuredBlock measured;
        measured.lines = block.lines;
        measured.font = block.font;
        measured.alignment = block.alignment;
        measured.lineSpacing = block.lineSpacing;
        measured.penWidth = block.penWidth;
        if (!measured.isEmpty()) {
            const QFontMetrics fm(measured.font);
            measured.height = measured.lines.size() * fm.height()
                              + qMax(0, measured.lines.size() - 1) * measured.lineSpacing;
            totalHeight += measured.height;
            measuredBlocks.append(measured);
        }
    }

    if (measuredBlocks.isEmpty()) {
        return;
    }

    totalHeight += qMax(0, measuredBlocks.size() - 1) * blockSpacing;
    const qreal top = contentRect.top() + (contentRect.height() - totalHeight) / 2.0;

    qreal y = top;
    for (int blockIndex = 0; blockIndex < measuredBlocks.size(); ++blockIndex) {
        const MeasuredBlock& block = measuredBlocks.at(blockIndex);
        const QFontMetrics fm(block.font);
        const int lineHeight = fm.height();
        painter->setFont(block.font);

        QPen pen(textColor);
        pen.setWidth(block.penWidth);
        pen.setCosmetic(true);

        qreal lineY = y;
        for (const QString& line : block.lines) {
            const QRectF lineRect(left, lineY, contentRect.width(), lineHeight);
            if (line.trimmed().isEmpty()) {
                lineY += lineHeight + block.lineSpacing;
                continue;
            }

            if (shadowFirstForSingleLine) {
                painter->setPen(QPen(shadowColor, block.penWidth));
                painter->drawText(lineRect.translated(0, 2), block.alignment | Qt::AlignVCenter | Qt::TextSingleLine, line);
                painter->setPen(pen);
                painter->drawText(lineRect, block.alignment | Qt::AlignVCenter | Qt::TextSingleLine, line);
            } else {
                painter->setPen(pen);
                painter->drawText(lineRect, block.alignment | Qt::AlignVCenter | Qt::TextSingleLine, line);
                painter->setPen(QPen(shadowColor, block.penWidth));
                painter->drawText(lineRect.adjusted(0, 2, 2, 0), block.alignment | Qt::AlignVCenter | Qt::TextSingleLine, line);
            }
            lineY += lineHeight + block.lineSpacing;
        }

        y += block.height + blockSpacing;
    }
}

bool isGeneticCircuitPartContext(const GraphicalModelItemRenderContext& context) {
    return context.semanticClassName == QStringLiteral("GeneticCircuitPart");
}

bool isGeneticRegulationContext(const GraphicalModelItemRenderContext& context) {
    return context.semanticClassName == QStringLiteral("GeneticRegulation");
}

QString geneticCircuitPartRole(const GraphicalModelItemRenderContext& context) {
    const QString source = context.semanticSubtype.isEmpty() ? context.primaryText : context.semanticSubtype;
    const QString lower = source.toLower();

    if (lower.contains("promoter") || lower.contains("pre-gene regulatory")) {
        return QStringLiteral("Pre-gene regulatory");
    }
    if (lower.contains("regulatory") || lower.contains("operator")) {
        return QStringLiteral("Regulatory");
    }
    if (lower.contains("rbs") || lower.contains("translation initiation")) {
        return QStringLiteral("Translation initiation");
    }
    if (lower.contains("cds") || lower.contains("coding") || lower.contains("gene")) {
        return QStringLiteral("Coding");
    }
    if (lower.contains("terminator") || lower.contains("termination")) {
        return QStringLiteral("Termination");
    }
    if (lower.contains("region") || lower.contains("pre-gene")) {
        return QStringLiteral("Pre-gene region");
    }
    return QStringLiteral("Other");
}

QString geneticRegulationRole(const GraphicalModelItemRenderContext& context) {
    const QString source = context.semanticSubtype.isEmpty() ? context.primaryText : context.semanticSubtype;
    const QString lower = source.toLower();

    if (lower.contains("activation")) {
        return QStringLiteral("Activation");
    }
    if (lower.contains("repression")) {
        return QStringLiteral("Repression");
    }
    if (lower.contains("dual")) {
        return QStringLiteral("Dual");
    }
    return QStringLiteral("Regulation");
}

QColor geneticCircuitPartAccentColor(const QString& role) {
    if (role == QStringLiteral("Pre-gene regulatory")) {
        return QColor(64, 170, 120);
    }
    if (role == QStringLiteral("Regulatory")) {
        return QColor(150, 92, 194);
    }
    if (role == QStringLiteral("Translation initiation")) {
        return QColor(229, 158, 44);
    }
    if (role == QStringLiteral("Coding")) {
        return QColor(52, 152, 219);
    }
    if (role == QStringLiteral("Termination")) {
        return QColor(214, 82, 102);
    }
    if (role == QStringLiteral("Pre-gene region")) {
        return QColor(119, 136, 153);
    }
    return QColor(110, 110, 110);
}

QColor geneticRegulationAccentColor(const QString& role) {
    if (role == QStringLiteral("Activation")) {
        return QColor(64, 170, 120);
    }
    if (role == QStringLiteral("Repression")) {
        return QColor(214, 82, 102);
    }
    if (role == QStringLiteral("Dual")) {
        return QColor(229, 158, 44);
    }
    return QColor(110, 110, 110);
}

QPainterPath geneticCircuitPartShape(const GraphicalModelItemRenderContext& context) {
    const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
    const QString role = geneticCircuitPartRole(context);
    const qreal left = body.left();
    const qreal right = body.right();
    const qreal top = body.top();
    const qreal bottom = body.bottom();
    const qreal centerY = body.center().y();
    const qreal insetX = qMax<qreal>(6.0, body.width() * 0.12);
    const qreal headX = qMax<qreal>(10.0, body.width() * 0.18);
    const qreal midBand = qMax<qreal>(4.0, body.height() * 0.12);

    QPainterPath path;
    if (role == QStringLiteral("Regulatory")) {
        path.moveTo(body.center().x(), top);
        path.lineTo(right, centerY);
        path.lineTo(body.center().x(), bottom);
        path.lineTo(left, centerY);
        path.closeSubpath();
        return path;
    }

    if (role == QStringLiteral("Pre-gene region")) {
        path.addRoundedRect(body.adjusted(insetX * 0.5, 0.0, -insetX * 0.5, 0.0), body.height() / 3.0, body.height() / 3.0);
        return path;
    }

    if (role == QStringLiteral("Translation initiation")) {
        path.addRoundedRect(body.adjusted(insetX * 0.6, body.height() * 0.22, -insetX * 0.6, -body.height() * 0.22),
                            body.height() / 3.0,
                            body.height() / 3.0);
        return path;
    }

    if (role == QStringLiteral("Termination")) {
        path.addRoundedRect(body.adjusted(insetX * 0.35, body.height() * 0.18, -headX * 0.8, -body.height() * 0.18),
                            body.height() / 4.0,
                            body.height() / 4.0);
        return path;
    }

    // Pre-gene regulatory, coding, and fallback use a directional cassette shape.
    path.moveTo(left + insetX, top);
    path.lineTo(right - headX, top);
    path.lineTo(right, centerY);
    path.lineTo(right - headX, bottom);
    path.lineTo(left + insetX, bottom);
    path.lineTo(left, centerY);
    path.closeSubpath();

    if (role == QStringLiteral("Other")) {
        QPainterPath notch;
        notch.addRoundedRect(body.adjusted(insetX * 0.3, body.height() * 0.25, -headX * 0.7, -body.height() * 0.25),
                             midBand,
                             midBand);
        path.addPath(notch);
    }

    return path;
}

QPainterPath geneticRegulationShape(const GraphicalModelItemRenderContext& context) {
    const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
    const QString role = geneticRegulationRole(context);
    const qreal left = body.left();
    const qreal right = body.right();
    const qreal top = body.top();
    const qreal bottom = body.bottom();
    const qreal centerY = body.center().y();
    const qreal insetX = qMax<qreal>(6.0, body.width() * 0.14);
    const qreal capsuleRadius = body.height() / 2.0;

    QPainterPath path;
    if (role == QStringLiteral("Dual")) {
        path.addRoundedRect(body.adjusted(insetX * 0.35, body.height() * 0.18, -insetX * 0.35, -body.height() * 0.18),
                            capsuleRadius,
                            capsuleRadius);
        return path;
    }

    path.addRoundedRect(body.adjusted(insetX * 0.35, body.height() * 0.22, -insetX * 0.35, -body.height() * 0.22),
                        capsuleRadius,
                        capsuleRadius);

    QPainterPath tip;
    if (role == QStringLiteral("Activation")) {
        tip.moveTo(right - insetX * 0.25, centerY);
        tip.lineTo(right - insetX * 0.8, top + body.height() * 0.18);
        tip.lineTo(right - insetX * 0.8, bottom - body.height() * 0.18);
        tip.closeSubpath();
    } else if (role == QStringLiteral("Repression")) {
        tip.addRect(QRectF(right - insetX * 0.7, top + body.height() * 0.2, insetX * 0.18, body.height() * 0.6));
    } else {
        tip.moveTo(right - insetX * 0.25, centerY);
        tip.lineTo(right - insetX * 0.8, top + body.height() * 0.18);
        tip.lineTo(right - insetX * 0.8, bottom - body.height() * 0.18);
        tip.closeSubpath();
    }

    path.addPath(tip);
    return path;
}

void drawGeneticCircuitPartDetails(QPainter* painter, const GraphicalModelItemRenderContext& context) {
    const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
    const QString role = geneticCircuitPartRole(context);
    const QColor accent = geneticCircuitPartAccentColor(role);
    QPen accentPen(accent.lighter(140), 2);
    accentPen.setCosmetic(true);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(accentPen);

    const qreal midY = body.center().y();
    const qreal left = body.left();
    const qreal right = body.right();
    const qreal headX = qMax<qreal>(10.0, body.width() * 0.18);
    const qreal insetX = qMax<qreal>(6.0, body.width() * 0.12);

    if (role == QStringLiteral("Pre-gene regulatory")) {
        painter->drawLine(QPointF(left + insetX, midY), QPointF(right - headX, midY));
        painter->drawLine(QPointF(right - headX, midY), QPointF(right - headX - 8.0, midY - 5.0));
        painter->drawLine(QPointF(right - headX, midY), QPointF(right - headX - 8.0, midY + 5.0));
        return;
    }

    if (role == QStringLiteral("Regulatory")) {
        painter->drawLine(QPointF(body.center().x() - body.width() * 0.18, midY),
                          QPointF(body.center().x() + body.width() * 0.18, midY));
        painter->drawEllipse(QRectF(body.center().x() - 5.0, midY - 5.0, 10.0, 10.0));
        return;
    }

    if (role == QStringLiteral("Translation initiation")) {
        painter->drawLine(QPointF(left + insetX * 1.1, body.top() + body.height() * 0.22),
                          QPointF(left + insetX * 1.1, body.bottom() - body.height() * 0.22));
        return;
    }

    if (role == QStringLiteral("Coding")) {
        painter->drawLine(QPointF(left + insetX * 1.1, midY), QPointF(right - headX * 0.6, midY));
        return;
    }

    if (role == QStringLiteral("Termination")) {
        painter->drawLine(QPointF(right - headX * 0.3, body.top() + body.height() * 0.2),
                          QPointF(right - headX * 0.3, body.bottom() - body.height() * 0.2));
        return;
    }

    painter->drawLine(QPointF(left + insetX, midY), QPointF(right - headX, midY));
}

void drawGeneticRegulationDetails(QPainter* painter, const GraphicalModelItemRenderContext& context) {
    const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
    const QString role = geneticRegulationRole(context);
    const QColor accent = geneticRegulationAccentColor(role);
    QPen accentPen(accent.lighter(145), 2);
    accentPen.setCosmetic(true);

    painter->setBrush(Qt::NoBrush);
    painter->setPen(accentPen);

    const qreal midY = body.center().y();
    const qreal left = body.left();
    const qreal right = body.right();
    const qreal insetX = qMax<qreal>(6.0, body.width() * 0.14);

    painter->drawLine(QPointF(left + insetX * 1.1, midY), QPointF(right - insetX * 1.4, midY));

    if (role == QStringLiteral("Activation")) {
        painter->drawLine(QPointF(right - insetX * 1.4, midY), QPointF(right - insetX * 1.95, midY - 5.5));
        painter->drawLine(QPointF(right - insetX * 1.4, midY), QPointF(right - insetX * 1.95, midY + 5.5));
    } else if (role == QStringLiteral("Repression")) {
        painter->drawLine(QPointF(right - insetX * 1.55, body.top() + body.height() * 0.24),
                          QPointF(right - insetX * 1.55, body.bottom() - body.height() * 0.24));
    } else if (role == QStringLiteral("Dual")) {
        painter->drawLine(QPointF(right - insetX * 1.35, midY), QPointF(right - insetX * 1.75, midY - 5.5));
        painter->drawLine(QPointF(right - insetX * 1.35, midY), QPointF(right - insetX * 1.75, midY + 5.5));
        painter->drawLine(QPointF(right - insetX * 1.95, body.top() + body.height() * 0.24),
                          QPointF(right - insetX * 1.95, body.bottom() - body.height() * 0.24));
    }
}

class ClassicGraphicalModelItemRenderStrategy final : public GraphicalModelItemRenderStrategy {
public:
    const char* name() const override {
        return "classic";
    }

    QPainterPath shape(const GraphicalModelItemRenderContext& context) const override {
        if (isGeneticCircuitPartContext(context)) {
            return geneticCircuitPartShape(context);
        }
        if (isGeneticRegulationContext(context)) {
            return geneticRegulationShape(context);
        }
        QPainterPath path;
        path.addRect(context.bounds);
        return path;
    }

    void paint(QPainter* painter, const GraphicalModelItemRenderContext& context) const override {
        painter->save();

        if (isGeneticCircuitPartContext(context)) {
            painter->setRenderHint(QPainter::Antialiasing, true);

            const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
            const QString role = geneticCircuitPartRole(context);
            const QColor accent = geneticCircuitPartAccentColor(role);

            QPainterPath partPath = geneticCircuitPartShape(context);
            QColor shadow = colorFromRgba(context.borderColor);
            shadow.setAlpha(context.selected ? 120 : 60);
            painter->setPen(Qt::NoPen);
            painter->setBrush(shadow);
            painter->drawPath(partPath.translated(0, context.selected ? 3 : 2));

            QLinearGradient gradient(body.topLeft(), body.bottomRight());
            gradient.setColorAt(0.0, context.fillColor.lighter(170));
            gradient.setColorAt(0.55, context.fillColor);
            gradient.setColorAt(1.0, context.fillColor.darker(140));
            painter->setBrush(QBrush(gradient));

            QPen bodyPen(accent.darker(120));
            bodyPen.setWidth(context.selected ? context.penWidth + 2 : context.penWidth + 1);
            bodyPen.setCosmetic(true);
            painter->setPen(bodyPen);
            painter->drawPath(partPath);

            painter->setBrush(QBrush(accent.lighter(160)));
            painter->setPen(Qt::NoPen);
            if (role == QStringLiteral("Regulatory")) {
                const QRectF markerRect(body.center().x() - body.width() * 0.08,
                                        body.center().y() - body.height() * 0.08,
                                        body.width() * 0.16,
                                        body.height() * 0.16);
                painter->drawEllipse(markerRect);
            } else if (role == QStringLiteral("Termination")) {
                const QRectF markerRect(body.right() - qMax<qreal>(6.0, body.width() * 0.07),
                                        body.top() + body.height() * 0.2,
                                        qMax<qreal>(4.0, body.width() * 0.04),
                                        body.height() * 0.6);
                painter->drawRect(markerRect);
            }

            drawGeneticCircuitPartDetails(painter, context);

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
                painter->drawPath(partPath);
            }

            painter->restore();
            return;
        }

        if (isGeneticRegulationContext(context)) {
            painter->setRenderHint(QPainter::Antialiasing, true);

            const QRectF body = innerRect(context).adjusted(context.raise, context.raise, -context.raise, -context.raise);
            const QString role = geneticRegulationRole(context);
            const QColor accent = geneticRegulationAccentColor(role);

            QPainterPath regulationPath = geneticRegulationShape(context);
            QColor shadow = colorFromRgba(context.borderColor);
            shadow.setAlpha(context.selected ? 120 : 60);
            painter->setPen(Qt::NoPen);
            painter->setBrush(shadow);
            painter->drawPath(regulationPath.translated(0, context.selected ? 3 : 2));

            QLinearGradient gradient(body.topLeft(), body.bottomRight());
            gradient.setColorAt(0.0, context.fillColor.lighter(170));
            gradient.setColorAt(0.5, context.fillColor);
            gradient.setColorAt(1.0, context.fillColor.darker(145));
            painter->setBrush(QBrush(gradient));

            QPen bodyPen(accent.darker(115));
            bodyPen.setWidth(context.selected ? context.penWidth + 2 : context.penWidth + 1);
            bodyPen.setCosmetic(true);
            painter->setPen(bodyPen);
            painter->drawPath(regulationPath);

            drawGeneticRegulationDetails(painter, context);

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
                painter->drawPath(regulationPath);
            }

            painter->restore();
            return;
        }

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
        if (isGeneticCircuitPartContext(context)) {
            return geneticCircuitPartShape(context);
        }
        if (isGeneticRegulationContext(context)) {
            return geneticRegulationShape(context);
        }
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

        if (isGeneticCircuitPartContext(context)) {
            const QRectF body = bodyRect(context);
            const QString role = geneticCircuitPartRole(context);
            const QColor accent = geneticCircuitPartAccentColor(role);
            const QPainterPath partPath = geneticCircuitPartShape(context);

            QColor shadow = colorFromRgba(context.borderColor);
            shadow.setAlpha(context.selected ? 120 : 60);
            painter->setPen(Qt::NoPen);
            painter->setBrush(shadow);
            painter->drawPath(partPath.translated(0, context.selected ? 3 : 2));

            QRadialGradient gradient(body.center() - QPointF(body.width() * 0.16, body.height() * 0.22),
                                     qMax(body.width(), body.height()) * 0.74);
            gradient.setColorAt(0.0, context.fillColor.lighter(175));
            gradient.setColorAt(0.58, context.fillColor);
            gradient.setColorAt(1.0, context.fillColor.darker(140));
            painter->setBrush(QBrush(gradient));

            QPen bodyPen(accent);
            bodyPen.setWidth(context.selected ? context.penWidth + 3 : context.penWidth + 1);
            bodyPen.setCosmetic(true);
            painter->setPen(bodyPen);
            painter->drawPath(partPath);

            drawGeneticCircuitPartDetails(painter, context);

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
                painter->drawPath(partPath);
            }

            painter->restore();
            return;
        }

        if (isGeneticRegulationContext(context)) {
            const QRectF body = bodyRect(context);
            const QString role = geneticRegulationRole(context);
            const QColor accent = geneticRegulationAccentColor(role);
            const QPainterPath regulationPath = geneticRegulationShape(context);

            QColor shadow = colorFromRgba(context.borderColor);
            shadow.setAlpha(context.selected ? 120 : 60);
            painter->setPen(Qt::NoPen);
            painter->setBrush(shadow);
            painter->drawPath(regulationPath.translated(0, context.selected ? 3 : 2));

            QRadialGradient gradient(body.center() - QPointF(body.width() * 0.15, body.height() * 0.18),
                                     qMax(body.width(), body.height()) * 0.78);
            gradient.setColorAt(0.0, context.fillColor.lighter(175));
            gradient.setColorAt(0.55, context.fillColor);
            gradient.setColorAt(1.0, context.fillColor.darker(145));
            painter->setBrush(QBrush(gradient));

            QPen bodyPen(accent);
            bodyPen.setWidth(context.selected ? context.penWidth + 3 : context.penWidth + 1);
            bodyPen.setCosmetic(true);
            painter->setPen(bodyPen);
            painter->drawPath(regulationPath);

            drawGeneticRegulationDetails(painter, context);

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
                painter->drawPath(regulationPath);
            }

            painter->restore();
            return;
        }

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
