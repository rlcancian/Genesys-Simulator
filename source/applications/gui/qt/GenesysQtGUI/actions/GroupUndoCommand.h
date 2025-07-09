#ifndef GROUPUNDOCOMMAND_H
#define GROUPUNDOCOMMAND_H

#include <QUndoCommand>
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalModelComponent.h"

class GroupUndoCommand: public QUndoCommand {
public:
    GroupUndoCommand(QList<GraphicalModelComponent *> graphicalComponents, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~GroupUndoCommand();

    void undo() override;
    void redo() override;

private:
    QList<GraphicalModelComponent *> _myGraphicalComponents;
    QGraphicsItemGroup *_group;
    ModelGraphicsScene *_myGraphicsScene;
    QPointF _initialPosition;
};

#endif // GROUPUNDOCOMMAND_H
