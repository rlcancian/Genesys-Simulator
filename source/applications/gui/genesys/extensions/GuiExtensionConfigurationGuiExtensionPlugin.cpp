#include "GuiExtensionPluginCatalog.h"
#include "GuiExtensionConfigurationDialog.h"
#include "../mainwindow.h"

#include <QMainWindow>
#include <QWidget>

class GuiExtensionConfigurationGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.configuration.manager";
	}

	std::string displayName() const override {
		return "Graphical Extensions Manager";
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

		GuiWindowContribution managerWindow;
		managerWindow.id = "actionGuiExtensionsManage";
		managerWindow.menuPath = "Tools/Extensions";
		managerWindow.text = "Manage Graphical Extensions...";
		managerWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr;
		};
		managerWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr) {
				return;
			}
			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			auto* dialog = new GuiExtensionConfigurationDialog(
				parentWidget,
				GuiExtensionPluginCatalog::plugins(),
				[mainWindow = context.mainWindow]() {
					if (mainWindow == nullptr) {
						return;
					}
					if (MainWindow* typedMainWindow = qobject_cast<MainWindow*>(mainWindow)) {
						typedMainWindow->refreshPluginCatalog();
						typedMainWindow->refreshGuiExtensions();
					}
				});
			dialog->setAttribute(Qt::WA_DeleteOnClose);
			dialog->show();
			dialog->raise();
			dialog->activateWindow();
		};
		registry->addWindow(std::move(managerWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(GuiExtensionConfigurationGuiExtensionPlugin);
