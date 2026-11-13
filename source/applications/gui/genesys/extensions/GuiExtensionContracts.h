#ifndef GUIEXTENSIONCONTRACTS_H
#define GUIEXTENSIONCONTRACTS_H

#include <functional>
#include <any>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <QStringList>

class Simulator;
class QMainWindow;
class QWidget;
class QGraphicsItem;
class QMenu;
class QUndoCommand;
class ModelGraphicsView;
class ModelGraphicsScene;
class Model;
class ModelDataDefinition;
class Plugin;

using GuiDiagramLayouts = std::map<unsigned long, std::any>;
using GuiDiagramLayoutsByExtension = std::unordered_map<std::string, GuiDiagramLayouts>;
using GuiDiagramLayoutsByModel = std::unordered_map<Model*, GuiDiagramLayoutsByExtension>;
using GuiUnknownDiagramSections = std::map<QString, QStringList>;
using GuiUnknownDiagramSectionsByModel = std::unordered_map<Model*, GuiUnknownDiagramSections>;

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

struct GuiDiagramProjectionContribution {
	std::string id;
	std::string sectionName;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<void(Simulator*, ModelGraphicsScene*)> synchronize;
	std::function<std::any(Simulator*, ModelDataDefinition*, ModelGraphicsScene*)> captureLayout;
	std::function<void(Simulator*, ModelDataDefinition*, ModelGraphicsScene*, const std::any&)> restoreLayout;
	std::function<QStringList(Simulator*, const GuiDiagramLayouts&)> serializeLayouts;
	std::function<GuiDiagramLayouts(Model*, const QStringList&)> parseLayouts;
};

struct GuiNetworkEditorContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<bool(ModelDataDefinition*, ModelDataDefinition*)> ownsEditableDataDefinition;
	std::function<QUndoCommand*(ModelDataDefinition*, ModelDataDefinition*, const std::string&, const std::string&)> createDataDefinitionPropertyCommand;
	// Builds a real domain relation between two selected graphical nodes.
	std::function<bool(ModelDataDefinition*, QGraphicsItem*)> canStartRelationCreation;
	std::function<QUndoCommand*(ModelDataDefinition*, QGraphicsItem*, QGraphicsItem*)> createRelationCommand;
	struct ElementProperty {
		enum class Kind { String, Boolean, Integer, Double, Enumeration };
		std::string name;
		std::string value;
		std::vector<std::string> choices;
		Kind kind = Kind::String;
		bool editable = false;
	};
	std::function<std::vector<ElementProperty>(ModelDataDefinition*, QGraphicsItem*)> elementProperties;
	std::function<QUndoCommand*(ModelDataDefinition*, QGraphicsItem*, const std::string&, const std::string&)>
		createElementPropertyCommand;
	std::function<bool(ModelDataDefinition*, const std::string&, bool)> acceptsDrop;
	std::function<void(ModelDataDefinition*, ModelDataDefinition*)> onDataDefinitionDropped;
	std::function<void(ModelDataDefinition*, ModelDataDefinition*)> onDataDefinitionRemoved;
};

struct GuiSimulationStartContribution {
    std::string id;
    std::function<void(Simulator*, QMainWindow*)> onSimulationStart;
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

	void addDiagramProjection(GuiDiagramProjectionContribution contribution) {
		_diagramProjections.push_back(std::move(contribution));
	}

	void addSimulationStart(GuiSimulationStartContribution contribution) {
		_simulationStarts.push_back(std::move(contribution));
	}

	void addNetworkEditor(GuiNetworkEditorContribution contribution) {
		_networkEditors.push_back(std::move(contribution));
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

	const std::vector<GuiDiagramProjectionContribution>& diagramProjections() const {
		return _diagramProjections;
	}

	const std::vector<GuiSimulationStartContribution>& simulationStarts() const {
		return _simulationStarts;
	}

	const std::vector<GuiNetworkEditorContribution>& networkEditors() const {
		return _networkEditors;
	}

	void clear() {
		_actions.clear();
		_docks.clear();
		_windows.clear();
		_contextMenus.clear();
		_diagramProjections.clear();
		_networkEditors.clear();
		_simulationStarts.clear();
	}

private:
	std::vector<GuiActionContribution> _actions;
	std::vector<GuiDockContribution> _docks;
	std::vector<GuiWindowContribution> _windows;
	std::vector<GuiContextMenuContribution> _contextMenus;
	std::vector<GuiDiagramProjectionContribution> _diagramProjections;
	std::vector<GuiNetworkEditorContribution> _networkEditors;
	std::vector<GuiSimulationStartContribution> _simulationStarts;
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
