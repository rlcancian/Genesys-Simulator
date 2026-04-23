#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <algorithm>
#include <string>
#include <vector>

namespace {
std::vector<std::string> parseNameList(const QString& text) {
	std::vector<std::string> names;
	const QStringList lines = text.split(QRegularExpression("[,;\\n\\r]+"), Qt::SkipEmptyParts);
	names.reserve(static_cast<size_t>(lines.size()));
	for (const QString& line : lines) {
		const QString trimmed = line.trimmed();
		if (trimmed.isEmpty()) {
			continue;
		}
		names.push_back(trimmed.toStdString());
	}
	std::sort(names.begin(), names.end());
	names.erase(std::unique(names.begin(), names.end()), names.end());
	return names;
}

QString namesToMultiline(const std::vector<std::string>& names) {
	QStringList lines;
	for (const std::string& name : names) {
		lines << QString::fromStdString(name);
	}
	return lines.join("\n");
}
} // namespace

class BioReactionKineticsGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.reaction.kinetics";
	}

	std::string displayName() const override {
		return "BioReaction Kinetics Editor";
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
		editorWindow.id = "actionGuiExtensionsBioReactionKinetics";
		editorWindow.menuPath = "Tools/Extensions/Biochemical";
		editorWindow.text = "Edit BioReaction Kinetics...";
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
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Editor"), QObject::tr("No opened model."));
				return;
			}

			List<ModelDataDefinition*>* reactions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioReaction>());
			if (reactions == nullptr || reactions->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioReaction Editor"),
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
				QObject::tr("BioReaction Editor"),
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
				QMessageBox::warning(parentWidget, QObject::tr("BioReaction Editor"),
				                     QObject::tr("Could not resolve selected BioReaction."));
				return;
			}

			QDialog dialog(parentWidget);
			dialog.setWindowTitle(QObject::tr("BioReaction Kinetics - %1").arg(selectedName));
			dialog.setModal(true);

			QFormLayout* form = new QFormLayout(&dialog);
			auto* reversibleCheck = new QCheckBox(&dialog);
			reversibleCheck->setChecked(reaction->isReversible());

			auto* rateConstant = new QDoubleSpinBox(&dialog);
			rateConstant->setRange(0.0, 1e12);
			rateConstant->setDecimals(9);
			rateConstant->setValue(reaction->getRateConstant());

			auto* reverseRateConstant = new QDoubleSpinBox(&dialog);
			reverseRateConstant->setRange(0.0, 1e12);
			reverseRateConstant->setDecimals(9);
			reverseRateConstant->setValue(reaction->getReverseRateConstant());

			auto* rateParam = new QLineEdit(QString::fromStdString(reaction->getRateConstantParameterName()), &dialog);
			auto* reverseRateParam = new QLineEdit(QString::fromStdString(reaction->getReverseRateConstantParameterName()), &dialog);
			auto* kineticLaw = new QLineEdit(QString::fromStdString(reaction->getKineticLawExpression()), &dialog);
			auto* reverseKineticLaw = new QLineEdit(QString::fromStdString(reaction->getReverseKineticLawExpression()), &dialog);

			auto* modifiersEditor = new QPlainTextEdit(&dialog);
			modifiersEditor->setPlainText(namesToMultiline(reaction->getModifiers()));
			modifiersEditor->setPlaceholderText(QObject::tr("One species name per line"));
			modifiersEditor->setFixedHeight(110);

			form->addRow(QObject::tr("Reversible"), reversibleCheck);
			form->addRow(QObject::tr("Rate constant"), rateConstant);
			form->addRow(QObject::tr("Rate parameter"), rateParam);
			form->addRow(QObject::tr("Kinetic law"), kineticLaw);
			form->addRow(QObject::tr("Reverse rate constant"), reverseRateConstant);
			form->addRow(QObject::tr("Reverse rate parameter"), reverseRateParam);
			form->addRow(QObject::tr("Reverse kinetic law"), reverseKineticLaw);
			form->addRow(QObject::tr("Modifiers"), modifiersEditor);

			auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
			form->addRow(buttons);
			QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
			QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

			if (dialog.exec() != QDialog::Accepted) {
				return;
			}

			reaction->setReversible(reversibleCheck->isChecked());
			reaction->setRateConstant(rateConstant->value());
			reaction->setRateConstantParameterName(rateParam->text().trimmed().toStdString());
			reaction->setKineticLawExpression(kineticLaw->text().trimmed().toStdString());
			reaction->setReverseRateConstant(reverseRateConstant->value());
			reaction->setReverseRateConstantParameterName(reverseRateParam->text().trimmed().toStdString());
			reaction->setReverseKineticLawExpression(reverseKineticLaw->text().trimmed().toStdString());
			reaction->clearModifiers();
			for (const std::string& modifier : parseNameList(modifiersEditor->toPlainText())) {
				reaction->addModifier(modifier);
			}
			reaction->setHasChanged(true);
			model->setHasChanged(true);

			QMessageBox::information(parentWidget, QObject::tr("BioReaction Editor"),
			                         QObject::tr("BioReaction kinetics updated. Run model check/simulation to validate expressions and symbols."));
		};
		registry->addWindow(std::move(editorWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioReactionKineticsGuiExtensionPlugin);
