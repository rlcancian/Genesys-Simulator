#ifndef MODELINSPECTORCONTROLLER_H
#define MODELINSPECTORCONTROLLER_H

#include <QTreeWidget>

class Simulator;
class ModelGraphicsView;

// Document the controller boundary for model-inspection and tree/scene synchronization.
/**
 * @brief Controller for model inspection views and scene-selection synchronization.
 *
 * In the refactored GUI architecture, this controller receives delegated callbacks from
 * MainWindow to keep model-inspector responsibilities outside the composition root while
 * preserving the existing UI contract. It synchronizes Components/Data Definitions trees
 * with the current kernel model and keeps tree-to-scene selection behavior consistent.
 *
 * Responsibilities:
 * - refresh model component and data-definition trees from Simulator state;
 * - manage in-place rename flow for editable data-definition names;
 * - propagate current tree selection to the graphical scene and viewport.
 *
 * Boundaries:
 * - it does not create/destroy models or files (model lifecycle remains elsewhere);
 * - it does not own scene command logic, undo/redo, or simulation commands;
 * - it does not act as a persistence or export service.
 */
class ModelInspectorController {
public:
    /** @brief Creates the model-inspector delegated controller. */
    ModelInspectorController(Simulator* simulator,
                             QTreeWidget* componentsTree,
                             QTreeWidget* dataDefinitionsTree,
                             ModelGraphicsView* graphicsView);

    /** @brief Synchronizes the Components tree from the current model representation. */
    void actualizeModelComponents(bool force) const;
    /** @brief Synchronizes the Data Definitions tree from current model data definitions. */
    void actualizeModelDataDefinitions(bool force) const;
    /** @brief Starts in-place rename editing for editable data-definition entries. */
    void beginDataDefinitionNameEdit(QTreeWidgetItem* item, int column) const;
    /** @brief Commits edited data-definition names back to the kernel model. */
    void applyDataDefinitionNameChange(QTreeWidgetItem* item, int column) const;
    /** @brief Synchronizes tree selection with scene selection and viewport focus. */
    void syncSelectedComponentTreeItemToScene() const;

private:
    // Keep simulator access scoped to model-inspection responsibilities.
    Simulator* _simulator;
    // Keep direct access to the Components tree used by this controller.
    QTreeWidget* _componentsTree;
    // Keep direct access to the Data Definitions tree used by this controller.
    QTreeWidget* _dataDefinitionsTree;
    // Keep direct access to the graphics view for selection synchronization.
    ModelGraphicsView* _graphicsView;
};

#endif // MODELINSPECTORCONTROLLER_H
