#ifndef GUIEXTENSIONCONFIGURATIONDIALOG_H
#define GUIEXTENSIONCONFIGURATIONDIALOG_H

#include "GuiExtensionContracts.h"

#include <QDialog>
#include <QJsonObject>

#include <functional>
#include <vector>

class QTableWidget;
class QPushButton;

class GuiExtensionConfigurationDialog : public QDialog {
public:
	GuiExtensionConfigurationDialog(
		QWidget* parent,
		std::vector<const GuiExtensionPlugin*> plugins,
		std::function<void()> onSaved = {});

private:
	void reloadFromDisk();
	void saveToDisk();
	QString _configPath() const;
	QJsonObject _loadConfigRoot() const;
	bool _writeConfigRoot(const QJsonObject& root) const;
	void _populateFromConfig(const QJsonObject& root);
	void _configureTable();
	static std::string _normalizeId(const std::string& text);

private:
	std::vector<const GuiExtensionPlugin*> _plugins;
	std::function<void()> _onSaved;
	QTableWidget* _table = nullptr;
	QPushButton* _reloadButton = nullptr;
	QPushButton* _saveButton = nullptr;
	QPushButton* _closeButton = nullptr;
};

#endif // GUIEXTENSIONCONFIGURATIONDIALOG_H
