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

// Document edit command orchestration extracted from MainWindow wrappers.
/**
 * @brief Controller for edit/clipboard/grouping command orchestration in the scene.
 *
 * This Phase-9 controller keeps MainWindow slots as compatibility wrappers while centralizing
 * clipboard buffers, grouping operations, and undo-aware edit command dispatch against the
 * current graphical scene.
 *
 * Responsibilities:
 * - execute undo/redo/cut/copy/paste/delete/group/ungroup command flows;
 * - preserve legacy clipboard structures used by graphical components and drawings;
 * - trigger action refresh callbacks after command execution.
 *
 * Boundaries:
 * - it does not own scene lifetime (resolved from injected graphics view);
 * - it does not persist clipboard state across sessions;
 * - it does not manage simulation events, model lifecycle, or property-editor synchronization.
 */
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
