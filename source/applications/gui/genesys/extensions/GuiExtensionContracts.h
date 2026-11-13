#ifndef GUIEXTENSIONCONTRACTS_H
#define GUIEXTENSIONCONTRACTS_H

#include <functional>
#include <string>
#include <vector>

class Simulator;
class QMainWindow;
class QWidget;
class QGraphicsItem;
class QMenu;
class ModelGraphicsView;
class ModelGraphicsScene;

namespace Ui {
class MainWindow;
}

struct GuiExtensionRuntimeContext {
	Simulator* simulator = nullptr;
	QMainWindow* mainWindow = nullptr;
	Ui::MainWindow* ui = nullptr;
	ModelGraphicsView* graphicsView = nullptr;
	ModelGraphicsScene* graphicsScene = nullptr;
};

struct GuiActionContribution {
	std::string id;
	std::string text;
	std::string menuPath;
	std::string toolBarId;
	std::string statusTip;
	std::string shortcut;
	std::function<bool(const GuiExtensionRuntimeContext&)> isVisible;
	std::function<bool(const GuiExtensionRuntimeContext&)> isEnabled;
	std::function<void(const GuiExtensionRuntimeContext&)> trigger;
};

struct GuiDockContribution {
	std::string id;
	std::string title;
	std::string areaHint;
	std::function<bool(const GuiExtensionRuntimeContext&)> isVisible;
	std::function<QWidget*(const GuiExtensionRuntimeContext&)> createWidget;
};

struct GuiWindowContribution {
	std::string id;
	std::string menuPath;
	std::string text;
	std::function<bool(const GuiExtensionRuntimeContext&)> isVisible;
	std::function<void(const GuiExtensionRuntimeContext&)> open;
};

// Formalism GUI extensions can add context-specific actions without teaching the
// generic scene controller about domain model classes.
struct GuiContextMenuContribution {
	std::string id;
	std::function<bool(const GuiExtensionRuntimeContext&, QGraphicsItem*)> isVisible;
	std::function<void(QMenu*, const GuiExtensionRuntimeContext&, QGraphicsItem*)> populate;
};

class GuiExtensionRegistry {
public:
	void addAction(GuiActionContribution contribution) {
		_actions.push_back(std::move(contribution));
	}

	void addDock(GuiDockContribution contribution) {
		_docks.push_back(std::move(contribution));
	}

	void addWindow(GuiWindowContribution contribution) {
		_windows.push_back(std::move(contribution));
	}

	void addContextMenu(GuiContextMenuContribution contribution) {
		_contextMenus.push_back(std::move(contribution));
	}

	const std::vector<GuiActionContribution>& actions() const {
		return _actions;
	}

	const std::vector<GuiDockContribution>& docks() const {
		return _docks;
	}

	const std::vector<GuiWindowContribution>& windows() const {
		return _windows;
	}

	const std::vector<GuiContextMenuContribution>& contextMenus() const {
		return _contextMenus;
	}

	void clear() {
		_actions.clear();
		_docks.clear();
		_windows.clear();
		_contextMenus.clear();
	}

private:
	std::vector<GuiActionContribution> _actions;
	std::vector<GuiDockContribution> _docks;
	std::vector<GuiWindowContribution> _windows;
	std::vector<GuiContextMenuContribution> _contextMenus;
};

class GuiExtensionPlugin {
public:
	virtual ~GuiExtensionPlugin() = default;

	virtual std::string extensionId() const = 0;
	virtual std::string displayName() const = 0;
	virtual std::string version() const = 0;
	virtual std::vector<std::string> requiredModelPlugins() const = 0;
	virtual void registerContributions(GuiExtensionRegistry* registry) const = 0;
};

#endif /* GUIEXTENSIONCONTRACTS_H */
