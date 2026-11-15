#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelItemRenderStrategy.h"

#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"
#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

#include <QColor>
#include <QPainterPath>

#include <algorithm>

namespace {

QString partRole(const QString& value) {
	const QString lower = value.toLower();
	if (lower.contains("promoter") || lower.contains("pre-gene regulatory")) return "Pre-gene regulatory";
	if (lower.contains("regulatory") || lower.contains("operator")) return "Regulatory";
	if (lower.contains("rbs") || lower.contains("translation initiation")) return "Translation initiation";
	if (lower.contains("cds") || lower.contains("coding") || lower.contains("gene")) return "Coding";
	if (lower.contains("terminator") || lower.contains("termination")) return "Termination";
	if (lower.contains("region") || lower.contains("pre-gene")) return "Pre-gene region";
	return "Other";
}

QColor partAccent(const QString& role) {
	if (role == "Pre-gene regulatory") return QColor(64, 170, 120);
	if (role == "Regulatory") return QColor(150, 92, 194);
	if (role == "Translation initiation") return QColor(229, 158, 44);
	if (role == "Coding") return QColor(52, 152, 219);
	if (role == "Termination") return QColor(214, 82, 102);
	if (role == "Pre-gene region") return QColor(119, 136, 153);
	return QColor(110, 110, 110);
}

QString regulationRole(const QString& value) {
	const QString lower = value.toLower();
	if (lower.contains("activation")) return "Activation";
	if (lower.contains("repression")) return "Repression";
	if (lower.contains("dual")) return "Dual";
	return "Regulation";
}

QColor regulationAccent(const QString& role) {
	if (role == "Activation") return QColor(64, 170, 120);
	if (role == "Repression") return QColor(214, 82, 102);
	if (role == "Dual") return QColor(229, 158, 44);
	return QColor(110, 110, 110);
}

QRectF itemBody(const GraphicalModelItemRenderContext& context) {
	return QRectF(context.margin, context.margin,
	              context.width - 2 * context.margin - context.penWidth,
	              context.height - 2 * context.margin - context.penWidth)
		.adjusted(context.raise, context.raise, -context.raise, -context.raise);
}

QPainterPath partShape(const QRectF& body, const QString& role) {
	const qreal left = body.left(), right = body.right(), top = body.top(), bottom = body.bottom();
	const qreal centerY = body.center().y();
	const qreal insetX = qMax<qreal>(6.0, body.width() * .12);
	const qreal headX = qMax<qreal>(10.0, body.width() * .18);
	QPainterPath path;
	if (role == "Regulatory") {
		path.moveTo(body.center().x(), top); path.lineTo(right, centerY);
		path.lineTo(body.center().x(), bottom); path.lineTo(left, centerY); path.closeSubpath();
	} else if (role == "Pre-gene region") {
		path.addRoundedRect(body.adjusted(insetX * .5, 0, -insetX * .5, 0), body.height() / 3, body.height() / 3);
	} else if (role == "Translation initiation") {
		path.addRoundedRect(body.adjusted(insetX * .6, body.height() * .22, -insetX * .6, -body.height() * .22), body.height() / 3, body.height() / 3);
	} else if (role == "Termination") {
		path.addRoundedRect(body.adjusted(insetX * .35, body.height() * .18, -headX * .8, -body.height() * .18), body.height() / 4, body.height() / 4);
	} else {
		path.moveTo(left + insetX, top); path.lineTo(right - headX, top); path.lineTo(right, centerY);
		path.lineTo(right - headX, bottom); path.lineTo(left + insetX, bottom); path.lineTo(left, centerY); path.closeSubpath();
		if (role == "Other") {
			QPainterPath notch;
			const qreal band = qMax<qreal>(4.0, body.height() * .12);
			notch.addRoundedRect(body.adjusted(insetX * .3, body.height() * .25, -headX * .7, -body.height() * .25), band, band);
			path.addPath(notch);
		}
	}
	return path;
}

QPainterPath partDetails(const QRectF& body, const QString& role) {
	QPainterPath path;
	const qreal midY = body.center().y(), left = body.left(), right = body.right();
	const qreal headX = qMax<qreal>(10.0, body.width() * .18), insetX = qMax<qreal>(6.0, body.width() * .12);
	if (role == "Pre-gene regulatory") {
		path.moveTo(left + insetX, midY); path.lineTo(right - headX, midY);
		path.moveTo(right - headX, midY); path.lineTo(right - headX - 8, midY - 5);
		path.moveTo(right - headX, midY); path.lineTo(right - headX - 8, midY + 5);
	} else if (role == "Regulatory") {
		path.moveTo(body.center().x() - body.width() * .18, midY); path.lineTo(body.center().x() + body.width() * .18, midY);
		path.addEllipse(QRectF(body.center().x() - 5, midY - 5, 10, 10));
	} else if (role == "Translation initiation") {
		path.moveTo(left + insetX * 1.1, body.top() + body.height() * .22);
		path.lineTo(left + insetX * 1.1, body.bottom() - body.height() * .22);
	} else if (role == "Coding") {
		path.moveTo(left + insetX * 1.1, midY); path.lineTo(right - headX * .6, midY);
	} else if (role == "Termination") {
		path.moveTo(right - headX * .3, body.top() + body.height() * .2);
		path.lineTo(right - headX * .3, body.bottom() - body.height() * .2);
	} else {
		path.moveTo(left + insetX, midY); path.lineTo(right - headX, midY);
	}
	return path;
}

QPainterPath regulationShape(const QRectF& body, const QString& role) {
	const qreal right = body.right(), top = body.top(), bottom = body.bottom(), centerY = body.center().y();
	const qreal insetX = qMax<qreal>(6.0, body.width() * .14), radius = body.height() / 2;
	QPainterPath path;
	if (role == "Dual") {
		path.addRoundedRect(body.adjusted(insetX * .35, body.height() * .18, -insetX * .35, -body.height() * .18), radius, radius);
		return path;
	}
	path.addRoundedRect(body.adjusted(insetX * .35, body.height() * .22, -insetX * .35, -body.height() * .22), radius, radius);
	QPainterPath tip;
	if (role == "Repression") tip.addRect(QRectF(right - insetX * .7, top + body.height() * .2, insetX * .18, body.height() * .6));
	else { tip.moveTo(right - insetX * .25, centerY); tip.lineTo(right - insetX * .8, top + body.height() * .18); tip.lineTo(right - insetX * .8, bottom - body.height() * .18); tip.closeSubpath(); }
	path.addPath(tip);
	return path;
}

QPainterPath regulationDetails(const QRectF& body, const QString& role) {
	QPainterPath path;
	const qreal midY = body.center().y(), right = body.right(), insetX = qMax<qreal>(6.0, body.width() * .14);
	path.moveTo(body.left() + insetX * 1.1, midY); path.lineTo(right - insetX * 1.4, midY);
	if (role == "Activation" || role == "Dual") {
		path.moveTo(right - insetX * 1.4, midY); path.lineTo(right - insetX * 1.95, midY - 5.5);
		path.moveTo(right - insetX * 1.4, midY); path.lineTo(right - insetX * 1.95, midY + 5.5);
	}
	if (role == "Repression" || role == "Dual") {
		path.moveTo(right - insetX * 1.55, body.top() + body.height() * .24);
		path.lineTo(right - insetX * 1.55, body.bottom() - body.height() * .24);
	}
	return path;
}

void configureGeneticRendering(ModelDataDefinition* definition, GraphicalModelItemRenderContext* context) {
	if (definition == nullptr || context == nullptr) return;
	QRectF body = itemBody(*context);
	if (auto* part = dynamic_cast<GeneticCircuitPart*>(definition)) {
		const QString type = QString::fromStdString(part->getPartType());
		const QString role = partRole(type);
		context->hasSpecializedGlyph = true;
		context->semanticClassName = QStringLiteral("SpecializedGlyph");
		context->primaryText = type;
		context->secondaryText = QString::fromStdString(part->getName());
		context->tertiaryText = QString("role=%1").arg(role);
		if (!part->getSequence().empty()) context->tertiaryText += QString(" | sequenceLength=%1").arg(static_cast<int>(part->getSequence().size()));
		context->semanticSubtype = type;
		context->specializedAccent = partAccent(role);
		context->specializedAccent.setAlpha(context->fillColor.alpha());
		context->specializedShape = partShape(body, role);
		context->specializedDetails = partDetails(body, role);
		context->fillColor = context->specializedAccent;
	} else if (auto* regulation = dynamic_cast<GeneticRegulation*>(definition)) {
		const QString type = QString::fromStdString(regulation->getRegulationType());
		const QString role = regulationRole(type);
		context->hasSpecializedGlyph = true;
		context->semanticClassName = QStringLiteral("SpecializedGlyph");
		context->primaryText = type;
		context->secondaryText = QString::fromStdString(regulation->getTargetPartName());
		context->tertiaryText = QString("from=%1").arg(QString::fromStdString(regulation->getRegulatorSpeciesName()));
		context->semanticSubtype = type;
		context->specializedAccent = regulationAccent(role);
		context->specializedAccent.setAlpha(context->fillColor.alpha());
		context->specializedShape = regulationShape(body, role);
		context->specializedDetails = regulationDetails(body, role);
		context->fillColor = context->specializedAccent;
	}
}

class GeneticCircuitRenderingGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override { return "biochemical.genetic.rendering.gui"; }
	std::string displayName() const override { return "Genetic Circuit Rendering"; }
	std::string version() const override { return "1.0.0"; }
	std::vector<std::string> requiredModelPlugins() const override { return {"geneticcircuit"}; }
	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) return;
		GuiDataDefinitionRenderingContribution rendering;
		rendering.id = extensionId();
		rendering.supports = [](ModelDataDefinition* definition) {
			return dynamic_cast<GeneticCircuitPart*>(definition) != nullptr
				|| dynamic_cast<GeneticRegulation*>(definition) != nullptr;
		};
		rendering.configure = configureGeneticRendering;
		registry->addDataDefinitionRendering(std::move(rendering));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(GeneticCircuitRenderingGuiExtensionPlugin);

} // namespace
