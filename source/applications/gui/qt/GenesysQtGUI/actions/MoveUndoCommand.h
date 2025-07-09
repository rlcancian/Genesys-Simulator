#ifndef MOVEUNDOCOMMAND_H
#define MOVEUNDOCOMMAND_H

#include <QUndoCommand>
#include "graphicals/ModelGraphicsScene.h"

class MoveUndoCommand : public QUndoCommand
{
public:
    MoveUndoCommand(QList<QGraphicsItem *> item, ModelGraphicsScene *scene, QList<QPointF> &oldPos, QList<QPointF> &newPos, QUndoCommand *parent = nullptr);
    ~MoveUndoCommand();

    void undo() override;
    void redo() override;

    //bool mergeWith(const QUndoCommand *command) override;

private:
    QList<QGraphicsItem*> _myGraphicalItem;
    ModelGraphicsScene *_myGraphicsScene;
    QList<QPointF> _myOldPos;
    QList<QPointF> _myNewPos;
};

#endif // MOVEUNDOCOMMAND_H
