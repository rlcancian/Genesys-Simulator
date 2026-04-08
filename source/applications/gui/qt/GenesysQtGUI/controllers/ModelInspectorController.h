#ifndef MODELINSPECTORCONTROLLER_H
#define MODELINSPECTORCONTROLLER_H

#include <QTreeWidget>

class Simulator;
class ModelGraphicsView;

// Encapsulate Phase 3 model-inspection and tree/scene synchronization logic.
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
