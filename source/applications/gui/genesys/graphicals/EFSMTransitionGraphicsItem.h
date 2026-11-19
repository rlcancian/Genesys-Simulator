#ifndef EFSMTRANSITIONGRAPHICSITEM_H
#define EFSMTRANSITIONGRAPHICSITEM_H

#include <QGraphicsPathItem>
#include <QPointF>

class QGraphicsSceneMouseEvent;

class EFSMTransition;

/**
 * @brief Selectable, bendable graphical projection of one mathematical EFSM transition.
 *
 * The item observes the transition and never owns or mutates it. It is kept
 * separate from GraphicalConnection because an EFSM relation is not an Entity
 * flow connection.
 */
class EFSMTransitionGraphicsItem final : public QGraphicsPathItem {
public:
    EFSMTransitionGraphicsItem(EFSMTransition* transition, int ordinal, QGraphicsItem* parent = nullptr);

    EFSMTransition* transition() const;
    int ordinal() const;
    QString transitionLabel() const;
    void setFired(bool fired);
    bool isFired() const;
    void setGeometry(const QPointF& sourceCenter, const QPointF& destinationCenter, int parallelRank,
                     int parallelCount);
    bool hasManualBendPoint() const;
    QPointF manualBendPoint() const;
    void setManualBendPoint(const QPointF& point);
    void clearManualBendPoint();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    EFSMTransition* _transition = nullptr;
    int _ordinal = -1;
    bool _fired = false;
    bool _manualBend = false;
    bool _draggingBend = false;
    bool _beforeDragWasManual = false;
    QPointF _manualBendPoint;
    QPointF _beforeDragBendPoint;
    QPointF _sourceCenter;
    QPointF _destinationCenter;
    int _parallelRank = 0;
    int _parallelCount = 1;
    QPointF _handlePosition() const;
    void _rebuildGeometry();
};

#endif
