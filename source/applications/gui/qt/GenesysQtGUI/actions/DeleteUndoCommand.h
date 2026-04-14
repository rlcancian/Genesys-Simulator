#ifndef DELETEUNDOCOMMAND_H
#define DELETEUNDOCOMMAND_H

#include <QUndoCommand>
#include <QSet>
#include "AddUndoCommand.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelDataDefinition.h"

struct DataDefinitionItem {
    GraphicalModelDataDefinition *graphicalDataDefinition;
    QPointF initialPosition;
    bool initiallySelected;
};

class DeleteUndoCommand : public QUndoCommand
{
public:
    explicit DeleteUndoCommand(QList<QGraphicsItem *> items, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~DeleteUndoCommand();

    void undo() override;
    void redo() override;

private:
    void captureInternalDataDefinitions(GraphicalModelComponent *component);
    void collectInternalDataDefinitions(ModelDataDefinition *owner, QSet<ModelDataDefinition *> *visited);
    void removeInternalDataDefinitionsFromModel();
    void restoreInternalDataDefinitionsToModel();
    void detachInternalDataDefinitionItems();
    void restoreInternalDataDefinitionItems();

    QList<ComponentItem> *_myComponentItems;
    QList<GraphicalConnection *> *_myConnectionItems;
    QList<DrawingItem> *_myDrawingItems;
    QList<GroupItem> *_myGroupItems;
    QList<DataDefinitionItem> *_myInternalDataDefinitionItems;
    QList<ModelDataDefinition *> *_myInternalDataDefinitions;
    ModelGraphicsScene *_myGraphicsScene;
};

#endif // DELETEUNDOCOMMAND_H
