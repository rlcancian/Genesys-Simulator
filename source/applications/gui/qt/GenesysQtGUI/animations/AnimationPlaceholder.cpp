#include "AnimationPlaceholder.h"

#include <QFontMetricsF>
#include <QStyleOptionGraphicsItem>

AnimationPlaceholder::AnimationPlaceholder(const QString& animationType)
    : _animationType(animationType) {
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setActive(true);
    setSelected(false);
}

void AnimationPlaceholder::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QRectF bounds = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(Qt::black, 1.5));
    painter->setBrush(QColor(245, 245, 245));
    painter->drawRect(bounds);

    painter->setPen(QPen(Qt::red, 3.0));
    painter->drawLine(bounds.topLeft(), bounds.bottomRight());
    painter->drawLine(bounds.bottomLeft(), bounds.topRight());

    QString label = _animationType;
    if (!_targetName.trimmed().isEmpty()) {
        label += "\n" + _targetName.trimmed();
    }

    QFont font = painter->font();
    const int fontSize = qMax(8, qMin(static_cast<int>(bounds.width() / 8.0), static_cast<int>(bounds.height() / 4.0)));
    font.setPixelSize(fontSize);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(bounds.adjusted(4.0, 4.0, -4.0, -4.0), Qt::AlignCenter | Qt::TextWordWrap, label);

    if (isSelected()) {
        const qreal cornerSize = 10.0;
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        painter->drawRect(QRectF(-cornerSize, -cornerSize, cornerSize, cornerSize));
        painter->drawRect(QRectF(bounds.topRight() - QPointF(0, cornerSize), QSizeF(cornerSize, cornerSize)));
        painter->drawRect(QRectF(-cornerSize, bounds.height(), cornerSize, cornerSize));
        painter->drawRect(QRectF(bounds.bottomRight(), QSizeF(cornerSize, cornerSize)));
    }
}

QString AnimationPlaceholder::getAnimationType() const {
    return _animationType;
}

QString AnimationPlaceholder::getTargetName() const {
    return _targetName;
}

void AnimationPlaceholder::setTargetName(const QString& targetName) {
    _targetName = targetName;
    update();
}

void AnimationPlaceholder::startDrawing(QGraphicsSceneMouseEvent* event) {
    _isDrawingInicialized = true;
    _isResizing = true;
    _startPoint = event->scenePos();
    setPos(_startPoint);
}

void AnimationPlaceholder::continueDrawing(QGraphicsSceneMouseEvent* event) {
    if (!_isResizing) {
        return;
    }

    const QPointF delta = event->scenePos() - _startPoint;
    setRect(QRectF(0, 0, delta.x(), delta.y()).normalized());
    update();
}

void AnimationPlaceholder::stopDrawing(QGraphicsSceneMouseEvent* event) {
    adjustSizeAndPosition(event);
    _isResizing = false;
    _isDrawingFinalized = true;
}

void AnimationPlaceholder::adjustSizeAndPosition(QGraphicsSceneMouseEvent* event) {
    const qreal minimumX = qMin(_startPoint.x(), event->scenePos().x());
    const qreal minimumY = qMin(_startPoint.y(), event->scenePos().y());
    const qreal maximumX = qMax(_startPoint.x(), event->scenePos().x());
    const qreal maximumY = qMax(_startPoint.y(), event->scenePos().y());

    setRect(QRectF(0, 0, maximumX - minimumX, maximumY - minimumY).normalized());
    setPos(QPointF(minimumX, minimumY));
    update();
}

bool AnimationPlaceholder::isDrawingInicialized() const {
    return _isDrawingInicialized;
}

bool AnimationPlaceholder::isDrawingFinalized() const {
    return _isDrawingFinalized;
}

AnimationAttribute::AnimationAttribute() : AnimationPlaceholder("Attribute") {
}

AnimationEntity::AnimationEntity() : AnimationPlaceholder("Entity") {
}

AnimationEvent::AnimationEvent() : AnimationPlaceholder("Event") {
}

AnimationExpression::AnimationExpression() : AnimationPlaceholder("Expression") {
}

AnimationPlot::AnimationPlot() : AnimationPlaceholder("Plot") {
}

AnimationQueueDisplay::AnimationQueueDisplay() : AnimationPlaceholder("Queue") {
}

AnimationResource::AnimationResource() : AnimationPlaceholder("Resource") {
}

AnimationStation::AnimationStation() : AnimationPlaceholder("Station") {
}

AnimationStatistics::AnimationStatistics() : AnimationPlaceholder("Statistics") {
}
