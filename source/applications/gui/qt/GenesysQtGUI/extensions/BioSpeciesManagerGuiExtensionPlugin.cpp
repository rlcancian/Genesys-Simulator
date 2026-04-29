#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

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
#include <vector>

namespace {

QString boolAsText(bool value) {
	return value ? QStringLiteral("true") : QStringLiteral("false");
}

bool promptBoolean(QWidget* parentWidget, const QString& title, const QString& label, bool currentValue, bool* ok) {
	const QStringList options = {QStringLiteral("false"), QStringLiteral("true")};
	const QString selected = QInputDialog::getItem(
		parentWidget,
		title,
		label,
		options,
		currentValue ? 1 : 0,
		false,
		ok);
	if (ok != nullptr && !*ok) {
		return currentValue;
	}
	return selected.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0;
}

QString formatSpeciesLine(const BioSpecies* species) {
	if (species == nullptr) {
		return QString();
	}
	return QStringLiteral("%1 | initial=%2 | amount=%3 | constant=%4 | boundary=%5 | unit=%6")
		.arg(QString::fromStdString(species->getName()))
		.arg(QString::number(species->getInitialAmount(), 'g', 15))
		.arg(QString::number(species->getAmount(), 'g', 15))
		.arg(boolAsText(species->isConstant()))
		.arg(boolAsText(species->isBoundaryCondition()))
		.arg(QString::fromStdString(species->getUnit()));
}

void showTextDialog(QWidget* parentWidget, const QString& title, const QString& text) {
	auto* dialog = new QDialog(parentWidget);
	dialog->setWindowTitle(title);
	dialog->resize(860, 520);
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

} // namespace

class BioSpeciesManagerGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.species.manager";
	}

	std::string displayName() const override {
		return "BioSpecies Manager";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"biospecies"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiWindowContribution managerWindow;
		managerWindow.id = "actionGuiExtensionsBioSpeciesManager";
		managerWindow.menuPath = "Tools/Extensions/Biochemical";
		managerWindow.text = "Manage BioSpecies...";
		managerWindow.isVisible = [](const GuiExtensionRuntimeContext& context) {
			return context.mainWindow != nullptr && context.simulator != nullptr;
		};
		managerWindow.open = [](const GuiExtensionRuntimeContext& context) {
			if (context.mainWindow == nullptr || context.simulator == nullptr) {
				return;
			}

			QWidget* parentWidget = static_cast<QWidget*>(context.mainWindow);
			Model* model = context.simulator->getModelManager() != nullptr
			               ? context.simulator->getModelManager()->current()
			               : nullptr;
			if (model == nullptr || model->getDataManager() == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("No opened model."));
				return;
			}

			QStringList operations;
			operations << QObject::tr("Create")
			           << QObject::tr("Edit")
			           << QObject::tr("List");
			bool ok = false;
			const QString operation = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioSpecies Manager"),
				QObject::tr("Operation:"),
				operations,
				0,
				false,
				&ok);
			if (!ok || operation.isEmpty()) {
				return;
			}

			List<ModelDataDefinition*>* definitions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());

			if (operation == QObject::tr("List")) {
				QStringList lines;
				for (ModelDataDefinition* definition : *definitions->list()) {
					auto* species = dynamic_cast<BioSpecies*>(definition);
					if (species != nullptr) {
						lines << formatSpeciesLine(species);
					}
				}
				lines.sort(Qt::CaseInsensitive);
				const QString body = lines.isEmpty()
				                     ? QObject::tr("No BioSpecies definitions in current model.")
				                     : lines.join("\n");
				showTextDialog(parentWidget, QObject::tr("BioSpecies List"), body);
				return;
			}

			if (operation == QObject::tr("Create")) {
				const QString name = QInputDialog::getText(
					parentWidget,
					QObject::tr("BioSpecies Manager"),
					QObject::tr("BioSpecies name:"),
					QLineEdit::Normal,
					QString(),
					&ok).trimmed();
				if (!ok) {
					return;
				}
				if (name.isEmpty()) {
					QMessageBox::warning(parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("BioSpecies name cannot be empty."));
					return;
				}
				if (model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), name.toStdString()) != nullptr) {
					QMessageBox::warning(parentWidget, QObject::tr("BioSpecies Manager"),
					                     QObject::tr("A BioSpecies with this name already exists."));
					return;
				}

				const double initialAmount = QInputDialog::getDouble(
					parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Initial amount (>= 0):"),
					0.0, 0.0, 1e12, 9, &ok);
				if (!ok) {
					return;
				}
				const double amount = QInputDialog::getDouble(
					parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Current amount (>= 0):"),
					initialAmount, 0.0, 1e12, 9, &ok);
				if (!ok) {
					return;
				}
				const bool constant = promptBoolean(
					parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Constant flag:"), false, &ok);
				if (!ok) {
					return;
				}
				const bool boundaryCondition = promptBoolean(
					parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Boundary condition flag:"), false, &ok);
				if (!ok) {
					return;
				}
				const QString unit = QInputDialog::getText(
					parentWidget,
					QObject::tr("BioSpecies Manager"),
					QObject::tr("Unit (optional):"),
					QLineEdit::Normal,
					QString(),
					&ok).trimmed();
				if (!ok) {
					return;
				}

				auto* species = new BioSpecies(model, name.toStdString());
				species->setInitialAmount(initialAmount);
				species->setAmount(amount);
				species->setConstant(constant);
				species->setBoundaryCondition(boundaryCondition);
				species->setUnit(unit.toStdString());
				species->setHasChanged(true);
				model->setHasChanged(true);
				QMessageBox::information(parentWidget, QObject::tr("BioSpecies Manager"),
				                         QObject::tr("BioSpecies created successfully."));
				return;
			}

			if (definitions == nullptr || definitions->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioSpecies Manager"),
				                         QObject::tr("No BioSpecies definitions available in current model."));
				return;
			}

			QStringList names;
			for (ModelDataDefinition* definition : *definitions->list()) {
				if (definition != nullptr) {
					names << QString::fromStdString(definition->getName());
				}
			}
			names.sort(Qt::CaseInsensitive);
			const QString selectedName = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioSpecies Manager"),
				QObject::tr("Select BioSpecies:"),
				names,
				0,
				false,
				&ok);
			if (!ok || selectedName.isEmpty()) {
				return;
			}

			auto* species = dynamic_cast<BioSpecies*>(model->getDataManager()->getDataDefinition(
				Util::TypeOf<BioSpecies>(), selectedName.toStdString()));
			if (species == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioSpecies Manager"),
				                     QObject::tr("Could not resolve selected BioSpecies."));
				return;
			}

			const double initialAmount = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Initial amount (>= 0):"),
				species->getInitialAmount(), 0.0, 1e12, 9, &ok);
			if (!ok) {
				return;
			}
			const double amount = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Current amount (>= 0):"),
				species->getAmount(), 0.0, 1e12, 9, &ok);
			if (!ok) {
				return;
			}
			const bool constant = promptBoolean(
				parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Constant flag:"),
				species->isConstant(), &ok);
			if (!ok) {
				return;
			}
			const bool boundaryCondition = promptBoolean(
				parentWidget, QObject::tr("BioSpecies Manager"), QObject::tr("Boundary condition flag:"),
				species->isBoundaryCondition(), &ok);
			if (!ok) {
				return;
			}
			const QString unit = QInputDialog::getText(
				parentWidget,
				QObject::tr("BioSpecies Manager"),
				QObject::tr("Unit (optional):"),
				QLineEdit::Normal,
				QString::fromStdString(species->getUnit()),
				&ok).trimmed();
			if (!ok) {
				return;
			}

			species->setInitialAmount(initialAmount);
			species->setAmount(amount);
			species->setConstant(constant);
			species->setBoundaryCondition(boundaryCondition);
			species->setUnit(unit.toStdString());
			species->setHasChanged(true);
			model->setHasChanged(true);
			QMessageBox::information(parentWidget, QObject::tr("BioSpecies Manager"),
			                         QObject::tr("BioSpecies updated successfully."));
		};
		registry->addWindow(std::move(managerWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioSpeciesManagerGuiExtensionPlugin);
