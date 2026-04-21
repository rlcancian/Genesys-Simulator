#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QRegularExpression>
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

class BioNetworkEditorGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.network.editor";
	}

	std::string displayName() const override {
		return "BioNetwork Editor";
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

		GuiWindowContribution editorWindow;
		editorWindow.id = "actionGuiExtensionsBioNetworkEditor";
		editorWindow.menuPath = "Tools/Extensions/Biochemical";
		editorWindow.text = "Edit BioNetwork Membership...";
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
				QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Editor"), QObject::tr("No opened model."));
				return;
			}

			List<ModelDataDefinition*>* networks = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
			if (networks == nullptr || networks->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioNetwork Editor"),
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
			const QString selectedName = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioNetwork Editor"),
				QObject::tr("Select BioNetwork:"),
				networkNames,
				0,
				false,
				&ok);
			if (!ok || selectedName.isEmpty()) {
				return;
			}

			auto* network = dynamic_cast<BioNetwork*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), selectedName.toStdString()));
			if (network == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioNetwork Editor"),
				                     QObject::tr("Could not resolve selected BioNetwork."));
				return;
			}

			const QString speciesInput = QInputDialog::getMultiLineText(
				parentWidget,
				QObject::tr("BioNetwork Editor"),
				QObject::tr("Species membership (one per line or separated by comma):"),
				namesToMultiline(network->getSpeciesNames()),
				&ok);
			if (!ok) {
				return;
			}

			const QString reactionsInput = QInputDialog::getMultiLineText(
				parentWidget,
				QObject::tr("BioNetwork Editor"),
				QObject::tr("Reaction membership (one per line or separated by comma):"),
				namesToMultiline(network->getReactionNames()),
				&ok);
			if (!ok) {
				return;
			}

			bool startOk = false;
			const double startTime = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioNetwork Editor"), QObject::tr("Start time:"),
				network->getStartTime(), -1e12, 1e12, 6, &startOk);
			if (!startOk) {
				return;
			}
			bool stopOk = false;
			const double stopTime = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioNetwork Editor"), QObject::tr("Stop time:"),
				network->getStopTime(), -1e12, 1e12, 6, &stopOk);
			if (!stopOk) {
				return;
			}
			bool stepOk = false;
			const double stepSize = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioNetwork Editor"), QObject::tr("Step size (> 0):"),
				network->getStepSize(), 1e-12, 1e12, 9, &stepOk);
			if (!stepOk) {
				return;
			}

			network->clearSpecies();
			for (const std::string& name : parseNameList(speciesInput)) {
				network->addSpecies(name);
			}
			network->clearReactions();
			for (const std::string& name : parseNameList(reactionsInput)) {
				network->addReaction(name);
			}
			network->setStartTime(startTime);
			network->setStopTime(stopTime);
			network->setStepSize(stepSize);
			network->setHasChanged(true);
			model->setHasChanged(true);

			QMessageBox::information(parentWidget, QObject::tr("BioNetwork Editor"),
			                         QObject::tr("BioNetwork updated. Run model check/simulation to validate kinetics and memberships."));
		};
		registry->addWindow(std::move(editorWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioNetworkEditorGuiExtensionPlugin);
