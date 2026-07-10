#ifndef DELETEUNDOCOMMAND_H
#define DELETEUNDOCOMMAND_H

#include <QUndoCommand>
#include <string>
#include "AddUndoCommand.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelDataDefinition.h"

struct DataDefinitionItem {
    GraphicalModelDataDefinition *graphicalDataDefinition;
    QPointF initialPosition;
    bool initiallySelected;
};

struct DataDefinitionRelationItem {
    ModelDataDefinition *owner;
    ModelDataDefinition *dataDefinition;
    std::string key;
    bool internal;
};

class DeleteUndoCommand : public QUndoCommand
{
public:
    explicit DeleteUndoCommand(QList<QGraphicsItem *> items, ModelGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~DeleteUndoCommand();

    void undo() override;
    void redo() override;

private:
    void captureDataDefinitionsRemovedWithSelectedItems();
    void captureDataDefinitionRelations();
    void removeDataDefinitionRelations();
    void restoreDataDefinitionRelations();
    void removeDataDefinitionsFromModel();
    void restoreDataDefinitionsToModel();
    void detachDataDefinitionItems();
    void restoreDataDefinitionItems();

    QList<ComponentItem> *_myComponentItems;
    QList<GraphicalConnection *> *_myConnectionItems;
    QList<DrawingItem> *_myDrawingItems;
    QList<GroupItem> *_myGroupItems;
    QList<DataDefinitionItem> *_myDataDefinitionItems;
    QList<DataDefinitionRelationItem> *_myDataDefinitionRelationItems;
    QList<ModelDataDefinition *> *_myDataDefinitions;
    ModelGraphicsScene *_myGraphicsScene;
};

#endif // DELETEUNDOCOMMAND_H
