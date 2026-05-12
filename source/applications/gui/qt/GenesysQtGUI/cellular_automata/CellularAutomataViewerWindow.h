#pragma once

#include <QMainWindow>
#include <QString>

class QAction;
class QComboBox;
class QLabel;
class CellularAutomataLatticeWidget;
class CellularAutomataViewerController;

class CellularAutomataViewerWindow : public QMainWindow {
public:
	explicit CellularAutomataViewerWindow(QWidget* parent = nullptr);

private:
	void _updateTimeLabel(unsigned int time);
	void _updateRunningState(bool running);
	void _updatePaintStateLabel();
	void _updateConfigurationLabel();
	void _updateStatusMessage(const QString& message);
	void _refreshUi();
	void _syncControlsFromController();
	void _onRulePresetChanged(int index);
	void _onNeighborhoodPresetChanged(int index);
	void _onStatePresetChanged(int index);
	bool _promptSavePath(QString* path) const;
	bool _promptLoadPath(QString* path) const;

private:
	CellularAutomataViewerController* _controller = nullptr;
	CellularAutomataLatticeWidget* _latticeWidget = nullptr;
	QComboBox* _ruleCombo = nullptr;
	QComboBox* _neighborhoodCombo = nullptr;
	QComboBox* _stateCombo = nullptr;
	QLabel* _timeLabel = nullptr;
	QLabel* _runningLabel = nullptr;
	QLabel* _paintStateLabel = nullptr;
	QLabel* _configurationLabel = nullptr;
	QLabel* _statusLabel = nullptr;
	QAction* _startAction = nullptr;
	QAction* _stepAction = nullptr;
	QAction* _continueAction = nullptr;
	QAction* _pauseAction = nullptr;
	QAction* _stopAction = nullptr;
	QAction* _restartAction = nullptr;
	QAction* _saveAction = nullptr;
	QAction* _loadAction = nullptr;
};
