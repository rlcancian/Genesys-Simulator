#include "extensions/GuiExtensionContracts.h"
#include "extensions/GuiExtensionPluginCatalog.h"

#include "actions/DecideConditionUndoCommand.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"
#include "plugins/components/Decisions/Decide.h"

namespace {
GraphicalModelComponent* findGraphicalComponent(ModelGraphicsScene* scene, Decide* decide) {
	if (scene == nullptr || decide == nullptr) return nullptr;
	auto* graphical = scene->findGraphicalModelComponent(decide->getId());
	return graphical != nullptr && graphical->getComponent() == decide ? graphical : nullptr;
}

class DecideGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override { return "gui.extensions.decide"; }
	std::string displayName() const override { return "Decide graphical integration"; }
	std::string version() const override { return "1.0.0"; }
	std::vector<std::string> requiredModelPlugins() const override { return {"Decide"}; }

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) return;
		GuiPropertyListContribution contribution;
		contribution.id = "gui.decide.condition-list-reconciliation";
		contribution.supports = [](ModelDataDefinition* owner) {
			return dynamic_cast<Decide*>(owner) != nullptr;
		};
		contribution.reconcile = [](ModelDataDefinition* owner, ModelGraphicsScene* scene) {
			auto* decide = dynamic_cast<Decide*>(owner);
			if (decide == nullptr) return;
			auto* graphical = findGraphicalComponent(scene, decide);
			unsigned int index = 0;
			bool inserted = false;
			bool portsChanged = false;
			while (decide->takeConditionPortEdit(index, inserted)) {
				if (graphical != nullptr) {
					graphical->editOutputPort(index, inserted);
					portsChanged = true;
				}
			}
			Decide::ConditionEdit edit;
			const bool hasEdit = decide->takeConditionEdit(edit);
			if (portsChanged && scene != nullptr) {
				scene->update();
				scene->notifyGraphicalModelChange(GraphicalModelEvent::EventType::EDIT,
					GraphicalModelEvent::EventObjectType::COMPONENT, graphical);
			}
			if (hasEdit && scene != nullptr && scene->getUndoStack() != nullptr) {
				scene->getUndoStack()->push(new DecideConditionUndoCommand(decide, graphical, std::move(edit)));
			}
		};
		contribution.reconcileMove = [](ModelDataDefinition* owner, ModelGraphicsScene* scene, int from, int to) {
			auto* decide = dynamic_cast<Decide*>(owner);
			auto* graphical = findGraphicalComponent(scene, decide);
			if (decide == nullptr || graphical == nullptr || from < 0 || to < 0) return false;
			graphical->moveOutputPort(static_cast<unsigned int>(from), static_cast<unsigned int>(to));
			return true;
		};
		registry->addPropertyList(std::move(contribution));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(DecideGuiExtensionPlugin);
} // namespace
