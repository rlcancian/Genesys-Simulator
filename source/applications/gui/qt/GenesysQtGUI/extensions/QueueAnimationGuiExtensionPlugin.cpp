#include "GuiExtensionPluginCatalog.h"

#include "animations/AnimationPlaceholder.h"
#include "animations/AnimationQueue.h"

class AnimationQueueDisplay : public AnimationPlaceholder {
public:
	AnimationQueueDisplay() : AnimationPlaceholder("Queue") {}
};

class QueueAnimationGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.animation.queue";
	}

	std::string displayName() const override {
		return "Queue Animation";
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

	void registerAnimations(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}
		registry->addAnimationContribution({
			"Queue",
			[]() -> AnimationPlaceholder* {
				return new AnimationQueueDisplay();
			},
			[](ModelGraphicsScene* scene, const GuiSimAnimationEvent& event) {
				if (scene == nullptr || event.component == nullptr) {
					return;
				}
				AnimationQueue aq(scene, event.component);
				if (event.type == GuiSimAnimationEvent::Type::Insert) {
					aq.addAnimationQueue(event.visible);
				} else {
					aq.removeAnimationQueue();
				}
			}
		});
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(QueueAnimationGuiExtensionPlugin);
