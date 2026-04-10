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
    // Inject only the narrow dependencies required by the model-inspector workflow.
    ModelInspectorController(Simulator* simulator,
                             QTreeWidget* componentsTree,
                             QTreeWidget* dataDefinitionsTree,
                             ModelGraphicsView* graphicsView);

    // Synchronize the Components tree with the current model state.
    void actualizeModelComponents(bool force) const;
    // Synchronize the Data Definitions tree with the current model state.
    void actualizeModelDataDefinitions(bool force) const;
    // Start in-place edition for a data-definition name when column is editable.
    void beginDataDefinitionNameEdit(QTreeWidgetItem* item, int column) const;
    // Apply renamed data-definition names back into the current model.
    void applyDataDefinitionNameChange(QTreeWidgetItem* item, int column) const;
    // Synchronize tree selection with graphical scene selection and viewport.
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
