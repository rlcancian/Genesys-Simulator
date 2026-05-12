#include "CellularAutomataViewerWindow.h"

#include "CellularAutomataDemoBuilder.h"
#include "CellularAutomataLatticeWidget.h"
#include "CellularAutomataViewerController.h"

#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <initializer_list>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <utility>

namespace {
template <typename EnumType>
void populatePresetCombo(QComboBox* combo,
                         const std::initializer_list<std::pair<EnumType, QString>>& entries,
                         EnumType currentValue) {
	if (combo == nullptr) {
		return;
	}
	combo->clear();
	for (const auto& entry : entries) {
		combo->addItem(entry.second, static_cast<int>(entry.first));
	}
	const int index = combo->findData(static_cast<int>(currentValue));
	if (index >= 0) {
		combo->setCurrentIndex(index);
	}
}
} // namespace

CellularAutomataViewerWindow::CellularAutomataViewerWindow(QWidget* parent)
	: QMainWindow(parent),
	  _controller(new CellularAutomataViewerController(this)) {
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("Cellular Automata"));
	resize(1120, 760);

	auto* central = new QWidget(this);
	auto* layout = new QVBoxLayout(central);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	auto* configurationBar = new QToolBar(tr("Cellular Automata Configuration"), central);
	configurationBar->setObjectName(QStringLiteral("cellularAutomataConfigurationBar"));
	configurationBar->setMovable(false);
	configurationBar->setFloatable(false);

	auto* configurationContainer = new QWidget(configurationBar);
	auto* configurationLayout = new QHBoxLayout(configurationContainer);
	configurationLayout->setContentsMargins(4, 2, 4, 2);
	configurationLayout->setSpacing(8);

	auto* ruleLabel = new QLabel(tr("Rule:"), configurationContainer);
	_ruleCombo = new QComboBox(configurationContainer);
	_ruleCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_ruleCombo->setMinimumWidth(150);
	configurationLayout->addWidget(ruleLabel);
	configurationLayout->addWidget(_ruleCombo);

	auto* neighborhoodLabel = new QLabel(tr("Neighborhood:"), configurationContainer);
	_neighborhoodCombo = new QComboBox(configurationContainer);
	_neighborhoodCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_neighborhoodCombo->setMinimumWidth(150);
	configurationLayout->addWidget(neighborhoodLabel);
	configurationLayout->addWidget(_neighborhoodCombo);

	auto* stateLabel = new QLabel(tr("State type:"), configurationContainer);
	_stateCombo = new QComboBox(configurationContainer);
	_stateCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	_stateCombo->setMinimumWidth(150);
	configurationLayout->addWidget(stateLabel);
	configurationLayout->addWidget(_stateCombo);
	configurationLayout->addStretch(1);

	configurationBar->addWidget(configurationContainer);
	layout->addWidget(configurationBar);

	_latticeWidget = new CellularAutomataLatticeWidget(central);
	_latticeWidget->setController(_controller);
	layout->addWidget(_latticeWidget, 1);
	setCentralWidget(central);

	auto* toolbar = addToolBar(tr("Cellular Automata Controls"));
	toolbar->setObjectName(QStringLiteral("cellularAutomataToolbar"));
	toolbar->setMovable(false);

	_startAction = toolbar->addAction(tr("Start"));
	_stepAction = toolbar->addAction(tr("Step"));
	_continueAction = toolbar->addAction(tr("Continue"));
	_pauseAction = toolbar->addAction(tr("Pause"));
	_stopAction = toolbar->addAction(tr("Stop"));
	_restartAction = toolbar->addAction(tr("Restart"));
	toolbar->addSeparator();
	_saveAction = toolbar->addAction(tr("Save Config"));
	_loadAction = toolbar->addAction(tr("Load Config"));

	_timeLabel = new QLabel(this);
	_runningLabel = new QLabel(this);
	_paintStateLabel = new QLabel(this);
	_configurationLabel = new QLabel(this);
	_statusLabel = new QLabel(this);
	_timeLabel->setMinimumWidth(120);
	_runningLabel->setMinimumWidth(120);
	_paintStateLabel->setMinimumWidth(160);
	_configurationLabel->setMinimumWidth(360);
	_statusLabel->setMinimumWidth(220);
	statusBar()->addPermanentWidget(_statusLabel, 1);
	statusBar()->addPermanentWidget(_configurationLabel, 1);
	statusBar()->addPermanentWidget(_timeLabel);
	statusBar()->addPermanentWidget(_runningLabel);
	statusBar()->addPermanentWidget(_paintStateLabel);

	connect(_startAction, &QAction::triggered, this, [this]() { _controller->start(); });
	connect(_continueAction, &QAction::triggered, this, [this]() { _controller->start(); });
	connect(_stepAction, &QAction::triggered, this, [this]() { _controller->step(); });
	connect(_pauseAction, &QAction::triggered, this, [this]() { _controller->pause(); });
	connect(_stopAction, &QAction::triggered, this, [this]() { _controller->stop(); });
	connect(_restartAction, &QAction::triggered, this, [this]() { _controller->resetToInitialState(); });
	connect(_saveAction, &QAction::triggered, this, [this]() {
		QString filePath;
		if (!_promptSavePath(&filePath)) {
			return;
		}
		QString errorMessage;
		if (!_controller->saveConfiguration(filePath, &errorMessage)) {
			_updateStatusMessage(errorMessage);
			return;
		}
		_updateStatusMessage(tr("Saved configuration to %1").arg(filePath));
	});
	connect(_loadAction, &QAction::triggered, this, [this]() {
		QString filePath;
		if (!_promptLoadPath(&filePath)) {
			return;
		}
		QString errorMessage;
		if (!_controller->loadConfiguration(filePath, &errorMessage)) {
			_updateStatusMessage(errorMessage);
			return;
		}
		_updateStatusMessage(tr("Loaded configuration from %1").arg(filePath));
		_refreshUi();
	});

	connect(_ruleCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &CellularAutomataViewerWindow::_onRulePresetChanged);
	connect(_neighborhoodCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &CellularAutomataViewerWindow::_onNeighborhoodPresetChanged);
	connect(_stateCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &CellularAutomataViewerWindow::_onStatePresetChanged);

	connect(_controller, &CellularAutomataViewerController::timeChanged, this, &CellularAutomataViewerWindow::_updateTimeLabel);
	connect(_controller, &CellularAutomataViewerController::runningChanged, this, &CellularAutomataViewerWindow::_updateRunningState);
	connect(_controller, &CellularAutomataViewerController::paintStateChanged, this, &CellularAutomataViewerWindow::_updatePaintStateLabel);
	connect(_controller, &CellularAutomataViewerController::settingsChanged, this, &CellularAutomataViewerWindow::_syncControlsFromController);
	connect(_controller, &CellularAutomataViewerController::settingsChanged, this, &CellularAutomataViewerWindow::_updateConfigurationLabel);
	connect(_controller, &CellularAutomataViewerController::statusMessage, this, &CellularAutomataViewerWindow::_updateStatusMessage);
	connect(_controller, &CellularAutomataViewerController::modelChanged, this, [this]() {
		if (_latticeWidget != nullptr) {
			_latticeWidget->update();
		}
	});

	_syncControlsFromController();
	_updateTimeLabel(_controller->simulatedTime());
	_updateRunningState(_controller->isRunning());
	_updatePaintStateLabel();
	_updateConfigurationLabel();
	_updateStatusMessage(tr("Ready."));
}

void CellularAutomataViewerWindow::_updateTimeLabel(unsigned int time) {
	if (_timeLabel != nullptr) {
		_timeLabel->setText(tr("t = %1").arg(time));
	}
}

void CellularAutomataViewerWindow::_updateRunningState(bool running) {
	if (_runningLabel != nullptr) {
		_runningLabel->setText(running ? tr("Running") : tr("Paused"));
	}
	if (_startAction != nullptr) {
		_startAction->setEnabled(!running);
	}
	if (_continueAction != nullptr) {
		_continueAction->setEnabled(!running);
	}
	if (_pauseAction != nullptr) {
		_pauseAction->setEnabled(running);
	}
	if (_stopAction != nullptr) {
		_stopAction->setEnabled(running);
	}
}

void CellularAutomataViewerWindow::_updatePaintStateLabel() {
	if (_paintStateLabel != nullptr && _controller != nullptr) {
		_paintStateLabel->setText(tr("Paint = %1").arg(_controller->selectedPaintStateText()));
	}
}

void CellularAutomataViewerWindow::_updateConfigurationLabel() {
	if (_configurationLabel != nullptr && _controller != nullptr) {
		_configurationLabel->setText(_controller->configurationSummary());
	}
}

void CellularAutomataViewerWindow::_updateStatusMessage(const QString& message) {
	if (_statusLabel != nullptr) {
		_statusLabel->setText(message);
	}
}

void CellularAutomataViewerWindow::_syncControlsFromController() {
	if (_controller == nullptr) {
		return;
	}

	const CellularAutomataDemoSettings settings = _controller->settings();

	{
		QSignalBlocker blockRule(_ruleCombo);
		populatePresetCombo(
		    _ruleCombo,
		    {
		        {CellularAutomataRulePreset::GameOfLife, cellularAutomataRulePresetText(CellularAutomataRulePreset::GameOfLife)},
		        {CellularAutomataRulePreset::Growty, cellularAutomataRulePresetText(CellularAutomataRulePreset::Growty)},
		        {CellularAutomataRulePreset::Identity, cellularAutomataRulePresetText(CellularAutomataRulePreset::Identity)},
		        {CellularAutomataRulePreset::ForestFire, cellularAutomataRulePresetText(CellularAutomataRulePreset::ForestFire)},
		    },
		    settings.rulePreset);
	}

	{
		QSignalBlocker blockNeighborhood(_neighborhoodCombo);
		populatePresetCombo(
		    _neighborhoodCombo,
		    {
		        {CellularAutomataNeighborhoodPreset::Moore, cellularAutomataNeighborhoodPresetText(CellularAutomataNeighborhoodPreset::Moore)},
		        {CellularAutomataNeighborhoodPreset::VonNeumann,
		         cellularAutomataNeighborhoodPresetText(CellularAutomataNeighborhoodPreset::VonNeumann)},
		    },
		    settings.neighborhoodPreset);
	}

	{
		QSignalBlocker blockState(_stateCombo);
		populatePresetCombo(
		    _stateCombo,
		    {
		        {CellularAutomataStatePreset::Binary, cellularAutomataStatePresetText(CellularAutomataStatePreset::Binary)},
		        {CellularAutomataStatePreset::Enumerated, cellularAutomataStatePresetText(CellularAutomataStatePreset::Enumerated)},
		        {CellularAutomataStatePreset::Numeric, cellularAutomataStatePresetText(CellularAutomataStatePreset::Numeric)},
		    },
		    settings.statePreset);
	}
}

void CellularAutomataViewerWindow::_refreshUi() {
	_syncControlsFromController();
	_updateTimeLabel(_controller->simulatedTime());
	_updateRunningState(_controller->isRunning());
	_updatePaintStateLabel();
	_updateConfigurationLabel();
	if (_latticeWidget != nullptr) {
		_latticeWidget->update();
	}
}

void CellularAutomataViewerWindow::_onRulePresetChanged(int index) {
	if (_controller == nullptr || index < 0) {
		return;
	}
	const auto preset = static_cast<CellularAutomataRulePreset>(_ruleCombo->currentData().toInt());
	_controller->setRulePreset(preset);
}

void CellularAutomataViewerWindow::_onNeighborhoodPresetChanged(int index) {
	if (_controller == nullptr || index < 0) {
		return;
	}
	const auto preset = static_cast<CellularAutomataNeighborhoodPreset>(_neighborhoodCombo->currentData().toInt());
	_controller->setNeighborhoodPreset(preset);
}

void CellularAutomataViewerWindow::_onStatePresetChanged(int index) {
	if (_controller == nullptr || index < 0) {
		return;
	}
	const auto preset = static_cast<CellularAutomataStatePreset>(_stateCombo->currentData().toInt());
	_controller->setStatePreset(preset);
}

bool CellularAutomataViewerWindow::_promptSavePath(QString* path) const {
	if (path == nullptr) {
		return false;
	}
	*path = QFileDialog::getSaveFileName(
		const_cast<CellularAutomataViewerWindow*>(this),
		tr("Save Cellular Automata Configuration"),
		QString(),
		tr("JSON files (*.json);;All files (*)"));
	return !path->isEmpty();
}

bool CellularAutomataViewerWindow::_promptLoadPath(QString* path) const {
	if (path == nullptr) {
		return false;
	}
	*path = QFileDialog::getOpenFileName(
		const_cast<CellularAutomataViewerWindow*>(this),
		tr("Load Cellular Automata Configuration"),
		QString(),
		tr("JSON files (*.json);;All files (*)"));
	return !path->isEmpty();
}
