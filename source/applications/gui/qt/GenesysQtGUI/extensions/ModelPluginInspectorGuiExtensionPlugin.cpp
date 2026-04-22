#include "GuiExtensionPluginCatalog.h"

#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"

#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QMainWindow>
#include <QWidget>

class ModelPluginInspectorGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.model.plugin.inspector";
	}

	std::string displayName() const override {
		return "Model Plugin Inspector";
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

		GuiWindowContribution inspectorWindow;
		inspectorWindow.id = "actionGuiExtensionsModelPluginInspector";
		inspectorWindow.menuPath = "Tools/Extensions";
		inspectorWindow.text = "Loaded Model Plugins";
		inspectorWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		inspectorWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr || context.simulator->getPluginManager() == nullptr) {
				return;
			}

			PluginManager* pluginManager = context.simulator->getPluginManager();
			QStringList pluginTypenames;
			for (unsigned int index = 0; index < pluginManager->size(); ++index) {
				Plugin* plugin = pluginManager->getAtRank(index);
				if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
					continue;
				}
				const std::string pluginTypename = plugin->getPluginInfo()->getPluginTypename();
				if (!pluginTypename.empty()) {
					pluginTypenames << QString::fromStdString(pluginTypename);
				}
			}
			pluginTypenames.sort(Qt::CaseInsensitive);

			const QString body = pluginTypenames.isEmpty()
			                     ? QObject::tr("No model plugins are currently loaded.")
			                     : pluginTypenames.join("\n");
			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			QMessageBox::information(parentWidget,
			                         QObject::tr("Loaded Model Plugins"),
			                         body);
		};
		registry->addWindow(std::move(inspectorWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(ModelPluginInspectorGuiExtensionPlugin);
