#include "ModelInspectorController.h"

#include "graphicals/ModelGraphicsView.h"

#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalModelComponent.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/util/Util.h"

#include <Qt>

// Build the controller with narrow Qt/kernel dependencies for Phase 3.
ModelInspectorController::ModelInspectorController(Simulator* simulator,
                                                   QTreeWidget* componentsTree,
                                                   QTreeWidget* dataDefinitionsTree,
                                                   ModelGraphicsView* graphicsView)
    : _simulator(simulator),
      _componentsTree(componentsTree),
      _dataDefinitionsTree(dataDefinitionsTree),
      _graphicsView(graphicsView) {
}

// Keep the Components tree actualization behavior equivalent to legacy MainWindow logic.
void ModelInspectorController::actualizeModelComponents(bool force) const {
    Q_UNUSED(force)

    Model* m = _simulator->getModelManager()->current();
    _componentsTree->clear();
    if (m == nullptr) {
        return;
    }

    for (ModelComponent* comp : *m->getComponentManager()->getAllComponents()) {
        QList<QTreeWidgetItem*> items = _componentsTree->findItems(
            QString::fromStdString(std::to_string(comp->getId())),
            Qt::MatchExactly | Qt::MatchRecursive,
            0);
        if (items.size() == 0) {
            QTreeWidgetItem* treeComp = new QTreeWidgetItem(_componentsTree);
            treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
            treeComp->setText(1, QString::fromStdString(comp->getClassname()));
            treeComp->setText(2, QString::fromStdString(comp->getName()));
            std::string properties = "";
            for (auto prop : *comp->getProperties()->list()) {
                properties += prop->getName() + ":" + prop->getValue() + ", ";
            }
            properties = properties.substr(0, properties.length() - 2);
            treeComp->setText(3, QString::fromStdString(properties));
        }
    }

    _componentsTree->resizeColumnToContents(0);
    _componentsTree->resizeColumnToContents(1);
    _componentsTree->resizeColumnToContents(2);
}

// Keep the Data Definitions tree actualization behavior equivalent to legacy MainWindow logic.
void ModelInspectorController::actualizeModelDataDefinitions(bool force) const {
    Q_UNUSED(force)

    Model* m = _simulator->getModelManager()->current();
    _dataDefinitionsTree->clear();
    if (m == nullptr) {
        return;
    }

    // Iterate over a value snapshot of class names to populate the tree without manual ownership handling.
    for (std::string dataTypename : m->getDataManager()->getDataDefinitionClassnames()) {
        for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {
            QList<QTreeWidgetItem*> items = _dataDefinitionsTree->findItems(
                QString::fromStdString(std::to_string(comp->getId())),
                Qt::MatchExactly | Qt::MatchRecursive,
                0);
            if (items.size() == 0) {
                QTreeWidgetItem* treeComp = new QTreeWidgetItem(_dataDefinitionsTree);
                treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
                treeComp->setText(1, QString::fromStdString(comp->getClassname()));
                treeComp->setText(2, QString::fromStdString(comp->getName()));
                std::string properties = "";
                for (auto prop : *comp->getProperties()->list()) {
                    properties += prop->getName() + ":" + prop->getValue() + ", ";
                }
                properties = properties.substr(0, properties.length() - 2);
                treeComp->setText(3, QString::fromStdString(properties));
            }
        }
    }

    _dataDefinitionsTree->resizeColumnToContents(0);
    _dataDefinitionsTree->resizeColumnToContents(1);
    _dataDefinitionsTree->resizeColumnToContents(2);
}

// Preserve rename-start behavior by enabling editability only for the name column.
void ModelInspectorController::beginDataDefinitionNameEdit(QTreeWidgetItem* item, int column) const {
    if (item == nullptr) {
        return;
    }

    if (column == 2) {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        _dataDefinitionsTree->editItem(item, column);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

// Preserve rename-apply behavior by updating matched data-definition names in the model.
void ModelInspectorController::applyDataDefinitionNameChange(QTreeWidgetItem* item, int column) const {
    if (item == nullptr) {
        return;
    }

    if (column == 2) {
        QString after = item->text(column);
        Model* m = _simulator->getModelManager()->current();
        if (m == nullptr) {
            return;
        }

        // Iterate over a value snapshot of class names when applying renames to matching data definitions.
        for (std::string dataTypename : m->getDataManager()->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {
                QString id = QString::fromStdString(Util::StrIndex(comp->getId()));
                if (id.contains(item->text(0))) {
                    comp->setName(after.toStdString());
                }
            }
        }
    }
}

// Preserve tree-to-scene selection synchronization and viewport centering behavior.
void ModelInspectorController::syncSelectedComponentTreeItemToScene() const {
    QList<QTreeWidgetItem*> selectedItems = _componentsTree->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    bool ok = false;
    const Util::identification compId = selectedItems.first()->text(0).toULongLong(&ok);
    if (!ok) {
        return;
    }

    ModelGraphicsScene* scene = _graphicsView->getScene();
    if (scene == nullptr) {
        return;
    }

    GraphicalModelComponent* gmc = scene->findGraphicalModelComponent(compId);
    if (gmc == nullptr) {
        return;
    }

    if (scene->selectedItems().size() == 1 && scene->selectedItems().first() == gmc) {
        _graphicsView->ensureVisible(gmc);
        return;
    }

    scene->clearSelection();
    gmc->setSelected(true);
    _graphicsView->ensureVisible(gmc);
    _graphicsView->centerOn(gmc);
}
