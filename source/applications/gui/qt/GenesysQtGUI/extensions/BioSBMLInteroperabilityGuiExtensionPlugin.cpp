#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include <string>

namespace {

void showTextDialog(QWidget* parentWidget, const QString& title, const QString& text) {
	auto* dialog = new QDialog(parentWidget);
	dialog->setWindowTitle(title);
	dialog->resize(980, 620);
	dialog->setAttribute(Qt::WA_DeleteOnClose);

	auto* layout = new QVBoxLayout(dialog);
	auto* editor = new QPlainTextEdit(dialog);
	editor->setReadOnly(true);
	editor->setPlainText(text);
	layout->addWidget(editor);

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
	QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::close);
	QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::close);
	layout->addWidget(buttons);

	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

BioSimulatorRunner* ensureRunner(Model* model, QWidget* parentWidget) {
	List<ModelDataDefinition*>* runners = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSimulatorRunner>());
	if (runners != nullptr && runners->size() > 0) {
		QStringList runnerNames;
		for (ModelDataDefinition* definition : *runners->list()) {
			if (definition != nullptr) {
				runnerNames << QString::fromStdString(definition->getName());
			}
		}
		runnerNames.sort(Qt::CaseInsensitive);
		bool ok = false;
		const QString selectedRunnerName = QInputDialog::getItem(
			parentWidget,
			QObject::tr("SBML Interoperability"),
			QObject::tr("Select BioSimulatorRunner:"),
			runnerNames,
			0,
			false,
			&ok);
		if (!ok || selectedRunnerName.isEmpty()) {
			return nullptr;
		}
		return dynamic_cast<BioSimulatorRunner*>(
			model->getDataManager()->getDataDefinition(Util::TypeOf<BioSimulatorRunner>(), selectedRunnerName.toStdString()));
	}

	const QMessageBox::StandardButton createRunner = QMessageBox::question(
		parentWidget,
		QObject::tr("SBML Interoperability"),
		QObject::tr("No BioSimulatorRunner found. Create one now?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::Yes);
	if (createRunner != QMessageBox::Yes) {
		return nullptr;
	}

	std::string name = "BioSimulatorRunner";
	unsigned int suffix = 1;
	while (model->getDataManager()->getDataDefinition(Util::TypeOf<BioSimulatorRunner>(), name) != nullptr) {
		name = "BioSimulatorRunner_" + std::to_string(suffix++);
	}

	auto* runner = new BioSimulatorRunner(model, name);
	return runner;
}

std::string commandWithOptionalName(const std::string& commandName, const QString& optionalName) {
	if (optionalName.trimmed().isEmpty()) {
		return commandName + "()";
	}
	const std::string name = optionalName.trimmed().toStdString();
	return commandName + "(\"" + name + "\")";
}

} // namespace

class BioSBMLInteroperabilityGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.sbml.interoperability";
	}

	std::string displayName() const override {
		return "SBML Interoperability";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"biosimulatorrunner"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiWindowContribution window;
		window.id = "actionGuiExtensionsBioSBMLInteroperability";
		window.menuPath = "Tools/Extensions/Biochemical";
		window.text = "SBML Import/Export...";
		window.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		window.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("SBML Interoperability"), QObject::tr("No opened model."));
				return;
			}

			QStringList operations;
			operations << QObject::tr("Import SBML")
			           << QObject::tr("Export SBML");
			bool ok = false;
			const QString operation = QInputDialog::getItem(
				parentWidget,
				QObject::tr("SBML Interoperability"),
				QObject::tr("Operation:"),
				operations,
				0,
				false,
				&ok);
			if (!ok || operation.isEmpty()) {
				return;
			}

			BioSimulatorRunner* runner = ensureRunner(model, parentWidget);
			if (runner == nullptr) {
				return;
			}

			if (operation == QObject::tr("Import SBML")) {
				QStringList sourceModes;
				sourceModes << QObject::tr("Paste SBML Text")
				            << QObject::tr("Read SBML File");
				const QString sourceMode = QInputDialog::getItem(
					parentWidget,
					QObject::tr("SBML Interoperability"),
					QObject::tr("Source:"),
					sourceModes,
					0,
					false,
					&ok);
				if (!ok || sourceMode.isEmpty()) {
					return;
				}

				QString sourceValue;
				if (sourceMode == QObject::tr("Paste SBML Text")) {
					sourceValue = QInputDialog::getMultiLineText(
						parentWidget,
						QObject::tr("SBML Interoperability"),
						QObject::tr("Paste SBML document:"),
						QString(),
						&ok);
					if (!ok || sourceValue.trimmed().isEmpty()) {
						return;
					}
					runner->setModelSourceType("SBMLString");
					runner->setModelSource(sourceValue.toStdString());
				} else {
					sourceValue = QInputDialog::getText(
						parentWidget,
						QObject::tr("SBML Interoperability"),
						QObject::tr("SBML file path:"),
						QLineEdit::Normal,
						QString(),
						&ok).trimmed();
					if (!ok || sourceValue.isEmpty()) {
						return;
					}
					runner->setModelSourceType("SBMLFile");
					runner->setModelSource(sourceValue.toStdString());
				}

				const QString networkName = QInputDialog::getText(
					parentWidget,
					QObject::tr("SBML Interoperability"),
					QObject::tr("Target BioNetwork name (optional):"),
					QLineEdit::Normal,
					QString(),
					&ok);
				if (!ok) {
					return;
				}
				if (networkName.contains("\"")) {
					QMessageBox::warning(parentWidget, QObject::tr("SBML Interoperability"),
					                     QObject::tr("Network name cannot contain double quotes."));
					return;
				}

				runner->setCommand(commandWithOptionalName("importSBML", networkName));
				std::string errorMessage;
				if (!runner->executeCommand(errorMessage)) {
					QMessageBox::warning(parentWidget, QObject::tr("SBML Interoperability"),
					                     QString::fromStdString(errorMessage));
					return;
				}

				model->setHasChanged(true);
				showTextDialog(parentWidget, QObject::tr("SBML Import Result"),
				               QString::fromStdString(runner->getLastResponsePayload()));
				return;
			}

			QStringList networkChoices;
			networkChoices << QObject::tr("<auto>");
			List<ModelDataDefinition*>* networks = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
			if (networks != nullptr) {
				for (ModelDataDefinition* definition : *networks->list()) {
					if (definition != nullptr) {
						networkChoices << QString::fromStdString(definition->getName());
					}
				}
			}
			networkChoices.sort(Qt::CaseInsensitive);
			networkChoices.removeAll(QObject::tr("<auto>"));
			networkChoices.prepend(QObject::tr("<auto>"));

			const QString selectedNetwork = QInputDialog::getItem(
				parentWidget,
				QObject::tr("SBML Interoperability"),
				QObject::tr("BioNetwork:"),
				networkChoices,
				0,
				false,
				&ok);
			if (!ok || selectedNetwork.isEmpty()) {
				return;
			}

			QStringList destinationModes;
			destinationModes << QObject::tr("Return SBML Text")
			                 << QObject::tr("Write SBML File");
			const QString destinationMode = QInputDialog::getItem(
				parentWidget,
				QObject::tr("SBML Interoperability"),
				QObject::tr("Destination:"),
				destinationModes,
				0,
				false,
				&ok);
			if (!ok || destinationMode.isEmpty()) {
				return;
			}

			if (destinationMode == QObject::tr("Write SBML File")) {
				const QString outputPath = QInputDialog::getText(
					parentWidget,
					QObject::tr("SBML Interoperability"),
					QObject::tr("SBML output file path:"),
					QLineEdit::Normal,
					QString(),
					&ok).trimmed();
				if (!ok || outputPath.isEmpty()) {
					return;
				}
				runner->setModelSourceType("SBMLFile");
				runner->setModelSource(outputPath.toStdString());
			} else {
				runner->setModelSourceType("SBMLString");
				runner->setModelSource("");
			}

			if (selectedNetwork.contains("\"")) {
				QMessageBox::warning(parentWidget, QObject::tr("SBML Interoperability"),
				                     QObject::tr("BioNetwork name cannot contain double quotes."));
				return;
			}
			runner->setCommand(commandWithOptionalName(
				"exportSBML",
				selectedNetwork == QObject::tr("<auto>") ? QString() : selectedNetwork));

			std::string errorMessage;
			if (!runner->executeCommand(errorMessage)) {
				QMessageBox::warning(parentWidget, QObject::tr("SBML Interoperability"),
				                     QString::fromStdString(errorMessage));
				return;
			}

			if (destinationMode == QObject::tr("Return SBML Text")) {
				showTextDialog(parentWidget, QObject::tr("Exported SBML"), QString::fromStdString(runner->getModelSource()));
			}
			showTextDialog(parentWidget, QObject::tr("SBML Export Result"),
			               QString::fromStdString(runner->getLastResponsePayload()));
		};
		registry->addWindow(std::move(window));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioSBMLInteroperabilityGuiExtensionPlugin);
