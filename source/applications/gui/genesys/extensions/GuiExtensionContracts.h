#ifndef GUIEXTENSIONCONTRACTS_H
#define GUIEXTENSIONCONTRACTS_H

#include <functional>
#include <memory>
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
class ModelComponent;
class GraphicalModelComponent;
struct GraphicalModelItemRenderContext;
class ObjectPropertyBrowser;
class Plugin;

using GuiDiagramLayouts = std::map<unsigned long, std::any>;
using GuiDiagramLayoutsByExtension = std::unordered_map<std::string, GuiDiagramLayouts>;
using GuiDiagramLayoutsByModel = std::unordered_map<Model*, GuiDiagramLayoutsByExtension>;
using GuiUnknownDiagramSections = std::map<QString, QStringList>;
using GuiUnknownDiagramSectionsByModel = std::unordered_map<Model*, GuiUnknownDiagramSections>;

namespace Ui {
class MainWindow;
}

// Declarative GUI-only eligibility metadata. Legacy requiredModelPlugins() remains
// an ALL/AND group; these new groups are opt-in and do not own model plugins.
enum class GuiExtensionDependencyKind { ModelPlugin, GuiExtension, SystemCapability };
enum class GuiExtensionDependencyMode { All, Any };
struct GuiExtensionDependencyGroup {
    GuiExtensionDependencyKind kind = GuiExtensionDependencyKind::ModelPlugin;
    GuiExtensionDependencyMode mode = GuiExtensionDependencyMode::All;
    std::vector<std::string> ids;
    bool required = true;
    // Optional minimum numeric dotted version (e.g. 1.2.3), common to the group.
    // Unknown versions do not satisfy an explicitly versioned requirement.
    std::string minimumVersion;
};
using GuiExtensionDependencyGroups = std::vector<GuiExtensionDependencyGroup>;

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
	GuiExtensionDependencyGroups eligibility;
};

struct GuiDockContribution {
	std::string id;
	std::string title;
	std::string areaHint;
	std::function<bool(const GuiExtensionRuntimeContext&)> isVisible;
	std::function<QWidget*(const GuiExtensionRuntimeContext&)> createWidget;
	GuiExtensionDependencyGroups eligibility;
};

struct GuiWindowContribution {
	std::string id;
	std::string menuPath;
	std::string text;
	std::function<bool(const GuiExtensionRuntimeContext&)> isVisible;
	std::function<void(const GuiExtensionRuntimeContext&)> open;
	GuiExtensionDependencyGroups eligibility;
};

// Formalism GUI extensions can add context-specific actions without teaching the
// generic scene controller about domain model classes.
struct GuiContextMenuContribution {
	std::string id;
	std::function<bool(const GuiExtensionRuntimeContext&, QGraphicsItem*)> isVisible;
	std::function<void(QMenu*, const GuiExtensionRuntimeContext&, QGraphicsItem*)> populate;
	GuiExtensionDependencyGroups eligibility;
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
	GuiExtensionDependencyGroups eligibility;
};

struct GuiNetworkEditorContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<bool(ModelDataDefinition*, ModelDataDefinition*)> ownsEditableDataDefinition;
	std::function<bool(ModelDataDefinition*, QGraphicsItem*)> canDeleteElement;
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
	GuiExtensionDependencyGroups eligibility;
};

// Domain-specific reconciliation after the generic Property Editor changes a list.
struct GuiPropertyListContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<void(ModelDataDefinition*, ModelGraphicsScene*)> reconcile;
	std::function<bool(ModelDataDefinition*, ModelGraphicsScene*, int, int)> reconcileMove;
	GuiExtensionDependencyGroups eligibility;
};

struct GuiEditingContextContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<ModelDataDefinition*(ModelDataDefinition*)> resolveOwner;
	std::function<bool(ModelDataDefinition*)> isNetwork;
	std::function<bool(Model*, ModelDataDefinition*)> canOpen;
	GuiExtensionDependencyGroups eligibility;
};

struct GuiModelReferenceContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<ModelDataDefinition*(ModelDataDefinition*)> currentReference;
	std::function<ModelDataDefinition*(ModelDataDefinition*, const std::string&)> resolveChoice;
	std::function<std::string(ModelDataDefinition*, const std::string&, ModelDataDefinition*)> formatChoice;
	std::function<bool(ModelDataDefinition*, ModelDataDefinition*, ModelDataDefinition*, QWidget*)> confirmChange;
	std::function<void(ModelDataDefinition*, ModelDataDefinition*, ModelDataDefinition*, ModelGraphicsScene*, ObjectPropertyBrowser*)>
		referenceChanged;
	std::function<void(ModelDataDefinition*, const std::string&, const std::string&, const std::string&,
		ModelGraphicsScene*, ObjectPropertyBrowser*)> propertyChanged;
	GuiExtensionDependencyGroups eligibility;
};

struct GuiComponentVisualizationContribution {
	std::string id;
	std::function<bool(ModelComponent*)> supports;
	std::function<void(GraphicalModelComponent*)> configure;
	std::function<void(ModelGraphicsScene*, GraphicalModelComponent*, bool)> animateArrival;
	std::function<void(ModelGraphicsScene*, GraphicalModelComponent*)> animateDeparture;
	GuiExtensionDependencyGroups eligibility;
};

struct GuiDataDefinitionRenderingContribution {
	std::string id;
	std::function<bool(ModelDataDefinition*)> supports;
	std::function<void(ModelDataDefinition*, GraphicalModelItemRenderContext*)> configure;
	GuiExtensionDependencyGroups eligibility;
};

// F2: a GUI extension supplies a real scene item without extending the
// generic scene's domain-specific drawing-mode switch. The factory owns its
// result until the GUI host successfully hands it to the scene/undo stack.
// Persistence and simulation event binding are separate F3 contracts.
struct GuiAnimationToolContribution {
    std::string id;
    std::string text;
    std::string statusTip;
    std::string menuPath = "View/Animations";
    std::function<bool(const GuiExtensionRuntimeContext&)> isEnabled;
    std::function<std::unique_ptr<QGraphicsItem>(const GuiExtensionRuntimeContext&)> createItem;
    GuiExtensionDependencyGroups eligibility;
};

class GuiExtensionRegistry {
public:
	void addAnimationTool(GuiAnimationToolContribution contribution) {
        _animationTools.push_back(std::move(contribution));
    }

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

	void addNetworkEditor(GuiNetworkEditorContribution contribution) {
		_networkEditors.push_back(std::move(contribution));
	}

	void addPropertyList(GuiPropertyListContribution contribution) {
		_propertyLists.push_back(std::move(contribution));
	}

	void addEditingContext(GuiEditingContextContribution contribution) {
		_editingContexts.push_back(std::move(contribution));
	}

	void addModelReference(GuiModelReferenceContribution contribution) {
		_modelReferences.push_back(std::move(contribution));
	}

	void addComponentVisualization(GuiComponentVisualizationContribution contribution) {
		_componentVisualizations.push_back(std::move(contribution));
	}

	void addDataDefinitionRendering(GuiDataDefinitionRenderingContribution contribution) {
		_dataDefinitionRenderings.push_back(std::move(contribution));
	}

	const std::vector<GuiAnimationToolContribution>& animationTools() const {
        return _animationTools;
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

	const std::vector<GuiNetworkEditorContribution>& networkEditors() const {
		return _networkEditors;
	}

	const std::vector<GuiPropertyListContribution>& propertyLists() const {
		return _propertyLists;
	}

	const std::vector<GuiEditingContextContribution>& editingContexts() const {
		return _editingContexts;
	}

	const std::vector<GuiModelReferenceContribution>& modelReferences() const {
		return _modelReferences;
	}

	const std::vector<GuiComponentVisualizationContribution>& componentVisualizations() const {
		return _componentVisualizations;
	}

	const std::vector<GuiDataDefinitionRenderingContribution>& dataDefinitionRenderings() const {
		return _dataDefinitionRenderings;
	}

    // Copies only contribution categories whose local eligibility is satisfied.
    // Used by both the GUI manager and the static catalog to avoid policy drift.
    void appendEligibleFrom(const GuiExtensionRegistry& other,
                            const std::function<bool(const GuiExtensionDependencyGroups&)>& eligible) {
        const auto append = [&eligible](auto& destination, const auto& source) {
            for (const auto& item : source) {
                if (!eligible || eligible(item.eligibility)) destination.push_back(item);
            }
        };
        append(_actions, other._actions);
        append(_animationTools, other._animationTools);
        append(_docks, other._docks);
        append(_windows, other._windows);
        append(_contextMenus, other._contextMenus);
        append(_diagramProjections, other._diagramProjections);
        append(_networkEditors, other._networkEditors);
        append(_propertyLists, other._propertyLists);
        append(_editingContexts, other._editingContexts);
        append(_modelReferences, other._modelReferences);
        append(_componentVisualizations, other._componentVisualizations);
        append(_dataDefinitionRenderings, other._dataDefinitionRenderings);
    }

	void clear() {
		_actions.clear();
        _animationTools.clear();
		_docks.clear();
		_windows.clear();
		_contextMenus.clear();
		_diagramProjections.clear();
		_networkEditors.clear();
		_propertyLists.clear();
		_editingContexts.clear();
		_modelReferences.clear();
		_componentVisualizations.clear();
		_dataDefinitionRenderings.clear();
	}

private:
	std::vector<GuiActionContribution> _actions;
    std::vector<GuiAnimationToolContribution> _animationTools;
	std::vector<GuiDockContribution> _docks;
	std::vector<GuiWindowContribution> _windows;
	std::vector<GuiContextMenuContribution> _contextMenus;
	std::vector<GuiDiagramProjectionContribution> _diagramProjections;
	std::vector<GuiNetworkEditorContribution> _networkEditors;
	std::vector<GuiPropertyListContribution> _propertyLists;
	std::vector<GuiEditingContextContribution> _editingContexts;
	std::vector<GuiModelReferenceContribution> _modelReferences;
	std::vector<GuiComponentVisualizationContribution> _componentVisualizations;
	std::vector<GuiDataDefinitionRenderingContribution> _dataDefinitionRenderings;
};

class GuiExtensionPlugin {
public:
	virtual ~GuiExtensionPlugin() = default;

	virtual std::string extensionId() const = 0;
	virtual std::string displayName() const = 0;
	virtual std::string version() const = 0;
	virtual std::vector<std::string> requiredModelPlugins() const = 0;
	// Non-pure for compatibility with all existing GUI extension implementations.
	virtual GuiExtensionDependencyGroups dependencyGroups() const { return {}; }
	virtual void registerContributions(GuiExtensionRegistry* registry) const = 0;
};

#endif /* GUIEXTENSIONCONTRACTS_H */
