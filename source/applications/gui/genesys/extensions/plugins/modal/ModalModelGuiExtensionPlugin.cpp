#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"

namespace {
class ModalModelGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override { return "gui.extensions.modal.context"; }
	std::string displayName() const override { return "ModalModel editing contexts"; }
	std::string version() const override { return "1.0.0"; }
	std::vector<std::string> requiredModelPlugins() const override { return {}; }

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) return;
		GuiEditingContextContribution context;
		context.id = "gui.modal.network-editing-context";
		context.supports = [](ModelDataDefinition* owner) {
			return dynamic_cast<ModalModelDefault*>(owner) != nullptr
				|| dynamic_cast<DefaultNetwork*>(owner) != nullptr;
		};
		context.resolveOwner = [](ModelDataDefinition* owner) -> ModelDataDefinition* {
			if (auto* adapter = dynamic_cast<ModalModelDefault*>(owner)) return adapter->getNetwork();
			return owner;
		};
		context.isNetwork = [](ModelDataDefinition* owner) {
			if (dynamic_cast<ModalModelDefault*>(owner) != nullptr) return true;
			return dynamic_cast<DefaultNetwork*>(owner) != nullptr;
		};
		context.canOpen = [](Model* model, ModelDataDefinition* owner) {
			if (model == nullptr || model->getDataManager() == nullptr) return false;
			if (auto* adapter = dynamic_cast<ModalModelDefault*>(owner)) owner = adapter->getNetwork();
			auto* network = dynamic_cast<DefaultNetwork*>(owner);
			return network != nullptr
				&& model->getDataManager()->getDataDefinition(network->getClassname(), network->getId()) == network;
		};
		registry->addEditingContext(std::move(context));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(ModalModelGuiExtensionPlugin);
} // namespace
