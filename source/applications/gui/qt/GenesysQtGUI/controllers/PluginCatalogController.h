#ifndef PLUGINCATALOGCONTROLLER_H
#define PLUGINCATALOGCONTROLLER_H

#include <map>
#include <string>

#include <QColor>

class Plugin;
class Simulator;
class QPlainTextEdit;
class QString;
class QTreeWidget;
class QTreeWidgetItem;

// Document the plugin-catalog controller extracted from MainWindow UI code.
/**
 * @brief Controller responsible for plugin catalog presentation and plugin palette behavior.
 *
 * This controller encapsulates the plugin-tree behavior used by MainWindow wrappers, keeping
 * plugin browsing and insertion affordances consistent while reducing direct widget logic in
 * the MainWindow compatibility façade.
 *
 * Responsibilities:
 * - populate plugin entries grouped by category with legacy color conventions;
 * - provide compatibility helpers for fake-plugin insertion paths;
 * - react to plugin-tree click/double-click events delegated by MainWindow.
 *
 * Boundaries:
 * - it does not load plugin binaries or manage kernel plugin lifecycle;
 * - it does not own textual model synchronization beyond delegated editor insertions;
 * - it is a UI controller layer, not a persistence/export service.
 */
class PluginCatalogController {
public:
    /** @brief Creates the plugin-catalog controller used by MainWindow compatibility slots. */
    PluginCatalogController(Simulator* simulator,
                            QTreeWidget* pluginsTree,
                            QPlainTextEdit* modelTextEditor,
                            std::map<std::string, QColor>* pluginCategoryColor);

    /** @brief Inserts one plugin entry into the categorized plugin tree representation. */
    void insertPluginUI(Plugin* plugin) const;
    /** @brief Rebuilds the categorized plugin tree from the current PluginManager state. */
    void reloadFromPluginManager() const;
    /** @brief Inserts compatibility fake plugins used by legacy GUI flows. */
    void insertFakePlugins() const;
    /** @brief Handles plugin double-click by delegating insertion/selection behavior. */
    void handlePluginItemDoubleClicked(QTreeWidgetItem* item, int column) const;
    /** @brief Handles plugin single-click behavior without mutating model lifecycle state. */
    void handlePluginItemClicked(QTreeWidgetItem* item, int column) const;

private:
    // Resolve or create the category root item with legacy styling and colors.
    QTreeWidgetItem* ensureCategoryRoot(const QString& category, const std::string& pluginCategoryName) const;

private:
    Simulator* _simulator;
    QTreeWidget* _pluginsTree;
    QPlainTextEdit* _modelTextEditor;
    std::map<std::string, QColor>* _pluginCategoryColor;
};

#endif /* PLUGINCATALOGCONTROLLER_H */
