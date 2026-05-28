/*
 * File:   AIAssistantWindow.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "AIAssistantWindow.h"

#include "kernel/simulator/Simulator.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QScrollBar>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

AIAssistantWindow::AIAssistantWindow(Simulator* simulator, QWidget* parent)
    : QMainWindow(parent)
    , _simulator(simulator)
    , _facade(simulator)
{
    setWindowTitle(tr("AI Assistant"));
    resize(860, 680);
    _assistant.setSimulatorFacade(&_facade);

    buildMenus();
    buildWorkspace();
    connectActions();
    setStatus(tr("Ready. Configure provider and permissions, then enter a prompt."));
}

AIAssistantWindow::~AIAssistantWindow() = default;

// ---------------------------------------------------------------------------
// buildMenus
// ---------------------------------------------------------------------------

void AIAssistantWindow::buildMenus() {
    auto* toolsMenu = menuBar()->addMenu(tr("Actions"));

    _applyConfigAction = toolsMenu->addAction(tr("Apply Configuration"));
    _applyConfigAction->setToolTip(tr("Apply current configuration to the AI assistant backend"));

    toolsMenu->addSeparator();
    _clearLogAction = toolsMenu->addAction(tr("Clear Execution Log"));
    _clearLogAction->setToolTip(tr("Clear the execution log panel"));

    auto* toolbar = addToolBar(tr("AI Assistant"));
    toolbar->setMovable(false);
    toolbar->addAction(_applyConfigAction);
    toolbar->addSeparator();
    toolbar->addAction(_clearLogAction);
}

// ---------------------------------------------------------------------------
// buildWorkspace
// ---------------------------------------------------------------------------

void AIAssistantWindow::buildWorkspace() {
    _tabs = new QTabWidget(this);
    _tabs->addTab(buildConfigTab(),    tr("Configuration"));
    _tabs->addTab(buildPromptTab(),    tr("Prompt && Plan"));
    _tabs->addTab(buildExecutionTab(), tr("Execution && Results"));
    setCentralWidget(_tabs);

    _statusLabel = new QLabel(this);
    _statusLabel->setContentsMargins(6, 0, 6, 0);
    statusBar()->addWidget(_statusLabel, 1);
}

// ---------------------------------------------------------------------------
// buildConfigTab
// ---------------------------------------------------------------------------

QWidget* AIAssistantWindow::buildConfigTab() {
    auto* tab = new QWidget;
    auto* outer = new QVBoxLayout(tab);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(12);

    // -- Provider group --
    auto* providerGroup = new QGroupBox(tr("Provider"), tab);
    auto* providerForm  = new QFormLayout(providerGroup);
    providerForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    _providerCombo = new QComboBox(tab);
    _providerCombo->addItem(tr("OpenAI"),    static_cast<int>(AIProvider::OpenAI));
    _providerCombo->addItem(tr("Anthropic"), static_cast<int>(AIProvider::Anthropic));
    _providerCombo->addItem(tr("Local (Ollama / LM Studio)"), static_cast<int>(AIProvider::Local));
    _providerCombo->addItem(tr("Custom HTTP"), static_cast<int>(AIProvider::CustomHttp));
    providerForm->addRow(tr("Provider:"), _providerCombo);

    _modelNameEdit = new QLineEdit(tab);
    _modelNameEdit->setPlaceholderText(tr("e.g. gpt-4o, claude-sonnet-4-6, llama3"));
    providerForm->addRow(tr("Model name:"), _modelNameEdit);

    auto* keyRow = new QHBoxLayout;
    _apiKeyEdit = new QLineEdit(tab);
    _apiKeyEdit->setEchoMode(QLineEdit::Password);
    _apiKeyEdit->setPlaceholderText(tr("Paste API key here (kept in memory only)"));
    _apiKeyToggleBtn = new QPushButton(tr("Show"), tab);
    _apiKeyToggleBtn->setFixedWidth(52);
    keyRow->addWidget(_apiKeyEdit);
    keyRow->addWidget(_apiKeyToggleBtn);
    providerForm->addRow(tr("API Key:"), keyRow);

    _apiKeyEnvVarEdit = new QLineEdit(tab);
    _apiKeyEnvVarEdit->setPlaceholderText(tr("e.g. OPENAI_API_KEY  (name, not value)"));
    providerForm->addRow(tr("API Key env-var:"), _apiKeyEnvVarEdit);

    _baseUrlEdit = new QLineEdit(tab);
    _baseUrlEdit->setPlaceholderText(tr("Override base URL (leave empty for provider default)"));
    providerForm->addRow(tr("Base URL:"), _baseUrlEdit);

    _timeoutSpin = new QSpinBox(tab);
    _timeoutSpin->setRange(10, 600);
    _timeoutSpin->setValue(120);
    _timeoutSpin->setSuffix(tr(" s"));
    providerForm->addRow(tr("Timeout:"), _timeoutSpin);

    _temperatureSpin = new QDoubleSpinBox(tab);
    _temperatureSpin->setRange(0.0, 2.0);
    _temperatureSpin->setSingleStep(0.05);
    _temperatureSpin->setDecimals(2);
    _temperatureSpin->setValue(0.2);
    providerForm->addRow(tr("Temperature:"), _temperatureSpin);

    outer->addWidget(providerGroup);

    // -- Permissions group --
    auto* permGroup = new QGroupBox(tr("Permissions  (deny-by-default — enable explicitly)"), tab);
    auto* permLayout = new QVBoxLayout(permGroup);

    _allowNetworkChk    = new QCheckBox(tr("Allow outbound network calls to the AI provider"), tab);
    _allowMutationChk   = new QCheckBox(tr("Allow model creation / mutation"), tab);
    _allowExecutionChk  = new QCheckBox(tr("Allow simulation execution"), tab);
    _allowFilesystemChk = new QCheckBox(tr("Allow filesystem writes (temp model file)"), tab);

    permLayout->addWidget(_allowNetworkChk);
    permLayout->addWidget(_allowMutationChk);
    permLayout->addWidget(_allowExecutionChk);
    permLayout->addWidget(_allowFilesystemChk);

    outer->addWidget(permGroup);

    auto* applyBtn = new QPushButton(tr("Apply Configuration"), tab);
    applyBtn->setToolTip(tr("Push these settings to the assistant backend"));
    outer->addWidget(applyBtn, 0, Qt::AlignLeft);
    connect(applyBtn, &QPushButton::clicked, this, &AIAssistantWindow::applyConfigurationToBackend);

    outer->addStretch(1);
    return tab;
}

// ---------------------------------------------------------------------------
// buildPromptTab
// ---------------------------------------------------------------------------

QWidget* AIAssistantWindow::buildPromptTab() {
    auto* tab    = new QWidget;
    auto* outer  = new QVBoxLayout(tab);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    // -- Input group --
    auto* inputGroup  = new QGroupBox(tr("Input"), tab);
    auto* inputLayout = new QFormLayout(inputGroup);
    inputLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    _modelDescEdit = new QPlainTextEdit(tab);
    _modelDescEdit->setPlaceholderText(tr("Describe the system to be modelled (e.g. single-server bank queue, factory floor, etc.)"));
    _modelDescEdit->setFixedHeight(80);
    inputLayout->addRow(tr("System description:"), _modelDescEdit);

    _experimentDescEdit = new QPlainTextEdit(tab);
    _experimentDescEdit->setPlaceholderText(tr("Describe experiment parameters (e.g. run 10 replications of 480 minutes with 60-minute warm-up)"));
    _experimentDescEdit->setFixedHeight(60);
    inputLayout->addRow(tr("Experiment description:"), _experimentDescEdit);

    _userPromptEdit = new QPlainTextEdit(tab);
    _userPromptEdit->setPlaceholderText(tr("Additional freeform instructions for the AI (optional)"));
    _userPromptEdit->setFixedHeight(60);
    inputLayout->addRow(tr("Extra prompt:"), _userPromptEdit);

    outer->addWidget(inputGroup);

    // -- Action buttons --
    auto* btnRow = new QHBoxLayout;
    _analyzeBtn  = new QPushButton(tr("Analyze Prompt"), tab);
    _planBtn     = new QPushButton(tr("Create Model Plan"), tab);
    _analyzeBtn->setToolTip(tr("Send the description to the AI and get a preliminary analysis"));
    _planBtn->setToolTip(tr("Ask the AI to produce a structured simulation model plan"));
    btnRow->addWidget(_analyzeBtn);
    btnRow->addWidget(_planBtn);
    btnRow->addStretch(1);
    outer->addLayout(btnRow);

    // -- Plan output --
    auto* planGroup  = new QGroupBox(tr("AI Response / Plan"), tab);
    auto* planLayout = new QVBoxLayout(planGroup);
    _planOutput = new QTextBrowser(tab);
    _planOutput->setOpenExternalLinks(false);
    planLayout->addWidget(_planOutput);
    outer->addWidget(planGroup, 1);

    return tab;
}

// ---------------------------------------------------------------------------
// buildExecutionTab
// ---------------------------------------------------------------------------

QWidget* AIAssistantWindow::buildExecutionTab() {
    auto* tab   = new QWidget;
    auto* outer = new QVBoxLayout(tab);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    // -- Step-by-step buttons --
    auto* stepsGroup  = new QGroupBox(tr("Step-by-Step Execution"), tab);
    auto* stepsLayout = new QHBoxLayout(stepsGroup);

    _buildModelBtn     = new QPushButton(tr("1. Build Model"), tab);
    _configureSimBtn   = new QPushButton(tr("2. Configure Simulation"), tab);
    _runSimBtn         = new QPushButton(tr("3. Run Simulation"), tab);
    _collectResultsBtn = new QPushButton(tr("4. Collect Results"), tab);

    _buildModelBtn->setToolTip(tr("Ask the AI to generate a .gen model file and load it"));
    _configureSimBtn->setToolTip(tr("Extract simulation parameters from the experiment description"));
    _runSimBtn->setToolTip(tr("Start simulation replications (synchronous)"));
    _collectResultsBtn->setToolTip(tr("Collect and display simulation results"));

    stepsLayout->addWidget(_buildModelBtn);
    stepsLayout->addWidget(_configureSimBtn);
    stepsLayout->addWidget(_runSimBtn);
    stepsLayout->addWidget(_collectResultsBtn);
    stepsLayout->addStretch(1);
    outer->addWidget(stepsGroup);

    // -- Full pipeline button --
    _fullPipelineBtn = new QPushButton(tr("Run Full AI Pipeline"), tab);
    _fullPipelineBtn->setToolTip(tr("Execute all steps: analyze → plan → build → configure → run → collect"));
    _fullPipelineBtn->setMinimumHeight(36);
    QFont boldFont = _fullPipelineBtn->font();
    boldFont.setBold(true);
    _fullPipelineBtn->setFont(boldFont);
    outer->addWidget(_fullPipelineBtn);

    // -- Execution log --
    auto* logGroup  = new QGroupBox(tr("Execution Log"), tab);
    auto* logLayout = new QVBoxLayout(logGroup);
    _executionLog = new QTextBrowser(tab);
    _executionLog->setOpenExternalLinks(false);
    logLayout->addWidget(_executionLog);
    outer->addWidget(logGroup, 1);

    return tab;
}

// ---------------------------------------------------------------------------
// connectActions
// ---------------------------------------------------------------------------

void AIAssistantWindow::connectActions() {
    connect(_applyConfigAction, &QAction::triggered,
            this, &AIAssistantWindow::applyConfigurationToBackend);
    connect(_clearLogAction, &QAction::triggered,
            this, [this]() { _executionLog->clear(); });

    connect(_apiKeyToggleBtn, &QPushButton::clicked, this, [this]() {
        if (_apiKeyEdit->echoMode() == QLineEdit::Password) {
            _apiKeyEdit->setEchoMode(QLineEdit::Normal);
            _apiKeyToggleBtn->setText(tr("Hide"));
        } else {
            _apiKeyEdit->setEchoMode(QLineEdit::Password);
            _apiKeyToggleBtn->setText(tr("Show"));
        }
    });

    connect(_analyzeBtn,         &QPushButton::clicked, this, &AIAssistantWindow::runAnalyzePrompt);
    connect(_planBtn,            &QPushButton::clicked, this, &AIAssistantWindow::runCreateModelPlan);
    connect(_buildModelBtn,      &QPushButton::clicked, this, &AIAssistantWindow::runBuildModel);
    connect(_configureSimBtn,    &QPushButton::clicked, this, &AIAssistantWindow::runConfigureSimulation);
    connect(_runSimBtn,          &QPushButton::clicked, this, &AIAssistantWindow::runSimulation);
    connect(_collectResultsBtn,  &QPushButton::clicked, this, &AIAssistantWindow::runCollectResults);
    connect(_fullPipelineBtn,    &QPushButton::clicked, this, &AIAssistantWindow::runFullPipeline);
}

// ---------------------------------------------------------------------------
// Configuration helpers
// ---------------------------------------------------------------------------

void AIAssistantWindow::applyConfigurationToBackend() {
    AIAssistantConfiguration cfg;

    const int idx = _providerCombo->currentIndex();
    cfg.provider = static_cast<AIProvider>(_providerCombo->itemData(idx).toInt());

    cfg.modelName                = _modelNameEdit->text().toStdString();
    cfg.apiKeyEnvironmentVariable = _apiKeyEnvVarEdit->text().toStdString();
    cfg.baseUrl                  = _baseUrlEdit->text().toStdString();
    cfg.temperature              = _temperatureSpin->value();
    cfg.timeoutSeconds           = static_cast<unsigned int>(_timeoutSpin->value());

    cfg.allowNetworkAccess       = _allowNetworkChk->isChecked();
    cfg.allowModelMutation       = _allowMutationChk->isChecked();
    cfg.allowSimulationExecution = _allowExecutionChk->isChecked();
    cfg.allowFileSystemWrites    = _allowFilesystemChk->isChecked();

    _assistant.setConfiguration(cfg);

    const QString rawKey = _apiKeyEdit->text().trimmed();
    if (!rawKey.isEmpty()) {
        _assistant.setApiKey(rawKey.toStdString());
    } else if (!cfg.apiKeyEnvironmentVariable.empty()) {
        _assistant.loadApiKeyFromEnvironment();
    }

    _assistant.createAndAttachProviderClient();
    _assistant.refreshKnowledgeBaseFromSimulator();

    const auto kbSummary = _assistant.getKnowledgeBaseSummary();
    appendLog(tr("Configuration applied. Provider: %1 | Model: %2 | Key: %3 | KB items: %4")
              .arg(QString::fromStdString(cfg.baseUrl.empty()
                       ? _providerCombo->currentText().toStdString()
                       : cfg.baseUrl))
              .arg(QString::fromStdString(cfg.modelName))
              .arg(QString::fromStdString(_assistant.getMaskedApiKey()))
              .arg(kbSummary.numberOfPlugins));
    setStatus(tr("Configuration applied."));
}

AIAssistantRequest AIAssistantWindow::buildRequest() const {
    AIAssistantRequest req;
    req.modelDescription      = _modelDescEdit->toPlainText().toStdString();
    req.experimentDescription = _experimentDescEdit->toPlainText().toStdString();
    req.userPrompt            = _userPromptEdit->toPlainText().toStdString();
    return req;
}

// ---------------------------------------------------------------------------
// Workflow actions
// ---------------------------------------------------------------------------

void AIAssistantWindow::runAnalyzePrompt() {
    setRunning(true);
    _tabs->setCurrentIndex(1);
    appendLog(tr("--- Analyzing prompt ---"));
    const auto resp = _assistant.analyzePrompt(buildRequest());
    if (resp.success) {
        _planOutput->setPlainText(QString::fromStdString(resp.message));
        appendLog(tr("Prompt analysis complete."));
        setStatus(tr("Analysis complete."));
    } else {
        _planOutput->setPlainText(QString::fromStdString(resp.message));
        appendLog(tr("Prompt analysis failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Analysis failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

void AIAssistantWindow::runCreateModelPlan() {
    setRunning(true);
    _tabs->setCurrentIndex(1);
    appendLog(tr("--- Creating model plan ---"));
    const auto resp = _assistant.createModelPlan(buildRequest());
    if (resp.success) {
        _planOutput->setPlainText(QString::fromStdString(resp.generatedPlan));
        appendLog(tr("Model plan created successfully."));
        setStatus(tr("Plan created."));
    } else {
        _planOutput->setPlainText(QString::fromStdString(resp.message));
        appendLog(tr("Plan creation failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Plan creation failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

void AIAssistantWindow::runBuildModel() {
    setRunning(true);
    _tabs->setCurrentIndex(2);
    appendLog(tr("--- Building model ---"));
    AIAssistantRequest req = buildRequest();
    req.buildModel = true;
    req.modelPlan  = _planOutput->toPlainText().toStdString();
    const auto resp = _assistant.buildModel(req);
    if (resp.success) {
        appendLog(tr("Model built successfully."));
        if (!resp.generatedModelLanguage.empty()) {
            appendLog(tr("[.gen file generated — %1 characters]")
                      .arg(static_cast<int>(resp.generatedModelLanguage.size())));
        }
        setStatus(tr("Model built."));
    } else {
        appendLog(tr("Build model failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Build failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

void AIAssistantWindow::runConfigureSimulation() {
    setRunning(true);
    _tabs->setCurrentIndex(2);
    appendLog(tr("--- Configuring simulation ---"));
    AIAssistantRequest req = buildRequest();
    req.configureSimulation = true;
    const auto resp = _assistant.configureSimulation(req);
    if (resp.success) {
        appendLog(tr("Simulation configured: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Simulation configured."));
    } else {
        appendLog(tr("Configure simulation failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Configuration failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

void AIAssistantWindow::runSimulation() {
    setRunning(true);
    _tabs->setCurrentIndex(2);
    appendLog(tr("--- Running simulation ---"));
    AIAssistantRequest req = buildRequest();
    req.runSimulation = true;
    const auto resp = _assistant.runSimulation(req);
    if (resp.success) {
        appendLog(tr("Simulation complete: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Simulation complete."));
    } else {
        appendLog(tr("Simulation failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Simulation failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

void AIAssistantWindow::runCollectResults() {
    setRunning(true);
    _tabs->setCurrentIndex(2);
    appendLog(tr("--- Collecting results ---"));
    AIAssistantRequest req = buildRequest();
    req.collectResults = true;
    const auto resp = _assistant.collectResults(req);
    if (resp.success) {
        _executionLog->append(QString::fromStdString(resp.message));
        appendLog(tr("Results collected."));
        setStatus(tr("Results ready."));
    } else {
        appendLog(tr("Collect results failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Results collection failed."));
    }
    setRunning(false);
}

void AIAssistantWindow::runFullPipeline() {
    setRunning(true);
    _tabs->setCurrentIndex(2);
    appendLog(tr("=== Starting full AI pipeline ==="));

    AIAssistantRequest req = buildRequest();
    req.buildModel          = true;
    req.configureSimulation = true;
    req.runSimulation       = true;
    req.collectResults      = true;

    const auto resp = _assistant.execute(req);
    if (resp.success) {
        if (!resp.message.empty()) {
            _executionLog->append(QString::fromStdString(resp.message));
        }
        appendLog(tr("=== Pipeline completed successfully ==="));
        setStatus(tr("Pipeline complete."));
    } else {
        appendLog(tr("Pipeline failed: %1").arg(QString::fromStdString(resp.message)));
        setStatus(tr("Pipeline failed."));
    }
    if (!resp.diagnostics.empty()) {
        appendLog(tr("[diag] %1").arg(QString::fromStdString(resp.diagnostics)));
    }
    setRunning(false);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void AIAssistantWindow::appendLog(const QString& text) {
    _executionLog->append(text);
    _executionLog->verticalScrollBar()->setValue(
        _executionLog->verticalScrollBar()->maximum());
}

void AIAssistantWindow::setStatus(const QString& text) {
    if (_statusLabel != nullptr) {
        _statusLabel->setText(text);
    }
}

void AIAssistantWindow::setRunning(bool running) {
    const bool enabled = !running;
    _analyzeBtn->setEnabled(enabled);
    _planBtn->setEnabled(enabled);
    _buildModelBtn->setEnabled(enabled);
    _configureSimBtn->setEnabled(enabled);
    _runSimBtn->setEnabled(enabled);
    _collectResultsBtn->setEnabled(enabled);
    _fullPipelineBtn->setEnabled(enabled);
    QApplication::processEvents();
}
