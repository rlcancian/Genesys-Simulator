#include "GuiExtensionPluginCatalog.h"

#include <QMessageBox>
#include <QMainWindow>
#include <QWidget>

class BioNetworkAwareGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.dependency.bionetwork";
	}

	std::string displayName() const override {
		return "BioNetwork-Aware GUI Extension";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"bionetwork"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiActionContribution action;
		action.id = "actionGuiExtensionsBioNetworkAware";
		action.text = "BioNetwork Extension Status";
		action.menuPath = "Tools/Extensions";
		action.statusTip = "Visible only when model plugin dependency 'bionetwork' is loaded.";
		action.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr;
		};
		action.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr;
		};
		action.trigger = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr) {
				return;
			}
			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			QMessageBox::information(
				parentWidget,
				QObject::tr("Dependency-Satisfied Extension"),
				QObject::tr("This GUI extension is active because model plugin dependency 'bionetwork' is loaded."));
		};
		registry->addAction(std::move(action));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioNetworkAwareGuiExtensionPlugin);
