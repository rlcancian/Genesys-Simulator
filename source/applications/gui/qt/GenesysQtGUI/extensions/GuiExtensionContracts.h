#ifndef GUIEXTENSIONCONTRACTS_H
#define GUIEXTENSIONCONTRACTS_H

#include <functional>
#include <string>
#include <vector>

class Simulator;
class QMainWindow;
class QWidget;
class ModelGraphicsView;
class ModelGraphicsScene;
class AnimationPlaceholder;
class ModelComponent;

namespace Ui {
class MainWindow;
}

// Event dispatched to animation plugins when a simulation event affects an animated component.
struct GuiSimAnimationEvent {
	enum class Type { Insert, Remove };
	Type type = Type::Insert;
	ModelComponent* component = nullptr;
	// When non-empty, plugin handlers match placeholders by this name (e.g. resource or station).
	std::string animationTargetName;
	bool visible = true;
};

// Contribution registered by a plugin to provide a drawing tool command (menu/toolbar action)
// for a specific animation type. The GUI extension manager creates a checkable QAction from this
// and wires it to activate the corresponding placeholder drawing mode on the graphics scene.
struct GuiDrawingToolContribution {
	std::string animationType;  // e.g. "Queue", "Resource", "Station"
	std::string text;           // Label shown in menu and toolbar
	std::string iconResource;   // Qt resource path, e.g. ":/resources/ToolBar/_animfila.bmp" (empty = no icon)
	std::string menuPath;       // e.g. "Animate/Plugin Tools"
	std::string toolBarId;      // Toolbar object name or title (empty = no toolbar button)
	std::string statusTip;      // Optional status bar hint
	std::string shortcut;       // Optional keyboard shortcut string
};

// Contribution registered by an animation plugin: how to create its placeholder and how to react
// to simulation events that involve that animation type.
struct GuiAnimationContribution {
	// Identifies the animation domain ("Queue", "Resource", "Station", …).
	std::string animationType;
	// Factory called by the scene to create the placeholder item in design mode.
	std::function<AnimationPlaceholder*(void)> createPlaceholder;
	// Called by the scene when a simulation event targets this animation type.
	std::function<void(ModelGraphicsScene*, const GuiSimAnimationEvent&)> onSimEvent;
};

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

	void addAnimationContribution(GuiAnimationContribution contribution) {
		_animations.push_back(std::move(contribution));
	}

	void addDrawingTool(GuiDrawingToolContribution contribution) {
		_drawingTools.push_back(std::move(contribution));
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

	const std::vector<GuiAnimationContribution>& animations() const {
		return _animations;
	}

	const std::vector<GuiDrawingToolContribution>& drawingTools() const {
		return _drawingTools;
	}

	void clear() {
		_actions.clear();
		_docks.clear();
		_windows.clear();
		_animations.clear();
		_drawingTools.clear();
	}

private:
	std::vector<GuiActionContribution> _actions;
	std::vector<GuiDockContribution> _docks;
	std::vector<GuiWindowContribution> _windows;
	std::vector<GuiAnimationContribution> _animations;
	std::vector<GuiDrawingToolContribution> _drawingTools;
};

class GuiExtensionPlugin {
public:
	virtual ~GuiExtensionPlugin() = default;

	virtual std::string extensionId() const = 0;
	virtual std::string displayName() const = 0;
	virtual std::string version() const = 0;
	virtual std::vector<std::string> requiredModelPlugins() const = 0;
	virtual void registerContributions(GuiExtensionRegistry* registry) const = 0;
	// Override to register animation contributions (placeholder factories + sim-event handlers).
	virtual void registerAnimations(GuiExtensionRegistry* /*registry*/) const {}
};

#endif /* GUIEXTENSIONCONTRACTS_H */
