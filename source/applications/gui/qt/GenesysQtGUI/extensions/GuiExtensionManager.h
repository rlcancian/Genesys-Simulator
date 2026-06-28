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
class AnimationPlaceholder;
class ModelGraphicsScene;
class ModelComponent;

class GuiExtensionManager {
public:
	explicit GuiExtensionManager(QMainWindow* mainWindow);
	~GuiExtensionManager();

	void setPlugins(std::vector<const GuiExtensionPlugin*> plugins);
	void setLoadedModelPluginIds(std::vector<std::string> modelPluginIds);
	void rebuild(const GuiExtensionRuntimeContext& context);
	void clear();

	// Returns the animation contributions collected from all active plugins.
	const std::vector<GuiAnimationContribution>& animationContributions() const;

	// Dispatches a simulation animation event to the matching plugin handler.
	void dispatchAnimationEvent(
		const std::string& animationType,
		ModelGraphicsScene* scene,
		const GuiSimAnimationEvent& event) const;

	// Creates the placeholder for the given animationType using the registered factory.
	// Returns nullptr if no plugin registered a factory for that type.
	AnimationPlaceholder* createAnimationPlaceholder(const std::string& animationType) const;

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
	std::vector<GuiAnimationContribution> _animationContributions;
};

#endif /* GUIEXTENSIONMANAGER_H */
