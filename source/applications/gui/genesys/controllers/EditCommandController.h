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
    /** @brief Creates the edit-command controller used by MainWindow compatibility slots. */
    EditCommandController(Simulator* simulator,
                          ModelGraphicsView* graphicsView,
                          std::function<void()> actualizeActions,
                          bool* cut,
                          QList<GraphicalModelComponent*>** gmcCopies,
                          QList<GraphicalConnection*>** portsCopies,
                          QList<QGraphicsItem*>** drawCopy,
                          QList<QGraphicsItemGroup*>** groupCopy);

    /** @brief Delegates undo command handling without changing scene undo semantics. */
    void onActionEditUndoTriggered() const;
    /** @brief Delegates redo command handling without changing scene undo semantics. */
    void onActionEditRedoTriggered() const;
    /** @brief Delegates cut command handling while preserving compatibility copy buffers. */
    void onActionEditCutTriggered() const;
    /** @brief Delegates copy command handling while preserving compatibility copy buffers. */
    void onActionEditCopyTriggered() const;
    /** @brief Delegates paste command handling while preserving compatibility copy buffers. */
    void onActionEditPasteTriggered() const;
    /** @brief Delegates delete command handling while preserving undo integration. */
    void onActionEditDeleteTriggered() const;
    /** @brief Delegates group command handling through scene grouping APIs. */
    void onActionEditGroupTriggered() const;
    /** @brief Delegates ungroup command handling through scene grouping APIs. */
    void onActionEditUngroupTriggered() const;
    /** @brief Delegates view-group command handling through scene grouping APIs. */
    void onActionViewGroupTriggered() const;
    /** @brief Delegates view-ungroup command handling through scene grouping APIs. */
    void onActionViewUngroupTriggered() const;

    /** @brief Stores eligible selected items in compatibility clipboard structures. */
    void saveItemForCopy(QList<GraphicalModelComponent*>* gmcList, QList<GraphicalConnection*>* connList) const;
    /** @brief Performs deep-copy preparation for subsequent paste command execution. */
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
