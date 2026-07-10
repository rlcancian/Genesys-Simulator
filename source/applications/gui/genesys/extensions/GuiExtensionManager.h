#ifndef GUIEXTENSIONMANAGER_H
#define GUIEXTENSIONMANAGER_H

#include "GuiExtensionContracts.h"

#include <string>
#include <vector>

class QAction;
class QDockWidget;
class QMainWindow;
class QMenu;
class QToolBar;

class GuiExtensionManager {
public:
	explicit GuiExtensionManager(QMainWindow* mainWindow);
	~GuiExtensionManager();

	void setPlugins(std::vector<const GuiExtensionPlugin*> plugins);
	void setLoadedModelPluginIds(std::vector<std::string> modelPluginIds);
	void rebuild(const GuiExtensionRuntimeContext& context);
	void clear();

private:
	QMenu* _resolveMenuPath(const std::string& menuPath);
	QToolBar* _resolveToolBar(const std::string& toolBarId) const;
	void _applyActionContribution(const GuiActionContribution& contribution, const GuiExtensionRuntimeContext& context);
	void _applyWindowContribution(const GuiWindowContribution& contribution, const GuiExtensionRuntimeContext& context);
	void _applyDockContribution(const GuiDockContribution& contribution, const GuiExtensionRuntimeContext& context);
	bool _isPluginDependenciesSatisfied(const GuiExtensionPlugin* plugin) const;

private:
	QMainWindow* _mainWindow = nullptr;
	std::vector<const GuiExtensionPlugin*> _plugins;
	std::vector<std::string> _loadedModelPluginIds;
	std::vector<QAction*> _createdActions;
	std::vector<QAction*> _createdMenuActions;
	std::vector<QDockWidget*> _createdDocks;
};

#endif /* GUIEXTENSIONMANAGER_H */
