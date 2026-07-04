#ifndef ADDUNDOCOMMAND_H
#define ADDUNDOCOMMAND_H

#include <QUndoCommand>
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelComponent.h"

struct ComponentItem {
    GraphicalModelComponent *graphicalComponent;
    QPointF initialPosition;
    QList<GraphicalConnection *> inputConnections;
    QList<GraphicalConnection *> outputConnections;
};

struct GroupItem {
    QGraphicsItemGroup *group;
    QList<ComponentItem> myComponentItems;
    QPointF initialPosition;
};

struct DrawingItem {
    QGraphicsItem *myDrawingItem;
    QPointF initialPosition;
};


class AddUndoCommand: public QUndoCommand {
public:
    AddUndoCommand(QGraphicsItem *item, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~AddUndoCommand();

    void undo() override;
    void redo() override;

private:
    ComponentItem _myComponentItem;
    GraphicalConnection *_myConnectionItem;
    QGraphicsItem *_myDrawingItem;
    ModelGraphicsScene *_myGraphicsScene;
    bool _firstExecution;
};

#endif // ADDUNDOCOMMAND_H
