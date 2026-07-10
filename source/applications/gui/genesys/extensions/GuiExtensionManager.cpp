#include "GuiExtensionManager.h"

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QToolBar>
#include <QStringList>
#include <QWidget>
#include <Qt>

#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <utility>

namespace {
std::string normalizePluginId(std::string text) {
	std::string normalized;
	normalized.reserve(text.size());
	for (unsigned char c : text) {
		if (!std::isspace(c)) {
			normalized.push_back(static_cast<char>(std::tolower(c)));
		}
	}
	if (normalized.size() > 3 && normalized.substr(normalized.size() - 3) == ".so") {
		normalized.erase(normalized.size() - 3);
	}
	return normalized;
}
} // namespace

GuiExtensionManager::GuiExtensionManager(QMainWindow* mainWindow) : _mainWindow(mainWindow) {
}

GuiExtensionManager::~GuiExtensionManager() {
	clear();
}

void GuiExtensionManager::setPlugins(std::vector<const GuiExtensionPlugin*> plugins) {
	_plugins = std::move(plugins);
}

void GuiExtensionManager::setLoadedModelPluginIds(std::vector<std::string> modelPluginIds) {
	_loadedModelPluginIds = std::move(modelPluginIds);
}

void GuiExtensionManager::rebuild(const GuiExtensionRuntimeContext& context) {
	clear();
	if (_mainWindow == nullptr) {
		return;
	}

	GuiExtensionRegistry registry;
	for (const GuiExtensionPlugin* plugin : _plugins) {
		if (plugin == nullptr) {
			continue;
		}
		if (!_isPluginDependenciesSatisfied(plugin)) {
			continue;
		}
		plugin->registerContributions(&registry);
	}

	for (const GuiActionContribution& action : registry.actions()) {
		_applyActionContribution(action, context);
	}
	for (const GuiWindowContribution& window : registry.windows()) {
		_applyWindowContribution(window, context);
	}
	for (const GuiDockContribution& dock : registry.docks()) {
		_applyDockContribution(dock, context);
	}
}

void GuiExtensionManager::clear() {
	for (QAction* action : _createdActions) {
		delete action;
	}
	_createdActions.clear();

	for (QDockWidget* dock : _createdDocks) {
		delete dock;
	}
	_createdDocks.clear();

	for (auto it = _createdMenuActions.rbegin(); it != _createdMenuActions.rend(); ++it) {
		delete *it;
	}
	_createdMenuActions.clear();
}

bool GuiExtensionManager::_isPluginDependenciesSatisfied(const GuiExtensionPlugin* plugin) const {
	if (plugin == nullptr) {
		return false;
	}

	const std::vector<std::string> required = plugin->requiredModelPlugins();
	if (required.empty()) {
		return true;
	}

	std::unordered_set<std::string> loaded;
	loaded.reserve(_loadedModelPluginIds.size());
	for (const std::string& id : _loadedModelPluginIds) {
		const std::string normalized = normalizePluginId(id);
		if (!normalized.empty()) {
			loaded.insert(normalized);
		}
	}

	for (const std::string& dependency : required) {
		const std::string normalized = normalizePluginId(dependency);
		if (normalized.empty()) {
			continue;
		}
		if (loaded.find(normalized) == loaded.end()) {
			return false;
		}
	}
	return true;
}

QMenu* GuiExtensionManager::_resolveMenuPath(const std::string& menuPath) {
	if (_mainWindow == nullptr || _mainWindow->menuBar() == nullptr || menuPath.empty()) {
		return nullptr;
	}

	QStringList tokens = QString::fromStdString(menuPath).split('/', Qt::SkipEmptyParts);
	if (tokens.isEmpty()) {
		return nullptr;
	}

	QMenu* currentMenu = nullptr;
	QAction* firstCreatedAction = nullptr;
	for (int i = 0; i < tokens.size(); ++i) {
		const QString token = tokens.at(i).trimmed();
		if (token.isEmpty()) {
			continue;
		}
		if (i == 0) {
			QAction* existingTopAction = nullptr;
			for (QAction* action : _mainWindow->menuBar()->actions()) {
				if (action != nullptr && action->menu() != nullptr && action->text() == token) {
					existingTopAction = action;
					break;
				}
			}
			if (existingTopAction == nullptr) {
				existingTopAction = _mainWindow->menuBar()->addMenu(token)->menuAction();
				if (firstCreatedAction == nullptr) {
					firstCreatedAction = existingTopAction;
				}
			}
			currentMenu = existingTopAction->menu();
		} else {
			QMenu* next = nullptr;
			for (QAction* action : currentMenu->actions()) {
				QMenu* submenu = action != nullptr ? action->menu() : nullptr;
				if (submenu != nullptr && submenu->title() == token) {
					next = submenu;
					break;
				}
			}
			if (next == nullptr) {
				next = currentMenu->addMenu(token);
				if (firstCreatedAction == nullptr) {
					firstCreatedAction = next->menuAction();
				}
			}
			currentMenu = next;
		}
	}

	if (firstCreatedAction != nullptr &&
		std::find(_createdMenuActions.begin(), _createdMenuActions.end(), firstCreatedAction) == _createdMenuActions.end()) {
		_createdMenuActions.push_back(firstCreatedAction);
	}

	return currentMenu;
}

QToolBar* GuiExtensionManager::_resolveToolBar(const std::string& toolBarId) const {
	if (_mainWindow == nullptr || toolBarId.empty()) {
		return nullptr;
	}

	const QString id = QString::fromStdString(toolBarId);
	if (QToolBar* byObjectName = _mainWindow->findChild<QToolBar*>(id)) {
		return byObjectName;
	}

	const QList<QToolBar*> toolBars = _mainWindow->findChildren<QToolBar*>();
	for (QToolBar* toolBar : toolBars) {
		if (toolBar != nullptr && toolBar->windowTitle() == id) {
			return toolBar;
		}
	}

	return nullptr;
}

void GuiExtensionManager::_applyActionContribution(const GuiActionContribution& contribution, const GuiExtensionRuntimeContext& context) {
	if (_mainWindow == nullptr) {
		return;
	}
	if (contribution.isVisible && !contribution.isVisible(context)) {
		return;
	}
	if (!contribution.trigger) {
		return;
	}

	QAction* action = new QAction(QString::fromStdString(contribution.text), _mainWindow);
	if (!contribution.id.empty()) {
		action->setObjectName(QString::fromStdString(contribution.id));
	}
	if (!contribution.statusTip.empty()) {
		action->setStatusTip(QString::fromStdString(contribution.statusTip));
	}
	if (!contribution.shortcut.empty()) {
		action->setShortcut(QKeySequence(QString::fromStdString(contribution.shortcut)));
	}
	if (contribution.isEnabled) {
		action->setEnabled(contribution.isEnabled(context));
	}
	QObject::connect(action, &QAction::triggered, _mainWindow, [contribution, context](bool) {
		contribution.trigger(context);
	});

	if (QMenu* menu = _resolveMenuPath(contribution.menuPath)) {
		menu->addAction(action);
	}
	if (QToolBar* toolBar = _resolveToolBar(contribution.toolBarId)) {
		toolBar->addAction(action);
	}

	_createdActions.push_back(action);
}

void GuiExtensionManager::_applyWindowContribution(const GuiWindowContribution& contribution, const GuiExtensionRuntimeContext& context) {
	if (_mainWindow == nullptr) {
		return;
	}
	if (contribution.isVisible && !contribution.isVisible(context)) {
		return;
	}
	if (!contribution.open) {
		return;
	}

	QAction* action = new QAction(QString::fromStdString(contribution.text), _mainWindow);
	if (!contribution.id.empty()) {
		action->setObjectName(QString::fromStdString(contribution.id));
	}
	QObject::connect(action, &QAction::triggered, _mainWindow, [contribution, context](bool) {
		contribution.open(context);
	});

	if (QMenu* menu = _resolveMenuPath(contribution.menuPath)) {
		menu->addAction(action);
	}

	_createdActions.push_back(action);
}

void GuiExtensionManager::_applyDockContribution(const GuiDockContribution& contribution, const GuiExtensionRuntimeContext& context) {
	if (_mainWindow == nullptr) {
		return;
	}
	if (contribution.isVisible && !contribution.isVisible(context)) {
		return;
	}
	if (!contribution.createWidget) {
		return;
	}

	QWidget* widget = contribution.createWidget(context);
	if (widget == nullptr) {
		return;
	}

	QDockWidget* dock = new QDockWidget(QString::fromStdString(contribution.title), _mainWindow);
	if (!contribution.id.empty()) {
		dock->setObjectName(QString::fromStdString(contribution.id));
	}
	dock->setWidget(widget);

	Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;
	const QString areaHint = QString::fromStdString(contribution.areaHint).trimmed().toLower();
	if (areaHint == "right") {
		area = Qt::RightDockWidgetArea;
	} else if (areaHint == "top") {
		area = Qt::TopDockWidgetArea;
	} else if (areaHint == "bottom") {
		area = Qt::BottomDockWidgetArea;
	}
	_mainWindow->addDockWidget(area, dock);
	_createdDocks.push_back(dock);
}
