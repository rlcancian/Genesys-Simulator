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
#include <QProcess>
#include <QBrush>
#include <QColor>
#include <QStandardPaths>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>

#include <map>
#include <memory>
#include <sstream>

namespace {

QString ShellSingleQuote(const QString& text)
{
	QString escaped = text;
	escaped.replace("'", "'\\''");
	return "'" + escaped + "'";
}

}

DialogPluginManager::DialogPluginManager(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogPluginManager)
{
	ui->setupUi(this);
	_configureLoadedPluginTable();
	_configurePluginIssuesTable();
	ui->textEditPluginDetails->setReadOnly(true);
	connect(ui->tableWidgetPluginIssues, &QTableWidget::itemSelectionChanged,
	        this, &DialogPluginManager::on_tableWidgetPlugins_itemSelectionChanged);
	connect(ui->tabWidgetPluginTables, &QTabWidget::currentChanged,
	        this, &DialogPluginManager::on_tableWidgetPlugins_itemSelectionChanged);
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

void DialogPluginManager::showProblemPluginsTab()
{
	ui->tabWidgetPluginTables->setCurrentWidget(ui->tabPluginIssues);
	if (ui->tableWidgetPluginIssues->rowCount() > 0) {
		ui->tableWidgetPluginIssues->selectRow(0);
	}
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
	PluginInsertionOptions options;
	_simulator->getPluginManager()->autoInsertPlugins(filename, ui->checkBoxFallbackDiscovery->isChecked(), options);
	_refreshPluginTable();
	if (_simulator->getPluginManager()->getPluginLoadIssues() != nullptr
	        && !_simulator->getPluginManager()->getPluginLoadIssues()->empty()) {
		showProblemPluginsTab();
	}
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
	Plugin* plugin = _simulator->getPluginManager()->insert(filename, options);
	_simulator->getPluginManager()->completePluginsFieldsAndTemplates();
	_refreshPluginTable();
	if (plugin == nullptr && _simulator->getPluginManager()->getPluginLoadIssues() != nullptr
	        && !_simulator->getPluginManager()->getPluginLoadIssues()->empty()) {
		showProblemPluginsTab();
	}
	_refreshPluginCatalog();
	_showOperationResult(
		tr("Insert plugin"),
		plugin != nullptr ? tr("Plugin inserted or already available.") : tr("Plugin could not be inserted.")
	);
}

void DialogPluginManager::on_pushButtonResolveSelected_clicked()
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	const PluginLoadIssue* selectedIssue = _selectedPluginLoadIssue();
	if (selectedIssue == nullptr) {
		_showOperationResult(
			tr("Resolve plugin dependencies"),
			tr("Select a blocked plugin row to resolve its system dependencies and load it.")
		);
		return;
	}
	const PluginLoadIssue issue = *selectedIssue;
	const std::string filename = issue.getFilename();

	if (!issue.hasSystemDependencyResult()) {
		_showOperationResult(
			tr("Resolve plugin dependencies"),
			tr("This plugin was not blocked by a system dependency that can be resolved automatically.\n\n%1")
				.arg(QString::fromStdString(issue.diagnosticText()))
		);
		return;
	}
	if (!issue.getSystemDependencyResult().canAttemptInstallForAllMissing()) {
		_showOperationResult(
			tr("Resolve plugin dependencies"),
			tr("The selected plugin has missing or unverifiable dependencies that cannot all be installed automatically.\n\n%1")
				.arg(QString::fromStdString(issue.diagnosticText()))
		);
		return;
	}

	if (!_runInstallCommandsForIssue(issue)) {
		_refreshPluginTable();
		_showOperationResult(
			tr("Resolve plugin dependencies"),
			tr("The install command did not complete successfully or the dependency is still unresolved. See the problem details for diagnostics.")
		);
		return;
	}

	PluginInsertionOptions options;
	Plugin* plugin = _simulator->getPluginManager()->insert(filename, options);
	_simulator->getPluginManager()->completePluginsFieldsAndTemplates();
	_refreshPluginTable();
	_refreshPluginCatalog();
	_showOperationResult(
		tr("Resolve plugin dependencies"),
		plugin != nullptr
			? tr("System dependencies were satisfied and the plugin is now loaded.")
			: tr("The plugin is still blocked. See the diagnostic row or trace output for the check and install command.")
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
	if (ui->tabWidgetPluginTables->currentWidget() == ui->tabPluginIssues) {
		const QList<QTableWidgetItem*> selectedItems = ui->tableWidgetPluginIssues->selectedItems();
		if (!selectedItems.isEmpty()) {
			QTableWidgetItem* fileItem = ui->tableWidgetPluginIssues->item(selectedItems.first()->row(), 0);
			if (fileItem != nullptr) {
				const QString details = fileItem->data(Qt::UserRole + 1).toString();
				if (!details.isEmpty()) {
					ui->textEditPluginDetails->setPlainText(details);
					return;
				}
			}
		}
		_showNoPluginDetails();
		return;
	}

	const QList<QTableWidgetItem*> selectedItems = ui->tableWidgetPlugins->selectedItems();
	if (!selectedItems.isEmpty()) {
		QTableWidgetItem* typeItem = ui->tableWidgetPlugins->item(selectedItems.first()->row(), 0);
		if (typeItem != nullptr) {
			const QString dependencyIssueDetails = typeItem->data(Qt::UserRole + 1).toString();
			if (!dependencyIssueDetails.isEmpty()) {
				ui->textEditPluginDetails->setPlainText(dependencyIssueDetails);
				return;
			}
		}
	}
	_showPluginDetails(_selectedPlugin());
}

void DialogPluginManager::_configureLoadedPluginTable()
{
	ui->tableWidgetPlugins->setColumnCount(11);
	ui->tableWidgetPlugins->setHorizontalHeaderLabels({
		tr("Plugin/File"),
		tr("State"),
		tr("Kind"),
		tr("Category"),
		tr("Version"),
		tr("Author"),
		tr("Inputs"),
		tr("Outputs"),
		tr("Flags"),
		tr("Dynamic deps"),
		tr("System deps")
	});
	_configureCommonTable(ui->tableWidgetPlugins);
}

void DialogPluginManager::_configurePluginIssuesTable()
{
	ui->tableWidgetPluginIssues->setColumnCount(8);
	ui->tableWidgetPluginIssues->setHorizontalHeaderLabels({
		tr("Plugin/File"),
		tr("Plugin type"),
		tr("Problem"),
		tr("Dependency"),
		tr("Status"),
		tr("Check command"),
		tr("Install command"),
		tr("Diagnostic")
	});
	_configureCommonTable(ui->tableWidgetPluginIssues);
}

void DialogPluginManager::_refreshPluginTable()
{
	ui->tableWidgetPlugins->setRowCount(0);
	ui->tableWidgetPluginIssues->setRowCount(0);
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
		ui->tableWidgetPlugins->setItem(row, 1, new QTableWidgetItem(tr("Loaded")));
		ui->tableWidgetPlugins->setItem(row, 2, new QTableWidgetItem(info->isComponent() ? tr("Component") : tr("DataDefinition")));
		ui->tableWidgetPlugins->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(info->getCategory())));
		ui->tableWidgetPlugins->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(info->getVersion())));
		ui->tableWidgetPlugins->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(info->getAuthor())));
		ui->tableWidgetPlugins->setItem(row, 6, new QTableWidgetItem(QString("%1..%2").arg(info->getMinimumInputs()).arg(info->getMaximumInputs())));
		ui->tableWidgetPlugins->setItem(row, 7, new QTableWidgetItem(QString("%1..%2").arg(info->getMinimumOutputs()).arg(info->getMaximumOutputs())));

		QStringList flags;
		if (info->isSource()) flags << tr("Source");
		if (info->isSink()) flags << tr("Sink");
		if (info->isSendTransfer()) flags << tr("SendTransfer");
		if (info->isReceiveTransfer()) flags << tr("ReceiveTransfer");
		if (info->isGenerateReport()) flags << tr("Report");
		ui->tableWidgetPlugins->setItem(row, 8, new QTableWidgetItem(flags.join(", ")));
		ui->tableWidgetPlugins->setItem(row, 9, new QTableWidgetItem(_formatDynamicDependencyTableText(info)));
		ui->tableWidgetPlugins->setItem(row, 10, new QTableWidgetItem(_formatSystemDependencyTableText(info)));
	}

	List<PluginLoadIssue>* issues = pluginManager->getPluginLoadIssues();
	if (issues != nullptr) {
		for (const PluginLoadIssue& issue : *issues->list()) {
			_appendPluginIssueRow(issue);
		}
	}

	ui->tableWidgetPlugins->sortItems(0);
	ui->tableWidgetPluginIssues->sortItems(0);
	if (ui->tableWidgetPluginIssues->rowCount() > 0 && ui->tabWidgetPluginTables->currentWidget() == ui->tabPluginIssues) {
		ui->tableWidgetPluginIssues->selectRow(0);
		return;
	}
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

void DialogPluginManager::_appendPluginIssueRow(const PluginLoadIssue& issue)
{
	const int row = ui->tableWidgetPluginIssues->rowCount();
	ui->tableWidgetPluginIssues->insertRow(row);
	const QColor dependencyProblemColor(255, 210, 210);

	auto makeItem = [dependencyProblemColor](const QString& text) {
		auto* item = new QTableWidgetItem(text);
		item->setBackground(QBrush(dependencyProblemColor));
		return item;
	};

	const QString detailedDiagnostic = QString::fromStdString(issue.diagnosticText()).trimmed();
	auto* fileItem = makeItem(QString::fromStdString(issue.getFilename()));
	fileItem->setData(Qt::UserRole + 1, detailedDiagnostic);
	fileItem->setData(Qt::UserRole + 2, QString::fromStdString(issue.getFilename()));
	ui->tableWidgetPluginIssues->setItem(row, 0, fileItem);
	ui->tableWidgetPluginIssues->setItem(row, 1, makeItem(QString::fromStdString(issue.getPluginTypename())));
	ui->tableWidgetPluginIssues->setItem(row, 2, makeItem(QString::fromStdString(PluginLoadIssue::reasonToString(issue.getReason()))));

	QStringList dependencies;
	QStringList statuses;
	QStringList checkCommands;
	QStringList installCommands;
	for (const SystemDependencyCheckEntry& entry : issue.getSystemDependencyResult().entries()) {
		if (!entry.blocksInsertion()) {
			continue;
		}
		dependencies << QString::fromStdString(entry.dependency().getName());
		statuses << QString::fromStdString(SystemDependencyCheckEntry::statusToString(entry.status()));
		checkCommands << QString::fromStdString(entry.dependency().getCheckCommand());
		installCommands << QString::fromStdString(entry.dependency().getInstallCommand());
	}
	ui->tableWidgetPluginIssues->setItem(row, 3, makeItem(dependencies.join("; ")));
	ui->tableWidgetPluginIssues->setItem(row, 4, makeItem(statuses.join("; ")));
	ui->tableWidgetPluginIssues->setItem(row, 5, makeItem(checkCommands.join("; ")));
	ui->tableWidgetPluginIssues->setItem(row, 6, makeItem(installCommands.join("; ")));
	QTableWidgetItem* diagnosticItem = makeItem(QString::fromStdString(issue.getMessage()));
	diagnosticItem->setToolTip(detailedDiagnostic);
	ui->tableWidgetPluginIssues->setItem(row, 7, diagnosticItem);
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

std::string DialogPluginManager::_selectedDependencyIssueFilename() const
{
	const QList<QTableWidgetItem*> selectedItems = ui->tableWidgetPluginIssues->selectedItems();
	if (selectedItems.isEmpty()) {
		return "";
	}
	QTableWidgetItem* typeItem = ui->tableWidgetPluginIssues->item(selectedItems.first()->row(), 0);
	if (typeItem == nullptr) {
		return "";
	}
	return typeItem->data(Qt::UserRole + 2).toString().toStdString();
}

const PluginLoadIssue* DialogPluginManager::_selectedPluginLoadIssue() const
{
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return nullptr;
	}
	const std::string filename = _selectedDependencyIssueFilename();
	if (filename.empty()) {
		return nullptr;
	}
	List<PluginLoadIssue>* issues = _simulator->getPluginManager()->getPluginLoadIssues();
	if (issues == nullptr) {
		return nullptr;
	}
	for (const PluginLoadIssue& issue : *issues->list()) {
		if (issue.getFilename() == filename) {
			return &issue;
		}
	}
	return nullptr;
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

QString DialogPluginManager::_formatDynamicDependencyTableText(const PluginInformation* info) const
{
	if (info == nullptr || info->getDynamicLibFilenameDependencies()->empty()) {
		return tr("none");
	}
	QStringList dependencies;
	for (const std::string& dependency : *info->getDynamicLibFilenameDependencies()) {
		dependencies << QString::fromStdString(dependency);
	}
	return dependencies.join(", ");
}

QString DialogPluginManager::_formatSystemDependencyTableText(const PluginInformation* info) const
{
	if (info == nullptr || info->getSystemDependencies()->empty()) {
		return tr("none");
	}
	QStringList dependencies;
	for (const SystemDependency& dependency : *info->getSystemDependencies()) {
		QString item = QString::fromStdString(dependency.getName());
		if (!dependency.getInstallCommand().empty()) {
			item += tr(" (install: %1)").arg(QString::fromStdString(dependency.getInstallCommand()));
		}
		dependencies << item;
	}
	return dependencies.join("; ");
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
	return QString::fromStdString(result.diagnosticText(false)).trimmed();
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

bool DialogPluginManager::_runInstallCommandsForIssue(const PluginLoadIssue& issue)
{
	QStringList installCommands;
	for (const SystemDependencyCheckEntry& entry : issue.getSystemDependencyResult().entries()) {
		if (entry.canAttemptInstall()) {
			installCommands << QString::fromStdString(entry.dependency().getInstallCommand());
		}
	}
	if (installCommands.isEmpty()) {
		return false;
	}

	const QMessageBox::StandardButton answer = QMessageBox::question(
		this,
		tr("Install plugin system dependencies"),
		tr("The selected plugin needs system dependencies before it can be loaded.\n\n%1\n\nCommand(s) to run:\n%2\n\nRun these command(s) now?")
			.arg(QString::fromStdString(issue.diagnosticText()), installCommands.join("\n")),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);
	if (answer != QMessageBox::Yes) {
		return false;
	}

	QString feedback;
	bool allSucceeded = true;
	for (const QString& command : installCommands) {
		QString commandFeedback;
		const bool succeeded = _runInstallCommandInteractive(command, &commandFeedback);
		allSucceeded = allSucceeded && succeeded;
		feedback += commandFeedback + "\n";
	}
	ui->textEditPluginDetails->setPlainText(feedback.trimmed());
	return allSucceeded;
}

bool DialogPluginManager::_runInstallCommandInteractive(const QString& command, QString* feedback)
{
	if (feedback != nullptr) {
		*feedback += tr("Running install command:\n%1\n\n").arg(command);
	}

	const QString script =
		command + "\n"
		"status=$?\n"
		"echo\n"
		"echo \"Command finished with exit code $status.\"\n"
		"echo \"Press Enter to close this terminal.\"\n"
		"read _\n"
		"exit $status\n";
	QString terminalProgram;
	QStringList terminalArguments;
	if (_terminalCommandForScript(script, &terminalProgram, &terminalArguments)) {
		const int exitCode = QProcess::execute(terminalProgram, terminalArguments);
		if (feedback != nullptr) {
			*feedback += tr("Terminal: %1\nExit code: %2\n").arg(terminalProgram).arg(exitCode);
		}
		return exitCode == 0;
	}

	QProcess process;
	process.setProgram("/bin/bash");
	process.setArguments({"-lc", command});
	process.start();
	const bool finished = process.waitForFinished(-1);
	const QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
	const QString error = QString::fromLocal8Bit(process.readAllStandardError());
	const int exitCode = finished ? process.exitCode() : -1;
	if (feedback != nullptr) {
		*feedback += tr("No supported terminal emulator was found. The command was executed without an interactive terminal.\n");
		*feedback += tr("Exit code: %1\n\nOutput:\n%2\nErrors:\n%3\n").arg(exitCode).arg(output, error);
	}
	return finished && exitCode == 0;
}

bool DialogPluginManager::_terminalCommandForScript(const QString& script, QString* program, QStringList* arguments) const
{
	if (program == nullptr || arguments == nullptr) {
		return false;
	}
	const QString bash = QStandardPaths::findExecutable("bash");
	if (bash.isEmpty()) {
		return false;
	}
	const QString quotedScript = ShellSingleQuote(script);

	const QString xTerminal = QStandardPaths::findExecutable("x-terminal-emulator");
	if (!xTerminal.isEmpty()) {
		*program = xTerminal;
		*arguments = {"-e", bash, "-lc", script};
		return true;
	}
	const QString gnomeTerminal = QStandardPaths::findExecutable("gnome-terminal");
	if (!gnomeTerminal.isEmpty()) {
		*program = gnomeTerminal;
		*arguments = {"--wait", "--", bash, "-lc", script};
		return true;
	}
	const QString konsole = QStandardPaths::findExecutable("konsole");
	if (!konsole.isEmpty()) {
		*program = konsole;
		*arguments = {"--nofork", "-e", bash, "-lc", script};
		return true;
	}
	const QString xterm = QStandardPaths::findExecutable("xterm");
	if (!xterm.isEmpty()) {
		*program = xterm;
		*arguments = {"-e", bash, "-lc", script};
		return true;
	}
	const QString xfceTerminal = QStandardPaths::findExecutable("xfce4-terminal");
	if (!xfceTerminal.isEmpty()) {
		*program = xfceTerminal;
		*arguments = {"--disable-server", "--command", bash + " -lc " + quotedScript};
		return true;
	}
	return false;
}

void DialogPluginManager::_configureCommonTable(QTableWidget* table) const
{
	if (table == nullptr) {
		return;
	}
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	table->horizontalHeader()->setStretchLastSection(true);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
}

void DialogPluginManager::_showOperationResult(const QString& title, const QString& message) const
{
	QMessageBox::information(const_cast<DialogPluginManager*>(this), title, message);
}
