#include "EFSMTransitionGraphicsItem.h"

#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"

#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QStringList>
#include <QtMath>

EFSMTransitionGraphicsItem::EFSMTransitionGraphicsItem(EFSMTransition* transition, int ordinal, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), _transition(transition), _ordinal(ordinal) {
    setZValue(-10.0);
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    // Give thin paths a practical hit target; paint() retains the visible pen.
    setPen(QPen(Qt::black, 12.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setToolTip(transitionLabel());
}

EFSMTransition* EFSMTransitionGraphicsItem::transition() const {
    return _transition;
}

int EFSMTransitionGraphicsItem::ordinal() const {
    return _ordinal;
}

QString EFSMTransitionGraphicsItem::transitionLabel() const {
    if (_transition == nullptr) {
        return {};
    }
    QStringList parts;
    const auto append = [&parts](const QString& value) {
        if (!value.isEmpty()) {
            parts.append(value);
        }
    };
    append(QString::fromStdString(_transition->getName()));
    if (!_transition->getInputEvent().empty()) {
        append(QStringLiteral("on %1").arg(QString::fromStdString(_transition->getInputEvent())));
    }
    if (!_transition->getTriggerEvent().empty()) {
        append(QStringLiteral("trigger %1").arg(QString::fromStdString(_transition->getTriggerEvent())));
    }
    if (!_transition->getGuardExpression().empty()) {
        append(QStringLiteral("[%1]").arg(QString::fromStdString(_transition->getGuardExpression())));
    }
    if (!_transition->getOutputExpression().empty()) {
        append(QStringLiteral("/ %1").arg(QString::fromStdString(_transition->getOutputExpression())));
    }
    if (!_transition->getProbabilityExpression().empty()) {
        append(QStringLiteral("p=%1").arg(QString::fromStdString(_transition->getProbabilityExpression())));
    }
    return parts.join(QLatin1Char(' '));
}

void EFSMTransitionGraphicsItem::setGeometry(const QPointF& sourceCenter, const QPointF& destinationCenter,
                                             int parallelRank, int parallelCount) {
    QPainterPath transitionPath;
    const QPointF delta = destinationCenter - sourceCenter;
    const double length = std::hypot(delta.x(), delta.y());
    const double parallelOffset = (parallelRank - (parallelCount - 1) / 2.0) * 26.0;

    if (length < 1.0) {
        const QRectF loop(sourceCenter.x() - 45.0, sourceCenter.y() - 80.0, 90.0, 80.0);
        transitionPath.moveTo(sourceCenter + QPointF(24.0, -18.0));
        transitionPath.cubicTo(loop.topRight(), loop.topLeft(), sourceCenter + QPointF(-24.0, -18.0));
    } else {
        const QPointF unit(delta.x() / length, delta.y() / length);
        const QPointF normal(-unit.y(), unit.x());
        const QPointF offset = normal * parallelOffset;
        const QPointF start = sourceCenter + offset + unit * 42.0;
        const QPointF end = destinationCenter + offset - unit * 42.0;
        const QPointF controlOffset = normal * (parallelOffset * 0.35);
        transitionPath.moveTo(start);
        transitionPath.cubicTo(start + delta * 0.33 + controlOffset,
                               start + delta * 0.66 + controlOffset,
                               end);
    }
    setPath(transitionPath);
}

void EFSMTransitionGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (painter == nullptr || path().isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor(45, 75, 130), 2.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path());

    const QPointF end = path().pointAtPercent(1.0);
    const QPointF beforeEnd = path().pointAtPercent(0.96);
    const QLineF direction(beforeEnd, end);
    const double angle = std::atan2(-direction.dy(), direction.dx());
    const double arrowLength = 11.0;
    const QPointF left = end - QPointF(std::cos(angle + M_PI / 6.0) * arrowLength,
                                      -std::sin(angle + M_PI / 6.0) * arrowLength);
    const QPointF right = end - QPointF(std::cos(angle - M_PI / 6.0) * arrowLength,
                                       -std::sin(angle - M_PI / 6.0) * arrowLength);
    painter->setBrush(QColor(45, 75, 130));
    painter->drawPolygon(QPolygonF() << end << left << right);

    const QString label = transitionLabel();
    if (!label.isEmpty()) {
        painter->setPen(QPen(QColor(30, 30, 30), 1.0));
        painter->drawText(path().pointAtPercent(0.5) + QPointF(5.0, -5.0), label);
    }
}
