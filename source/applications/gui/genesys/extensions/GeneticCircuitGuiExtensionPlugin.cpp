#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuit.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"
#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

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

std::string boolText(bool value) {
	return value ? "true" : "false";
}

std::string formatDouble(double value) {
	std::ostringstream out;
	out << std::setprecision(15) << value;
	return out.str();
}

GeneticCircuit* selectGeneticCircuit(Model* model, QWidget* parentWidget) {
	if (model == nullptr || model->getDataManager() == nullptr) {
		return nullptr;
	}

	List<ModelDataDefinition*>* circuits = model->getDataManager()->getDataDefinitionList(Util::TypeOf<GeneticCircuit>());
	if (circuits == nullptr || circuits->size() == 0) {
		QMessageBox::information(parentWidget, QObject::tr("Genetic Circuit"), QObject::tr("No GeneticCircuit definitions available in current model."));
		return nullptr;
	}

	QStringList circuitNames;
	for (ModelDataDefinition* definition : *circuits->list()) {
		if (definition != nullptr) {
			circuitNames << QString::fromStdString(definition->getName());
		}
	}
	circuitNames.sort(Qt::CaseInsensitive);

	bool ok = false;
	const QString selectedName = QInputDialog::getItem(
		parentWidget,
		QObject::tr("Genetic Circuit"),
		QObject::tr("Select GeneticCircuit:"),
		circuitNames,
		0,
		false,
		&ok);
	if (!ok || selectedName.isEmpty()) {
		return nullptr;
	}

	auto* circuit = dynamic_cast<GeneticCircuit*>(
		model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuit>(), selectedName.toStdString()));
	if (circuit == nullptr) {
		QMessageBox::warning(parentWidget, QObject::tr("Genetic Circuit"),
		                     QObject::tr("Could not resolve selected GeneticCircuit."));
		return nullptr;
	}
	return circuit;
}

QString buildGeneticCircuitInspectionReport(Model* model, GeneticCircuit* circuit) {
	// Reuse the circuit's own show() output so the GUI stays aligned with the model summary.
	std::ostringstream out;
	out << "Genetic Circuit Inspection\n";
	out << "circuit: " << (circuit != nullptr ? circuit->getName() : std::string("<null>")) << "\n";
	if (circuit == nullptr) {
		return QString::fromStdString(out.str());
	}

	out << "hostOrganism: " << circuit->getHostOrganism() << "\n";
	out << "compartment: " << circuit->getCompartment() << "\n";
	out << "enabled: " << boolText(circuit->isEnabled()) << "\n";
	out << "partCount: " << circuit->getPartNames().size() << "\n";
	out << "regulationCount: " << circuit->getRegulationNames().size() << "\n";
	out << "\nCircuit show()\n";
	out << circuit->show() << "\n";

	out << "\nParts\n";
	if (model != nullptr && model->getDataManager() != nullptr) {
		for (const std::string& partName : circuit->getPartNames()) {
			auto* part = dynamic_cast<GeneticCircuitPart*>(model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), partName));
			if (part == nullptr) {
				out << "  - " << partName << " <missing GeneticCircuitPart>\n";
				continue;
			}
			out << "  - " << part->show() << "\n";
		}

		out << "\nRegulations\n";
		for (const std::string& regulationName : circuit->getRegulationNames()) {
			auto* regulation = dynamic_cast<GeneticRegulation*>(model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticRegulation>(), regulationName));
			if (regulation == nullptr) {
				out << "  - " << regulationName << " <missing GeneticRegulation>\n";
				continue;
			}
			out << "  - " << regulation->show() << "\n";
		}
	}

	return QString::fromStdString(out.str());
}

QString buildGeneticCircuitSimulationReport(Model* model, GeneticCircuit* circuit,
                                            double startTime, double stopTime, double stepSize, bool applyRegulation,
                                            const GeneticCircuitSimulationSummary& summary) {
	std::ostringstream out;
	out << "Genetic Circuit Simulation\n";
	out << "circuit: " << (circuit != nullptr ? circuit->getName() : std::string("<null>")) << "\n";
	out << "startTime: " << formatDouble(startTime) << "\n";
	out << "stopTime: " << formatDouble(stopTime) << "\n";
	out << "stepSize: " << formatDouble(stepSize) << "\n";
	out << "applyRegulation: " << boolText(applyRegulation) << "\n";
	out << "succeeded: " << boolText(summary.succeeded) << "\n";
	out << "sampleCount: " << summary.sampleCount << "\n";
	out << "totalExpression: " << formatDouble(summary.totalExpression) << "\n";
	out << "message: " << summary.message << "\n";

	if (model != nullptr && model->getDataManager() != nullptr && circuit != nullptr) {
		out << "\nFinal species amounts\n";
		for (const std::string& partName : circuit->getPartNames()) {
			auto* part = dynamic_cast<GeneticCircuitPart*>(model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), partName));
			if (part == nullptr) {
				out << "  - " << partName << " <missing GeneticCircuitPart>\n";
				continue;
			}
			out << "  - " << part->show() << "\n";
			if (!part->getProductSpeciesName().empty()) {
				auto* species = dynamic_cast<BioSpecies*>(model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), part->getProductSpeciesName()));
				if (species != nullptr) {
					out << "    amount=" << formatDouble(species->getAmount())
					    << " initial=" << formatDouble(species->getInitialAmount())
					    << "\n";
				}
			}
		}
	}

	return QString::fromStdString(out.str());
}

} // namespace

class GeneticCircuitGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.genetic.circuit";
	}

	std::string displayName() const override {
		return "Genetic Circuit Viewer";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"geneticcircuit"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiActionContribution inspectAction;
		inspectAction.id = "actionGuiExtensionsGeneticCircuitInspect";
		inspectAction.menuPath = "Tools/Extensions/Biochemical";
		inspectAction.text = "Inspect Genetic Circuit...";
		inspectAction.statusTip = "Show the current GeneticCircuit, its parts, and its regulatory links.";
		inspectAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		inspectAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		inspectAction.trigger = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("Genetic Circuit"), QObject::tr("No opened model."));
				return;
			}

			GeneticCircuit* circuit = selectGeneticCircuit(model, parentWidget);
			if (circuit == nullptr) {
				return;
			}

			showTextDialog(parentWidget, QObject::tr("Genetic Circuit Inspection"),
			               buildGeneticCircuitInspectionReport(model, circuit));
		};
		registry->addAction(std::move(inspectAction));

		GuiActionContribution simulateAction;
		simulateAction.id = "actionGuiExtensionsGeneticCircuitSimulate";
		simulateAction.menuPath = "Tools/Extensions/Biochemical";
		simulateAction.text = "Run Genetic Circuit Simulation...";
		simulateAction.statusTip = "Simulate the selected GeneticCircuit with the standalone expression engine.";
		simulateAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		simulateAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		simulateAction.trigger = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("Genetic Circuit"), QObject::tr("No opened model."));
				return;
			}

			GeneticCircuit* circuit = selectGeneticCircuit(model, parentWidget);
			if (circuit == nullptr) {
				return;
			}

			const double startTime = 0.0;
			const double stopTime = 10.0;
			const double stepSize = 1.0;
			const bool applyRegulation = true;
			GeneticCircuitSimulationSummary summary;
			std::string errorMessage;
			if (!GeneticCircuitSimulate::simulateCircuit(model, circuit, startTime, stopTime, stepSize, applyRegulation, &summary, errorMessage)) {
				QMessageBox::warning(parentWidget, QObject::tr("Genetic Circuit"), QString::fromStdString(errorMessage));
				return;
			}

			// Show the standalone simulation summary together with the current part-level representation.
			showTextDialog(parentWidget, QObject::tr("Genetic Circuit Simulation"),
			               buildGeneticCircuitSimulationReport(model, circuit, startTime, stopTime, stepSize, applyRegulation, summary));
		};
		registry->addAction(std::move(simulateAction));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(GeneticCircuitGuiExtensionPlugin);
