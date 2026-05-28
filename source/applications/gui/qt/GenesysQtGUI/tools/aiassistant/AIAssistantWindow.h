#ifndef AIASSISTANTWINDOW_H
#define AIASSISTANTWINDOW_H

#include "../../../../../../tools/AIAssistantDefaultImpl.h"
#include "../../../../../../kernel/simulator/SimulatorFacade.h"

#include <QMainWindow>

class QAction;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QTextBrowser;
class Simulator;

class AIAssistantWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit AIAssistantWindow(Simulator* simulator, QWidget* parent = nullptr);
    ~AIAssistantWindow() override;

private:
    void buildMenus();
    void buildWorkspace();
    void connectActions();

    QWidget* buildConfigTab();
    QWidget* buildPromptTab();
    QWidget* buildExecutionTab();

    void applyConfigurationToBackend();
    void runAnalyzePrompt();
    void runCreateModelPlan();
    void runBuildModel();
    void runConfigureSimulation();
    void runSimulation();
    void runCollectResults();
    void runFullPipeline();
    void appendLog(const QString& text);
    void setStatus(const QString& text);
    void setRunning(bool running);
    AIAssistantRequest buildRequest() const;

private:
    Simulator*          _simulator = nullptr;
    SimulatorFacade     _facade;
    AIAssistantDefaultImpl _assistant;

    // Configuration tab
    QComboBox*        _providerCombo       = nullptr;
    QLineEdit*        _modelNameEdit       = nullptr;
    QLineEdit*        _apiKeyEdit          = nullptr;
    QPushButton*      _apiKeyToggleBtn     = nullptr;
    QLineEdit*        _apiKeyEnvVarEdit    = nullptr;
    QLineEdit*        _baseUrlEdit         = nullptr;
    QSpinBox*         _timeoutSpin         = nullptr;
    QDoubleSpinBox*   _temperatureSpin     = nullptr;
    QCheckBox*        _allowNetworkChk     = nullptr;
    QCheckBox*        _allowMutationChk    = nullptr;
    QCheckBox*        _allowExecutionChk   = nullptr;
    QCheckBox*        _allowFilesystemChk  = nullptr;

    // Prompt & Plan tab
    QPlainTextEdit*   _modelDescEdit       = nullptr;
    QPlainTextEdit*   _experimentDescEdit  = nullptr;
    QPlainTextEdit*   _userPromptEdit      = nullptr;
    QPushButton*      _analyzeBtn          = nullptr;
    QPushButton*      _planBtn             = nullptr;
    QTextBrowser*     _planOutput          = nullptr;

    // Execution tab
    QPushButton*      _buildModelBtn       = nullptr;
    QPushButton*      _configureSimBtn     = nullptr;
    QPushButton*      _runSimBtn           = nullptr;
    QPushButton*      _collectResultsBtn   = nullptr;
    QPushButton*      _fullPipelineBtn     = nullptr;
    QTextBrowser*     _executionLog        = nullptr;
    QLabel*           _statusLabel         = nullptr;

    QTabWidget*       _tabs                = nullptr;

    // Actions
    QAction*          _applyConfigAction   = nullptr;
    QAction*          _clearLogAction      = nullptr;
};

#endif // AIASSISTANTWINDOW_H
