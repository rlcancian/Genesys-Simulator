#include "systempreferences.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

#include <algorithm>

bool SystemPreferences::_startMaximized = true;
bool SystemPreferences::_autoLoadPlugins = true;
bool SystemPreferences::_checkSystemPackagesAtStart = true;
SystemPreferences::StartupModelMode SystemPreferences::_startupModelMode = SystemPreferences::StartupModelMode::NewModel;
std::string SystemPreferences::_modelfilenameToOpen = "";
std::string SystemPreferences::_lastModelFilename = "";
std::vector<std::string> SystemPreferences::_recentModelFiles = {};
unsigned int SystemPreferences::_recentModelFilesLimit = 10;
TraceManager::Level SystemPreferences::_traceLevel = TraceManager::Level::L9_mostDetailed;
SystemPreferences::VisualTheme SystemPreferences::_visualTheme = SystemPreferences::VisualTheme::Light;
SystemPreferences::InterfaceStyle SystemPreferences::_interfaceStyle = SystemPreferences::InterfaceStyle::Classic;
int SystemPreferences::_applicationFontPointSize = 0;
bool SystemPreferences::_diagramUsesThemeColors = true;
SystemPreferences::AutomaticPositioningStrategy SystemPreferences::_automaticPositioningStrategy = SystemPreferences::AutomaticPositioningStrategy::Centered;

namespace {
const char* const kVersionKey = "version";
const char* const kStartupKey = "startup";
const char* const kPluginsKey = "plugins";
const char* const kDiagnosticsKey = "diagnostics";
const char* const kViewKey = "view";
const char* const kModelStartupModeKey = "modelAtStart";
const char* const kSpecificModelKey = "specificModelFile";
const char* const kLastModelKey = "lastModelFile";
const char* const kRecentModelFilesKey = "recentModelFiles";
const char* const kRecentModelFilesLimitKey = "recentModelFilesLimit";
const char* const kAutomaticPositioningStrategyKey = "automaticPositioningStrategy";

std::vector<std::string> sanitizeRecentModelFiles(const QJsonArray& array, unsigned int limit) {
    std::vector<std::string> sanitized;
    sanitized.reserve(std::min<unsigned int>(array.size(), limit));
    for (const QJsonValue& value : array) {
        if (!value.isString()) {
            continue;
        }
        const std::string path = value.toString().trimmed().toStdString();
        if (path.empty()) {
            continue;
        }
        if (std::find(sanitized.begin(), sanitized.end(), path) != sanitized.end()) {
            continue;
        }
        sanitized.push_back(path);
        if (sanitized.size() >= limit) {
            break;
        }
    }
    return sanitized;
}

void clampRecentModelFiles(std::vector<std::string>* recentFiles, unsigned int limit) {
    if (recentFiles == nullptr) {
        return;
    }
    if (limit == 0) {
        limit = 1;
    }
    std::vector<std::string> deduplicated;
    deduplicated.reserve(recentFiles->size());
    for (const std::string& path : *recentFiles) {
        if (path.empty()) {
            continue;
        }
        if (std::find(deduplicated.begin(), deduplicated.end(), path) != deduplicated.end()) {
            continue;
        }
        deduplicated.push_back(path);
        if (deduplicated.size() >= limit) {
            break;
        }
    }
    *recentFiles = std::move(deduplicated);
}
}

QString SystemPreferences::defaultConfigDirectory() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (path.isEmpty()) {
        path = QDir::home().absoluteFilePath(".genesys");
    }
    return path;
}

QString SystemPreferences::configFilePath() {
    return QDir(defaultConfigDirectory()).absoluteFilePath("preferences.json");
}

void SystemPreferences::resetToDefaults() {
    _startMaximized = true;
    _autoLoadPlugins = true;
    _checkSystemPackagesAtStart = true;
    _startupModelMode = StartupModelMode::NewModel;
    _modelfilenameToOpen.clear();
    _lastModelFilename.clear();
    _recentModelFiles.clear();
    _recentModelFilesLimit = 10;
    _traceLevel = TraceManager::Level::L9_mostDetailed;
    _visualTheme = VisualTheme::Light;
    _interfaceStyle = InterfaceStyle::Classic;
    _applicationFontPointSize = 0;
    _diagramUsesThemeColors = true;
    _automaticPositioningStrategy = AutomaticPositioningStrategy::Centered;
}

bool SystemPreferences::load() {
    resetToDefaults();

    QFile file(configFilePath());
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        resetToDefaults();
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonObject startup = root.value(kStartupKey).toObject();
    const QJsonObject plugins = root.value(kPluginsKey).toObject();
    const QJsonObject diagnostics = root.value(kDiagnosticsKey).toObject();
    const QJsonObject view = root.value(kViewKey).toObject();

    _startMaximized = startup.value("startMaximized").toBool(_startMaximized);
    _startupModelMode = startupModelModeFromString(startup.value(kModelStartupModeKey).toString(startupModelModeToString(_startupModelMode)));
    _modelfilenameToOpen = startup.value(kSpecificModelKey).toString(QString::fromStdString(_modelfilenameToOpen)).toStdString();
    _lastModelFilename = startup.value(kLastModelKey).toString(QString::fromStdString(_lastModelFilename)).toStdString();
    _recentModelFilesLimit = static_cast<unsigned int>(std::clamp(startup.value(kRecentModelFilesLimitKey).toInt(static_cast<int>(_recentModelFilesLimit)), 1, 50));
    _recentModelFiles = sanitizeRecentModelFiles(startup.value(kRecentModelFilesKey).toArray(), _recentModelFilesLimit);
    if (!_lastModelFilename.empty() && std::find(_recentModelFiles.begin(), _recentModelFiles.end(), _lastModelFilename) == _recentModelFiles.end()) {
        _recentModelFiles.insert(_recentModelFiles.begin(), _lastModelFilename);
    }
    clampRecentModelFiles(&_recentModelFiles, _recentModelFilesLimit);

    _autoLoadPlugins = plugins.value("autoLoad").toBool(_autoLoadPlugins);
    _checkSystemPackagesAtStart = plugins.value("checkSystemPackagesAtStart").toBool(_checkSystemPackagesAtStart);

    _traceLevel = traceLevelFromInt(diagnostics.value("traceLevel").toInt(static_cast<int>(_traceLevel)));

    _visualTheme = visualThemeFromString(view.value("theme").toString(visualThemeToString(_visualTheme)));
    _interfaceStyle = interfaceStyleFromString(view.value("interfaceStyle").toString(interfaceStyleToString(_interfaceStyle)));
    _applicationFontPointSize = view.value("fontPointSize").toInt(_applicationFontPointSize);
    if (_applicationFontPointSize < 0) {
        _applicationFontPointSize = 0;
    }
    _diagramUsesThemeColors = view.value("diagramUsesThemeColors").toBool(_diagramUsesThemeColors);
    _automaticPositioningStrategy = automaticPositioningStrategyFromString(
        view.value(kAutomaticPositioningStrategyKey).toString(
            automaticPositioningStrategyToString(_automaticPositioningStrategy)));

    return true;
}

bool SystemPreferences::save() {
    QDir dir(defaultConfigDirectory());
    if (!dir.mkpath(".")) {
        return false;
    }

    QJsonObject startup;
    startup.insert("startMaximized", _startMaximized);
    startup.insert(kModelStartupModeKey, startupModelModeToString(_startupModelMode));
    startup.insert(kSpecificModelKey, QString::fromStdString(_modelfilenameToOpen));
    startup.insert(kLastModelKey, QString::fromStdString(_lastModelFilename));
    startup.insert(kRecentModelFilesLimitKey, static_cast<int>(_recentModelFilesLimit));
    QJsonArray recentModelFiles;
    for (const std::string& path : _recentModelFiles) {
        recentModelFiles.append(QString::fromStdString(path));
    }
    startup.insert(kRecentModelFilesKey, recentModelFiles);

    QJsonObject plugins;
    plugins.insert("autoLoad", _autoLoadPlugins);
    plugins.insert("checkSystemPackagesAtStart", _checkSystemPackagesAtStart);

    QJsonObject diagnostics;
    diagnostics.insert("traceLevel", static_cast<int>(_traceLevel));

    QJsonObject view;
    view.insert("theme", visualThemeToString(_visualTheme));
    view.insert("interfaceStyle", interfaceStyleToString(_interfaceStyle));
    view.insert("fontPointSize", _applicationFontPointSize);
    view.insert("diagramUsesThemeColors", _diagramUsesThemeColors);
    view.insert(kAutomaticPositioningStrategyKey, automaticPositioningStrategyToString(_automaticPositioningStrategy));

    QJsonObject root;
    root.insert(kVersionKey, 1);
    root.insert(kStartupKey, startup);
    root.insert(kPluginsKey, plugins);
    root.insert(kDiagnosticsKey, diagnostics);
    root.insert(kViewKey, view);

    QFile file(configFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool SystemPreferences::startMaximized() {
    return _startMaximized;
}

void SystemPreferences::setStartMaximized(bool newStartMaximized) {
    _startMaximized = newStartMaximized;
}

bool SystemPreferences::autoLoadPlugins() {
    return _autoLoadPlugins;
}

void SystemPreferences::setAutoLoadPlugins(bool newAutoLoadPlugins) {
    _autoLoadPlugins = newAutoLoadPlugins;
}

SystemPreferences::StartupModelMode SystemPreferences::startupModelMode() {
    return _startupModelMode;
}

void SystemPreferences::setStartupModelMode(StartupModelMode newStartupModelMode) {
    _startupModelMode = newStartupModelMode;
}

unsigned short SystemPreferences::modelAtStart() {
    return static_cast<unsigned short>(_startupModelMode);
}

void SystemPreferences::setModelAtStart(unsigned short newModelAtStart) {
    if (newModelAtStart <= static_cast<unsigned short>(StartupModelMode::OpenLastModel)) {
        _startupModelMode = static_cast<StartupModelMode>(newModelAtStart);
    }
}

std::string SystemPreferences::modelfilename() {
    return _modelfilenameToOpen;
}

void SystemPreferences::setModelfilename(const std::string& newModelfilename) {
    _modelfilenameToOpen = newModelfilename;
}

std::string SystemPreferences::lastModelFilename() {
    return _lastModelFilename;
}

void SystemPreferences::setLastModelFilename(const std::string& newLastModelFilename) {
    _lastModelFilename = newLastModelFilename;
}

std::vector<std::string> SystemPreferences::recentModelFiles() {
    return _recentModelFiles;
}

void SystemPreferences::setRecentModelFiles(const std::vector<std::string>& recentModelFiles) {
    _recentModelFiles = recentModelFiles;
    clampRecentModelFiles(&_recentModelFiles, _recentModelFilesLimit);
}

void SystemPreferences::pushRecentModelFile(const std::string& modelFilename) {
    if (modelFilename.empty()) {
        return;
    }
    _recentModelFiles.erase(std::remove(_recentModelFiles.begin(), _recentModelFiles.end(), modelFilename), _recentModelFiles.end());
    _recentModelFiles.insert(_recentModelFiles.begin(), modelFilename);
    _lastModelFilename = modelFilename;
    clampRecentModelFiles(&_recentModelFiles, _recentModelFilesLimit);
}

unsigned int SystemPreferences::recentModelFilesLimit() {
    return _recentModelFilesLimit;
}

void SystemPreferences::setRecentModelFilesLimit(unsigned int recentModelFilesLimit) {
    _recentModelFilesLimit = std::clamp(recentModelFilesLimit, 1u, 50u);
    clampRecentModelFiles(&_recentModelFiles, _recentModelFilesLimit);
}

bool SystemPreferences::checkSystemPackagesAtStart() {
    return _checkSystemPackagesAtStart;
}

void SystemPreferences::setCheckSystemPackagesAtStart(bool newCheckSystemPackagesAtStart) {
    _checkSystemPackagesAtStart = newCheckSystemPackagesAtStart;
}

TraceManager::Level SystemPreferences::traceLevel() {
    return _traceLevel;
}

void SystemPreferences::setTraceLevel(TraceManager::Level newTraceLevel) {
    _traceLevel = newTraceLevel;
}

SystemPreferences::VisualTheme SystemPreferences::visualTheme() {
    return _visualTheme;
}

void SystemPreferences::setVisualTheme(VisualTheme newVisualTheme) {
    _visualTheme = newVisualTheme;
}

std::string SystemPreferences::visualThemeName() {
    return visualThemeToString(_visualTheme).toStdString();
}

SystemPreferences::InterfaceStyle SystemPreferences::interfaceStyle() {
    return _interfaceStyle;
}

void SystemPreferences::setInterfaceStyle(InterfaceStyle newInterfaceStyle) {
    _interfaceStyle = newInterfaceStyle;
}

std::string SystemPreferences::interfaceStyleName() {
    return interfaceStyleToString(_interfaceStyle).toStdString();
}

int SystemPreferences::applicationFontPointSize() {
    return _applicationFontPointSize;
}

void SystemPreferences::setApplicationFontPointSize(int newApplicationFontPointSize) {
    _applicationFontPointSize = std::max(0, newApplicationFontPointSize);
}

bool SystemPreferences::diagramUsesThemeColors() {
    return _diagramUsesThemeColors;
}

void SystemPreferences::setDiagramUsesThemeColors(bool newDiagramUsesThemeColors) {
    _diagramUsesThemeColors = newDiagramUsesThemeColors;
}

SystemPreferences::AutomaticPositioningStrategy SystemPreferences::automaticPositioningStrategy() {
    return _automaticPositioningStrategy;
}

void SystemPreferences::setAutomaticPositioningStrategy(AutomaticPositioningStrategy strategy) {
    _automaticPositioningStrategy = strategy;
}

unsigned int SystemPreferences::canvasBackgroundColor() {
    return _visualTheme == VisualTheme::Dark ? 0x2A2A2AFF : 0xFFFF8040;
}

unsigned int SystemPreferences::canvasDisabledBackgroundColor() {
    return _visualTheme == VisualTheme::Dark ? 0x38383880 : 0xC0C0C020;
}

unsigned int SystemPreferences::gridColor() {
    return _visualTheme == VisualTheme::Dark ? 0x595959D0 : 0xC0C0C0E0;
}

SystemPreferences::StartupModelMode SystemPreferences::startupModelModeFromString(const QString& value) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == "none") {
        return StartupModelMode::NoModel;
    }
    if (normalized == "specific" || normalized == "open-specific") {
        return StartupModelMode::OpenSpecificModel;
    }
    if (normalized == "last" || normalized == "open-last") {
        return StartupModelMode::OpenLastModel;
    }
    return StartupModelMode::NewModel;
}

QString SystemPreferences::startupModelModeToString(StartupModelMode value) {
    switch (value) {
    case StartupModelMode::NoModel:
        return QStringLiteral("none");
    case StartupModelMode::OpenSpecificModel:
        return QStringLiteral("specific");
    case StartupModelMode::OpenLastModel:
        return QStringLiteral("last");
    case StartupModelMode::NewModel:
    default:
        return QStringLiteral("new");
    }
}

SystemPreferences::VisualTheme SystemPreferences::visualThemeFromString(const QString& value) {
    return value.trimmed().toLower() == "dark" ? VisualTheme::Dark : VisualTheme::Light;
}

QString SystemPreferences::visualThemeToString(VisualTheme value) {
    return value == VisualTheme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
}

SystemPreferences::InterfaceStyle SystemPreferences::interfaceStyleFromString(const QString& value) {
    return value.trimmed().toLower() == "modern" ? InterfaceStyle::Modern : InterfaceStyle::Classic;
}

QString SystemPreferences::interfaceStyleToString(InterfaceStyle value) {
    return value == InterfaceStyle::Modern ? QStringLiteral("modern") : QStringLiteral("classic");
}

SystemPreferences::AutomaticPositioningStrategy SystemPreferences::automaticPositioningStrategyFromString(const QString& value) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == "legacy") {
        return AutomaticPositioningStrategy::Legacy;
    }
    return AutomaticPositioningStrategy::Centered;
}

QString SystemPreferences::automaticPositioningStrategyToString(AutomaticPositioningStrategy value) {
    return value == AutomaticPositioningStrategy::Legacy ? QStringLiteral("legacy") : QStringLiteral("centered");
}

TraceManager::Level SystemPreferences::traceLevelFromInt(int value) {
    if (value < static_cast<int>(TraceManager::Level::L0_noTraces)) {
        return TraceManager::Level::L0_noTraces;
    }
    if (value > static_cast<int>(TraceManager::Level::L9_mostDetailed)) {
        return TraceManager::Level::L9_mostDetailed;
    }
    return static_cast<TraceManager::Level>(value);
}
