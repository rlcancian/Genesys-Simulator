#ifndef EFSMTRANSITIONGRAPHICSITEM_H
#define EFSMTRANSITIONGRAPHICSITEM_H

#include <QGraphicsPathItem>

class EFSMTransition;

/**
 * @brief Read-only graphical projection of one mathematical EFSM transition.
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
    void setGeometry(const QPointF& sourceCenter, const QPointF& destinationCenter, int parallelRank,
                     int parallelCount);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    EFSMTransition* _transition = nullptr;
    int _ordinal = -1;
};

#endif
