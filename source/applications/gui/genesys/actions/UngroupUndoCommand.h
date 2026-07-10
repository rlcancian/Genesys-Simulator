#ifndef UNGROUPUNDOCOMMAND_H
#define UNGROUPUNDOCOMMAND_H


#include <QUndoCommand>
#include "graphicals/ModelGraphicsScene.h"

class UngroupUndoCommand: public QUndoCommand {
public:
    UngroupUndoCommand(QGraphicsItemGroup* group, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~UngroupUndoCommand();

    void undo() override;
    void redo() override;

private:
    QGraphicsItemGroup *_group;
    QList<GraphicalModelComponent *> *_myComponentsGroup;
    ModelGraphicsScene *_myGraphicsScene;
    QPointF _initialPosition;
};

#endif // UNGROUPUNDOCOMMAND_H
