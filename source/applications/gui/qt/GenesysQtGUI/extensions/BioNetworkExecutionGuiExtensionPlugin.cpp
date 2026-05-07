#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "../../../../../kernel/simulator/model/Model.h"
#include "../../../../../kernel/simulator/model/ModelDataManager.h"
#include "../../../../../kernel/simulator/model/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
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

void showJsonDialog(QWidget* parentWidget, const QString& title, const QString& text, const QString& defaultFileName) {
	auto* dialog = new QDialog(parentWidget);
	dialog->setWindowTitle(title);
	dialog->resize(980, 620);
	dialog->setAttribute(Qt::WA_DeleteOnClose);

	auto* layout = new QVBoxLayout(dialog);
	auto* editor = new QPlainTextEdit(dialog);
	editor->setReadOnly(true);
	editor->setPlainText(text);
	layout->addWidget(editor);

	auto* buttons = new QDialogButtonBox(dialog);
	auto* copyButton = buttons->addButton(QObject::tr("Copy JSON"), QDialogButtonBox::ActionRole);
	auto* saveButton = buttons->addButton(QObject::tr("Save JSON..."), QDialogButtonBox::ActionRole);
	auto* closeButton = buttons->addButton(QDialogButtonBox::Close);
	QObject::connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
	QObject::connect(copyButton, &QPushButton::clicked, [editor]() {
		// Keep the raw payload available for paste into external tools.
		QApplication::clipboard()->setText(editor->toPlainText());
	});
	QObject::connect(saveButton, &QPushButton::clicked, [dialog, editor, defaultFileName, title]() {
		const QString selectedFile = QFileDialog::getSaveFileName(
			dialog,
			title,
			defaultFileName,
			QObject::tr("JSON Files (*.json);;Text Files (*.txt);;All Files (*)"));
		if (selectedFile.isEmpty()) {
			return;
		}

		QFile file(selectedFile);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
			QMessageBox::warning(dialog, QObject::tr("BioNetwork Execution"),
			                     QObject::tr("Could not save JSON to %1.").arg(selectedFile));
			return;
		}

		QTextStream stream(&file);
		stream << editor->toPlainText();
		file.close();
	});
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

std::string joinNames(const std::vector<std::string>& values, const std::string& separator) {
	std::string joined;
	for (const std::string& value : values) {
		if (!joined.empty()) {
			joined += separator;
		}
		joined += value;
	}
	return joined;
}

std::string formatStoichiometricTerm(const BioReaction::StoichiometricTerm& term) {
	if (std::abs(term.stoichiometry - 1.0) < 1e-12) {
		return term.speciesName;
	}
	std::ostringstream out;
	out << term.speciesName << ":" << std::setprecision(15) << term.stoichiometry;
	return out.str();
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

QString buildAnalysisReport(Model* model, const BioNetwork* network, bool simulatedThisPass) {
	std::ostringstream out;
	out << "BioNetwork Analysis Report\n";
	out << "network: " << (network != nullptr ? network->getName() : std::string("<null>")) << "\n";
	out << "status: " << (network != nullptr ? network->getLastStatus() : std::string("<null>")) << "\n";
	out << "startTime: " << (network != nullptr ? formatDouble(network->getStartTime()) : std::string("0")) << "\n";
	out << "stopTime: " << (network != nullptr ? formatDouble(network->getStopTime()) : std::string("0")) << "\n";
	out << "stepSize: " << (network != nullptr ? formatDouble(network->getStepSize()) : std::string("0")) << "\n";
	out << "currentTime: " << (network != nullptr ? formatDouble(network->getCurrentTime()) : std::string("0")) << "\n";
	out << "autoSchedule: " << (network != nullptr ? boolText(network->getAutoSchedule()) : "false") << "\n";
	out << "simulatedThisPass: " << boolText(simulatedThisPass) << "\n";

	out << "\nSpecies\n";
	if (model != nullptr && model->getDataManager() != nullptr && network != nullptr) {
		if (network->getSpeciesNames().empty()) {
			out << "  membership: implicit discovery from the model\n";
			List<ModelDataDefinition*>* allSpecies = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());
			if (allSpecies != nullptr) {
				for (ModelDataDefinition* definition : *allSpecies->list()) {
					auto* species = dynamic_cast<BioSpecies*>(definition);
					if (species == nullptr) {
						continue;
					}
					out << "  - " << species->getName()
					    << " | initial=" << formatDouble(species->getInitialAmount())
					    << " | amount=" << formatDouble(species->getAmount())
					    << " | constant=" << boolText(species->isConstant())
					    << " | boundary=" << boolText(species->isBoundaryCondition())
					    << " | unit=" << species->getUnit() << "\n";
				}
			}
		} else {
			out << "  membership: " << joinNames(network->getSpeciesNames(), ", ") << "\n";
			for (const std::string& speciesName : network->getSpeciesNames()) {
				auto* species = dynamic_cast<BioSpecies*>(model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName));
				if (species == nullptr) {
					out << "  - " << speciesName << " <missing BioSpecies definition>\n";
					continue;
				}
				out << "  - " << species->getName()
				    << " | initial=" << formatDouble(species->getInitialAmount())
				    << " | amount=" << formatDouble(species->getAmount())
				    << " | constant=" << boolText(species->isConstant())
				    << " | boundary=" << boolText(species->isBoundaryCondition())
				    << " | unit=" << species->getUnit() << "\n";
			}
		}
	}

	out << "\nReactions\n";
	if (model != nullptr && model->getDataManager() != nullptr && network != nullptr) {
		if (network->getReactionNames().empty()) {
			out << "  membership: implicit discovery from the model\n";
			List<ModelDataDefinition*>* allReactions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioReaction>());
			if (allReactions != nullptr) {
				for (ModelDataDefinition* definition : *allReactions->list()) {
					auto* reaction = dynamic_cast<BioReaction*>(definition);
					if (reaction != nullptr) {
						out << "  - " << reaction->getName()
						    << " | reversible=" << boolText(reaction->isReversible())
						    << " | rateConstant=" << formatDouble(reaction->getRateConstant())
						    << "\n";
					}
				}
			}
		} else {
			out << "  membership: " << joinNames(network->getReactionNames(), ", ") << "\n";
			for (const std::string& reactionName : network->getReactionNames()) {
				auto* reaction = dynamic_cast<BioReaction*>(model->getDataManager()->getDataDefinition(Util::TypeOf<BioReaction>(), reactionName));
				if (reaction == nullptr) {
					out << "  - " << reactionName << " <missing BioReaction definition>\n";
					continue;
				}
				out << "  - " << reaction->getName()
				    << " | reversible=" << boolText(reaction->isReversible())
				    << " | rateConstant=" << formatDouble(reaction->getRateConstant())
				    << " | reverseRateConstant=" << formatDouble(reaction->getReverseRateConstant())
				    << " | rateParameter=" << reaction->getRateConstantParameterName()
				    << " | reverseRateParameter=" << reaction->getReverseRateConstantParameterName() << "\n";
				out << "    reactants: ";
				if (reaction->getReactants().empty()) {
					out << "<none>";
				} else {
					bool first = true;
					for (const BioReaction::StoichiometricTerm& term : reaction->getReactants()) {
						if (!first) {
							out << " + ";
						}
						out << formatStoichiometricTerm(term);
						first = false;
					}
				}
				out << "\n";
				out << "    products: ";
				if (reaction->getProducts().empty()) {
					out << "<none>";
				} else {
					bool first = true;
					for (const BioReaction::StoichiometricTerm& term : reaction->getProducts()) {
						if (!first) {
							out << " + ";
						}
						out << formatStoichiometricTerm(term);
						first = false;
					}
				}
				out << "\n";
				out << "    modifiers: ";
				if (reaction->getModifiers().empty()) {
					out << "<none>";
				} else {
					out << joinNames(reaction->getModifiers(), ", ");
				}
				out << "\n";
			}
		}
	}

	out << "\nStoichiometry matrix\n";
	if (network != nullptr) {
		BioStoichiometryMatrix matrix;
		std::string matrixError;
		if (network->getStoichiometryMatrix(&matrix, &matrixError)) {
			out << "  species\\reaction";
			for (const std::string& reactionName : matrix.reactionNames) {
				out << " | " << reactionName;
			}
			out << "\n";
			for (unsigned int i = 0; i < matrix.speciesNames.size(); ++i) {
				out << "  " << matrix.speciesNames[i];
				for (unsigned int j = 0; j < matrix.reactionNames.size(); ++j) {
					out << " | " << formatDouble(matrix.coefficient(i, j));
				}
				out << "\n";
			}
		} else {
			out << "  unavailable: " << matrixError << "\n";
		}
	}

	out << "\nSimulation result\n";
	if (network == nullptr || network->getLastSimulationResult().empty()) {
		out << "  <no simulation result available>\n";
	} else {
		const BioSimulationResult& result = network->getLastSimulationResult();
		out << "  samples: " << result.sampleCount() << "\n";
		out << "  timeWindow: [" << formatDouble(result.getStartTime()) << ", " << formatDouble(result.getStopTime()) << "]\n";
		out << "  species: " << joinNames(result.getSpeciesNames(), ", ") << "\n";
		const BioSimulationSample& lastSample = result.getSamples().back();
		out << "  lastSampleTime: " << formatDouble(lastSample.time) << "\n";
		for (const BioSimulationSpeciesAmount& amount : lastSample.species) {
			out << "    - " << amount.speciesName << " = " << formatDouble(amount.amount) << "\n";
		}

		out << "\nReaction rates\n";
		BioReactionRateTimeCourse rates;
		std::string ratesError;
		if (network->getReactionRateTimeCourse(&rates, &ratesError)) {
			out << "  samples: " << rates.samples.size() << "\n";
			for (unsigned int reactionIndex = 0; reactionIndex < rates.reactionNames.size(); ++reactionIndex) {
				const double firstNet = rates.samples.empty() ? 0.0 : rates.samples.front().netRates[reactionIndex];
				const double lastNet = rates.samples.empty() ? 0.0 : rates.samples.back().netRates[reactionIndex];
				out << "  - " << rates.reactionNames[reactionIndex]
				    << " | firstNet=" << formatDouble(firstNet)
				    << " | lastNet=" << formatDouble(lastNet) << "\n";
			}
		} else {
			out << "  unavailable: " << ratesError << "\n";
		}

		out << "\nSteady-state\n";
		BioSteadyStateCheck steadyState;
		std::string steadyError;
		if (network->checkLastSampleSteadyState(1e-9, &steadyState, &steadyError)) {
			out << "  tolerance: " << formatDouble(steadyState.tolerance) << "\n";
			out << "  steady: " << boolText(steadyState.steady) << "\n";
			out << "  maxAbsoluteDerivative: " << formatDouble(steadyState.maxAbsoluteDerivative) << "\n";
		} else {
			out << "  unavailable: " << steadyError << "\n";
		}

		out << "\nSensitivity\n";
		BioSensitivityScan sensitivity;
		std::string sensitivityError;
		if (network->scanLocalParameterSensitivity(0.01, 1.0e-6, &sensitivity, &sensitivityError)) {
			out << "  entries: " << sensitivity.entries.size() << "\n";
			for (const BioParameterSensitivityEntry& entry : sensitivity.entries) {
				out << "  - " << entry.parameterName
				    << " | baseValue=" << formatDouble(entry.baseValue)
				    << " | delta=" << formatDouble(entry.delta)
				    << " | maxAbsoluteSensitivity=" << formatDouble(entry.maxAbsoluteSensitivity) << "\n";
			}
		} else {
			out << "  unavailable: " << sensitivityError << "\n";
		}
	}

	return QString::fromStdString(out.str());
}

QString buildAnalysisJsonReport(Model* model, BioNetwork* network, bool simulatedThisPass) {
	if (model == nullptr || network == nullptr) {
		return QObject::tr("BioNetwork Analysis JSON\n<no network available>\n");
	}

	// Reuse the runner command so the GUI shows the same JSON payload the backend emits.
	BioSimulatorRunner runner(model, network->getName());
	runner.setTargetBioNetworkName(network->getName());
	runner.setCommand("reportJson()");

	std::string errorMessage;
	if (!runner.executeCommand(errorMessage)) {
		std::ostringstream out;
		out << "BioNetwork Analysis JSON\n";
		out << "network: " << network->getName() << "\n";
		out << "simulatedThisPass: " << boolText(simulatedThisPass) << "\n";
		out << "status: failed\n";
		out << "error: " << errorMessage << "\n";
		return QString::fromStdString(out.str());
	}

	std::ostringstream out;
	out << "BioNetwork Analysis JSON\n";
	out << "network: " << network->getName() << "\n";
	out << "simulatedThisPass: " << boolText(simulatedThisPass) << "\n";
	out << "payload:\n";
	out << runner.getLastResponsePayload() << "\n";
	return QString::fromStdString(out.str());
}

bool ensureNetworkSimulation(Model* model, QWidget* parentWidget, BioNetwork* network, std::string* errorMessage) {
	if (network == nullptr) {
		if (errorMessage != nullptr) {
			*errorMessage = "No BioNetwork selected.";
		}
		return false;
	}
	if (!network->getLastSimulationResult().empty()) {
		return true;
	}

	resetSelectedSpecies(model, network);
	ModelDataDefinition::InitBetweenReplications(network);
	std::string simulationError;
	if (!network->simulate(simulationError)) {
		if (errorMessage != nullptr) {
			*errorMessage = simulationError;
		}
		QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Execution"), QString::fromStdString(simulationError));
		return false;
	}
	if (errorMessage != nullptr) {
		errorMessage->clear();
	}
	return true;
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

		GuiActionContribution analysisAction;
		analysisAction.id = "actionGuiExtensionsBioNetworkAnalysis";
		analysisAction.menuPath = "Tools/Extensions/Biochemical";
		analysisAction.text = "Inspect BioNetwork Analysis...";
		analysisAction.statusTip = "Show a consolidated report with membership, stoichiometry, time-course, steady-state and sensitivity data.";
		analysisAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		analysisAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		analysisAction.trigger = [](const GuiExtensionRuntimeContext& context) {
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

			bool simulatedThisPass = false;
			if (network->getLastSimulationResult().empty()) {
				std::string simulationError;
				if (!ensureNetworkSimulation(model, parentWidget, network, &simulationError)) {
					return;
				}
				simulatedThisPass = true;
				model->setHasChanged(true);
			}

			showTextDialog(parentWidget, QObject::tr("BioNetwork Analysis Report"),
			               buildAnalysisReport(model, network, simulatedThisPass));
		};
		registry->addAction(std::move(analysisAction));

		GuiActionContribution analysisJsonAction;
		analysisJsonAction.id = "actionGuiExtensionsBioNetworkAnalysisJson";
		analysisJsonAction.menuPath = "Tools/Extensions/Biochemical";
		analysisJsonAction.text = "Inspect BioNetwork Analysis JSON...";
		analysisJsonAction.statusTip = "Show the JSON report generated by BioSimulatorRunner for the selected BioNetwork.";
		analysisJsonAction.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		analysisJsonAction.isEnabled = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		analysisJsonAction.trigger = [](const GuiExtensionRuntimeContext& context) {
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

			bool simulatedThisPass = false;
			if (network->getLastSimulationResult().empty()) {
				std::string simulationError;
				if (!ensureNetworkSimulation(model, parentWidget, network, &simulationError)) {
					return;
				}
				simulatedThisPass = true;
				model->setHasChanged(true);
			}

			// Present the backend JSON in a read-only dialog for quick inspection.
			showJsonDialog(parentWidget, QObject::tr("BioNetwork Analysis JSON"),
			               buildAnalysisJsonReport(model, network, simulatedThisPass),
			               QString::fromStdString(network->getName() + "_analysis.json"));
		};
		registry->addAction(std::move(analysisJsonAction));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioNetworkExecutionGuiExtensionPlugin);
