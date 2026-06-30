#include "GuiExtensionPluginCatalog.h"

#include "animations/AnimationPlaceholder.h"
#include "graphicals/ModelGraphicsScene.h"
#include "kernel/simulator/model/ModelComponent.h"

class AnimationResource : public AnimationPlaceholder {
public:
	AnimationResource() : AnimationPlaceholder("Resource") {}

	void resetRuntimeState() override {
		setEntityCount(0);
	}

	void setEntityCount(int count) {
		_entityCount = qMax(0, count);
		update();
	}

	int entityCount() const { return _entityCount; }

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override {
		Q_UNUSED(option);
		Q_UNUSED(widget);

		const QRectF bounds = boundingRect();
		painter->setRenderHint(QPainter::Antialiasing);

		const bool busy = (_entityCount > 0);
		const QColor fillColor = busy ? QColor(255, 180, 100) : QColor(210, 240, 210);
		const QColor borderColor = busy ? QColor(180, 80, 0) : QColor(60, 140, 60);

		painter->setPen(QPen(borderColor, 1.5));
		painter->setBrush(fillColor);
		painter->drawRoundedRect(bounds.adjusted(1, 1, -1, -1), 4.0, 4.0);

		const QString name = getTargetName().trimmed().isEmpty()
			? QStringLiteral("Resource") : getTargetName().trimmed();
		const QString label = busy
			? QString("%1\n[%2]").arg(name).arg(_entityCount)
			: name;

		QFont font = painter->font();
		font.setPixelSize(qMax(8, qMin(12, static_cast<int>(bounds.height() / 4.0))));
		painter->setFont(font);
		painter->setPen(Qt::black);
		painter->drawText(bounds.adjusted(4, 4, -4, -4),
		                  Qt::AlignCenter | Qt::TextWordWrap, label);

		if (isSelected()) {
			const qreal cs = 10.0;
			painter->setPen(Qt::NoPen);
			painter->setBrush(Qt::black);
			painter->drawRect(QRectF(-cs, -cs, cs, cs));
			painter->drawRect(QRectF(bounds.topRight() - QPointF(0, cs), QSizeF(cs, cs)));
			painter->drawRect(QRectF(-cs, bounds.height(), cs, cs));
			painter->drawRect(QRectF(bounds.bottomRight(), QSizeF(cs, cs)));
		}
	}

private:
	int _entityCount = 0;
};

class ResourceAnimationGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.animation.resource";
	}

	std::string displayName() const override {
		return "Resource Animation";
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
			"Resource",
			[]() -> AnimationPlaceholder* {
				return new AnimationResource();
			},
			[](ModelGraphicsScene* scene, const GuiSimAnimationEvent& event) {
				if (scene == nullptr || event.component == nullptr) {
					return;
				}
				const QString name = !event.animationTargetName.empty()
					? QString::fromStdString(event.animationTargetName)
					: (event.component != nullptr
					           ? QString::fromStdString(event.component->getName())
					           : QString());
				QList<AnimationPlaceholder*>* placeholders = scene->getAnimationsPlaceholder();
				if (placeholders == nullptr || name.isEmpty()) {
					return;
				}
				for (AnimationPlaceholder* ph : *placeholders) {
					AnimationResource* res = dynamic_cast<AnimationResource*>(ph);
					if (res == nullptr || res->getTargetName().trimmed() != name) {
						continue;
					}
					const int delta = (event.type == GuiSimAnimationEvent::Type::Insert) ? 1 : -1;
					res->setEntityCount(res->entityCount() + delta);
				}
			}
		});
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(ResourceAnimationGuiExtensionPlugin);
