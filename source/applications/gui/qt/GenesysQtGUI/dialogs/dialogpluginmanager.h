#ifndef DIALOGPLUGINMANAGER_H
#define DIALOGPLUGINMANAGER_H

#include <QDialog>

#include <functional>
#include <string>

class Plugin;
class PluginInformation;
class PluginLoadIssue;
class SystemDependencyCheckResult;
class Simulator;
class QTableWidgetItem;
class QTableWidget;

namespace Ui {
	class DialogPluginManager;
}

class DialogPluginManager : public QDialog {
	Q_OBJECT

public:
	explicit DialogPluginManager(QWidget* parent = nullptr);
	~DialogPluginManager();
	void setSimulator(Simulator* simulator);
	void setPluginCatalogRefreshCallback(std::function<void()> refreshCallback);
	/*! \brief Selects the problems tab when the dialog is opened for startup diagnostics. */
	void showProblemPluginsTab();

private slots:
	void on_pushButtonBrowseAutoload_clicked();
	void on_pushButtonAutoLoadNow_clicked();
	void on_pushButtonCheck_clicked();
	void on_pushButtonInsert_clicked();
	void on_pushButtonResolveSelected_clicked();
	void on_pushButtonRemove_clicked();
	void on_pushButtonRefresh_clicked();
	void on_tableWidgetPlugins_itemSelectionChanged();

private:
	/*! \brief Rebuilds the loaded/problem plugin tables. */
	void _refreshPluginTable();
	/*! \brief Configures the columns and selection behavior of the loaded plugin table. */
	void _configureLoadedPluginTable();
	/*! \brief Configures the columns and selection behavior of the failed plugin table. */
	void _configurePluginIssuesTable();
	/*! \brief Refreshes the main plugin catalog/tree after plugin insertion/removal. */
	void _refreshPluginCatalog();
	/*! \brief Appends a red table row for a plugin candidate that could not be loaded. */
	void _appendPluginIssueRow(const PluginLoadIssue& issue);
	/*! \brief Shows details for a loaded plugin in the read-only side panel. */
	void _showPluginDetails(Plugin* plugin);
	/*! \brief Shows the empty-selection message in the side panel. */
	void _showNoPluginDetails();
	/*! \brief Returns the currently selected loaded plugin, or nullptr for issue rows. */
	Plugin* _selectedPlugin() const;
	/*! \brief Returns the selected blocked plugin filename, or an empty string when none is selected. */
	std::string _selectedDependencyIssueFilename() const;
	/*! \brief Returns the selected stored plugin load issue, or nullptr when none is selected. */
	const PluginLoadIssue* _selectedPluginLoadIssue() const;
	/*! \brief Returns true when the selected plugin is one of the built-in kernel plugins. */
	bool _isKernelPlugin(const Plugin* plugin) const;
	/*! \brief Returns true when the current model contains instances from this plugin. */
	bool _isPluginUsedByCurrentModel(const Plugin* plugin) const;
	/*! \brief Opens a file chooser and returns the selected plugin filename. */
	bool _choosePluginFilename(std::string& filename) const;
	/*! \brief Formats complete plugin metadata for the read-only details panel. */
	QString _formatPluginDetails(const Plugin* plugin) const;
	/*! \brief Formats dynamic library dependencies for compact table display. */
	QString _formatDynamicDependencyTableText(const PluginInformation* info) const;
	/*! \brief Formats declared system dependencies and install commands for compact table display. */
	QString _formatSystemDependencyTableText(const PluginInformation* info) const;
	/*! \brief Formats dynamic library dependencies for the details panel. */
	QString _formatDependencies(const PluginInformation* info) const;
	/*! \brief Formats declared system dependencies for the details panel. */
	QString _formatSystemDependencies(const PluginInformation* info) const;
	/*! \brief Formats a system dependency preflight result for confirmation and diagnostic dialogs. */
	QString _formatSystemDependencyPreflight(const SystemDependencyCheckResult& result) const;
	/*! \brief Formats plugin field declarations for the details panel. */
	QString _formatFields(const PluginInformation* info) const;
	/*! \brief Asks the user whether missing installable system dependencies may be installed. */
	bool _confirmSystemDependencyInstallation(const SystemDependencyCheckResult& result) const;
	/*! \brief Executes all install commands from one issue and returns true if every command started and exited with zero. */
	bool _runInstallCommandsForIssue(const PluginLoadIssue& issue);
	/*! \brief Runs one install command in an interactive terminal when possible, with a captured fallback. */
	bool _runInstallCommandInteractive(const QString& command, QString* feedback);
	/*! \brief Finds a usable terminal emulator command for interactive sudo/package commands. */
	bool _terminalCommandForScript(const QString& script, QString* program, QStringList* arguments) const;
	/*! \brief Configures a common table widget style used by both plugin tables. */
	void _configureCommonTable(QTableWidget* table) const;
	void _showOperationResult(const QString& title, const QString& message) const;

	Ui::DialogPluginManager* ui;
	Simulator* _simulator = nullptr;
	std::function<void()> _refreshPluginCatalogCallback;
};

#endif // DIALOGPLUGINMANAGER_H
