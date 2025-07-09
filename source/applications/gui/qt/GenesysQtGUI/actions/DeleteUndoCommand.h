#ifndef DELETEUNDOCOMMAND_H
#define DELETEUNDOCOMMAND_H

#include <QUndoCommand>
#include "AddUndoCommand.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"


class DeleteUndoCommand : public QUndoCommand
{
public:
    explicit DeleteUndoCommand(QList<QGraphicsItem *> items, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~DeleteUndoCommand();

    void undo() override;
    void redo() override;

private:
    QList<ComponentItem> *_myComponentItems;
    QList<GraphicalConnection *> *_myConnectionItems;
    QList<DrawingItem> *_myDrawingItems;
    QList<GroupItem> *_myGroupItems;
    ModelGraphicsScene *_myGraphicsScene;
};

#endif // DELETEUNDOCOMMAND_H
