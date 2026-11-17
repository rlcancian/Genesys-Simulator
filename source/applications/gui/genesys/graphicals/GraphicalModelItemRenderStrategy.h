#ifndef GRAPHICALMODELITEMRENDERSTRATEGY_H
#define GRAPHICALMODELITEMRENDERSTRATEGY_H

#include <cstdint>

#include <QColor>
#include <QFont>
#include <QVector>
#include <QPainterPath>
#include <QRectF>
#include <QString>

class QPainter;

struct GraphicalModelItemRenderContext {
    enum class ItemKind {
        DataDefinition,
        Component
    };

    ItemKind kind = ItemKind::DataDefinition;
    QRectF bounds;
    QColor fillColor;
    QString primaryText;
    QString secondaryText;
    QString tertiaryText;
    QString semanticClassName;
    QString semanticSubtype;
    bool hasSpecializedGlyph = false;
    QPainterPath specializedShape;
    QPainterPath specializedDetails;
    QColor specializedAccent;
    bool selected = false;
    bool breakpoint = false;

    qreal width = 0.0;
    qreal height = 0.0;
    int penWidth = 1;
    int raise = 0;
    int margin = 0;
    int selectionWidth = 0;

    qreal stretchPosTop = 0.5;
    qreal stretchPosBottom = 0.5;
    qreal stretchPosLeft = 0.5;
    qreal stretchPosRight = 0.5;
    qreal stretchRight = 0.0;
    qreal stretchLeft = 0.0;
    qreal stretchRightMiddle = 0.0;
    qreal stretchLeftMiddle = 0.0;
    qreal stretchTop = 0.0;
    qreal stretchBottom = 0.0;
    qreal stretchTopMiddle = 0.0;
    qreal stretchBottomMiddle = 0.0;

    uint64_t borderColor = 0;
    uint64_t raisedColor = 0;
    uint64_t sunkenColor = 0;
    uint64_t textColor = 0;
    uint64_t textShadowColor = 0;
    uint64_t selectionColor = 0;
    uint64_t breakpointColor = 0;
};

class GraphicalModelItemRenderStrategy {
public:
    virtual ~GraphicalModelItemRenderStrategy() = default;

    virtual const char* name() const = 0;
    virtual void paint(QPainter* painter, const GraphicalModelItemRenderContext& context) const = 0;
    virtual QPainterPath shape(const GraphicalModelItemRenderContext& context) const = 0;
};

struct GraphicalItemTextLine {
    QString text;
    QFont font;
    QRectF rect;
    Qt::Alignment alignment;
    int penWidth;
};

struct GraphicalItemTextLayout {
    QVector<GraphicalItemTextLine> lines;
    qreal scale = 1.0;
};

class GraphicalModelItemRenderer {
public:
    // Rectangles are in unscaled coordinates, scaled about contentRect.center().
    static GraphicalItemTextLayout layoutText(const GraphicalModelItemRenderContext& context,
                                             const QRectF& contentRect, const QFont& baseFont);
    static void paint(QPainter* painter, const GraphicalModelItemRenderContext& context);
    static QPainterPath shape(const GraphicalModelItemRenderContext& context);
    static const GraphicalModelItemRenderStrategy& currentStrategy();
};

#endif // GRAPHICALMODELITEMRENDERSTRATEGY_H
