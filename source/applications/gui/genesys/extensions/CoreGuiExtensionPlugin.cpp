#include "GuiExtensionPluginCatalog.h"

class CoreGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "core.gui.extension";
	}

	std::string displayName() const override {
		return "Core GUI Extension";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		(void)registry;
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(CoreGuiExtensionPlugin);
