#ifndef PASTEUNDOCOMMAND_H
#define PASTEUNDOCOMMAND_H

#include <QUndoCommand>
#include "AddUndoCommand.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"


class PasteUndoCommand : public QUndoCommand
{
public:
    explicit PasteUndoCommand(QList<GraphicalModelComponent *> *graphicalComponents, QList<GraphicalConnection *> *connections, QList<QGraphicsItemGroup *> *groups, QList<QGraphicsItem *> *drawing, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~PasteUndoCommand();

    void undo() override;
    void redo() override;

private:
    QList<ComponentItem> *_myComponentItems;
    QList<GraphicalConnection *> *_myConnectionItems;
    QList<DrawingItem> *_myDrawingItems;
    QList<GroupItem> *_myGroupItems;
    ModelGraphicsScene *_myGraphicsScene;
};

#endif // PASTEUNDOCOMMAND_H
