#include "dialogpluginmanager.h"
#include "ui_dialogpluginmanager.h"

#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"

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

#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

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

	const std::vector<PluginLoadIssue> selectedIssues = _selectedPluginLoadIssues();
	if (selectedIssues.empty()) {
		_showOperationResult(
			tr("Resolve / load plugins"),
			tr("Select one or more blocked plugin rows to resolve their system dependencies and retry loading.")
		);
		return;
	}

	PluginManager* pluginManager = _simulator->getPluginManager();
	QStringList loadedPlugins;
	QStringList stillBlockedPlugins;
	QStringList installFailures;
	QStringList feedback;

	for (const PluginLoadIssue& selectedIssue : selectedIssues) {
		const std::string filename = selectedIssue.getFilename();
		PluginLoadIssue issueForInstall = selectedIssue;

		if (selectedIssue.hasSystemDependencyResult()) {
			const SystemDependencyCheckResult currentSystemDependencies =
				pluginManager->checkSystemDependencies(filename);
			if (!currentSystemDependencies.entries().empty() && !currentSystemDependencies.canInsertPlugin()) {
				issueForInstall = PluginLoadIssue(
					selectedIssue.getFilename(),
					selectedIssue.getPluginTypename(),
					selectedIssue.getReason(),
					selectedIssue.getMessage(),
					currentSystemDependencies);
				if (currentSystemDependencies.canAttemptInstallForAllMissing()) {
					QString issueFeedback;
					if (!_runInstallCommandsForIssue(issueForInstall, &issueFeedback)) {
						installFailures << QString::fromStdString(filename);
					}
					if (!issueFeedback.trimmed().isEmpty()) {
						feedback << issueFeedback.trimmed();
					}
				}
			}
		}

		PluginInsertionOptions options;
		Plugin* plugin = pluginManager->insert(filename, options);
		if (plugin != nullptr && plugin->getPluginInfo() != nullptr) {
			loadedPlugins << QString::fromStdString(plugin->getPluginInfo()->getPluginTypename());
		} else {
			stillBlockedPlugins << QString::fromStdString(filename);
		}
	}

	pluginManager->completePluginsFieldsAndTemplates();
	_refreshPluginTable();
	_refreshPluginCatalog();

	QString message = tr("Resolve/load operation finished.");
	if (!loadedPlugins.isEmpty()) {
		message += tr("\n\nLoaded:\n%1").arg(loadedPlugins.join("\n"));
	}
	if (!stillBlockedPlugins.isEmpty()) {
		message += tr("\n\nStill blocked:\n%1").arg(stillBlockedPlugins.join("\n"));
	}
	if (!installFailures.isEmpty()) {
		message += tr("\n\nInstall command failed or was cancelled for:\n%1").arg(installFailures.join("\n"));
	}
	if (!feedback.isEmpty()) {
		message += tr("\n\nInstall command feedback:\n%1").arg(feedback.join("\n\n"));
	}
	_showOperationResult(tr("Resolve / load plugins"), message);
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

void DialogPluginManager::_configureLoadedPluginTable()
{
	ui->tableWidgetPlugins->setColumnCount(16);
	ui->tableWidgetPlugins->setHorizontalHeaderLabels({
		tr("Plugin"),
		tr("State"),
		tr("Kind"),
		tr("Category"),
		tr("Version"),
		tr("Date"),
		tr("Author"),
		tr("Inputs"),
		tr("Outputs"),
		tr("Flags"),
		tr("Dynamic deps"),
		tr("System deps"),
		tr("Fields"),
		tr("Observation"),
		tr("Description"),
		tr("Template")
	});
	_configureCommonTable(ui->tableWidgetPlugins);
}

void DialogPluginManager::_configurePluginIssuesTable()
{
	ui->tableWidgetPluginIssues->setColumnCount(9);
	ui->tableWidgetPluginIssues->setHorizontalHeaderLabels({
		tr("Plugin/File"),
		tr("Plugin type"),
		tr("Problem"),
		tr("System dependency"),
		tr("Status"),
		tr("Check command"),
		tr("Install command"),
		tr("Diagnostic"),
		tr("Suggested action")
	});
	_configureCommonTable(ui->tableWidgetPluginIssues);
	ui->tableWidgetPluginIssues->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void DialogPluginManager::_refreshPluginTable()
{
	ui->tableWidgetPlugins->setRowCount(0);
	ui->tableWidgetPluginIssues->setRowCount(0);
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return;
	}

	PluginManager* pluginManager = _simulator->getPluginManager();
	auto makeItem = [](const QString& text, const QString& tooltip = QString()) {
		QString displayText = text.simplified();
		if (displayText.size() > 120) {
			displayText = displayText.left(117) + "...";
		}
		auto* item = new QTableWidgetItem(displayText);
		const QString fullTooltip = tooltip.isEmpty() ? text : tooltip;
		if (!fullTooltip.trimmed().isEmpty()) {
			item->setToolTip(fullTooltip.trimmed());
		}
		return item;
	};

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
		typeItem->setToolTip(QString::fromStdString(info->getPluginTypename()));
		ui->tableWidgetPlugins->setItem(row, 0, typeItem);
		ui->tableWidgetPlugins->setItem(row, 1, makeItem(tr("Loaded")));
		ui->tableWidgetPlugins->setItem(row, 2, makeItem(info->isComponent() ? tr("Component") : tr("DataDefinition")));
		ui->tableWidgetPlugins->setItem(row, 3, makeItem(QString::fromStdString(info->getCategory())));
		ui->tableWidgetPlugins->setItem(row, 4, makeItem(QString::fromStdString(info->getVersion())));
		ui->tableWidgetPlugins->setItem(row, 5, makeItem(QString::fromStdString(info->getDate())));
		ui->tableWidgetPlugins->setItem(row, 6, makeItem(QString::fromStdString(info->getAuthor())));
		ui->tableWidgetPlugins->setItem(row, 7, makeItem(QString("%1..%2").arg(info->getMinimumInputs()).arg(info->getMaximumInputs())));
		ui->tableWidgetPlugins->setItem(row, 8, makeItem(QString("%1..%2").arg(info->getMinimumOutputs()).arg(info->getMaximumOutputs())));

		QStringList flags;
		if (info->isSource()) flags << tr("Source");
		if (info->isSink()) flags << tr("Sink");
		if (info->isSendTransfer()) flags << tr("SendTransfer");
		if (info->isReceiveTransfer()) flags << tr("ReceiveTransfer");
		if (info->isGenerateReport()) flags << tr("Report");
		ui->tableWidgetPlugins->setItem(row, 9, makeItem(flags.join(", ")));
		ui->tableWidgetPlugins->setItem(row, 10, makeItem(_formatDynamicDependencyTableText(info)));
		ui->tableWidgetPlugins->setItem(row, 11, makeItem(_formatSystemDependencyTableText(info)));
		ui->tableWidgetPlugins->setItem(row, 12, makeItem(_formatFields(info)));
		ui->tableWidgetPlugins->setItem(row, 13, makeItem(QString::fromStdString(info->getObservation())));
		ui->tableWidgetPlugins->setItem(row, 14, makeItem(QString::fromStdString(info->getDescriptionHelp())));
		ui->tableWidgetPlugins->setItem(row, 15, makeItem(QString::fromStdString(info->getLanguageTemplate())));
	}

	List<PluginLoadIssue>* issues = pluginManager->getPluginLoadIssues();
	if (issues != nullptr) {
		for (const PluginLoadIssue& issue : *issues->list()) {
			_appendPluginIssueRow(issue);
		}
	}

	ui->tableWidgetPlugins->sortItems(0);
	ui->tableWidgetPluginIssues->sortItems(0);
	ui->tableWidgetPlugins->resizeColumnsToContents();
	ui->tableWidgetPluginIssues->resizeColumnsToContents();
	if (ui->tableWidgetPluginIssues->rowCount() > 0 && ui->tabWidgetPluginTables->currentWidget() == ui->tabPluginIssues) {
		ui->tableWidgetPluginIssues->selectRow(0);
		return;
	}
	if (ui->tableWidgetPluginIssues->rowCount() == 0 && ui->tabWidgetPluginTables->currentWidget() == ui->tabPluginIssues) {
		ui->tabWidgetPluginTables->setCurrentWidget(ui->tabLoadedPlugins);
	}
	if (ui->tableWidgetPlugins->rowCount() > 0) {
		ui->tableWidgetPlugins->selectRow(0);
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
		QString displayText = text.simplified();
		if (displayText.size() > 120) {
			displayText = displayText.left(117) + "...";
		}
		auto* item = new QTableWidgetItem(displayText);
		item->setBackground(QBrush(dependencyProblemColor));
		if (!text.trimmed().isEmpty()) {
			item->setToolTip(text.trimmed());
		}
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
	QString suggestedAction;
	if (issue.hasSystemDependencyResult()) {
		if (issue.getSystemDependencyResult().canAttemptInstallForAllMissing()) {
			suggestedAction = tr("Install missing system dependencies, then retry load.");
		} else {
			suggestedAction = tr("Resolve system dependencies manually, then retry load.");
		}
	} else if (issue.getReason() == PluginLoadIssue::Reason::DynamicDependencyFailure) {
		suggestedAction = tr("Load or repair the dependent plugin/library, then retry load.");
	} else {
		suggestedAction = tr("Retry load after correcting the diagnostic condition.");
	}
	ui->tableWidgetPluginIssues->setItem(row, 8, makeItem(suggestedAction));
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

std::vector<PluginLoadIssue> DialogPluginManager::_selectedPluginLoadIssues() const
{
	std::vector<PluginLoadIssue> selectedIssues;
	if (ui->tabWidgetPluginTables->currentWidget() != ui->tabPluginIssues) {
		return selectedIssues;
	}
	if (_simulator == nullptr || _simulator->getPluginManager() == nullptr) {
		return selectedIssues;
	}
	List<PluginLoadIssue>* issues = _simulator->getPluginManager()->getPluginLoadIssues();
	if (issues == nullptr) {
		return selectedIssues;
	}

	std::vector<int> selectedRows;
	for (QTableWidgetItem* item : ui->tableWidgetPluginIssues->selectedItems()) {
		if (item != nullptr) {
			selectedRows.push_back(item->row());
		}
	}
	std::sort(selectedRows.begin(), selectedRows.end());
	selectedRows.erase(std::unique(selectedRows.begin(), selectedRows.end()), selectedRows.end());

	for (int row : selectedRows) {
		QTableWidgetItem* fileItem = ui->tableWidgetPluginIssues->item(row, 0);
		if (fileItem == nullptr) {
			continue;
		}
		const std::string filename = fileItem->data(Qt::UserRole + 2).toString().toStdString();
		if (filename.empty()) {
			continue;
		}
		for (const PluginLoadIssue& issue : *issues->list()) {
			if (issue.getFilename() == filename) {
				selectedIssues.push_back(issue);
				break;
			}
		}
	}
	return selectedIssues;
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

bool DialogPluginManager::_runInstallCommandsForIssue(const PluginLoadIssue& issue, QString* feedback)
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

	bool allSucceeded = true;
	for (const QString& command : installCommands) {
		QString commandFeedback;
		const bool succeeded = _runInstallCommandInteractive(command, &commandFeedback);
		allSucceeded = allSucceeded && succeeded;
		if (feedback != nullptr) {
			*feedback += commandFeedback + "\n";
		}
	}
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
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	table->horizontalHeader()->setStretchLastSection(true);
	table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setWordWrap(false);
}

void DialogPluginManager::_showOperationResult(const QString& title, const QString& message) const
{
	QMessageBox::information(const_cast<DialogPluginManager*>(this), title, message);
}
