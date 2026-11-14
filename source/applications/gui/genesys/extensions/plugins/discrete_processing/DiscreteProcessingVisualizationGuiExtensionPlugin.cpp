#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"

#include "plugins/components/Transfer/PickStation.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "graphicals/GraphicalModelComponent.h"
#include "QueueAnimationController.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include <algorithm>
#include <vector>

namespace {

class DiscreteProcessingVisualizationGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override { return "discrete-processing.visualization.gui"; }
	std::string displayName() const override { return "Discrete Processing Visualization"; }
	std::string version() const override { return "1.0.0"; }
	std::vector<std::string> requiredModelPlugins() const override { return {"Queue", "PickStation"}; }

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) return;
		GuiComponentVisualizationContribution visualization;
		visualization.id = extensionId();
		visualization.supports = [](ModelComponent* component) {
			if (component == nullptr) return false;
			if (dynamic_cast<PickStation*>(component) != nullptr) return true;
			auto containsQueue = [](const std::map<std::string, ModelDataDefinition*>* definitions) {
				if (definitions == nullptr) return false;
				return std::any_of(definitions->begin(), definitions->end(), [](const auto& entry) {
					return dynamic_cast<Queue*>(entry.second) != nullptr;
				});
			};
			return containsQueue(component->getInternalData()) || containsQueue(component->getAttachedData());
		};
		visualization.configure = [](GraphicalModelComponent* graphical) {
			if (graphical == nullptr || graphical->getComponent() == nullptr) return;
			auto* component = graphical->getComponent();
			std::vector<Queue*> queues;
			auto collect = [&queues](const std::map<std::string, ModelDataDefinition*>* definitions) {
				if (definitions == nullptr) return;
				for (const auto& [name, definition] : *definitions) {
					(void)name;
					if (auto* queue = dynamic_cast<Queue*>(definition)) queues.push_back(queue);
				}
			};
			collect(component->getInternalData());
			collect(component->getAttachedData());
			std::sort(queues.begin(), queues.end(), [](const Queue* left, const Queue* right) {
				return left->getId() < right->getId();
			});
			for (unsigned int index = 0; index < queues.size(); ++index) {
				Queue* queue = queues.at(index);
				graphical->addAnimationTrack(queue, index, static_cast<unsigned int>(queue->size()));
			}
			if (dynamic_cast<PickStation*>(component) != nullptr) graphical->addUnmappedAnimationTrack();
		};
		visualization.animateArrival = [](ModelGraphicsScene* scene, GraphicalModelComponent* component, bool visible) {
			QueueAnimationController animation(scene, component);
			animation.addAnimationQueue(visible);
		};
		visualization.animateDeparture = [](ModelGraphicsScene* scene, GraphicalModelComponent* component) {
			QueueAnimationController animation(scene, component);
			animation.removeAnimationQueue();
		};
		registry->addComponentVisualization(std::move(visualization));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(DiscreteProcessingVisualizationGuiExtensionPlugin);

} // namespace
