#pragma once

#include "ModelGraphicsScene.h"

#include <QGraphicsPathItem>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <QPolygonF>
#include <QPointer>
#include <QTimer>
#include <QUndoCommand>
#include <QUndoStack>
#include <QLineF>

#include <algorithm>
#include <cmath>

/**
 * Presentation-only route control for Graph, Markov and CPN relations.
 *
 * The mathematical relation is identified by its extension-local route key.
 * Its bend is never written into ModelDataDefinition. Each network extension
 * captures the bend in its optional .gui diagram section instead.
 */
class NetworkBendGraphicsItem final : public QGraphicsPathItem {
public:
    explicit NetworkBendGraphicsItem(QString extensionId, QString routeKey)
        : _extensionId(std::move(extensionId)), _routeKey(std::move(routeKey)) {
        setPen(_visiblePen);
        setBrush(Qt::NoBrush);
        setFlag(ItemIsSelectable, true);
        setAcceptedMouseButtons(Qt::LeftButton);
        setZValue(-1.0);
    }

    const QString& routeKey() const { return _routeKey; }
    bool hasArrowHead() const { return _directed; }
    bool hasManualBendPoint() const { return _manual; }
    QPointF manualBendPoint() const { return _bend; }

    void setVisiblePen(const QPen& pen) { _visiblePen = pen; setPen(pen); update(); }

    void setGeometry(const QPointF& from, const QPointF& to, int rank, int count,
                     bool opposite, bool directed) {
        _from = from;
        _to = to;
        _rank = rank;
        _count = std::max(1, count);
        _opposite = opposite;
        _directed = directed;
        rebuildPath();
    }

    void setManualBendPoint(const QPointF& point) {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y())) return;
        _bend = point;
        _manual = true;
        rebuildPath();
    }

    void clearManualBendPoint() {
        _manual = false;
        rebuildPath();
    }

    QPointF handlePosition() const { return path().pointAtPercent(0.5); }

    QPainterPath shape() const override {
        QPainterPathStroker hit;
        hit.setWidth(12.0);
        hit.setCapStyle(Qt::RoundCap);
        hit.setJoinStyle(Qt::RoundJoin);
        QPainterPath result = hit.createStroke(path());
        if (!path().isEmpty()) result.addEllipse(handlePosition(), 11.0, 11.0);
        return result;
    }

    QRectF boundingRect() const override {
        QRectF result = QGraphicsPathItem::boundingRect().adjusted(-14.0, -14.0, 14.0, 14.0);
        if (!path().isEmpty()) {
            const QPointF center = handlePosition();
            result = result.united(QRectF(center - QPointF(12, 12), QSizeF(24, 24)));
        }
        return result;
    }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        if (painter == nullptr || path().isEmpty()) return;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(_visiblePen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(path());
        if (_directed) {
            const QPointF tip = path().pointAtPercent(1.0);
            const QPointF tail = path().pointAtPercent(0.96);
            const double angle = std::atan2((tip - tail).y(), (tip - tail).x());
            constexpr double length = 10.0;
            constexpr double spread = 0.48;
            const QPointF a = tip - QPointF(std::cos(angle - spread) * length,
                                             std::sin(angle - spread) * length);
            const QPointF b = tip - QPointF(std::cos(angle + spread) * length,
                                             std::sin(angle + spread) * length);
            painter->setBrush(_visiblePen.color());
            painter->drawPolygon(QPolygonF() << tip << a << b);
        }
        if (isSelected()) {
            painter->setPen(QPen(QColor(36, 92, 165), 2.0));
            painter->setBrush(QColor(235, 245, 255));
            painter->drawEllipse(handlePosition(), 6.0, 6.0);
        }
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event != nullptr && event->button() == Qt::LeftButton && isSelected()
            && QLineF(event->pos(), handlePosition()).length() <= 11.0) {
            _dragging = true;
            _beforeManual = _manual;
            _beforeBend = _manual ? _bend : handlePosition();
            event->accept();
            return;
        }
        QGraphicsPathItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (_dragging && event != nullptr) {
            setManualBendPoint(event->pos());
            event->accept();
            return;
        }
        QGraphicsPathItem::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        if (!_dragging) {
            QGraphicsPathItem::mouseReleaseEvent(event);
            return;
        }
        _dragging = false;
        if (event != nullptr) event->accept();
        if (!_manual || _bend == _beforeBend) return;

        auto* modelScene = dynamic_cast<ModelGraphicsScene*>(scene());
        if (modelScene == nullptr || modelScene->getUndoStack() == nullptr) return;

        // QUndoStack emits indexChanged synchronously, causing overlay deletion.
        // Do not delete the active QGraphicsItem inside its own mouse event.
        const QPointer<ModelGraphicsScene> guarded(modelScene);
        ModelDataDefinition* const owner = modelScene->networkEditorOwner();
        const QString extension = _extensionId;
        const QString key = _routeKey;
        const bool wasManual = _beforeManual;
        const QPointF previous = _beforeBend;
        const QPointF changed = _bend;
        QTimer::singleShot(0, modelScene, [guarded, owner, extension, key,
                                           wasManual, previous, changed]() {
            if (!guarded || guarded->networkEditorOwner() != owner
                || guarded->getUndoStack() == nullptr) return;
            class BendUndo final : public QUndoCommand {
            public:
                BendUndo(ModelGraphicsScene* scene, ModelDataDefinition* owner,
                         QString group, QString key, bool beforeManual,
                         QPointF before, QPointF after)
                    : _scene(scene), _owner(owner), _group(std::move(group)),
                      _key(std::move(key)), _beforeManual(beforeManual),
                      _before(before), _after(after) {
                    setText(QObject::tr("Bend network relation"));
                }
                void redo() override { apply(true, _after); }
                void undo() override { apply(_beforeManual, _before); }
            private:
                void apply(bool manual, const QPointF& point) {
                    if (!_scene || _scene->networkEditorOwner() != _owner) {
                        setObsolete(true);
                        return;
                    }
                    for (QGraphicsItem* item : _scene->extensionOverlayItems(_group)) {
                        auto* relation = dynamic_cast<NetworkBendGraphicsItem*>(item);
                        if (relation == nullptr || relation->routeKey() != _key) continue;
                        if (manual) relation->setManualBendPoint(point);
                        else relation->clearManualBendPoint();
                        return;
                    }
                    setObsolete(true);
                }
                QPointer<ModelGraphicsScene> _scene;
                ModelDataDefinition* _owner = nullptr; // only identity comparison
                QString _group, _key;
                bool _beforeManual = false;
                QPointF _before, _after;
            };
            guarded->getUndoStack()->push(new BendUndo(
                guarded, owner, extension, key, wasManual, previous, changed));
        });
    }

private:
    void rebuildPath() {
        const QPointF delta = _to - _from;
        const double distance = std::hypot(delta.x(), delta.y());
        QPainterPath route;
        if (distance < 1.0) {
            const double radius = 45.0 + std::max(0, _rank) * 22.0;
            const QPointF start = _from + QPointF(22.0, -18.0);
            const QPointF end = _from + QPointF(-22.0, -18.0);
            route.moveTo(start);
            if (_manual)
                route.quadTo(_bend * 2.0 - (start + end) / 2.0, end);
            else
                route.cubicTo(_from + QPointF(radius, -radius * 1.8),
                              _from + QPointF(-radius, -radius * 1.8), end);
        } else {
            const QPointF normal(-delta.y() / distance, delta.x() / distance);
            const double offset = (_rank - (_count - 1) / 2.0) * 26.0
                + (_opposite ? 18.0 : 0.0);
            const QPointF start = _from;
            const QPointF end = _to;
            route.moveTo(start);
            if (_manual) {
                route.quadTo(_bend * 2.0 - (start + end) / 2.0, end);
            } else if (std::abs(offset) > 1e-6) {
                route.quadTo((start + end) / 2.0 + normal * offset, end);
            } else {
                route.lineTo(end);
            }
        }
        setPath(route);
    }

    QString _extensionId;
    QString _routeKey;
    QPen _visiblePen = QPen(QColor(65, 75, 90), 2.0);
    QPointF _from, _to, _bend, _beforeBend;
    int _rank = 0, _count = 1;
    bool _manual = false, _dragging = false, _beforeManual = false;
    bool _opposite = false, _directed = true;
};
