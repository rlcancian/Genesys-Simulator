#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"

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
#include <vector>

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

double speciesValueForSample(const BioSimulationSample& sample, const std::string& speciesName, bool* found) {
	for (const BioSimulationSpeciesAmount& amount : sample.species) {
		if (amount.speciesName == speciesName) {
			if (found != nullptr) {
				*found = true;
			}
			return amount.amount;
		}
	}
	if (found != nullptr) {
		*found = false;
	}
	return 0.0;
}

void appendSteadyStateSection(const BioSteadyStateCheck& check, std::ostringstream* out) {
	if (out == nullptr) {
		return;
	}
	*out << "Steady-State Check\n";
	*out << "steady: " << (check.steady ? "true" : "false") << "\n";
	*out << "tolerance: " << std::setprecision(15) << check.tolerance << "\n";
	*out << "maxAbsoluteDerivative: " << std::setprecision(15) << check.maxAbsoluteDerivative << "\n";
	*out << "derivatives (species, d/dt):\n";
	for (const BioSpeciesDerivative& derivative : check.derivatives) {
		*out << "  - " << derivative.speciesName << " = " << std::setprecision(15) << derivative.derivative << "\n";
	}
}

void appendSensitivitySection(const BioSensitivityScan& scan, std::ostringstream* out) {
	if (out == nullptr) {
		return;
	}
	*out << "Local Parameter Sensitivity Scan\n";
	*out << "time: " << std::setprecision(15) << scan.time << "\n";
	if (scan.entries.empty()) {
		*out << "No local parameters available for sensitivity scan.\n";
		return;
	}
	for (const BioParameterSensitivityEntry& entry : scan.entries) {
		*out << "parameter: " << entry.parameterName
		     << ", baseValue=" << std::setprecision(15) << entry.baseValue
		     << ", delta=" << std::setprecision(15) << entry.delta
		     << ", maxAbsSensitivity=" << std::setprecision(15) << entry.maxAbsoluteSensitivity << "\n";
		for (const BioSpeciesDerivative& derivative : entry.derivativeSensitivities) {
			*out << "  - d(d" << derivative.speciesName << "/dt)/d" << entry.parameterName
			     << " = " << std::setprecision(15) << derivative.derivative << "\n";
		}
	}
}

} // namespace

class BioSimulationResultsGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.results.viewer";
	}

	std::string displayName() const override {
		return "BioSimulation Results Viewer";
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

		GuiWindowContribution resultsWindow;
		resultsWindow.id = "actionGuiExtensionsBioResultsViewer";
		resultsWindow.menuPath = "Tools/Extensions/Biochemical";
		resultsWindow.text = "View BioSimulation Results...";
		resultsWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		resultsWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"), QObject::tr("No opened model."));
				return;
			}

			List<ModelDataDefinition*>* networks = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
			if (networks == nullptr || networks->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioSimulation Results"),
				                         QObject::tr("No BioNetwork definitions available in current model."));
				return;
			}

			QStringList networkNames;
			for (ModelDataDefinition* def : *networks->list()) {
				if (def != nullptr) {
					networkNames << QString::fromStdString(def->getName());
				}
			}
			networkNames.sort(Qt::CaseInsensitive);
			bool ok = false;
			const QString selectedNetworkName = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioSimulation Results"),
				QObject::tr("Select BioNetwork:"),
				networkNames,
				0,
				false,
				&ok);
			if (!ok || selectedNetworkName.isEmpty()) {
				return;
			}

			auto* network = dynamic_cast<BioNetwork*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), selectedNetworkName.toStdString()));
			if (network == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"),
				                     QObject::tr("Could not resolve selected BioNetwork."));
				return;
			}

			const BioSimulationResult& result = network->getLastSimulationResult();
			if (result.empty()) {
				QMessageBox::information(parentWidget, QObject::tr("BioSimulation Results"),
				                         QObject::tr("Selected BioNetwork has no simulation result yet."));
				return;
			}

			QStringList modes;
			modes << QObject::tr("Species Curves")
			      << QObject::tr("Reaction Rates")
			      << QObject::tr("Steady-State Check")
			      << QObject::tr("Sensitivity Scan")
			      << QObject::tr("Full Analysis Report");
			const QString mode = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioSimulation Results"),
				QObject::tr("View mode:"),
				modes,
				0,
				false,
				&ok);
			if (!ok || mode.isEmpty()) {
				return;
			}

			if (mode == QObject::tr("Species Curves")) {
				QStringList speciesNames;
				for (const std::string& speciesName : result.getSpeciesNames()) {
					speciesNames << QString::fromStdString(speciesName);
				}
				speciesNames.sort(Qt::CaseInsensitive);
				const QString selectedSpecies = QInputDialog::getItem(
					parentWidget,
					QObject::tr("BioSimulation Results"),
					QObject::tr("Select BioSpecies:"),
					speciesNames,
					0,
					false,
					&ok);
				if (!ok || selectedSpecies.isEmpty()) {
					return;
				}

				std::ostringstream out;
				out << "BioSpecies Time Course\n";
				out << "network: " << network->getName() << "\n";
				out << "species: " << selectedSpecies.toStdString() << "\n";
				out << "samples: " << result.sampleCount() << "\n\n";
				out << "time\tamount\n";
				for (const BioSimulationSample& sample : result.getSamples()) {
					bool found = false;
					const double amount = speciesValueForSample(sample, selectedSpecies.toStdString(), &found);
					if (!found) {
						continue;
					}
					out << std::setprecision(15) << sample.time << "\t" << std::setprecision(15) << amount << "\n";
				}
				showTextDialog(parentWidget, QObject::tr("BioSpecies Time Course"), QString::fromStdString(out.str()));
				return;
			}

			if (mode == QObject::tr("Reaction Rates")) {
				BioReactionRateTimeCourse timeCourse;
				std::string errorMessage;
				if (!network->getReactionRateTimeCourse(&timeCourse, &errorMessage)) {
					QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"),
					                     QString::fromStdString(errorMessage));
					return;
				}
				if (timeCourse.reactionNames.empty()) {
					QMessageBox::information(parentWidget, QObject::tr("BioSimulation Results"),
					                         QObject::tr("No reactions are available for rate analysis."));
					return;
				}

				QStringList reactionChoices;
				reactionChoices << QObject::tr("<all reactions>");
				for (const std::string& reactionName : timeCourse.reactionNames) {
					reactionChoices << QString::fromStdString(reactionName);
				}
				const QString selectedReaction = QInputDialog::getItem(
					parentWidget,
					QObject::tr("BioSimulation Results"),
					QObject::tr("Reaction:"),
					reactionChoices,
					0,
					false,
					&ok);
				if (!ok || selectedReaction.isEmpty()) {
					return;
				}

				std::ostringstream out;
				out << "Reaction-Rate Time Course\n";
				out << "network: " << network->getName() << "\n\n";
				if (selectedReaction == QObject::tr("<all reactions>")) {
					out << "time";
					for (const std::string& reactionName : timeCourse.reactionNames) {
						out << "\t" << reactionName << ".forward"
						    << "\t" << reactionName << ".reverse"
						    << "\t" << reactionName << ".net";
					}
					out << "\n";
					for (const BioReactionRateSample& sample : timeCourse.samples) {
						out << std::setprecision(15) << sample.time;
						for (unsigned int i = 0; i < timeCourse.reactionNames.size(); ++i) {
							out << "\t" << std::setprecision(15) << sample.forwardRates[i]
							    << "\t" << std::setprecision(15) << sample.reverseRates[i]
							    << "\t" << std::setprecision(15) << sample.netRates[i];
						}
						out << "\n";
					}
				} else {
					const std::string reactionName = selectedReaction.toStdString();
					const auto reactionIt = std::find(timeCourse.reactionNames.begin(), timeCourse.reactionNames.end(), reactionName);
					if (reactionIt == timeCourse.reactionNames.end()) {
						QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"),
						                     QObject::tr("Selected reaction is no longer available."));
						return;
					}
					const unsigned int reactionIndex = static_cast<unsigned int>(reactionIt - timeCourse.reactionNames.begin());
					out << "reaction: " << reactionName << "\n\n";
					out << "time\tforwardRate\treverseRate\tnetRate\n";
					for (const BioReactionRateSample& sample : timeCourse.samples) {
						out << std::setprecision(15) << sample.time
						    << "\t" << std::setprecision(15) << sample.forwardRates[reactionIndex]
						    << "\t" << std::setprecision(15) << sample.reverseRates[reactionIndex]
						    << "\t" << std::setprecision(15) << sample.netRates[reactionIndex] << "\n";
					}
				}

				showTextDialog(parentWidget, QObject::tr("Reaction Rates"), QString::fromStdString(out.str()));
				return;
			}

			if (mode == QObject::tr("Steady-State Check")) {
				const double tolerance = QInputDialog::getDouble(
					parentWidget,
					QObject::tr("BioSimulation Results"),
					QObject::tr("Tolerance (>= 0):"),
					1e-6,
					0.0,
					1e6,
					12,
					&ok);
				if (!ok) {
					return;
				}

				BioSteadyStateCheck steady;
				std::string errorMessage;
				if (!network->checkLastSampleSteadyState(tolerance, &steady, &errorMessage)) {
					QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"),
					                     QString::fromStdString(errorMessage));
					return;
				}

				std::ostringstream out;
				out << "BioNetwork Steady-State Check\n";
				out << "network: " << network->getName() << "\n\n";
				appendSteadyStateSection(steady, &out);
				showTextDialog(parentWidget, QObject::tr("Steady-State Check"), QString::fromStdString(out.str()));
				return;
			}

			if (mode == QObject::tr("Sensitivity Scan")) {
				const double relativeStep = QInputDialog::getDouble(
					parentWidget,
					QObject::tr("BioSimulation Results"),
					QObject::tr("Relative step (>= 0):"),
					0.01,
					0.0,
					1.0,
					8,
					&ok);
				if (!ok) {
					return;
				}
				const double absoluteStep = QInputDialog::getDouble(
					parentWidget,
					QObject::tr("BioSimulation Results"),
					QObject::tr("Absolute step (> 0):"),
					1e-6,
					1e-15,
					1e6,
					12,
					&ok);
				if (!ok) {
					return;
				}

				BioSensitivityScan scan;
				std::string errorMessage;
				if (!network->scanLocalParameterSensitivity(relativeStep, absoluteStep, &scan, &errorMessage)) {
					QMessageBox::warning(parentWidget, QObject::tr("BioSimulation Results"),
					                     QString::fromStdString(errorMessage));
					return;
				}

				std::ostringstream out;
				out << "BioNetwork Sensitivity Scan\n";
				out << "network: " << network->getName() << "\n";
				out << "relativeStep: " << std::setprecision(15) << relativeStep << "\n";
				out << "absoluteStep: " << std::setprecision(15) << absoluteStep << "\n\n";
				appendSensitivitySection(scan, &out);
				showTextDialog(parentWidget, QObject::tr("Sensitivity Scan"), QString::fromStdString(out.str()));
				return;
			}

			const double tolerance = QInputDialog::getDouble(
				parentWidget,
				QObject::tr("BioSimulation Results"),
				QObject::tr("Steady-state tolerance (>= 0):"),
				1e-6,
				0.0,
				1e6,
				12,
				&ok);
			if (!ok) {
				return;
			}
			const double relativeStep = QInputDialog::getDouble(
				parentWidget,
				QObject::tr("BioSimulation Results"),
				QObject::tr("Sensitivity relative step (>= 0):"),
				0.01,
				0.0,
				1.0,
				8,
				&ok);
			if (!ok) {
				return;
			}
			const double absoluteStep = QInputDialog::getDouble(
				parentWidget,
				QObject::tr("BioSimulation Results"),
				QObject::tr("Sensitivity absolute step (> 0):"),
				1e-6,
				1e-15,
				1e6,
				12,
				&ok);
			if (!ok) {
				return;
			}

			std::ostringstream out;
			out << "BioNetwork Full Analysis Report\n";
			out << "network: " << network->getName() << "\n";
			out << "status: " << network->getLastStatus() << "\n";
			out << "samples: " << result.sampleCount() << "\n";
			out << "timeRange: [" << std::setprecision(15) << result.getStartTime()
			    << ", " << std::setprecision(15) << result.getStopTime() << "]\n";
			out << "stepSize: " << std::setprecision(15) << result.getStepSize() << "\n";
			out << "speciesCount: " << result.getSpeciesNames().size() << "\n\n";

			if (!result.getSamples().empty()) {
				out << "Last Sample Species Amounts\n";
				const BioSimulationSample& lastSample = result.getSamples().back();
				out << "time: " << std::setprecision(15) << lastSample.time << "\n";
				for (const BioSimulationSpeciesAmount& amount : lastSample.species) {
					out << "  - " << amount.speciesName << " = " << std::setprecision(15) << amount.amount << "\n";
				}
				out << "\n";
			}

			BioReactionRateTimeCourse rates;
			std::string errorMessage;
			if (network->getReactionRateTimeCourse(&rates, &errorMessage) && !rates.samples.empty()) {
				out << "Last Sample Reaction Rates\n";
				const BioReactionRateSample& lastRateSample = rates.samples.back();
				for (unsigned int i = 0; i < rates.reactionNames.size(); ++i) {
					out << "  - " << rates.reactionNames[i]
					    << " (forward=" << std::setprecision(15) << lastRateSample.forwardRates[i]
					    << ", reverse=" << std::setprecision(15) << lastRateSample.reverseRates[i]
					    << ", net=" << std::setprecision(15) << lastRateSample.netRates[i] << ")\n";
				}
				out << "\n";
			} else {
				out << "Reaction-rate analysis unavailable: " << errorMessage << "\n\n";
			}

			BioSteadyStateCheck steady;
			errorMessage.clear();
			if (network->checkLastSampleSteadyState(tolerance, &steady, &errorMessage)) {
				appendSteadyStateSection(steady, &out);
				out << "\n";
			} else {
				out << "Steady-state analysis unavailable: " << errorMessage << "\n\n";
			}

			BioSensitivityScan scan;
			errorMessage.clear();
			if (network->scanLocalParameterSensitivity(relativeStep, absoluteStep, &scan, &errorMessage)) {
				appendSensitivitySection(scan, &out);
			} else {
				out << "Sensitivity analysis unavailable: " << errorMessage << "\n";
			}

			showTextDialog(parentWidget, QObject::tr("BioNetwork Full Analysis Report"), QString::fromStdString(out.str()));
		};
		registry->addWindow(std::move(resultsWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioSimulationResultsGuiExtensionPlugin);
