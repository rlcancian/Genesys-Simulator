#include "GuiExtensionPluginCatalog.h"

#include "../cellular_automata/CellularAutomataViewerWindow.h"

#include <QWidget>

class CellularAutomataGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.cellular.automata";
	}

	std::string displayName() const override {
		return "Cellular Automata Viewer";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiWindowContribution viewerWindow;
		viewerWindow.id = "actionGuiExtensionsCellularAutomata";
		viewerWindow.menuPath = "Tools/Extensions";
		viewerWindow.text = "Cellular Automata";
		viewerWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr;
		};
		viewerWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr) {
				return;
			}
			auto* window = new CellularAutomataViewerWindow(static_cast<QWidget*>(context.mainWindow));
			window->show();
			window->raise();
			window->activateWindow();
		};
		registry->addWindow(std::move(viewerWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(CellularAutomataGuiExtensionPlugin);
