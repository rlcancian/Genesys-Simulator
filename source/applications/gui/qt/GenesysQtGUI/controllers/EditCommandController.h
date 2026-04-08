#ifndef EDITCOMMANDCONTROLLER_H
#define EDITCOMMANDCONTROLLER_H

#include <QList>
#include <QGraphicsItem>
#include <functional>

class Simulator;
class ModelGraphicsView;
class GraphicalModelComponent;
class GraphicalConnection;
class QGraphicsItemGroup;

// Phase 9 controller that centralizes edit/clipboard command orchestration.
class EditCommandController {
public:
    // Inject only narrow dependencies required by edit command behavior.
    EditCommandController(Simulator* simulator,
                          ModelGraphicsView* graphicsView,
                          std::function<void()> actualizeActions,
                          bool* cut,
                          QList<GraphicalModelComponent*>** gmcCopies,
                          QList<GraphicalConnection*>** portsCopies,
                          QList<QGraphicsItem*>** drawCopy,
                          QList<QGraphicsItemGroup*>** groupCopy);

    // Delegate undo command handling without changing scene undo semantics.
    void onActionEditUndoTriggered() const;
    // Delegate redo command handling without changing scene undo semantics.
    void onActionEditRedoTriggered() const;
    // Delegate cut command handling while preserving copy buffers.
    void onActionEditCutTriggered() const;
    // Delegate copy command handling while preserving copy buffers.
    void onActionEditCopyTriggered() const;
    // Delegate paste command handling while preserving copy buffers.
    void onActionEditPasteTriggered() const;
    // Delegate delete command handling while preserving undo integration.
    void onActionEditDeleteTriggered() const;
    // Delegate group command handling through scene grouping APIs.
    void onActionEditGroupTriggered() const;
    // Delegate ungroup command handling through scene grouping APIs.
    void onActionEditUngroupTriggered() const;
    // Delegate view-group command handling through scene grouping APIs.
    void onActionViewGroupTriggered() const;
    // Delegate view-ungroup command handling through scene grouping APIs.
    void onActionViewUngroupTriggered() const;

    // Keep clipboard filtering helper behavior used by copy/cut workflows.
    void saveItemForCopy(QList<GraphicalModelComponent*>* gmcList, QList<GraphicalConnection*>* connList) const;
    // Keep deep-copy helper behavior used by paste workflows.
    void helpCopy() const;

private:
    // Resolve current scene from the injected graphics view.
    class ModelGraphicsScene* scene() const;

private:
    Simulator* _simulator;
    ModelGraphicsView* _graphicsView;
    std::function<void()> _actualizeActions;
    bool* _cut;
    QList<GraphicalModelComponent*>** _gmcCopies;
    QList<GraphicalConnection*>** _portsCopies;
    QList<QGraphicsItem*>** _drawCopy;
    QList<QGraphicsItemGroup*>** _groupCopy;
};

#endif // EDITCOMMANDCONTROLLER_H
