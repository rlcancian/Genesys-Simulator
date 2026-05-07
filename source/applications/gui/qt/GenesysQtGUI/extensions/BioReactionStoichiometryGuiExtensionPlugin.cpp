#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"

#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <cmath>
#include <string>
#include <vector>

namespace {

QString formatStoichiometry(double value) {
	QString formatted = QString::number(value, 'g', 15);
	if (formatted.isEmpty()) {
		return QStringLiteral("1");
	}
	return formatted;
}

QString termsToMultiline(const std::vector<BioReaction::StoichiometricTerm>& terms) {
	QStringList lines;
	for (const BioReaction::StoichiometricTerm& term : terms) {
		const QString species = QString::fromStdString(term.speciesName);
		if (species.isEmpty()) {
			continue;
		}
		if (std::abs(term.stoichiometry - 1.0) < 1e-12) {
			lines << species;
		} else {
			lines << (species + ":" + formatStoichiometry(term.stoichiometry));
		}
	}
	return lines.join("\n");
}

bool parseStoichiometricTerms(const QString& input,
                              std::vector<BioReaction::StoichiometricTerm>* terms,
                              QString* errorMessage) {
	if (terms == nullptr) {
		if (errorMessage != nullptr) {
			*errorMessage = QObject::tr("Internal error: invalid output term list.");
		}
		return false;
	}

	terms->clear();
	const QStringList chunks = input.split(QRegularExpression("[,;\\n\\r]+"), Qt::SkipEmptyParts);
	for (const QString& chunk : chunks) {
		const QString token = chunk.trimmed();
		if (token.isEmpty()) {
			continue;
		}

		QString speciesName = token;
		double stoichiometry = 1.0;
		const int separator = token.indexOf(':');
		if (separator >= 0) {
			speciesName = token.left(separator).trimmed();
			const QString stoichText = token.mid(separator + 1).trimmed();
			if (stoichText.isEmpty()) {
				if (errorMessage != nullptr) {
					*errorMessage = QObject::tr("Missing stoichiometry in entry \"%1\". Use \"Species:2.0\".").arg(token);
				}
				return false;
			}
			bool parsed = false;
			stoichiometry = stoichText.toDouble(&parsed);
			if (!parsed || !std::isfinite(stoichiometry) || stoichiometry <= 0.0) {
				if (errorMessage != nullptr) {
					*errorMessage = QObject::tr("Invalid stoichiometry in entry \"%1\". Expected a positive numeric value.").arg(token);
				}
				return false;
			}
		}

		speciesName = speciesName.trimmed();
		if (speciesName.isEmpty()) {
			if (errorMessage != nullptr) {
				*errorMessage = QObject::tr("Missing species name in entry \"%1\".").arg(token);
			}
			return false;
		}
		terms->push_back({speciesName.toStdString(), stoichiometry});
	}
	return true;
}

} // namespace

class BioReactionStoichiometryGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.reaction.stoichiometry";
	}

	std::string displayName() const override {
		return "BioReaction Stoichiometry Editor";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"bioreaction"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiWindowContribution editorWindow;
		editorWindow.id = "actionGuiExtensionsBioReactionStoichiometry";
		editorWindow.menuPath = "Tools/Extensions/Biochemical";
		editorWindow.text = "Edit BioReaction Stoichiometry...";
		editorWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		editorWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Stoichiometry"), QObject::tr("No opened model."));
				return;
			}

			List<ModelDataDefinition*>* reactions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioReaction>());
			if (reactions == nullptr || reactions->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioReaction Stoichiometry"),
				                         QObject::tr("No BioReaction definitions available in current model."));
				return;
			}

			QStringList reactionNames;
			for (ModelDataDefinition* def : *reactions->list()) {
				if (def != nullptr) {
					reactionNames << QString::fromStdString(def->getName());
				}
			}
			reactionNames.sort(Qt::CaseInsensitive);
			bool ok = false;
			const QString selectedName = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioReaction Stoichiometry"),
				QObject::tr("Select BioReaction:"),
				reactionNames,
				0,
				false,
				&ok);
			if (!ok || selectedName.isEmpty()) {
				return;
			}

			auto* reaction = dynamic_cast<BioReaction*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<BioReaction>(), selectedName.toStdString()));
			if (reaction == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Stoichiometry"),
				                     QObject::tr("Could not resolve selected BioReaction."));
				return;
			}

			const QString reactantsInput = QInputDialog::getMultiLineText(
				parentWidget,
				QObject::tr("BioReaction Stoichiometry"),
				QObject::tr("Reactants (one per line, optional :stoichiometry, e.g. A or A:2):"),
				termsToMultiline(reaction->getReactants()),
				&ok);
			if (!ok) {
				return;
			}

			const QString productsInput = QInputDialog::getMultiLineText(
				parentWidget,
				QObject::tr("BioReaction Stoichiometry"),
				QObject::tr("Products (one per line, optional :stoichiometry, e.g. B or B:3):"),
				termsToMultiline(reaction->getProducts()),
				&ok);
			if (!ok) {
				return;
			}

			std::vector<BioReaction::StoichiometricTerm> parsedReactants;
			std::vector<BioReaction::StoichiometricTerm> parsedProducts;
			QString parseError;
			if (!parseStoichiometricTerms(reactantsInput, &parsedReactants, &parseError)) {
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Stoichiometry"), parseError);
				return;
			}
			if (!parseStoichiometricTerms(productsInput, &parsedProducts, &parseError)) {
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Stoichiometry"), parseError);
				return;
			}
			if (parsedReactants.empty() && parsedProducts.empty()) {
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Stoichiometry"),
				                     QObject::tr("A BioReaction must define at least one reactant or product."));
				return;
			}

			reaction->clearReactants();
			for (const BioReaction::StoichiometricTerm& term : parsedReactants) {
				reaction->addReactant(term.speciesName, term.stoichiometry);
			}
			reaction->clearProducts();
			for (const BioReaction::StoichiometricTerm& term : parsedProducts) {
				reaction->addProduct(term.speciesName, term.stoichiometry);
			}
			reaction->setHasChanged(true);
			model->setHasChanged(true);

			QMessageBox::information(parentWidget, QObject::tr("BioReaction Stoichiometry"),
			                         QObject::tr("BioReaction stoichiometry updated. Run model check/simulation to validate species references."));
		};
		registry->addWindow(std::move(editorWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioReactionStoichiometryGuiExtensionPlugin);
