#include "GroupUndoCommand.h"
#include "graphicals/ModelGraphicsScene.h"

GroupUndoCommand::GroupUndoCommand(QList<GraphicalModelComponent *> graphicalComponents, ModelGraphicsScene *scene, QUndoCommand *parent)
    :QUndoCommand(parent), _myGraphicalComponents(graphicalComponents), _group(new QGraphicsItemGroup()), _myGraphicsScene(scene){

    setText(QObject::tr("Group"));
}

GroupUndoCommand::~GroupUndoCommand() {}

void GroupUndoCommand::undo() {
    // desagrupa
    _myGraphicsScene->ungroupModelComponents(_group);

    // atualiza a cena
    _myGraphicsScene->update();
}

void GroupUndoCommand::redo() {
    // agrupa
    _myGraphicsScene->groupModelComponents(&_myGraphicalComponents, _group);

    // seta a posicao do novo grupo criado para MoveUndo
    _myGraphicsScene->insertOldPositionItem(_group, _group->pos());

    // atualiza a cena
    _myGraphicsScene->update();
}
