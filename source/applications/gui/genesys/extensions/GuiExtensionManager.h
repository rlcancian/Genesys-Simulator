#ifndef GUIEXTENSIONMANAGER_H
#define GUIEXTENSIONMANAGER_H

#include "GuiExtensionContracts.h"

#include <string>
#include <utility>
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
	void refreshActionStates(const GuiExtensionRuntimeContext& context);
	void populateContextMenu(QMenu* menu, QGraphicsItem* item) const;
	void reconcilePropertyListChange(ModelDataDefinition* owner, ModelGraphicsScene* scene) const;
	bool reconcilePropertyListMove(ModelDataDefinition* owner, ModelGraphicsScene* scene, int from, int to) const;
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
	std::vector<std::pair<QAction*, GuiActionContribution>> _actionContributions;
	std::vector<QAction*> _createdMenuActions;
	std::vector<QDockWidget*> _createdDocks;
	std::vector<GuiContextMenuContribution> _contextMenuContributions;
	std::vector<GuiPropertyListContribution> _propertyListContributions;
	GuiExtensionRuntimeContext _runtimeContext;
};

#endif /* GUIEXTENSIONMANAGER_H */
