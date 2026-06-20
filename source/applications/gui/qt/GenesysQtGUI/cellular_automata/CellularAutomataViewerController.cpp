#include "CellularAutomataViewerController.h"

#include <algorithm>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr const char* kFormatId = "genesys.cellular-automata";
constexpr int kFormatVersion = 2;

QJsonArray encodeGrid(const CellularAutomataDemoModel* model) {
	QJsonArray rows;
	if (model == nullptr) {
		return rows;
	}
	for (int y = 0; y < model->rows(); ++y) {
		QJsonArray row;
		for (int x = 0; x < model->columns(); ++x) {
			row.append(static_cast<int>(model->stateAt(x, y)));
		}
		rows.append(row);
	}
	return rows;
}

QString describeFileError(const QFile& file, const QString& action) {
	return QObject::tr("Could not %1 '%2': %3").arg(action, file.fileName(), file.errorString());
}

int presetValue(CellularAutomataRulePreset preset) {
	return static_cast<int>(preset);
}

int presetValue(CellularAutomataNeighborhoodPreset preset) {
	return static_cast<int>(preset);
}

int presetValue(CellularAutomataStatePreset preset) {
	return static_cast<int>(preset);
}

CellularAutomataRulePreset rulePresetFromInt(int value) {
	switch (value) {
	case 1:
		return CellularAutomataRulePreset::Growty;
	case 2:
		return CellularAutomataRulePreset::Identity;
	case 3:
		return CellularAutomataRulePreset::ForestFire;
	case 0:
	default:
		return CellularAutomataRulePreset::GameOfLife;
	}
}

CellularAutomataNeighborhoodPreset neighborhoodPresetFromInt(int value) {
	switch (value) {
	case 1:
		return CellularAutomataNeighborhoodPreset::VonNeumann;
	case 0:
	default:
		return CellularAutomataNeighborhoodPreset::Moore;
	}
}

CellularAutomataStatePreset statePresetFromInt(int value) {
	switch (value) {
	case 1:
		return CellularAutomataStatePreset::Enumerated;
	case 2:
		return CellularAutomataStatePreset::Numeric;
	case 0:
	default:
		return CellularAutomataStatePreset::Binary;
	}
}
} // namespace

CellularAutomataViewerController::CellularAutomataViewerController(QObject* parent)
	: QObject(parent) {
	_timer.setSingleShot(false);
	_timer.setTimerType(Qt::CoarseTimer);
	connect(&_timer, &QTimer::timeout, this, &CellularAutomataViewerController::_onTimerTimeout);
	rebuildDemo();
}

CellularAutomataDemoModel* CellularAutomataViewerController::model() const {
	return _demo.get();
}

CellularAutomataDemoSettings CellularAutomataViewerController::settings() const {
	return _settings;
}

QSize CellularAutomataViewerController::latticeSize() const {
	return _demo != nullptr ? _demo->settings.latticeSize : QSize{};
}

bool CellularAutomataViewerController::hasDemo() const {
	return _demo != nullptr && _demo->lattice != nullptr;
}

unsigned int CellularAutomataViewerController::simulatedTime() const {
	return _simulatedTime;
}

bool CellularAutomataViewerController::isRunning() const {
	return _running;
}

QString CellularAutomataViewerController::configurationSummary() const {
	return QStringLiteral("%1 / %2 / %3 / %4x%5")
	    .arg(cellularAutomataRulePresetText(_settings.rulePreset))
	    .arg(cellularAutomataNeighborhoodPresetText(_settings.neighborhoodPreset))
	    .arg(cellularAutomataStatePresetText(_settings.statePreset))
	    .arg(_settings.latticeSize.width())
	    .arg(_settings.latticeSize.height());
}

std::vector<long> CellularAutomataViewerController::availablePaintStates() const {
	return _demo != nullptr ? _demo->availablePaintStates() : std::vector<long>{0, 1};
}

long CellularAutomataViewerController::cellState(int x, int y) const {
	return _demo != nullptr ? _demo->stateAt(x, y) : 0;
}

bool CellularAutomataViewerController::setCellState(int x, int y, long value) {
	if (_demo == nullptr) {
		return false;
	}
	const bool changed = _demo->setStateAt(x, y, value);
	if (changed) {
		emit modelChanged();
	}
	return changed;
}

void CellularAutomataViewerController::fill(long value) {
	if (_demo == nullptr) {
		return;
	}
	_demo->fill(value);
	emit modelChanged();
}

bool CellularAutomataViewerController::hasSelectedPaintState() const {
	return _selectedPaintState.has_value();
}

std::optional<long> CellularAutomataViewerController::selectedPaintState() const {
	return _selectedPaintState;
}

QString CellularAutomataViewerController::selectedPaintStateText() const {
	return _selectedPaintState.has_value() ? QString::number(*_selectedPaintState) : QObject::tr("unset");
}

void CellularAutomataViewerController::setSelectedPaintState(long state) {
	if (_selectedPaintState.has_value() && *_selectedPaintState == state) {
		return;
	}
	_selectedPaintState = state;
	emit paintStateChanged();
}

void CellularAutomataViewerController::clearSelectedPaintState() {
	if (!_selectedPaintState.has_value()) {
		return;
	}
	_selectedPaintState.reset();
	emit paintStateChanged();
}

int CellularAutomataViewerController::autoStepIntervalMs() const {
	return _autoStepIntervalMs;
}

void CellularAutomataViewerController::setAutoStepIntervalMs(int intervalMs) {
	_autoStepIntervalMs = std::max(1, intervalMs);
	if (_running) {
		_timer.start(_autoStepIntervalMs);
	}
}

void CellularAutomataViewerController::rebuildDemo(CellularAutomataDemoSettings settings) {
	settings.latticeSize = settings.latticeSize.expandedTo(QSize{8, 8});
	_demo = CellularAutomataDemoBuilder::buildDemo(settings);
	_settings = _demo->settings;
	_resetRuntimeState();

	const std::vector<long> paintStates = availablePaintStates();
	if (!paintStates.empty()) {
		if (!_selectedPaintState.has_value() ||
		    std::find(paintStates.begin(), paintStates.end(), *_selectedPaintState) == paintStates.end()) {
			_selectedPaintState = paintStates.front();
		}
	} else {
		_selectedPaintState.reset();
	}

	emit modelChanged();
	emit settingsChanged();
	emit timeChanged(_simulatedTime);
	emit runningChanged(_running);
	emit paintStateChanged();
}

void CellularAutomataViewerController::setSettings(CellularAutomataDemoSettings settings) {
	rebuildDemo(settings);
}

void CellularAutomataViewerController::setRulePreset(CellularAutomataRulePreset preset) {
	if (_settings.rulePreset == preset) {
		return;
	}
	auto settings = _settings;
	settings.rulePreset = preset;
	if (preset == CellularAutomataRulePreset::ForestFire) {
		settings.statePreset = CellularAutomataStatePreset::Enumerated;
	}
	rebuildDemo(settings);
}

void CellularAutomataViewerController::setNeighborhoodPreset(CellularAutomataNeighborhoodPreset preset) {
	if (_settings.neighborhoodPreset == preset) {
		return;
	}
	auto settings = _settings;
	settings.neighborhoodPreset = preset;
	rebuildDemo(settings);
}

void CellularAutomataViewerController::setStatePreset(CellularAutomataStatePreset preset) {
	if (_settings.statePreset == preset) {
		return;
	}
	auto settings = _settings;
	settings.statePreset = preset;
	rebuildDemo(settings);
}

void CellularAutomataViewerController::resetToInitialState() {
	rebuildDemo(_settings);
}

void CellularAutomataViewerController::step() {
	if (!hasDemo()) {
		return;
	}
	_demo->automaton->step();
	++_simulatedTime;
	emit timeChanged(_simulatedTime);
	emit modelChanged();
}

void CellularAutomataViewerController::start() {
	if (!hasDemo()) {
		return;
	}
	if (_running) {
		return;
	}
	_running = true;
	_timer.start(_autoStepIntervalMs);
	emit runningChanged(true);
	emit statusMessage(QObject::tr("Simulation started."));
}

void CellularAutomataViewerController::pause() {
	if (!_running) {
		return;
	}
	_running = false;
	_timer.stop();
	emit runningChanged(false);
	emit statusMessage(QObject::tr("Simulation paused."));
}

void CellularAutomataViewerController::stop() {
	if (!_running) {
		return;
	}
	_running = false;
	_timer.stop();
	emit runningChanged(false);
	emit statusMessage(QObject::tr("Simulation stopped."));
}

bool CellularAutomataViewerController::saveConfiguration(const QString& filePath, QString* errorMessage) const {
	if (!hasDemo()) {
		if (errorMessage != nullptr) {
			*errorMessage = QObject::tr("There is no automaton to save.");
		}
		return false;
	}

	QJsonObject root;
	root["format"] = QString::fromLatin1(kFormatId);
	root["version"] = kFormatVersion;
	root["name"] = cellularAutomataRulePresetText(_settings.rulePreset);
	root["time"] = static_cast<int>(_simulatedTime);
	root["selectedPaintState"] = _selectedPaintState.has_value()
	                                 ? QJsonValue(static_cast<int>(*_selectedPaintState))
	                                 : QJsonValue();

	QJsonObject settings;
	settings["latticeWidth"] = _settings.latticeSize.width();
	settings["latticeHeight"] = _settings.latticeSize.height();
	settings["rulePreset"] = presetValue(_settings.rulePreset);
	settings["neighborhoodPreset"] = presetValue(_settings.neighborhoodPreset);
	settings["statePreset"] = presetValue(_settings.statePreset);
	root["settings"] = settings;

	QJsonObject metadata;
	metadata["lattice"] = QStringLiteral("reticular");
	metadata["boundary"] = QStringLiteral("closed");
	metadata["neighborhood"] = cellularAutomataNeighborhoodPresetText(_settings.neighborhoodPreset);
	metadata["radius"] = 1;
	metadata["rule"] = cellularAutomataRulePresetText(_settings.rulePreset);
	metadata["statePreset"] = cellularAutomataStatePresetText(_settings.statePreset);
	root["metadata"] = metadata;
	root["cells"] = encodeGrid(_demo.get());

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		if (errorMessage != nullptr) {
			*errorMessage = describeFileError(file, QObject::tr("save"));
		}
		return false;
	}

	const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
	if (file.write(json) != json.size()) {
		if (errorMessage != nullptr) {
			*errorMessage = describeFileError(file, QObject::tr("write"));
		}
		return false;
	}
	return true;
}

bool CellularAutomataViewerController::loadConfiguration(const QString& filePath, QString* errorMessage) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		if (errorMessage != nullptr) {
			*errorMessage = describeFileError(file, QObject::tr("open"));
		}
		return false;
	}

	const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
	if (!document.isObject()) {
		if (errorMessage != nullptr) {
			*errorMessage = QObject::tr("The selected file does not contain a JSON object.");
		}
		return false;
	}

	const QJsonObject root = document.object();
	if (QString::fromLatin1(kFormatId) != root.value("format").toString()) {
		if (errorMessage != nullptr) {
			*errorMessage = QObject::tr("The selected file is not a Cellular Automata configuration.");
		}
		return false;
	}

	const QJsonObject settingsObject = root.value("settings").toObject();
	CellularAutomataDemoSettings loadedSettings = _settings;
	loadedSettings.latticeSize = QSize(
	    std::max(8, settingsObject.value("latticeWidth").toInt(root.value("columns").toInt(0))),
	    std::max(8, settingsObject.value("latticeHeight").toInt(root.value("rows").toInt(0))));
	loadedSettings.rulePreset = rulePresetFromInt(settingsObject.value("rulePreset").toInt(presetValue(_settings.rulePreset)));
	loadedSettings.neighborhoodPreset =
	    neighborhoodPresetFromInt(settingsObject.value("neighborhoodPreset").toInt(presetValue(_settings.neighborhoodPreset)));
	loadedSettings.statePreset = statePresetFromInt(settingsObject.value("statePreset").toInt(presetValue(_settings.statePreset)));
	if (loadedSettings.rulePreset == CellularAutomataRulePreset::ForestFire) {
		loadedSettings.statePreset = CellularAutomataStatePreset::Enumerated;
	}

	const QJsonArray rowsArray = root.value("cells").toArray();
	if (rowsArray.isEmpty()) {
		if (errorMessage != nullptr) {
			*errorMessage = QObject::tr("The configuration does not contain any cell data.");
		}
		return false;
	}

	std::vector<std::vector<long>> loadedCells;
	loadedCells.reserve(static_cast<size_t>(rowsArray.size()));
	int loadedWidth = -1;
	for (const QJsonValue& rowValue : rowsArray) {
		const QJsonArray row = rowValue.toArray();
		if (row.isEmpty()) {
			if (errorMessage != nullptr) {
				*errorMessage = QObject::tr("The configuration does not contain a valid cell matrix.");
			}
			return false;
		}
		if (loadedWidth < 0) {
			loadedWidth = row.size();
		} else if (row.size() != loadedWidth) {
			if (errorMessage != nullptr) {
				*errorMessage = QObject::tr("The stored cell matrix is not rectangular.");
			}
			return false;
		}
		std::vector<long> rowValues;
		rowValues.reserve(static_cast<size_t>(row.size()));
		for (const QJsonValue& cellValue : row) {
			rowValues.push_back(static_cast<long>(cellValue.toInt(0)));
		}
		loadedCells.push_back(std::move(rowValues));
	}

	loadedSettings.latticeSize = QSize(loadedWidth, static_cast<int>(loadedCells.size()));

	const unsigned int loadedTime = static_cast<unsigned int>(std::max(0, root.value("time").toInt(0)));
	const bool hasSelectedPaintState = root.contains("selectedPaintState") && root.value("selectedPaintState").isDouble();
	const long loadedSelectedPaintState = hasSelectedPaintState ? static_cast<long>(root.value("selectedPaintState").toInt(0)) : 0;
	const bool wasRunning = _running;
	pause();

	rebuildDemo(loadedSettings);
	_simulatedTime = loadedTime;

	for (int y = 0; y < loadedSettings.latticeSize.height(); ++y) {
		for (int x = 0; x < loadedSettings.latticeSize.width(); ++x) {
			_demo->setStateAt(x, y, loadedCells.at(static_cast<size_t>(y)).at(static_cast<size_t>(x)));
		}
	}

	const std::vector<long> paintStates = availablePaintStates();
	if (hasSelectedPaintState &&
	    std::find(paintStates.begin(), paintStates.end(), loadedSelectedPaintState) != paintStates.end()) {
		setSelectedPaintState(loadedSelectedPaintState);
	} else if (!paintStates.empty()) {
		setSelectedPaintState(paintStates.front());
	} else {
		clearSelectedPaintState();
	}

	emit timeChanged(_simulatedTime);
	emit modelChanged();
	if (wasRunning) {
		start();
	}
	return true;
}

void CellularAutomataViewerController::_onTimerTimeout() {
	step();
}

void CellularAutomataViewerController::_resetRuntimeState() {
	_timer.stop();
	_running = false;
	_simulatedTime = 0;
}
