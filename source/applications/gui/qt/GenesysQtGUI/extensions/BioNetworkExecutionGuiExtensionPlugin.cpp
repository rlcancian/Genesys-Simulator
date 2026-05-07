#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

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

BioNetwork* selectBioNetwork(Model* model, QWidget* parentWidget) {
	if (model == nullptr || model->getDataManager() == nullptr) {
		return nullptr;
	}

	List<ModelDataDefinition*>* networks = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
	if (networks == nullptr || networks->size() == 0) {
		QMessageBox::information(parentWidget, QObject::tr("BioNetwork Execution"),
		                         QObject::tr("No BioNetwork definitions available in current model."));
		return nullptr;
	}

	QStringList networkNames;
	for (ModelDataDefinition* definition : *networks->list()) {
		if (definition != nullptr) {
			networkNames << QString::fromStdString(definition->getName());
		}
	}
	networkNames.sort(Qt::CaseInsensitive);

	bool ok = false;
	const QString selectedName = QInputDialog::getItem(
		parentWidget,
		QObject::tr("BioNetwork Execution"),
		QObject::tr("Select BioNetwork:"),
		networkNames,
		0,
		false,
		&ok);
	if (!ok || selectedName.isEmpty()) {
		return nullptr;
	}

	auto* network = dynamic_cast<BioNetwork*>(
		model->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), selectedName.toStdString()));
	if (network == nullptr) {
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"),
		                     QObject::tr("Could not resolve selected BioNetwork."));
		return nullptr;
	}
	return network;
}

void resetSelectedSpecies(Model* model, const BioNetwork* network) {
	if (model == nullptr || network == nullptr || model->getDataManager() == nullptr) {
		return;
	}

	const std::vector<std::string>& speciesNames = network->getSpeciesNames();
	if (speciesNames.empty()) {
		List<ModelDataDefinition*>* allSpecies = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());
		if (allSpecies == nullptr) {
			return;
		}
		for (ModelDataDefinition* definition : *allSpecies->list()) {
			if (definition != nullptr) {
				// Keep the GUI action reproducible by restoring the species initial amount before simulation.
				ModelDataDefinition::InitBetweenReplications(definition);
			}
		}
		return;
	}

	for (const std::string& speciesName : speciesNames) {
		ModelDataDefinition* definition = model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName);
		if (definition != nullptr) {
			ModelDataDefinition::InitBetweenReplications(definition);
		}
	}
}

QString buildSimulationReport(const BioNetwork* network, double startTime, double stopTime, double stepSize) {
	std::ostringstream out;
	out << "BioNetwork Simulation\n";
	out << "network: " << (network != nullptr ? network->getName() : std::string("<null>")) << "\n";
	out << "startTime: " << std::setprecision(15) << startTime << "\n";
	out << "stopTime: " << std::setprecision(15) << stopTime << "\n";
	out << "stepSize: " << std::setprecision(15) << stepSize << "\n";
	if (network != nullptr) {
		const BioSimulationResult& result = network->getLastSimulationResult();
		out << "status: " << network->getLastStatus() << "\n";
		out << "samples: " << result.sampleCount() << "\n";
		if (!result.empty()) {
			const BioSimulationSample& lastSample = result.getSamples().back();
			out << "lastTime: " << std::setprecision(15) << lastSample.time << "\n";
			out << "lastSampleSpecies:\n";
			for (const BioSimulationSpeciesAmount& amount : lastSample.species) {
				out << "  - " << amount.speciesName << " = " << std::setprecision(15) << amount.amount << "\n";
			}
		}
		out << "\nlastResponsePayload:\n";
		out << network->getLastResponsePayload() << "\n";
	}
	return QString::fromStdString(out.str());
}

QString buildSteadyStateReport(const BioNetwork* network, const BioSteadyStateCheck& check, double tolerance) {
	std::ostringstream out;
	out << "BioNetwork Steady-State Check\n";
	out << "network: " << (network != nullptr ? network->getName() : std::string("<null>")) << "\n";
	out << "tolerance: " << std::setprecision(15) << tolerance << "\n";
	out << "steady: " << (check.steady ? "true" : "false") << "\n";
	out << "maxAbsoluteDerivative: " << std::setprecision(15) << check.maxAbsoluteDerivative << "\n";
	out << "derivatives:\n";
	for (const BioSpeciesDerivative& derivative : check.derivatives) {
		out << "  - " << derivative.speciesName << " = " << std::setprecision(15) << derivative.derivative << "\n";
	}
	if (network != nullptr) {
		out << "\nlastResponsePayload:\n";
		out << network->getLastResponsePayload() << "\n";
	}
	return QString::fromStdString(out.str());
}

void runSimulationForNetwork(QWidget* parentWidget, Model* model, BioNetwork* network) {
	if (parentWidget == nullptr || model == nullptr || network == nullptr) {
		return;
	}

	bool ok = false;
	const double startTime = QInputDialog::getDouble(
		parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("Start time:"),
		network->getStartTime(), -1e12, 1e12, 6, &ok);
	if (!ok) {
		return;
	}
	const double stopTime = QInputDialog::getDouble(
		parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("Stop time:"),
		network->getStopTime(), -1e12, 1e12, 6, &ok);
	if (!ok) {
		return;
	}
	const double stepSize = QInputDialog::getDouble(
		parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("Step size (> 0):"),
		network->getStepSize(), 1e-12, 1e12, 9, &ok);
	if (!ok) {
		return;
	}
	if (stopTime <= startTime) {
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"),
		                     QObject::tr("Stop time must be greater than start time."));
		return;
	}
	if (stepSize <= 0.0) {
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"),
		                     QObject::tr("Step size must be greater than zero."));
		return;
	}

	// Restore the selected network to its initial amounts before executing the new time course.
	resetSelectedSpecies(model, network);
	ModelDataDefinition::InitBetweenReplications(network);

	std::string errorMessage;
	if (!network->simulate(startTime, stopTime, stepSize, errorMessage)) {
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"), QString::fromStdString(errorMessage));
		return;
	}

	model->setHasChanged(true);
	showTextDialog(parentWidget, QObject::tr("BioNetwork Simulation Result"),
	               buildSimulationReport(network, startTime, stopTime, stepSize));
}

void checkSteadyStateForNetwork(QWidget* parentWidget, Model* model, BioNetwork* network) {
	if (parentWidget == nullptr || model == nullptr || network == nullptr) {
		return;
	}

	bool ok = false;
	const double tolerance = QInputDialog::getDouble(
		parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("Steady-state tolerance (>= 0):"),
		1e-6, 0.0, 1e6, 12, &ok);
	if (!ok) {
		return;
	}

	if (network->getLastSimulationResult().empty()) {
		// If the user has not run a time course yet, simulate once using the network's own settings.
		resetSelectedSpecies(model, network);
		ModelDataDefinition::InitBetweenReplications(network);
		std::string simulationError;
		if (!network->simulate(simulationError)) {
			QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"),
			                     QString::fromStdString(simulationError));
			return;
		}
	}

	BioSteadyStateCheck check;
	std::string errorMessage;
	if (!network->checkLastSampleSteadyState(tolerance, &check, &errorMessage)) {
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"), QString::fromStdString(errorMessage));
		return;
	}

	model->setHasChanged(true);
	showTextDialog(parentWidget, QObject::tr("BioNetwork Steady-State Result"),
	               buildSteadyStateReport(network, check, tolerance));
}

} // namespace

class BioNetworkExecutionGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.network.execution";
	}

	std::string displayName() const override {
		return "BioNetwork Execution";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"bionetwork"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiActionContribution simulateAction;
		simulateAction.id = "actionGuiExtensionsBioNetworkSimulate";
		simulateAction.menuPath = "Tools/Extensions/Biochemical";
		simulateAction.text = "Run BioNetwork Simulation...";
		simulateAction.statusTip = "Execute the selected BioNetwork with the native deterministic solver.";
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
				QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("No opened model."));
				return;
			}

			BioNetwork* network = selectBioNetwork(model, parentWidget);
			if (network == nullptr) {
				return;
			}
			runSimulationForNetwork(parentWidget, model, network);
		};
		registry->addAction(std::move(simulateAction));

		GuiActionContribution steadyAction;
		steadyAction.id = "actionGuiExtensionsBioNetworkSteadyState";
		steadyAction.menuPath = "Tools/Extensions/Biochemical";
		steadyAction.text = "Check BioNetwork Steady-State...";
		steadyAction.statusTip = "Evaluate steady-state convergence for the selected BioNetwork.";
		steadyAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		steadyAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		steadyAction.trigger = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"), QObject::tr("No opened model."));
				return;
			}

			BioNetwork* network = selectBioNetwork(model, parentWidget);
			if (network == nullptr) {
				return;
			}
			checkSteadyStateForNetwork(parentWidget, model, network);
		};
		registry->addAction(std::move(steadyAction));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioNetworkExecutionGuiExtensionPlugin);
