#include "dialogpluginmanager.h"
#include "ui_dialogpluginmanager.h"

#include "../../../../../kernel/simulator/ComponentManager.h"
#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/simulator/Plugin.h"
#include "../../../../../kernel/simulator/PluginInformation.h"
#include "../../../../../kernel/simulator/PluginManager.h"
#include "../../../../../kernel/simulator/Simulator.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>

#include <map>
#include <sstream>

DialogPluginManager::DialogPluginManager(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogPluginManager)
{
	ui->setupUi(this);
	ui->tableWidgetPlugins->setColumnCount(8);
	ui->tableWidgetPlugins->setHorizontalHeaderLabels({
		tr("Type"),
		tr("Kind"),
		tr("Category"),
		tr("Version"),
		tr("Author"),
		tr("Inputs"),
		tr("Outputs"),
		tr("Flags")
	});
	ui->tableWidgetPlugins->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidgetPlugins->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidgetPlugins->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableWidgetPlugins->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->textEditPluginDetails->setReadOnly(true);
}

DialogPluginManager::~DialogPluginManager()
{
	delete ui;
}

void DialogPluginManager::setSimulator(Simulator* simulator)
{
	_simulator = simulator;
	_refreshPluginTable();
}

void DialogPluginManager::setPluginCatalogRefreshCallback(std::function<void()> refreshCallback)
{
	_refreshPluginCatalogCallback = std::move(refreshCallback);
}

void DialogPluginManager::on_pushButtonBrowseAutoload_clicked()
{
	const QString filename = QFileDialog::getOpenFileName(
		this,
		tr("Select plugin autoload list"),
		QString(),
		tr("Text files (*.txt);;All files (*)")
	);
	if (!filename.isEmpty()) {
		ui->lineEditAutoloadFilename->setText(filename);
	}
}

void DialogPluginManager::on_pushButtonAutoLoadNow_clicked()
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	const std::string filename = ui->lineEditAutoloadFilename->text().toStdString();
	_simulator->getPluginManager()->autoInsertPlugins(filename, ui->checkBoxFallbackDiscovery->isChecked());
	_refreshPluginTable();
	_refreshPluginCatalog();
	_showOperationResult(tr("Auto load plugins"), tr("Plugin auto-load operation finished."));
}

void DialogPluginManager::on_pushButtonCheck_clicked()
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	std::string filename;
	if (!_choosePluginFilename(filename)) {
		return;
	}

	const bool valid = _simulator->getPluginManager()->check(filename);
	if (valid) {
		const SystemDependencyCheckResult preflight = _simulator->getPluginManager()->checkSystemDependencies(filename);
		if (!preflight.entries().empty() && !preflight.canInsertPlugin()) {
			_showOperationResult(
				tr("Check plugin"),
				tr("The selected plugin is structurally valid, but its system dependencies are not satisfied:\n\n%1")
					.arg(_formatSystemDependencyPreflight(preflight))
			);
			return;
		}
	}
	_showOperationResult(
		tr("Check plugin"),
		valid ? tr("The selected plugin can be inserted.") : tr("The selected plugin could not be checked as a valid plugin.")
	);
}

void DialogPluginManager::on_pushButtonInsert_clicked()
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	std::string filename;
	if (!_choosePluginFilename(filename)) {
		return;
	}

	PluginInsertionOptions options;
	options.confirmSystemDependencyInstallation = [this](const SystemDependencyCheckResult& result) {
		return _confirmSystemDependencyInstallation(result);
	};
	Plugin* plugin = _simulator->getPluginManager()->insert(filename, options);
	_simulator->getPluginManager()->completePluginsFieldsAndTemplates();
	_refreshPluginTable();
	_refreshPluginCatalog();
	_showOperationResult(
		tr("Insert plugin"),
		plugin != nullptr ? tr("Plugin inserted or already available.") : tr("Plugin could not be inserted.")
	);
}

void DialogPluginManager::on_pushButtonRemove_clicked()
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	Plugin* plugin = _selectedPlugin();
	if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		_showOperationResult(tr("Remove plugin"), tr("Select a plugin to remove."));
		return;
	}
	if (_isKernelPlugin(plugin)) {
		_showOperationResult(tr("Remove plugin"), tr("Kernel plugins cannot be removed from the simulator."));
		return;
	}
	if (_isPluginUsedByCurrentModel(plugin)) {
		_showOperationResult(tr("Remove plugin"), tr("This plugin is used by the current model and cannot be removed."));
		return;
	}

	const QString pluginName = QString::fromStdString(plugin->getPluginInfo()->getPluginTypename());
	const QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		tr("Remove plugin"),
		tr("Remove plugin \"%1\" from the available plugin list?").arg(pluginName),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No
	);
	if (answer != QMessageBox::Yes) {
		return;
	}

	const bool removed = _simulator->getPluginManager()->remove(plugin);
	_refreshPluginTable();
	_refreshPluginCatalog();
	_showOperationResult(
		tr("Remove plugin"),
		removed ? tr("Plugin removed from the available plugin list.") : tr("Plugin could not be removed.")
	);
}

void DialogPluginManager::on_pushButtonRefresh_clicked()
{
	_refreshPluginTable();
}

void DialogPluginManager::on_tableWidgetPlugins_itemSelectionChanged()
{
	_showPluginDetails(_selectedPlugin());
}

void DialogPluginManager::_refreshPluginTable()
{
	ui->tableWidgetPlugins->setRowCount(0);
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		_showNoPluginDetails();
		return;
	}

	PluginManager* pluginManager = _simulator->getPluginManager();
	for (unsigned int i = 0; i < pluginManager->size(); i++) {
		Plugin* plugin = pluginManager->getAtRank(i);
		if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
			continue;
		}
		PluginInformation* info = plugin->getPluginInfo();
		const int row = ui->tableWidgetPlugins->rowCount();
		ui->tableWidgetPlugins->insertRow(row);

		auto* typeItem = new QTableWidgetItem(QString::fromStdString(info->getPluginTypename()));
		typeItem->setData(Qt::UserRole, QVariant::fromValue<quintptr>(reinterpret_cast<quintptr>(plugin)));
		ui->tableWidgetPlugins->setItem(row, 0, typeItem);
		ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(info->isComponent() ? tr("Component") : tr("DataDefinition")));
		ui->tableWidgetPlugins->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(info->getCategory())));
		ui->tableWidgetPlugins->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(info->getVersion())));
		ui->tableWidgetPlugins->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(info->getAuthor())));
		ui->tableWidgetPlugins->setItem(row, 5, new QTableWidgetItem(QString("%1..%2").arg(info->getMinimumInputs()).arg(info->getMaximumInputs())));
		ui->tableWidgetPlugins->setItem(row, 6, new QTableWidgetItem(QString("%1..%2").arg(info->getMinimumOutputs()).arg(info->getMaximumOutputs())));

		QStringList flags;
		if (info->isSource()) flags << tr("Source");
		if (info->isSink()) flags << tr("Sink");
		if (info->isSendTransfer()) flags << tr("SendTransfer");
		if (info->isReceiveTransfer()) flags << tr("ReceiveTransfer");
		if (info->isGenerateReport()) flags << tr("Report");
		ui->tableWidgetPlugins->setItem(row, 7, new QTableWidgetItem(flags.join(", ")));
	}

	ui->tableWidgetPlugins->sortItems(0);
	if (ui->tableWidgetPlugins->rowCount() > 0) {
		ui->tableWidgetPlugins->selectRow(0);
	} else {
		_showNoPluginDetails();
	}
}

void DialogPluginManager::_refreshPluginCatalog()
{
	if (_refreshPluginCatalogCallback) {
		_refreshPluginCatalogCallback();
	}
}

void DialogPluginManager::_showPluginDetails(Plugin* plugin)
{
	if (plugin == nullptr) {
		_showNoPluginDetails();
		return;
	}
	ui->textEditPluginDetails->setPlainText(_formatPluginDetails(plugin));
}

void DialogPluginManager::_showNoPluginDetails()
{
	ui->textEditPluginDetails->setPlainText(tr("Select a plugin to see its read-only PluginInformation."));
}

Plugin* DialogPluginManager::_selectedPlugin() const
{
	const QList<QTableWidgetItem*> selectedItems = ui->tableWidgetPlugins->selectedItems();
	if (selectedItems.isEmpty()) {
		return nullptr;
	}
	QTableWidgetItem* typeItem = ui->tableWidgetPlugins->item(selectedItems.first()->row(), 0);
	if (typeItem == nullptr) {
		return nullptr;
	}
	const quintptr pointer = typeItem->data(Qt::UserRole).value<quintptr>();
	return reinterpret_cast<Plugin*>(pointer);
}

bool DialogPluginManager::_isKernelPlugin(const Plugin* plugin) const
{
	if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		return false;
	}
	const std::string type = plugin->getPluginInfo()->getPluginTypename();
	return type == "EntityType" || type == "Attribute" || type == "Counter" || type == "StatisticsCollector";
}

bool DialogPluginManager::_isPluginUsedByCurrentModel(const Plugin* plugin) const
{
	if (_simulator == nullptr || plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		return false;
	}
	Model* model = _simulator->getModelManager()->current();
	if (model == nullptr) {
		return false;
	}

	const std::string type = plugin->getPluginInfo()->getPluginTypename();
	if (plugin->getPluginInfo()->isComponent()) {
		for (ModelComponent* component : *model->getComponentManager()->getAllComponents()) {
			if (component != nullptr && component->getClassname() == type) {
				return true;
			}
		}
		return false;
	}

	return model->getDataManager()->getNumberOfDataDefinitions(type) > 0;
}

bool DialogPluginManager::_choosePluginFilename(std::string& filename) const
{
	const QString selected = QFileDialog::getOpenFileName(
		const_cast<DialogPluginManager*>(this),
		tr("Select plugin library"),
		QString(),
		tr("Plugin libraries (*.so *.dll *.dylib);;All files (*)")
	);
	if (selected.isEmpty()) {
		return false;
	}
	filename = selected.toStdString();
	return true;
}

QString DialogPluginManager::_formatPluginDetails(const Plugin* plugin) const
{
	if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		return tr("No plugin selected.");
	}

	const PluginInformation* info = plugin->getPluginInfo();
	QString details;
	details += tr("Type: %1\n").arg(QString::fromStdString(info->getPluginTypename()));
	details += tr("Kind: %1\n").arg(info->isComponent() ? tr("Component") : tr("DataDefinition"));
	details += tr("Category: %1\n").arg(QString::fromStdString(info->getCategory()));
	details += tr("Version: %1\n").arg(QString::fromStdString(info->getVersion()));
	details += tr("Author: %1\n").arg(QString::fromStdString(info->getAuthor()));
	details += tr("Date: %1\n").arg(QString::fromStdString(info->getDate()));
	details += tr("Inputs: %1..%2\n").arg(info->getMinimumInputs()).arg(info->getMaximumInputs());
	details += tr("Outputs: %1..%2\n").arg(info->getMinimumOutputs()).arg(info->getMaximumOutputs());
	details += tr("Source: %1\n").arg(info->isSource() ? tr("yes") : tr("no"));
	details += tr("Sink: %1\n").arg(info->isSink() ? tr("yes") : tr("no"));
	details += tr("Send transfer: %1\n").arg(info->isSendTransfer() ? tr("yes") : tr("no"));
	details += tr("Receive transfer: %1\n").arg(info->isReceiveTransfer() ? tr("yes") : tr("no"));
	details += tr("Generate report: %1\n\n").arg(info->isGenerateReport() ? tr("yes") : tr("no"));
	details += tr("Observation:\n%1\n\n").arg(QString::fromStdString(info->getObservation()));
	details += tr("Description help:\n%1\n\n").arg(QString::fromStdString(info->getDescriptionHelp()));
	details += tr("Language template:\n%1\n\n").arg(QString::fromStdString(info->getLanguageTemplate()));
	details += _formatDependencies(info);
	details += _formatSystemDependencies(info);
	details += _formatFields(info);
	return details;
}

QString DialogPluginManager::_formatDependencies(const PluginInformation* info) const
{
	QString text = tr("Dynamic library dependencies:\n");
	if (info == nullptr || info->getDynamicLibFilenameDependencies()->empty()) {
		return text + tr("  none\n\n");
	}
	for (const std::string& dependency : *info->getDynamicLibFilenameDependencies()) {
		text += "  " + QString::fromStdString(dependency) + "\n";
	}
	return text + "\n";
}

QString DialogPluginManager::_formatSystemDependencies(const PluginInformation* info) const
{
	QString text = tr("System dependencies:\n");
	if (info == nullptr || info->getSystemDependencies()->empty()) {
		return text + tr("  none\n\n");
	}
	for (const SystemDependency& dependency : *info->getSystemDependencies()) {
		text += "  " + QString::fromStdString(dependency.show()) + "\n";
	}
	return text + "\n";
}

QString DialogPluginManager::_formatSystemDependencyPreflight(const SystemDependencyCheckResult& result) const
{
	QString text;
	for (const SystemDependencyCheckEntry& entry : result.entries()) {
		const SystemDependency& dependency = entry.dependency();
		text += tr("Dependency: %1\n").arg(QString::fromStdString(dependency.getName()));
		text += tr("  OS: %1\n").arg(QString::fromStdString(SystemDependency::osToString(dependency.getOS())));
		text += tr("  Status: %1\n").arg(QString::fromStdString(SystemDependencyCheckEntry::statusToString(entry.status())));
		text += tr("  Check command: %1\n").arg(dependency.getCheckCommand().empty()
			? tr("<not declared>")
			: QString::fromStdString(dependency.getCheckCommand()));
		text += tr("  Install command: %1\n").arg(dependency.getInstallCommand().empty()
			? tr("<not declared>")
			: QString::fromStdString(dependency.getInstallCommand()));
		if (entry.checkResult().started) {
			text += tr("  Check exit code: %1\n").arg(entry.checkResult().exitCode);
			if (!entry.checkResult().output.empty()) {
				text += tr("  Check output: %1\n").arg(QString::fromStdString(entry.checkResult().output).trimmed());
			}
		}
		if (!entry.message().empty()) {
			text += tr("  Diagnostic: %1\n").arg(QString::fromStdString(entry.message()));
		}
		text += "\n";
	}
	return text.trimmed();
}

QString DialogPluginManager::_formatFields(const PluginInformation* info) const
{
	QString text = tr("Fields:\n");
	if (info == nullptr || info->getFields()->empty()) {
		return text + tr("  none\n");
	}
	for (const auto& field : *info->getFields()) {
		text += "  " + QString::fromStdString(field.first) + ": " + QString::fromStdString(field.second) + "\n";
	}
	return text;
}

bool DialogPluginManager::_confirmSystemDependencyInstallation(const SystemDependencyCheckResult& result) const
{
	const QString diagnostics = _formatSystemDependencyPreflight(result);
	if (!result.canAttemptInstallForAllMissing()) {
		QMessageBox::warning(
			const_cast<DialogPluginManager*>(this),
			tr("System dependencies"),
			tr("The plugin declares missing or unverifiable system dependencies that cannot all be installed automatically.\n\n%1\n\nThe plugin will not be inserted.")
				.arg(diagnostics)
		);
		return false;
	}

	QStringList installCommands;
	for (const SystemDependencyCheckEntry& entry : result.entries()) {
		if (entry.canAttemptInstall()) {
			installCommands << QString::fromStdString(entry.dependency().getInstallCommand());
		}
	}

	// This is the GUI boundary for package installation: the kernel has only reported diagnostics.
	const QMessageBox::StandardButton answer = QMessageBox::question(
		const_cast<DialogPluginManager*>(this),
		tr("Install system dependencies"),
		tr("The plugin needs additional system dependencies before it can be inserted.\n\n%1\n\nInstall command(s):\n%2\n\nRun these command(s) now?")
			.arg(diagnostics, installCommands.join("\n")),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No
	);
	return answer == QMessageBox::Yes;
}

void DialogPluginManager::_showOperationResult(const QString& title, const QString& message) const
{
	QMessageBox::information(const_cast<DialogPluginManager*>(this), title, message);
}
