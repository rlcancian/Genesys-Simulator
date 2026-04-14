#include "PluginCatalogController.h"

#include "../../../../../kernel/simulator/Plugin.h"
#include "../../../../../kernel/simulator/PluginInformation.h"
#include "../../../../../kernel/simulator/Simulator.h"

#include <QBrush>
#include <QFont>
#include <QIcon>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Store narrow dependencies for Phase 5 plugin-catalog delegation.
PluginCatalogController::PluginCatalogController(Simulator* simulator,
                                                 QTreeWidget* pluginsTree,
                                                 QPlainTextEdit* modelTextEditor,
                                                 std::map<std::string, QColor>* pluginCategoryColor)
    : _simulator(simulator),
      _pluginsTree(pluginsTree),
      _modelTextEditor(modelTextEditor),
      _pluginCategoryColor(pluginCategoryColor) {
}

// Preserve legacy plugin insertion behavior while keeping MainWindow thin.
void PluginCatalogController::insertPluginUI(Plugin* plugin) const {
    if (plugin == nullptr || !plugin->isIsValidPlugin() || _pluginsTree == nullptr) {
        return;
    }

    // Keep the legacy plugin metadata formatting for tooltip/status text.
    QTreeWidgetItem* treeItemChild = new QTreeWidgetItem();
    std::string plugtextAdds = "[" + plugin->getPluginInfo()->getCategory() + "]: ";
    if (plugin->getPluginInfo()->isComponent()) {
        plugtextAdds += " Component";
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/component.ico"));
    } else {
        plugtextAdds += " DataDefinition";
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/calendarred.ico"));
    }
    if (plugin->getPluginInfo()->isSink()) {
        plugtextAdds += ", Sink";
        treeItemChild->setForeground(0, Qt::blue);
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
    }
    if (plugin->getPluginInfo()->isSource()) {
        plugtextAdds += ", Source";
        treeItemChild->setForeground(0, Qt::blue);
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
    }
    if (plugin->getPluginInfo()->isReceiveTransfer()) {
        plugtextAdds += ", ReceiveTransfer";
        treeItemChild->setForeground(0, Qt::blue);
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
    }
    if (plugin->getPluginInfo()->isSendTransfer()) {
        plugtextAdds += ", SendTransfer";
        treeItemChild->setForeground(0, Qt::blue);
        treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
    }
    plugtextAdds += "\n\nDescrption: " + plugin->getPluginInfo()->getDescriptionHelp();
    plugtextAdds += "\n\nTemplate: " + plugin->getPluginInfo()->getLanguageTemplate() + " (double click to add to model)";

    // Preserve the component/data-definition category routing logic.
    QString category = plugin->getPluginInfo()->isComponent()
            ? QString::fromStdString(plugin->getPluginInfo()->getCategory())
            : "Data Definition";
    QTreeWidgetItem* treeRootItem = ensureCategoryRoot(category, plugin->getPluginInfo()->getCategory());
    if (treeRootItem == nullptr) {
        delete treeItemChild;
        return;
    }
    treeItemChild->setData(0, Qt::UserRole, treeRootItem->background(0).color());

    // Keep legacy icon color selection based on root category color.
    if (plugin->getPluginInfo()->isComponent()
            && !plugin->getPluginInfo()->isSendTransfer()
            && !plugin->getPluginInfo()->isReceiveTransfer()
            && !plugin->getPluginInfo()->isSink()
            && !plugin->getPluginInfo()->isSource()) {
        const QColor rootColor = treeRootItem->background(0).color();
        if (rootColor.blue() < 32 && rootColor.green() < 32 && rootColor.red() < 32) {
            treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblack.ico"));
        } else if (rootColor.red() >= rootColor.blue() && rootColor.red() > rootColor.green()) {
            treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentred.ico"));
        } else if (rootColor.blue() > rootColor.red() && rootColor.blue() > rootColor.green()) {
            treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblue.ico"));
        } else if (rootColor.red() > rootColor.blue() && rootColor.green() > rootColor.blue()) {
            treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentyellow.ico"));
        }
    }

    // Keep plugin metadata fields used by drag/drop and tooltip/status interactions.
    treeItemChild->setWhatsThis(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
    treeItemChild->setText(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
    treeItemChild->setToolTip(0, QString::fromStdString(plugtextAdds));
    treeItemChild->setStatusTip(0, QString::fromStdString(plugin->getPluginInfo()->getLanguageTemplate()));
    treeRootItem->addChild(treeItemChild);
}

// Keep fake-plugin entry point available even when no fake plugins are inserted.
void PluginCatalogController::insertFakePlugins() const {
}

// Preserve plugin-tree expansion behavior for non-text-editing mode.
void PluginCatalogController::handlePluginItemDoubleClicked(QTreeWidgetItem* item, int column) const {
    Q_UNUSED(column);
    if (_pluginsTree == nullptr || item == nullptr) {
        return;
    }
    if (_modelTextEditor != nullptr && _modelTextEditor->isEnabled()) {
        return;
    }
    for (int i = 0; i < _pluginsTree->topLevelItemCount(); i++) {
        _pluginsTree->topLevelItem(i)->setExpanded(false);
    }
    _pluginsTree->expandItem(item);
}

// Keep single-click handler as a no-op to preserve current behavior.
void PluginCatalogController::handlePluginItemClicked(QTreeWidgetItem* item, int column) const {
    Q_UNUSED(item);
    Q_UNUSED(column);
}

// Create category roots with the same legacy names, colors, and map updates.
QTreeWidgetItem* PluginCatalogController::ensureCategoryRoot(const QString& category, const std::string& pluginCategoryName) const {
    if (_pluginsTree == nullptr) {
        return nullptr;
    }
    QList<QTreeWidgetItem*> founds = _pluginsTree->findItems(category, Qt::MatchContains);
    if (founds.size() != 0) {
        return *founds.begin();
    }

    // Keep the legacy category-root visual formatting.
    QTreeWidgetItem* treeRootItem = new QTreeWidgetItem(_pluginsTree);
    treeRootItem->setText(0, category);
    treeRootItem->setForeground(0, QBrush(Qt::white));
    QBrush bbackground(Qt::black);
    if (category == "Data Definition") {
        bbackground.setColor(Qt::darkRed);
    } else if (category == "Discrete Processing") {
        bbackground.setColor(Qt::darkGreen);
    } else if (category == "Decisions") {
        bbackground.setColor(Qt::darkYellow);
    } else if (category == "Grouping") {
        bbackground.setColor(Qt::magenta);
    } else if (category == "Input Output") {
        bbackground.setColor(Qt::darkCyan);
    } else if (category == "Material Handling") {
        bbackground.setColor(Qt::darkBlue);
    }
    treeRootItem->setBackground(0, bbackground);
    treeRootItem->setFont(0, QFont("Nimbus Sans", 12, QFont::Bold));
    treeRootItem->setExpanded(true);

    // Preserve category-color map updates for graphical-model integration.
    if (_pluginCategoryColor != nullptr) {
        const QColor categoryColor = bbackground.color();
        _pluginCategoryColor->insert({category.toStdString(), categoryColor});
        _pluginCategoryColor->insert({pluginCategoryName, categoryColor});
    }
    return treeRootItem;
}
