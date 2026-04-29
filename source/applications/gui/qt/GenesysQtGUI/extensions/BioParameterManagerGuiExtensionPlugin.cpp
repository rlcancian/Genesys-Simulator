#include "GuiExtensionPluginCatalog.h"

#include "kernel/TraitsKernel.h"
#include "kernel/util/List.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"

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

QString formatParameterLine(const BioParameter* parameter) {
	if (parameter == nullptr) {
		return QString();
	}
	return QStringLiteral("%1 | value=%2 | unit=%3")
		.arg(QString::fromStdString(parameter->getName()))
		.arg(QString::number(parameter->getValue(), 'g', 15))
		.arg(QString::fromStdString(parameter->getUnit()));
}

void showTextDialog(QWidget* parentWidget, const QString& title, const QString& text) {
	auto* dialog = new QDialog(parentWidget);
	dialog->setWindowTitle(title);
	dialog->resize(760, 460);
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

class BioParameterManagerGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override {
		return "gui.extensions.bio.parameter.manager";
	}

	std::string displayName() const override {
		return "BioParameter Manager";
	}

	std::string version() const override {
		return "1.0.0";
	}

	std::vector<std::string> requiredModelPlugins() const override {
		return {"bioparameter"};
	}

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) {
			return;
		}

		GuiWindowContribution managerWindow;
		managerWindow.id = "actionGuiExtensionsBioParameterManager";
		managerWindow.menuPath = "Tools/Extensions/Biochemical";
		managerWindow.text = "Manage BioParameters...";
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
				QMessageBox::warning(parentWidget, QObject::tr("BioParameter Manager"), QObject::tr("No opened model."));
				return;
			}

			QStringList operations;
			operations << QObject::tr("Create")
			           << QObject::tr("Edit")
			           << QObject::tr("List");
			bool ok = false;
			const QString operation = QInputDialog::getItem(
				parentWidget,
				QObject::tr("BioParameter Manager"),
				QObject::tr("Operation:"),
				operations,
				0,
				false,
				&ok);
			if (!ok || operation.isEmpty()) {
				return;
			}

			List<ModelDataDefinition*>* definitions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioParameter>());

			if (operation == QObject::tr("List")) {
				QStringList lines;
				for (ModelDataDefinition* definition : *definitions->list()) {
					auto* parameter = dynamic_cast<BioParameter*>(definition);
					if (parameter != nullptr) {
						lines << formatParameterLine(parameter);
					}
				}
				lines.sort(Qt::CaseInsensitive);
				const QString body = lines.isEmpty()
				                     ? QObject::tr("No BioParameter definitions in current model.")
				                     : lines.join("\n");
				showTextDialog(parentWidget, QObject::tr("BioParameter List"), body);
				return;
			}

			if (operation == QObject::tr("Create")) {
				const QString name = QInputDialog::getText(
					parentWidget,
					QObject::tr("BioParameter Manager"),
					QObject::tr("BioParameter name:"),
					QLineEdit::Normal,
					QString(),
					&ok).trimmed();
				if (!ok) {
					return;
				}
				if (name.isEmpty()) {
					QMessageBox::warning(parentWidget, QObject::tr("BioParameter Manager"), QObject::tr("BioParameter name cannot be empty."));
					return;
				}
				if (model->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), name.toStdString()) != nullptr) {
					QMessageBox::warning(parentWidget, QObject::tr("BioParameter Manager"),
					                     QObject::tr("A BioParameter with this name already exists."));
					return;
				}

				const double value = QInputDialog::getDouble(
					parentWidget, QObject::tr("BioParameter Manager"), QObject::tr("Value:"),
					0.0, -1e12, 1e12, 9, &ok);
				if (!ok) {
					return;
				}
				const QString unit = QInputDialog::getText(
					parentWidget,
					QObject::tr("BioParameter Manager"),
					QObject::tr("Unit (optional):"),
					QLineEdit::Normal,
					QString(),
					&ok).trimmed();
				if (!ok) {
					return;
				}

				auto* parameter = new BioParameter(model, name.toStdString());
				parameter->setValue(value);
				parameter->setUnit(unit.toStdString());
				parameter->setHasChanged(true);
				model->setHasChanged(true);
				QMessageBox::information(parentWidget, QObject::tr("BioParameter Manager"),
				                         QObject::tr("BioParameter created successfully."));
				return;
			}

			if (definitions == nullptr || definitions->size() == 0) {
				QMessageBox::information(parentWidget, QObject::tr("BioParameter Manager"),
				                         QObject::tr("No BioParameter definitions available in current model."));
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
				QObject::tr("BioParameter Manager"),
				QObject::tr("Select BioParameter:"),
				names,
				0,
				false,
				&ok);
			if (!ok || selectedName.isEmpty()) {
				return;
			}

			auto* parameter = dynamic_cast<BioParameter*>(model->getDataManager()->getDataDefinition(
				Util::TypeOf<BioParameter>(), selectedName.toStdString()));
			if (parameter == nullptr) {
				QMessageBox::warning(parentWidget, QObject::tr("BioParameter Manager"),
				                     QObject::tr("Could not resolve selected BioParameter."));
				return;
			}

			const double value = QInputDialog::getDouble(
				parentWidget, QObject::tr("BioParameter Manager"), QObject::tr("Value:"),
				parameter->getValue(), -1e12, 1e12, 9, &ok);
			if (!ok) {
				return;
			}
			const QString unit = QInputDialog::getText(
				parentWidget,
				QObject::tr("BioParameter Manager"),
				QObject::tr("Unit (optional):"),
				QLineEdit::Normal,
				QString::fromStdString(parameter->getUnit()),
				&ok).trimmed();
			if (!ok) {
				return;
			}

			parameter->setValue(value);
			parameter->setUnit(unit.toStdString());
			parameter->setHasChanged(true);
			model->setHasChanged(true);
			QMessageBox::information(parentWidget, QObject::tr("BioParameter Manager"),
			                         QObject::tr("BioParameter updated successfully."));
		};
		registry->addWindow(std::move(managerWindow));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(BioParameterManagerGuiExtensionPlugin);
