#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QSize>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QObject>

#include "CellularAutomataDemoBuilder.h"

class CellularAutomataViewerController : public QObject {
	Q_OBJECT
public:
	explicit CellularAutomataViewerController(QObject* parent = nullptr);

	CellularAutomataDemoModel* model() const;
	CellularAutomataDemoSettings settings() const;
	QSize latticeSize() const;
	bool hasDemo() const;
	unsigned int simulatedTime() const;
	bool isRunning() const;
	QString configurationSummary() const;
	std::vector<long> availablePaintStates() const;

	long cellState(int x, int y) const;
	bool setCellState(int x, int y, long value);
	void fill(long value);

	bool hasSelectedPaintState() const;
	std::optional<long> selectedPaintState() const;
	QString selectedPaintStateText() const;
	void setSelectedPaintState(long state);
	void clearSelectedPaintState();

	int autoStepIntervalMs() const;
	void setAutoStepIntervalMs(int intervalMs);

	void rebuildDemo(CellularAutomataDemoSettings settings = {});
	void setSettings(CellularAutomataDemoSettings settings);
	void setRulePreset(CellularAutomataRulePreset preset);
	void setNeighborhoodPreset(CellularAutomataNeighborhoodPreset preset);
	void setBoundaryPreset(CellularAutomataBoundaryPreset preset);
	void setStatePreset(CellularAutomataStatePreset preset);
	void resetToInitialState();
	void step();
	void start();
	void pause();
	void stop();

	bool saveConfiguration(const QString& filePath, QString* errorMessage = nullptr) const;
	bool loadConfiguration(const QString& filePath, QString* errorMessage = nullptr);

signals:
	void modelChanged();
	void settingsChanged();
	void timeChanged(unsigned int time);
	void runningChanged(bool running);
	void paintStateChanged();
	void statusMessage(const QString& message);

private slots:
	void _onTimerTimeout();

private:
	void _resetRuntimeState();

private:
	std::unique_ptr<CellularAutomataDemoModel> _demo;
	CellularAutomataDemoSettings _settings;
	QTimer _timer;
	unsigned int _simulatedTime = 0;
	bool _running = false;
	int _autoStepIntervalMs = 0;
	std::optional<long> _selectedPaintState;
};
