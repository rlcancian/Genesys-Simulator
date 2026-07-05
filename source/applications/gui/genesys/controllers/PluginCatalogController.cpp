#include "PluginCatalogController.h"

#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"

#include <QBrush>
#include <QAction>
#include <QFont>
#include <QIcon>
#include <QPlainTextEdit>
#include <QMenu>
#include <QPoint>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <algorithm>

namespace {

constexpr int kCategoryPathRole = Qt::UserRole + 1;
constexpr int kItemKindRole = Qt::UserRole + 2;
constexpr int kItemKindCategory = 1;
constexpr int kItemKindPlugin = 2;

QString canonicalCategoryPath(const QString& categoryPath) {
    QString normalized = categoryPath.trimmed();
    if (normalized.isEmpty()) {
        normalized = QStringLiteral("Uncategorized");
    }
    const QStringList segments = normalized.split('/', Qt::SkipEmptyParts);
    return segments.isEmpty() ? QStringLiteral("Uncategorized") : segments.join(QStringLiteral("/"));
}

QString categoryPrefix(const QStringList& segments, int lastIndex) {
    QString path;
    for (int index = 0; index <= lastIndex && index < segments.size(); ++index) {
        if (!path.isEmpty()) {
            path += '/';
        }
        path += segments.at(index).trimmed();
    }
    return path;
}

} // namespace

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

    const QString categoryPath = canonicalCategoryPath(QString::fromStdString(plugin->getPluginInfo()->getCategory()));
    const QString topLevelCategoryName = topLevelCategory(categoryPath);
    rememberTopLevelCategory(topLevelCategoryName);

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

    QTreeWidgetItem* treeRootItem = ensureCategoryPath(categoryPath);
    if (treeRootItem == nullptr) {
        delete treeItemChild;
        return;
    }
    treeItemChild->setData(0, Qt::UserRole, treeRootItem->background(0).color());
    treeItemChild->setData(0, kCategoryPathRole, categoryPath);
    treeItemChild->setData(0, kItemKindRole, kItemKindPlugin);

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
    treeRootItem->setExpanded(true);
}

void PluginCatalogController::reloadFromPluginManager() const {
    if (_pluginsTree == nullptr || _simulator == nullptr || _simulator->getPluginManager() == nullptr) {
        return;
    }

    // Rebuild the palette from kernel plugin state after plugin-manager mutations.
    _pluginsTree->clear();
    for (unsigned int i = 0; i < _simulator->getPluginManager()->size(); i++) {
        insertPluginUI(_simulator->getPluginManager()->getAtRank(i));
    }
    applyTopLevelCategoryOrder();
    applyCategoryExpansionPolicy();
}

void PluginCatalogController::applyCategoryExpansionPolicy() const {
    if (_pluginsTree == nullptr) {
        return;
    }
    for (int i = 0; i < _pluginsTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* root = _pluginsTree->topLevelItem(i);
        if (root != nullptr) {
            applyExpansionPolicyRecursive(root);
        }
    }
}

void PluginCatalogController::showContextMenu(const QPoint& pos) const {
    if (_pluginsTree == nullptr) {
        return;
    }

    QTreeWidgetItem* item = _pluginsTree->itemAt(pos);
    if (item == nullptr || item->parent() != nullptr) {
        return;
    }

    QMenu menu(_pluginsTree);
    QAction* moveUp = menu.addAction(QObject::tr("Move Category Up"));
    QAction* moveDown = menu.addAction(QObject::tr("Move Category Down"));
    const int index = _pluginsTree->indexOfTopLevelItem(item);
    moveUp->setEnabled(index > 0);
    moveDown->setEnabled(index >= 0 && index < _pluginsTree->topLevelItemCount() - 1);

    QObject::connect(moveUp, &QAction::triggered, _pluginsTree, [this, item]() {
        moveTopLevelCategory(item, -1);
    });
    QObject::connect(moveDown, &QAction::triggered, _pluginsTree, [this, item]() {
        moveTopLevelCategory(item, +1);
    });

    menu.exec(_pluginsTree->viewport()->mapToGlobal(pos));
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
    if (item->parent() != nullptr) {
        _pluginsTree->expandItem(item);
    }
    applyCategoryExpansionPolicy();
}

// Keep single-click handler as a no-op to preserve current behavior.
void PluginCatalogController::handlePluginItemClicked(QTreeWidgetItem* item, int column) const {
    Q_UNUSED(item);
    Q_UNUSED(column);
}

void PluginCatalogController::moveTopLevelCategory(QTreeWidgetItem* item, int delta) const {
    if (_pluginsTree == nullptr) {
        return;
    }
    if (item == nullptr || item->parent() != nullptr || delta == 0) {
        return;
    }

    const int currentIndex = _pluginsTree->indexOfTopLevelItem(item);
    if (currentIndex < 0) {
        return;
    }

    const int newIndex = std::clamp(currentIndex + delta, 0, _pluginsTree->topLevelItemCount() - 1);
    if (newIndex == currentIndex) {
        return;
    }

    const bool expanded = item->isExpanded();
    QTreeWidgetItem* moved = _pluginsTree->takeTopLevelItem(currentIndex);
    _pluginsTree->insertTopLevelItem(newIndex, moved);
    moved->setExpanded(expanded);
    _pluginsTree->setCurrentItem(moved);

    const QString movedName = moved->text(0);
    const std::string movedNameStd = movedName.toStdString();
    auto it = std::find(_topLevelCategoryOrder.begin(), _topLevelCategoryOrder.end(), movedNameStd);
    if (it != _topLevelCategoryOrder.end()) {
        _topLevelCategoryOrder.erase(it);
        _topLevelCategoryOrder.insert(_topLevelCategoryOrder.begin() + newIndex, movedNameStd);
    }
}

QString PluginCatalogController::topLevelCategory(const QString& categoryPath) {
    const QString normalized = canonicalCategoryPath(categoryPath);
    const int slashIndex = normalized.indexOf('/');
    return slashIndex < 0 ? normalized : normalized.left(slashIndex);
}

QTreeWidgetItem* PluginCatalogController::findDirectChild(QTreeWidgetItem* parent, const QString& text) const {
    if (parent == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem* child = parent->child(i);
        if (child != nullptr && child->text(0) == text) {
            return child;
        }
    }
    return nullptr;
}

QTreeWidgetItem* PluginCatalogController::ensureCategoryChild(QTreeWidgetItem* parent, const QString& segment) const {
    QTreeWidgetItem* existing = findDirectChild(parent, segment);
    if (existing != nullptr) {
        return existing;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setText(0, segment);
    item->setForeground(0, QBrush(Qt::white));
    item->setFont(0, QFont("Nimbus Sans", 12, QFont::Bold));
    item->setData(0, kItemKindRole, kItemKindCategory);
    return item;
}

QTreeWidgetItem* PluginCatalogController::ensureCategoryPath(const QString& categoryPath) const {
    if (_pluginsTree == nullptr) {
        return nullptr;
    }

    const QString normalizedPath = canonicalCategoryPath(categoryPath);
    const QStringList segments = normalizedPath.split('/', Qt::SkipEmptyParts);
    QTreeWidgetItem* current = nullptr;
    QString pathSoFar;
    for (int i = 0; i < segments.size(); ++i) {
        if (!pathSoFar.isEmpty()) {
            pathSoFar += '/';
        }
        pathSoFar += segments.at(i).trimmed();

        QTreeWidgetItem* next = nullptr;
        if (current == nullptr) {
            for (int topIndex = 0; topIndex < _pluginsTree->topLevelItemCount(); ++topIndex) {
                QTreeWidgetItem* topItem = _pluginsTree->topLevelItem(topIndex);
                if (topItem != nullptr
                        && topItem->data(0, kItemKindRole).toInt() == kItemKindCategory
                        && topItem->text(0) == segments.at(i)) {
                    next = topItem;
                    break;
                }
            }
            if (next == nullptr) {
                next = new QTreeWidgetItem(_pluginsTree);
                next->setText(0, segments.at(i).trimmed());
                next->setForeground(0, QBrush(Qt::white));
                next->setFont(0, QFont("Nimbus Sans", 12, QFont::Bold));
                next->setData(0, kItemKindRole, kItemKindCategory);
            }
            rememberTopLevelCategory(segments.at(i).trimmed());
        } else {
            next = ensureCategoryChild(current, segments.at(i).trimmed());
        }

        if (next == nullptr) {
            return nullptr;
        }
        next->setBackground(0, backgroundColorForCategory(pathSoFar));
        next->setData(0, kCategoryPathRole, pathSoFar);
        next->setExpanded(true);
        registerCategoryColorPrefixes(pathSoFar, next->background(0).color());
        current = next;
    }
    return current;
}

QColor PluginCatalogController::backgroundColorForCategory(const QString& categoryPath) const {
    const QString top = topLevelCategory(categoryPath);
    if (top == QStringLiteral("Logic")) {
        return QColor(Qt::darkRed);
    }
    if (top == QStringLiteral("Grouping")) {
        return QColor(Qt::magenta);
    }
    if (top == QStringLiteral("InputOutput")) {
        return QColor(Qt::darkCyan);
    }
    if (top == QStringLiteral("MaterialHandling")) {
        return QColor(Qt::darkBlue);
    }
    if (top == QStringLiteral("Decisions")) {
        return QColor(Qt::darkYellow);
    }
    if (top == QStringLiteral("Continuous")) {
        return QColor(Qt::darkGreen);
    }
    if (top == QStringLiteral("ModalModel")) {
        return QColor(Qt::darkBlue);
    }
    if (top == QStringLiteral("Synchronization")) {
        return QColor(Qt::darkGreen);
    }
    if (top == QStringLiteral("BiochemicalSimulation")) {
        return QColor(Qt::darkMagenta);
    }
    if (top == QStringLiteral("ElectronicsSimulation")) {
        return QColor(Qt::darkYellow);
    }
    if (top == QStringLiteral("ExternalIntegration")) {
        return QColor(Qt::darkCyan);
    }
    if (top == QStringLiteral("BiologicalModeling")) {
        return QColor(Qt::darkGreen);
    }
    if (top == QStringLiteral("AnalyticalModeling")) {
        return QColor(Qt::darkRed);
    }
    if (top == QStringLiteral("Template")) {
        return QColor(Qt::darkGray);
    }
    if (top == QStringLiteral("Uncategorized")) {
        return QColor(Qt::black);
    }
    return QColor(Qt::darkGray);
}

void PluginCatalogController::rememberTopLevelCategory(const QString& topLevelCategory) const {
    const std::string key = topLevelCategory.toStdString();
    if (std::find(_topLevelCategoryOrder.begin(), _topLevelCategoryOrder.end(), key) == _topLevelCategoryOrder.end()) {
        _topLevelCategoryOrder.push_back(key);
    }
}

void PluginCatalogController::applyTopLevelCategoryOrder() const {
    if (_pluginsTree == nullptr) {
        return;
    }

    int targetIndex = 0;
    for (const std::string& categoryName : _topLevelCategoryOrder) {
        for (int currentIndex = targetIndex; currentIndex < _pluginsTree->topLevelItemCount(); ++currentIndex) {
            QTreeWidgetItem* item = _pluginsTree->topLevelItem(currentIndex);
            if (item != nullptr && item->text(0).toStdString() == categoryName) {
                if (currentIndex != targetIndex) {
                    QTreeWidgetItem* moved = _pluginsTree->takeTopLevelItem(currentIndex);
                    _pluginsTree->insertTopLevelItem(targetIndex, moved);
                }
                ++targetIndex;
                break;
            }
        }
    }
}

void PluginCatalogController::applyExpansionPolicyRecursive(QTreeWidgetItem* item) const {
    if (item == nullptr) {
        return;
    }
    if (item->data(0, kItemKindRole).toInt() == kItemKindCategory) {
        item->setExpanded(true);
    }
    for (int i = 0; i < item->childCount(); ++i) {
        applyExpansionPolicyRecursive(item->child(i));
    }
}

void PluginCatalogController::registerCategoryColorPrefixes(const QString& categoryPath, const QColor& categoryColor) const {
    if (_pluginCategoryColor == nullptr) {
        return;
    }

    const QStringList segments = canonicalCategoryPath(categoryPath).split('/', Qt::SkipEmptyParts);
    for (int i = 0; i < segments.size(); ++i) {
        const QString prefix = categoryPrefix(segments, i);
        _pluginCategoryColor->insert({prefix.toStdString(), categoryColor});
    }
}
