#include "EFSMTransitionGraphicsItem.h"

#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"

#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPathStroker>
#include <QPointer>
#include <QUndoStack>
#include "ModelGraphicsScene.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include <QStringList>
#include <QtMath>

namespace {
class EFSMBendUndoCommand final : public QUndoCommand {
public:
    EFSMBendUndoCommand(ModelGraphicsScene* scene, EFSMTransition* transition,
                        bool beforeManual, QPointF before, QPointF after)
        : _scene(scene), _transition(transition), _beforeManual(beforeManual),
          _before(before), _after(after) {
        setText(QObject::tr("Bend EFSM transition"));
    }
    void undo() override { apply(_beforeManual, _before); }
    void redo() override { apply(true, _after); }
private:
    void apply(bool manual, const QPointF& bend) {
        if (_scene == nullptr) { setObsolete(true); return; }
        auto* network = dynamic_cast<EFSMNetwork*>(_scene->networkEditorOwner());
        if (network == nullptr || network->getTransitions() == nullptr
            || network->getTransitions()->find(_transition) == network->getTransitions()->list()->end()) {
            setObsolete(true);
            return;
        }
        for (auto* graphic : _scene->extensionOverlayItems(QStringLiteral("efsm"))) {
            auto* item = dynamic_cast<EFSMTransitionGraphicsItem*>(graphic);
            if (item == nullptr || item->transition() != _transition) continue;
            if (manual) item->setManualBendPoint(bend);
            else item->clearManualBendPoint();
            break;
        }
    }
    QPointer<ModelGraphicsScene> _scene;
    EFSMTransition* _transition;
    bool _beforeManual;
    QPointF _before;
    QPointF _after;
};
} // namespace

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

void EFSMTransitionGraphicsItem::setFired(bool fired) {
    _fired = fired;
    update();
}

bool EFSMTransitionGraphicsItem::isFired() const {
    return _fired;
}

bool EFSMTransitionGraphicsItem::hasManualBendPoint() const {
    return _manualBend;
}

QPointF EFSMTransitionGraphicsItem::manualBendPoint() const {
    return _manualBendPoint;
}

void EFSMTransitionGraphicsItem::setManualBendPoint(const QPointF& point) {
    if (!std::isfinite(point.x()) || !std::isfinite(point.y())) return;
    _manualBend = true;
    _manualBendPoint = point;
    _rebuildGeometry();
}

void EFSMTransitionGraphicsItem::clearManualBendPoint() {
    _manualBend = false;
    _rebuildGeometry();
}

void EFSMTransitionGraphicsItem::setGeometry(const QPointF& sourceCenter, const QPointF& destinationCenter,
                                             int parallelRank, int parallelCount) {
    _sourceCenter = sourceCenter;
    _destinationCenter = destinationCenter;
    _parallelRank = parallelRank;
    _parallelCount = std::max(1, parallelCount);
    _rebuildGeometry();
}

void EFSMTransitionGraphicsItem::_rebuildGeometry() {
    QPainterPath transitionPath;
    const QPointF delta = _destinationCenter - _sourceCenter;
    const double length = std::hypot(delta.x(), delta.y());
    const double parallelOffset = (_parallelRank - (_parallelCount - 1) / 2.0) * 26.0;

    if (length < 1.0) {
        const QRectF loop(_sourceCenter.x() - 45.0, _sourceCenter.y() - 80.0, 90.0, 80.0);
        const QPointF start = _sourceCenter + QPointF(24.0, -18.0);
        const QPointF end = _sourceCenter + QPointF(-24.0, -18.0);
        transitionPath.moveTo(start);
        if (_manualBend) {
            transitionPath.quadTo(_manualBendPoint * 2.0 - (start + end) / 2.0, end);
        } else {
            transitionPath.cubicTo(loop.topRight(), loop.topLeft(), end);
        }
    } else {
        const QPointF unit(delta.x() / length, delta.y() / length);
        const QPointF normal(-unit.y(), unit.x());
        const QPointF offset = normal * parallelOffset;
        const QPointF start = _sourceCenter + offset + unit * 42.0;
        const QPointF end = _destinationCenter + offset - unit * 42.0;
        transitionPath.moveTo(start);
        if (_manualBend) {
            transitionPath.quadTo(_manualBendPoint * 2.0 - (start + end) / 2.0, end);
        } else {
            const QPointF controlOffset = normal * (parallelOffset * 0.35);
            transitionPath.cubicTo(start + delta * 0.33 + controlOffset,
                                   start + delta * 0.66 + controlOffset, end);
        }
    }
    setPath(transitionPath);
}

QPointF EFSMTransitionGraphicsItem::_handlePosition() const {
    return path().pointAtPercent(0.5);
}

QPainterPath EFSMTransitionGraphicsItem::shape() const {
    QPainterPath hitPath = QGraphicsPathItem::shape();
    if (!path().isEmpty()) hitPath.addEllipse(_handlePosition(), 10.0, 10.0);
    return hitPath;
}

QRectF EFSMTransitionGraphicsItem::boundingRect() const {
    QRectF bounds = QGraphicsPathItem::boundingRect();
    if (!path().isEmpty()) {
        const QPointF center = _handlePosition();
        bounds = bounds.united(QRectF(center.x() - 11.0, center.y() - 11.0, 22.0, 22.0));
    }
    return bounds;
}

void EFSMTransitionGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event != nullptr && event->button() == Qt::LeftButton && isSelected()
        && QLineF(event->pos(), _handlePosition()).length() <= 11.0) {
        _draggingBend = true;
        _beforeDragWasManual = _manualBend;
        _beforeDragBendPoint = _manualBend ? _manualBendPoint : _handlePosition();
        event->accept();
        return;
    }
    QGraphicsPathItem::mousePressEvent(event);
}

void EFSMTransitionGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (_draggingBend && event != nullptr) {
        setManualBendPoint(event->pos());
        event->accept();
        return;
    }
    QGraphicsPathItem::mouseMoveEvent(event);
}

void EFSMTransitionGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (_draggingBend) {
        _draggingBend = false;
        auto* networkScene = dynamic_cast<ModelGraphicsScene*>(scene());
        if (networkScene != nullptr && networkScene->getUndoStack() != nullptr
            && _manualBendPoint != _beforeDragBendPoint) {
            networkScene->getUndoStack()->push(new EFSMBendUndoCommand(
                networkScene, _transition, _beforeDragWasManual,
                _beforeDragBendPoint, _manualBendPoint));
        }
        if (event != nullptr) event->accept();
        return;
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

void EFSMTransitionGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (painter == nullptr || path().isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(_fired ? QColor(225, 90, 35) : QColor(45, 75, 130), _fired ? 4.0 : 2.0));
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
    painter->setBrush(_fired ? QColor(225, 90, 35) : QColor(45, 75, 130));
    painter->drawPolygon(QPolygonF() << end << left << right);

    if (isSelected()) {
        painter->setPen(QPen(QColor(35, 90, 165), 2.0));
        painter->setBrush(QBrush(QColor(235, 245, 255)));
        painter->drawEllipse(_handlePosition(), 6.0, 6.0);
    }

    const QString label = transitionLabel();
    if (!label.isEmpty()) {
        painter->setPen(QPen(QColor(30, 30, 30), 1.0));
        painter->drawText(path().pointAtPercent(0.5) + QPointF(5.0, -5.0), label);
    }
}
