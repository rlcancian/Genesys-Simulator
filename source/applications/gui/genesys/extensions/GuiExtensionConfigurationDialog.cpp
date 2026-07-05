#include "GuiExtensionConfigurationDialog.h"

#include "../systempreferences.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace {
constexpr int kColumnEnabled = 0;
constexpr int kColumnOrder = 1;
constexpr int kColumnExtensionId = 2;
constexpr int kColumnName = 3;
constexpr int kColumnVersion = 4;
constexpr int kColumnCategory = 5;
constexpr int kColumnGroup = 6;
constexpr int kColumnPriority = 7;
constexpr int kColumnRequiredModelPlugins = 8;

QTableWidgetItem* readOnlyItem(const QString& text) {
	QTableWidgetItem* item = new QTableWidgetItem(text);
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	return item;
}
} // namespace

GuiExtensionConfigurationDialog::GuiExtensionConfigurationDialog(
	QWidget* parent,
	std::vector<const GuiExtensionPlugin*> plugins,
	std::function<void()> onSaved)
	: QDialog(parent), _plugins(std::move(plugins)), _onSaved(std::move(onSaved)) {
	setWindowTitle(tr("Graphical Extensions"));
	setMinimumSize(960, 520);
	setModal(true);

	_configureTable();
	_reloadButton = new QPushButton(tr("Reload"), this);
	_saveButton = new QPushButton(tr("Save"), this);
	_closeButton = new QPushButton(tr("Close"), this);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(_reloadButton);
	buttonLayout->addStretch();
	buttonLayout->addWidget(_saveButton);
	buttonLayout->addWidget(_closeButton);

	QVBoxLayout* rootLayout = new QVBoxLayout(this);
	rootLayout->addWidget(_table);
	rootLayout->addLayout(buttonLayout);

	connect(_reloadButton, &QPushButton::clicked, this, &GuiExtensionConfigurationDialog::reloadFromDisk);
	connect(_saveButton, &QPushButton::clicked, this, &GuiExtensionConfigurationDialog::saveToDisk);
	connect(_closeButton, &QPushButton::clicked, this, &QDialog::accept);

	std::sort(_plugins.begin(), _plugins.end(), [](const GuiExtensionPlugin* left, const GuiExtensionPlugin* right) {
		if (left == nullptr || right == nullptr) {
			return left != nullptr;
		}
		if (left->displayName() != right->displayName()) {
			return left->displayName() < right->displayName();
		}
		return left->extensionId() < right->extensionId();
	});

	reloadFromDisk();
}

void GuiExtensionConfigurationDialog::_configureTable() {
	_table = new QTableWidget(this);
	_table->setColumnCount(9);
	_table->setAlternatingRowColors(true);
	_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	_table->setSelectionMode(QAbstractItemView::SingleSelection);
	_table->setHorizontalHeaderLabels({
		tr("Enabled"),
		tr("Order"),
		tr("Extension Id"),
		tr("Name"),
		tr("Version"),
		tr("Category"),
		tr("Group"),
		tr("Priority"),
		tr("Required Model Plugins")
	});
	_table->horizontalHeader()->setSectionResizeMode(kColumnEnabled, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnOrder, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnExtensionId, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnName, QHeaderView::Stretch);
	_table->horizontalHeader()->setSectionResizeMode(kColumnVersion, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnCategory, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnGroup, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnPriority, QHeaderView::ResizeToContents);
	_table->horizontalHeader()->setSectionResizeMode(kColumnRequiredModelPlugins, QHeaderView::Stretch);
}

QString GuiExtensionConfigurationDialog::_configPath() const {
	const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	const QString envPath = env.value("GENESYS_GUI_EXTENSIONS_CONFIG").trimmed();
	if (!envPath.isEmpty()) {
		return envPath;
	}

	QFileInfo preferencesPath(SystemPreferences::configFilePath());
	const QDir baseDir = preferencesPath.dir();
	return baseDir.absoluteFilePath("gui_extensions.json");
}

QJsonObject GuiExtensionConfigurationDialog::_loadConfigRoot() const {
	const QString path = _configPath();
	QFile file(path);
	if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return {};
	}

	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
	if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
		return {};
	}
	return document.object();
}

bool GuiExtensionConfigurationDialog::_writeConfigRoot(const QJsonObject& root) const {
	const QString path = _configPath();
	QFileInfo info(path);
	QDir dir = info.dir();
	if (!dir.mkpath(".")) {
		return false;
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		return false;
	}
	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	return true;
}

std::string GuiExtensionConfigurationDialog::_normalizeId(const std::string& text) {
	std::string normalized;
	normalized.reserve(text.size());
	for (unsigned char c : text) {
		if (std::isspace(c)) {
			continue;
		}
		normalized.push_back(static_cast<char>(std::tolower(c)));
	}
	return normalized;
}

void GuiExtensionConfigurationDialog::_populateFromConfig(const QJsonObject& root) {
	std::unordered_set<std::string> enabled;
	std::unordered_set<std::string> disabled;
	std::unordered_map<std::string, int> orderById;
	std::unordered_map<std::string, QJsonObject> presentationById;

	const QJsonArray enabledArray = root.value("enabled").toArray();
	for (const QJsonValue& value : enabledArray) {
		if (value.isString()) {
			enabled.insert(_normalizeId(value.toString().toStdString()));
		}
	}
	const QJsonArray disabledArray = root.value("disabled").toArray();
	for (const QJsonValue& value : disabledArray) {
		if (value.isString()) {
			disabled.insert(_normalizeId(value.toString().toStdString()));
		}
	}
	const QJsonArray orderArray = root.value("order").toArray();
	for (int i = 0; i < orderArray.size(); ++i) {
		const QJsonValue value = orderArray.at(i);
		if (!value.isString()) {
			continue;
		}
		orderById[_normalizeId(value.toString().toStdString())] = i + 1;
	}
	const QJsonObject presentationObject = root.value("presentation").toObject();
	for (auto it = presentationObject.constBegin(); it != presentationObject.constEnd(); ++it) {
		if (it.value().isObject()) {
			presentationById[_normalizeId(it.key().toStdString())] = it.value().toObject();
		}
	}

	_table->setRowCount(0);
	_table->setRowCount(static_cast<int>(_plugins.size()));
	for (int row = 0; row < static_cast<int>(_plugins.size()); ++row) {
		const GuiExtensionPlugin* plugin = _plugins.at(row);
		if (plugin == nullptr) {
			continue;
		}
		const std::string id = plugin->extensionId();
		const std::string normalizedId = _normalizeId(id);
		const bool hasEnabledAllowlist = !enabled.empty();
		const bool isEnabledByConfig = hasEnabledAllowlist
		                               ? enabled.find(normalizedId) != enabled.end()
		                               : disabled.find(normalizedId) == disabled.end();

		QTableWidgetItem* enabledItem = readOnlyItem(QString());
		enabledItem->setCheckState(isEnabledByConfig ? Qt::Checked : Qt::Unchecked);
		enabledItem->setFlags(enabledItem->flags() | Qt::ItemIsUserCheckable);
		_table->setItem(row, kColumnEnabled, enabledItem);

		QSpinBox* orderEditor = new QSpinBox(_table);
		orderEditor->setMinimum(0);
		orderEditor->setMaximum(9999);
		orderEditor->setSpecialValueText(tr("none"));
		orderEditor->setValue(orderById.count(normalizedId) ? orderById[normalizedId] : 0);
		_table->setCellWidget(row, kColumnOrder, orderEditor);

		_table->setItem(row, kColumnExtensionId, readOnlyItem(QString::fromStdString(id)));
		_table->setItem(row, kColumnName, readOnlyItem(QString::fromStdString(plugin->displayName())));
		_table->setItem(row, kColumnVersion, readOnlyItem(QString::fromStdString(plugin->version())));

		QJsonObject presentation = presentationById.count(normalizedId) ? presentationById[normalizedId] : QJsonObject{};
		_table->setItem(row, kColumnCategory, new QTableWidgetItem(presentation.value("category").toString()));
		_table->setItem(row, kColumnGroup, new QTableWidgetItem(presentation.value("group").toString()));

		QSpinBox* priorityEditor = new QSpinBox(_table);
		priorityEditor->setMinimum(-100000);
		priorityEditor->setMaximum(100000);
		priorityEditor->setValue(presentation.value("priority").toInt(0));
		_table->setCellWidget(row, kColumnPriority, priorityEditor);

		QStringList required;
		for (const std::string& dep : plugin->requiredModelPlugins()) {
			required << QString::fromStdString(dep);
		}
		_table->setItem(row, kColumnRequiredModelPlugins, readOnlyItem(required.join(", ")));
	}
}

void GuiExtensionConfigurationDialog::reloadFromDisk() {
	_populateFromConfig(_loadConfigRoot());
}

void GuiExtensionConfigurationDialog::saveToDisk() {
	QJsonArray enabled;
	QJsonArray disabled;
	std::vector<std::pair<int, QString>> ordered;
	QJsonObject presentation;

	for (int row = 0; row < _table->rowCount(); ++row) {
		QTableWidgetItem* idItem = _table->item(row, kColumnExtensionId);
		QTableWidgetItem* enabledItem = _table->item(row, kColumnEnabled);
		if (idItem == nullptr || enabledItem == nullptr) {
			continue;
		}
		const QString extensionId = idItem->text().trimmed();
		if (extensionId.isEmpty()) {
			continue;
		}
		const bool isEnabled = enabledItem->checkState() == Qt::Checked;
		if (isEnabled) {
			enabled.append(extensionId);
		} else {
			disabled.append(extensionId);
		}

		QSpinBox* orderEditor = qobject_cast<QSpinBox*>(_table->cellWidget(row, kColumnOrder));
		if (orderEditor != nullptr && orderEditor->value() > 0) {
			ordered.emplace_back(orderEditor->value(), extensionId);
		}

		const QString category = _table->item(row, kColumnCategory) != nullptr
		                         ? _table->item(row, kColumnCategory)->text().trimmed()
		                         : QString();
		const QString group = _table->item(row, kColumnGroup) != nullptr
		                      ? _table->item(row, kColumnGroup)->text().trimmed()
		                      : QString();
		QSpinBox* priorityEditor = qobject_cast<QSpinBox*>(_table->cellWidget(row, kColumnPriority));
		const int priority = priorityEditor != nullptr ? priorityEditor->value() : 0;
		if (!category.isEmpty() || !group.isEmpty() || priority != 0) {
			QJsonObject item;
			if (!category.isEmpty()) {
				item.insert("category", category);
			}
			if (!group.isEmpty()) {
				item.insert("group", group);
			}
			if (priority != 0) {
				item.insert("priority", priority);
			}
			presentation.insert(extensionId, item);
		}
	}

	std::sort(ordered.begin(), ordered.end(), [](const auto& left, const auto& right) {
		if (left.first != right.first) {
			return left.first < right.first;
		}
		return left.second < right.second;
	});
	QJsonArray order;
	for (const auto& pair : ordered) {
		order.append(pair.second);
	}

	QJsonObject root = _loadConfigRoot();
	root.insert("enabled", enabled);
	root.insert("disabled", disabled);
	root.insert("order", order);
	root.insert("presentation", presentation);

	if (!_writeConfigRoot(root)) {
		QMessageBox::critical(this, tr("Graphical Extensions"), tr("Could not save graphical extension configuration."));
		return;
	}

	if (_onSaved) {
		_onSaved();
	}

	QMessageBox::information(this, tr("Graphical Extensions"), tr("Configuration saved to:\n%1").arg(_configPath()));
}
