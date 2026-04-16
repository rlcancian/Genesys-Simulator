#ifndef OPTIMIZERWINDOW_H
#define OPTIMIZERWINDOW_H

#include "tools/OptimizerDefaultImpl1.h"

#include <QList>
#include <QMainWindow>
#include <QString>
#include <QVector>

#include <vector>

class QAction;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;
class QTabWidget;
class QTextBrowser;
class Simulator;
class SimulationControl;
class SimulationResponse;

class OptimizerWindow : public QMainWindow {
public:
    explicit OptimizerWindow(Simulator* simulator, QWidget* parent = nullptr);

private:
    class ProgressPlot;

    void buildMenus();
    void buildWorkspace();
    void connectActions();
    void refreshFromCurrentModel();
    void refreshControlsTable();
    void refreshResponsesTable();
    void refreshReport();
    void applyConfigurationToBackend();
    void refreshRunState();
    void addObjective();
    void removeSelectedObjective();
    void addConstraint();
    void removeSelectedConstraint();
    void configureTechnique();
    void checkReadiness();
    void startOptimization();
    void stepOptimization();
    void pauseOptimization();
    void resumeOptimization();
    void stopOptimization();
    void showSkeletonMessage(const QString& featureName);

    static QString formatNumber(double value);
    static QString stateText(Optimizer_if::ExecutionState state);
    static QString techniqueDescription(const QString& technique);
    static bool stringIsNumeric(const QString& value);
    static QTableWidgetItem* readOnlyItem(const QString& text);
    static QTableWidgetItem* editableItem(const QString& text);
    static QTableWidgetItem* checkableItem(bool checked, bool enabled = true);

private:
    Simulator* _simulator = nullptr;
    OptimizerDefaultImpl1 _optimizer;
    std::vector<SimulationControl*> _controls;
    std::vector<SimulationResponse*> _responses;
    QVector<double> _progressValues;

    QAction* _refreshModelAction = nullptr;
    QAction* _configureTechniqueAction = nullptr;
    QAction* _checkAction = nullptr;
    QAction* _startAction = nullptr;
    QAction* _stepAction = nullptr;
    QAction* _pauseAction = nullptr;
    QAction* _resumeAction = nullptr;
    QAction* _stopAction = nullptr;

    QTabWidget* _tabs = nullptr;
    QLabel* _modelStatusLabel = nullptr;
    QLabel* _runStatusLabel = nullptr;
    QTableWidget* _controlsTable = nullptr;
    QTableWidget* _responsesTable = nullptr;
    QTableWidget* _objectivesTable = nullptr;
    QTableWidget* _constraintsTable = nullptr;
    QTableWidget* _bestSolutionsTable = nullptr;
    QComboBox* _techniqueCombo = nullptr;
    QSpinBox* _maxIterationsSpin = nullptr;
    QSpinBox* _maxSimulationsSpin = nullptr;
    QSpinBox* _replicationsSpin = nullptr;
    QSpinBox* _bestSolutionsSpin = nullptr;
    QSpinBox* _randomSeedSpin = nullptr;
    QSpinBox* _populationSizeSpin = nullptr;
    QDoubleSpinBox* _mutationRateSpin = nullptr;
    QDoubleSpinBox* _crossoverRateSpin = nullptr;
    QDoubleSpinBox* _improvementToleranceSpin = nullptr;
    QDoubleSpinBox* _timeLimitSpin = nullptr;
    QPlainTextEdit* _techniqueNotes = nullptr;
    QTextBrowser* _overviewText = nullptr;
    QTextBrowser* _reportText = nullptr;
    ProgressPlot* _progressPlot = nullptr;
};

#endif // OPTIMIZERWINDOW_H
