#ifndef DIALOGPLUGINMANAGER_H
#define DIALOGPLUGINMANAGER_H

#include <QDialog>

#include <functional>
#include <string>

class Plugin;
class PluginInformation;
class SystemDependencyCheckResult;
class Simulator;
class QTableWidgetItem;

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

private slots:
	void on_pushButtonBrowseAutoload_clicked();
	void on_pushButtonAutoLoadNow_clicked();
	void on_pushButtonCheck_clicked();
	void on_pushButtonInsert_clicked();
	void on_pushButtonRemove_clicked();
	void on_pushButtonRefresh_clicked();
	void on_tableWidgetPlugins_itemSelectionChanged();

private:
	void _refreshPluginTable();
	void _refreshPluginCatalog();
	void _showPluginDetails(Plugin* plugin);
	void _showNoPluginDetails();
	Plugin* _selectedPlugin() const;
	bool _isKernelPlugin(const Plugin* plugin) const;
	bool _isPluginUsedByCurrentModel(const Plugin* plugin) const;
	bool _choosePluginFilename(std::string& filename) const;
	QString _formatPluginDetails(const Plugin* plugin) const;
	QString _formatDependencies(const PluginInformation* info) const;
	QString _formatSystemDependencies(const PluginInformation* info) const;
	/*! \brief Formats a system dependency preflight result for confirmation and diagnostic dialogs. */
	QString _formatSystemDependencyPreflight(const SystemDependencyCheckResult& result) const;
	QString _formatFields(const PluginInformation* info) const;
	/*! \brief Asks the user whether missing installable system dependencies may be installed. */
	bool _confirmSystemDependencyInstallation(const SystemDependencyCheckResult& result) const;
	void _showOperationResult(const QString& title, const QString& message) const;

	Ui::DialogPluginManager* ui;
	Simulator* _simulator = nullptr;
	std::function<void()> _refreshPluginCatalogCallback;
};

#endif // DIALOGPLUGINMANAGER_H
