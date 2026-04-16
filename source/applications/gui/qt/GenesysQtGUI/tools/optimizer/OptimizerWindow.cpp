#include "OptimizerWindow.h"

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Simulator.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

class OptimizerWindow::ProgressPlot : public QWidget {
public:
    explicit ProgressPlot(QWidget* parent = nullptr)
        : QWidget(parent) {
        setMinimumHeight(210);
    }

    void setValues(const QVector<double>& values) {
        _values = values;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor(250, 251, 252));

        const QRect plotRect = rect().adjusted(48, 20, -22, -36);
        painter.setPen(QPen(QColor(190, 198, 206)));
        painter.drawRect(plotRect);
        painter.setPen(QColor(69, 90, 100));
        painter.drawText(rect().adjusted(8, 0, -8, -8), Qt::AlignBottom | Qt::AlignLeft,
                         QObject::tr("Objective progress preview"));

        if (_values.isEmpty()) {
            painter.setPen(QColor(96, 112, 128));
            painter.drawText(plotRect, Qt::AlignCenter,
                             QObject::tr("Configure objectives and start the optimizer."));
            return;
        }

        auto minmax = std::minmax_element(_values.begin(), _values.end());
        const double minValue = *minmax.first;
        const double maxValue = *minmax.second;
        const double range = std::max(1e-9, maxValue - minValue);
        QPainterPath path;
        for (int i = 0; i < _values.size(); ++i) {
            const double x = _values.size() == 1 ? 0.0 : static_cast<double>(i) / (_values.size() - 1);
            const double y = (_values.at(i) - minValue) / range;
            const QPointF point(plotRect.left() + x * plotRect.width(),
                                plotRect.bottom() - y * plotRect.height());
            if (i == 0) {
                path.moveTo(point);
            } else {
                path.lineTo(point);
            }
        }
        painter.setPen(QPen(QColor(0, 105, 92), 3));
        painter.drawPath(path);
        painter.setBrush(QColor(255, 152, 0));
        painter.setPen(Qt::NoPen);
        const QPointF lastPoint = path.currentPosition();
        painter.drawEllipse(lastPoint, 5, 5);
    }

private:
    QVector<double> _values;
};

OptimizerWindow::OptimizerWindow(Simulator* simulator, QWidget* parent)
    : QMainWindow(parent),
      _simulator(simulator) {
    setWindowTitle(tr("Genesys Optimizer"));
    resize(1220, 780);
    buildMenus();
    buildWorkspace();
    connectActions();
    refreshFromCurrentModel();
    refreshRunState();
}

void OptimizerWindow::buildMenus() {
    QMenu* fileMenu = menuBar()->addMenu(tr("Arquivo"));
    _refreshModelAction = fileMenu->addAction(tr("Atualizar modelo atual"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Salvar estudo de otimizacao..."), this,
                        [this]() { showSkeletonMessage(tr("optimization study persistence")); });
    fileMenu->addAction(tr("Carregar estudo de otimizacao..."), this,
                        [this]() { showSkeletonMessage(tr("optimization study loading")); });
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Fechar"), this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu(tr("Editar"));
    editMenu->addAction(tr("Copiar selecao"), this,
                        [this]() { showSkeletonMessage(tr("copy selected optimizer table rows")); });
    editMenu->addAction(tr("Preferencias"), this,
                        [this]() { showSkeletonMessage(tr("optimizer preferences")); });

    QMenu* modelMenu = menuBar()->addMenu(tr("Modelo"));
    modelMenu->addAction(_refreshModelAction);
    modelMenu->addAction(tr("Coletar Controls e Responses"), this, [this]() { refreshFromCurrentModel(); });

    QMenu* configureMenu = menuBar()->addMenu(tr("Configurar"));
    configureMenu->addAction(tr("Adicionar objetivo..."), this, [this]() { addObjective(); });
    configureMenu->addAction(tr("Adicionar constraint..."), this, [this]() { addConstraint(); });
    _configureTechniqueAction = configureMenu->addAction(tr("Tecnica de otimizacao..."));

    QMenu* runMenu = menuBar()->addMenu(tr("Executar"));
    _checkAction = runMenu->addAction(tr("Verificar configuracao"));
    _startAction = runMenu->addAction(tr("Iniciar"));
    _stepAction = runMenu->addAction(tr("Passo"));
    _pauseAction = runMenu->addAction(tr("Pausar"));
    _resumeAction = runMenu->addAction(tr("Continuar"));
    _stopAction = runMenu->addAction(tr("Parar"));

    QMenu* helpMenu = menuBar()->addMenu(tr("Ajuda"));
    helpMenu->addAction(tr("Sobre o Optimizer"), this, [this]() {
        QMessageBox::information(this, tr("Genesys Optimizer"),
                                 tr("Prototype of an OptQuest-like optimization workstation for Genesys models."));
    });

    QToolBar* toolbar = addToolBar(tr("Optimizer"));
    toolbar->addAction(_refreshModelAction);
    toolbar->addAction(_configureTechniqueAction);
    toolbar->addSeparator();
    toolbar->addAction(_checkAction);
    toolbar->addAction(_startAction);
    toolbar->addAction(_stepAction);
    toolbar->addAction(_pauseAction);
    toolbar->addAction(_resumeAction);
    toolbar->addAction(_stopAction);
}

void OptimizerWindow::buildWorkspace() {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(6, 6, 6, 6);
    _modelStatusLabel = new QLabel(central);
    _modelStatusLabel->setWordWrap(true);
    _modelStatusLabel->setStyleSheet(QStringLiteral("font-weight: 600;"));
    layout->addWidget(_modelStatusLabel);

    _tabs = new QTabWidget(central);
    layout->addWidget(_tabs, 1);
    setCentralWidget(central);

    _overviewText = new QTextBrowser(_tabs);
    _overviewText->setHtml(tr(
            "<h2>Genesys Optimizer</h2>"
            "<p>This workspace is organized as an optimization study for the current model.</p>"
            "<ul>"
            "<li>Select model controls that the optimizer may vary.</li>"
            "<li>Select model responses used by objectives, constraints and monitoring.</li>"
            "<li>Create objective and constraint expressions using Genesys parser syntax.</li>"
            "<li>Choose an optimization technique and configure its execution limits.</li>"
            "<li>Run the backend incrementally and monitor progress and best solutions.</li>"
            "</ul>"
            "<p>The first implementation connects to <b>OptimizerDefaultImpl1</b>; concrete search algorithms are still isolated behind the optimizer interface.</p>"));
    _tabs->addTab(_overviewText, tr("Start"));

    auto* controlsTab = new QWidget(_tabs);
    auto* controlsLayout = new QVBoxLayout(controlsTab);
    auto* controlsIntro = new QLabel(tr("Writable numeric controls are the natural decision variables. Bounds are editable optimization metadata for the future algorithm layer."), controlsTab);
    controlsIntro->setWordWrap(true);
    _controlsTable = new QTableWidget(controlsTab);
    _controlsTable->setColumnCount(9);
    _controlsTable->setHorizontalHeaderLabels({
            tr("Use"), tr("Element"), tr("Class"), tr("Control"), tr("Current value"), tr("Type"),
            tr("Readonly"), tr("Lower bound"), tr("Upper bound")
    });
    _controlsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _controlsTable->verticalHeader()->setVisible(false);
    controlsLayout->addWidget(controlsIntro);
    controlsLayout->addWidget(_controlsTable, 1);
    _tabs->addTab(controlsTab, tr("Controls"));

    auto* responsesTab = new QWidget(_tabs);
    auto* responsesLayout = new QVBoxLayout(responsesTab);
    auto* responsesIntro = new QLabel(tr("Responses are observed KPIs that can be referenced by objective and constraint expressions."), responsesTab);
    responsesIntro->setWordWrap(true);
    _responsesTable = new QTableWidget(responsesTab);
    _responsesTable->setColumnCount(7);
    _responsesTable->setHorizontalHeaderLabels({
            tr("Use"), tr("Element"), tr("Class"), tr("Response"), tr("Current value"), tr("Type"), tr("Description")
    });
    _responsesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _responsesTable->verticalHeader()->setVisible(false);
    responsesLayout->addWidget(responsesIntro);
    responsesLayout->addWidget(_responsesTable, 1);
    _tabs->addTab(responsesTab, tr("Responses"));

    auto* objectivesTab = new QWidget(_tabs);
    auto* objectivesLayout = new QVBoxLayout(objectivesTab);
    _objectivesTable = new QTableWidget(objectivesTab);
    _objectivesTable->setColumnCount(5);
    _objectivesTable->setHorizontalHeaderLabels({tr("Enabled"), tr("Sense"), tr("Name"), tr("Expression"), tr("Weight")});
    _objectivesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _objectivesTable->verticalHeader()->setVisible(false);
    auto* objectiveButtons = new QHBoxLayout();
    auto* addObjectiveButton = new QPushButton(tr("Add Objective"), objectivesTab);
    auto* removeObjectiveButton = new QPushButton(tr("Remove Selected"), objectivesTab);
    objectiveButtons->addWidget(addObjectiveButton);
    objectiveButtons->addWidget(removeObjectiveButton);
    objectiveButtons->addStretch();
    objectivesLayout->addWidget(new QLabel(tr("Objectives are aggregated by the backend. Expressions should reference selected responses and controls."), objectivesTab));
    objectivesLayout->addWidget(_objectivesTable, 1);
    objectivesLayout->addLayout(objectiveButtons);
    _tabs->addTab(objectivesTab, tr("Objectives"));

    auto* constraintsTab = new QWidget(_tabs);
    auto* constraintsLayout = new QVBoxLayout(constraintsTab);
    _constraintsTable = new QTableWidget(constraintsTab);
    _constraintsTable->setColumnCount(3);
    _constraintsTable->setHorizontalHeaderLabels({tr("Enabled"), tr("Name"), tr("Expression")});
    _constraintsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _constraintsTable->verticalHeader()->setVisible(false);
    auto* constraintButtons = new QHBoxLayout();
    auto* addConstraintButton = new QPushButton(tr("Add Constraint"), constraintsTab);
    auto* removeConstraintButton = new QPushButton(tr("Remove Selected"), constraintsTab);
    constraintButtons->addWidget(addConstraintButton);
    constraintButtons->addWidget(removeConstraintButton);
    constraintButtons->addStretch();
    constraintsLayout->addWidget(new QLabel(tr("Constraints are feasible when their expression evaluates to non-zero."), constraintsTab));
    constraintsLayout->addWidget(_constraintsTable, 1);
    constraintsLayout->addLayout(constraintButtons);
    _tabs->addTab(constraintsTab, tr("Constraints"));

    auto* settingsTab = new QWidget(_tabs);
    auto* settingsLayout = new QVBoxLayout(settingsTab);
    auto* techniqueBox = new QGroupBox(tr("Optimization technique"), settingsTab);
    auto* techniqueLayout = new QFormLayout(techniqueBox);
    _techniqueCombo = new QComboBox(techniqueBox);
    _techniqueCombo->addItems({
            tr("Genetic Algorithm"),
            tr("Hill Climbing"),
            tr("Particle Swarm"),
            tr("Multiobjective Genetic Algorithm (NSGA-II preview)")
    });
    _populationSizeSpin = new QSpinBox(techniqueBox);
    _populationSizeSpin->setRange(2, 100000);
    _populationSizeSpin->setValue(30);
    _mutationRateSpin = new QDoubleSpinBox(techniqueBox);
    _mutationRateSpin->setRange(0.0, 1.0);
    _mutationRateSpin->setSingleStep(0.01);
    _mutationRateSpin->setDecimals(4);
    _mutationRateSpin->setValue(0.08);
    _crossoverRateSpin = new QDoubleSpinBox(techniqueBox);
    _crossoverRateSpin->setRange(0.0, 1.0);
    _crossoverRateSpin->setSingleStep(0.01);
    _crossoverRateSpin->setDecimals(4);
    _crossoverRateSpin->setValue(0.75);
    _techniqueNotes = new QPlainTextEdit(techniqueBox);
    _techniqueNotes->setReadOnly(true);
    _techniqueNotes->setMaximumHeight(86);
    techniqueLayout->addRow(tr("Technique:"), _techniqueCombo);
    techniqueLayout->addRow(tr("Population / neighborhood size:"), _populationSizeSpin);
    techniqueLayout->addRow(tr("Mutation / perturbation rate:"), _mutationRateSpin);
    techniqueLayout->addRow(tr("Crossover / recombination rate:"), _crossoverRateSpin);
    techniqueLayout->addRow(tr("Notes:"), _techniqueNotes);
    _techniqueNotes->setPlainText(techniqueDescription(_techniqueCombo->currentText()));

    auto* executionBox = new QGroupBox(tr("Execution limits"), settingsTab);
    auto* executionLayout = new QFormLayout(executionBox);
    _maxIterationsSpin = new QSpinBox(executionBox);
    _maxIterationsSpin->setRange(1, 10000000);
    _maxIterationsSpin->setValue(100);
    _maxSimulationsSpin = new QSpinBox(executionBox);
    _maxSimulationsSpin->setRange(0, 10000000);
    _maxSimulationsSpin->setValue(0);
    _replicationsSpin = new QSpinBox(executionBox);
    _replicationsSpin->setRange(1, 100000);
    _replicationsSpin->setValue(1);
    _bestSolutionsSpin = new QSpinBox(executionBox);
    _bestSolutionsSpin->setRange(1, 10000);
    _bestSolutionsSpin->setValue(10);
    _randomSeedSpin = new QSpinBox(executionBox);
    _randomSeedSpin->setRange(0, 2147483647);
    _randomSeedSpin->setValue(0);
    _improvementToleranceSpin = new QDoubleSpinBox(executionBox);
    _improvementToleranceSpin->setDecimals(10);
    _improvementToleranceSpin->setRange(0.0, 1.0);
    _timeLimitSpin = new QDoubleSpinBox(executionBox);
    _timeLimitSpin->setRange(0.0, 10000000.0);
    _timeLimitSpin->setSuffix(tr(" s"));
    executionLayout->addRow(tr("Max iterations:"), _maxIterationsSpin);
    executionLayout->addRow(tr("Max simulations (0 = unlimited):"), _maxSimulationsSpin);
    executionLayout->addRow(tr("Replications per solution:"), _replicationsSpin);
    executionLayout->addRow(tr("Best solutions to keep:"), _bestSolutionsSpin);
    executionLayout->addRow(tr("Random seed (0 = automatic):"), _randomSeedSpin);
    executionLayout->addRow(tr("Improvement tolerance:"), _improvementToleranceSpin);
    executionLayout->addRow(tr("Time limit:"), _timeLimitSpin);
    settingsLayout->addWidget(techniqueBox);
    settingsLayout->addWidget(executionBox);
    settingsLayout->addStretch();
    _tabs->addTab(settingsTab, tr("Technique / Settings"));

    auto* runTab = new QWidget(_tabs);
    auto* runLayout = new QVBoxLayout(runTab);
    _runStatusLabel = new QLabel(runTab);
    _runStatusLabel->setWordWrap(true);
    auto* runButtons = new QHBoxLayout();
    auto* checkButton = new QPushButton(tr("Check"), runTab);
    auto* startButton = new QPushButton(tr("Start"), runTab);
    auto* stepButton = new QPushButton(tr("Step"), runTab);
    auto* pauseButton = new QPushButton(tr("Pause"), runTab);
    auto* resumeButton = new QPushButton(tr("Resume"), runTab);
    auto* stopButton = new QPushButton(tr("Stop"), runTab);
    for (QPushButton* button : {checkButton, startButton, stepButton, pauseButton, resumeButton, stopButton}) {
        runButtons->addWidget(button);
    }
    runButtons->addStretch();
    _progressPlot = new ProgressPlot(runTab);
    _bestSolutionsTable = new QTableWidget(runTab);
    _bestSolutionsTable->setColumnCount(5);
    _bestSolutionsTable->setHorizontalHeaderLabels({tr("Iteration"), tr("Simulation"), tr("Feasible"), tr("Objective"), tr("Description")});
    _bestSolutionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _bestSolutionsTable->verticalHeader()->setVisible(false);
    runLayout->addWidget(_runStatusLabel);
    runLayout->addLayout(runButtons);
    runLayout->addWidget(_progressPlot);
    runLayout->addWidget(new QLabel(tr("Best solutions retained by the optimization backend:"), runTab));
    runLayout->addWidget(_bestSolutionsTable, 1);
    _tabs->addTab(runTab, tr("Run Monitor"));

    _reportText = new QTextBrowser(_tabs);
    _tabs->addTab(_reportText, tr("Report"));

    connect(addObjectiveButton, &QPushButton::clicked, this, [this]() { addObjective(); });
    connect(removeObjectiveButton, &QPushButton::clicked, this, [this]() { removeSelectedObjective(); });
    connect(addConstraintButton, &QPushButton::clicked, this, [this]() { addConstraint(); });
    connect(removeConstraintButton, &QPushButton::clicked, this, [this]() { removeSelectedConstraint(); });
    connect(checkButton, &QPushButton::clicked, this, [this]() { checkReadiness(); });
    connect(startButton, &QPushButton::clicked, this, [this]() { startOptimization(); });
    connect(stepButton, &QPushButton::clicked, this, [this]() { stepOptimization(); });
    connect(pauseButton, &QPushButton::clicked, this, [this]() { pauseOptimization(); });
    connect(resumeButton, &QPushButton::clicked, this, [this]() { resumeOptimization(); });
    connect(stopButton, &QPushButton::clicked, this, [this]() { stopOptimization(); });
}

void OptimizerWindow::connectActions() {
    connect(_refreshModelAction, &QAction::triggered, this, [this]() { refreshFromCurrentModel(); });
    connect(_configureTechniqueAction, &QAction::triggered, this, [this]() { configureTechnique(); });
    connect(_checkAction, &QAction::triggered, this, [this]() { checkReadiness(); });
    connect(_startAction, &QAction::triggered, this, [this]() { startOptimization(); });
    connect(_stepAction, &QAction::triggered, this, [this]() { stepOptimization(); });
    connect(_pauseAction, &QAction::triggered, this, [this]() { pauseOptimization(); });
    connect(_resumeAction, &QAction::triggered, this, [this]() { resumeOptimization(); });
    connect(_stopAction, &QAction::triggered, this, [this]() { stopOptimization(); });
    connect(_techniqueCombo, &QComboBox::currentTextChanged, this, [this](const QString& technique) {
        _techniqueNotes->setPlainText(techniqueDescription(technique));
        refreshReport();
    });
}

void OptimizerWindow::refreshFromCurrentModel() {
    Model* model = _simulator != nullptr && _simulator->getModelManager() != nullptr
                       ? _simulator->getModelManager()->current()
                       : nullptr;
    _optimizer.setModel(model);
    _controls.clear();
    _responses.clear();
    if (_optimizer.getAvailableControls() != nullptr) {
        for (SimulationControl* control : *_optimizer.getAvailableControls()->list()) {
            _controls.push_back(control);
        }
    }
    if (_optimizer.getAvailableResponses() != nullptr) {
        for (SimulationResponse* response : *_optimizer.getAvailableResponses()->list()) {
            _responses.push_back(response);
        }
    }
    _modelStatusLabel->setText(model != nullptr
                                   ? tr("Current model linked. Available controls: %1. Available responses: %2.")
                                         .arg(_controls.size())
                                         .arg(_responses.size())
                                   : tr("No current model is available. Open or create a Genesys model before running optimization."));
    refreshControlsTable();
    refreshResponsesTable();
    if (_objectivesTable->rowCount() == 0) {
        const int row = _objectivesTable->rowCount();
        _objectivesTable->insertRow(row);
        _objectivesTable->setItem(row, 0, checkableItem(true));
        _objectivesTable->setItem(row, 1, editableItem(tr("minimize")));
        _objectivesTable->setItem(row, 2, editableItem(tr("Primary objective")));
        _objectivesTable->setItem(row, 3, editableItem(
                _responses.empty() ? QStringLiteral("response_value") : QString::fromStdString(_responses.front()->getName())));
        _objectivesTable->setItem(row, 4, editableItem(QStringLiteral("1.0")));
    }
    refreshReport();
    refreshRunState();
    statusBar()->showMessage(tr("Optimizer model context refreshed."), 3000);
}

void OptimizerWindow::refreshControlsTable() {
    _controlsTable->setRowCount(0);
    for (SimulationControl* control : _controls) {
        if (control == nullptr) {
            continue;
        }
        const QString currentValue = QString::fromStdString(control->getValue());
        const bool numeric = stringIsNumeric(currentValue);
        const bool usable = numeric && !control->isReadOnly();
        const double value = currentValue.toDouble();
        const int row = _controlsTable->rowCount();
        _controlsTable->insertRow(row);
        _controlsTable->setItem(row, 0, checkableItem(usable, usable));
        _controlsTable->setItem(row, 1, readOnlyItem(QString::fromStdString(control->getElementName())));
        _controlsTable->setItem(row, 2, readOnlyItem(QString::fromStdString(control->getClassname())));
        _controlsTable->setItem(row, 3, readOnlyItem(QString::fromStdString(control->getName())));
        _controlsTable->setItem(row, 4, readOnlyItem(currentValue));
        _controlsTable->setItem(row, 5, readOnlyItem(QString::fromStdString(control->propertyType())));
        _controlsTable->setItem(row, 6, readOnlyItem(control->isReadOnly() ? tr("yes") : tr("no")));
        _controlsTable->setItem(row, 7, editableItem(numeric ? formatNumber(value * 0.5) : QString()));
        _controlsTable->setItem(row, 8, editableItem(numeric ? formatNumber(value == 0.0 ? 1.0 : value * 1.5) : QString()));
    }
}

void OptimizerWindow::refreshResponsesTable() {
    _responsesTable->setRowCount(0);
    for (SimulationResponse* response : _responses) {
        if (response == nullptr) {
            continue;
        }
        const QString currentValue = QString::fromStdString(response->getValue());
        const bool numeric = stringIsNumeric(currentValue);
        const int row = _responsesTable->rowCount();
        _responsesTable->insertRow(row);
        _responsesTable->setItem(row, 0, checkableItem(numeric));
        _responsesTable->setItem(row, 1, readOnlyItem(QString::fromStdString(response->getElementName())));
        _responsesTable->setItem(row, 2, readOnlyItem(QString::fromStdString(response->getClassname())));
        _responsesTable->setItem(row, 3, readOnlyItem(QString::fromStdString(response->getName())));
        _responsesTable->setItem(row, 4, readOnlyItem(currentValue));
        _responsesTable->setItem(row, 5, readOnlyItem(QString::fromStdString(response->propertyType())));
        _responsesTable->setItem(row, 6, readOnlyItem(QString::fromStdString(response->whatsThis())));
    }
}

void OptimizerWindow::refreshReport() {
    QString objectiveSummary = tr("%1 objective(s)").arg(_objectivesTable != nullptr ? _objectivesTable->rowCount() : 0);
    QString constraintSummary = tr("%1 constraint(s)").arg(_constraintsTable != nullptr ? _constraintsTable->rowCount() : 0);
    _reportText->setHtml(tr(
            "<h3>Optimization Study</h3>"
            "<p><b>Model controls:</b> %1 available.</p>"
            "<p><b>Model responses:</b> %2 available.</p>"
            "<p><b>Configured expressions:</b> %3, %4.</p>"
            "<p><b>Technique:</b> %5.</p>"
            "<p><b>Backend:</b> OptimizerDefaultImpl1 is currently connected. The algorithm selection is recorded in the GUI and ready for routing to future concrete optimizer implementations.</p>")
            .arg(_controls.size())
            .arg(_responses.size())
            .arg(objectiveSummary, constraintSummary, _techniqueCombo->currentText()));
}

void OptimizerWindow::applyConfigurationToBackend() {
    List<SimulationControl*> selectedControls;
    for (int row = 0; row < _controlsTable->rowCount() && row < static_cast<int>(_controls.size()); ++row) {
        QTableWidgetItem* useItem = _controlsTable->item(row, 0);
        if (useItem != nullptr && useItem->checkState() == Qt::Checked) {
            selectedControls.insert(_controls.at(row));
        }
    }
    _optimizer.setSelectedControls(&selectedControls);

    List<SimulationResponse*> selectedResponses;
    for (int row = 0; row < _responsesTable->rowCount() && row < static_cast<int>(_responses.size()); ++row) {
        QTableWidgetItem* useItem = _responsesTable->item(row, 0);
        if (useItem != nullptr && useItem->checkState() == Qt::Checked) {
            selectedResponses.insert(_responses.at(row));
        }
    }
    _optimizer.setSelectedResponses(&selectedResponses);

    _optimizer.clearObjectives();
    for (int row = 0; row < _objectivesTable->rowCount(); ++row) {
        Optimizer_if::ObjectiveDefinition objective;
        objective.enabled = _objectivesTable->item(row, 0) == nullptr
                            || _objectivesTable->item(row, 0)->checkState() == Qt::Checked;
        const QString sense = _objectivesTable->item(row, 1) != nullptr
                                  ? _objectivesTable->item(row, 1)->text().trimmed().toLower()
                                  : QString();
        objective.sense = sense.startsWith(QStringLiteral("max"))
                              ? Optimizer_if::ObjectiveSense::MAXIMIZE
                              : Optimizer_if::ObjectiveSense::MINIMIZE;
        objective.name = _objectivesTable->item(row, 2) != nullptr
                             ? _objectivesTable->item(row, 2)->text().toStdString()
                             : "Objective";
        objective.expression = _objectivesTable->item(row, 3) != nullptr
                                   ? _objectivesTable->item(row, 3)->text().toStdString()
                                   : "";
        objective.weight = _objectivesTable->item(row, 4) != nullptr
                               ? _objectivesTable->item(row, 4)->text().toDouble()
                               : 1.0;
        _optimizer.addObjective(objective);
    }

    _optimizer.clearConstraints();
    for (int row = 0; row < _constraintsTable->rowCount(); ++row) {
        Optimizer_if::ConstraintDefinition constraint;
        constraint.enabled = _constraintsTable->item(row, 0) == nullptr
                             || _constraintsTable->item(row, 0)->checkState() == Qt::Checked;
        constraint.name = _constraintsTable->item(row, 1) != nullptr
                              ? _constraintsTable->item(row, 1)->text().toStdString()
                              : "Constraint";
        constraint.expression = _constraintsTable->item(row, 2) != nullptr
                                    ? _constraintsTable->item(row, 2)->text().toStdString()
                                    : "";
        _optimizer.addConstraint(constraint);
    }

    Optimizer_if::OptimizationSettings settings;
    settings.maxIterations = static_cast<unsigned int>(_maxIterationsSpin->value());
    settings.maxSimulations = static_cast<unsigned int>(_maxSimulationsSpin->value());
    settings.replicationsPerSolution = static_cast<unsigned int>(_replicationsSpin->value());
    settings.bestSolutionsToKeep = static_cast<unsigned int>(_bestSolutionsSpin->value());
    settings.randomSeed = static_cast<unsigned int>(_randomSeedSpin->value());
    settings.improvementTolerance = _improvementToleranceSpin->value();
    settings.timeLimitSeconds = _timeLimitSpin->value();
    _optimizer.setSettings(settings);
    refreshReport();
}

void OptimizerWindow::refreshRunState() {
    _runStatusLabel->setText(tr("State: %1 | Iteration: %2 | Simulations: %3 | Technique: %4")
                             .arg(stateText(_optimizer.getExecutionState()))
                             .arg(_optimizer.getCurrentIteration())
                             .arg(_optimizer.getTotalSimulations())
                             .arg(_techniqueCombo->currentText()));
    _bestSolutionsTable->setRowCount(0);
    if (_optimizer.getBestSolutions() != nullptr) {
        for (const Optimizer_if::SolutionSummary& solution : *_optimizer.getBestSolutions()->list()) {
            const int row = _bestSolutionsTable->rowCount();
            _bestSolutionsTable->insertRow(row);
            _bestSolutionsTable->setItem(row, 0, readOnlyItem(QString::number(solution.iteration)));
            _bestSolutionsTable->setItem(row, 1, readOnlyItem(QString::number(solution.simulationNumber)));
            _bestSolutionsTable->setItem(row, 2, readOnlyItem(solution.feasible ? tr("yes") : tr("no")));
            _bestSolutionsTable->setItem(row, 3, readOnlyItem(formatNumber(solution.aggregatedObjectiveValue)));
            _bestSolutionsTable->setItem(row, 4, readOnlyItem(QString::fromStdString(solution.description)));
        }
    }
    _progressPlot->setValues(_progressValues);
}

void OptimizerWindow::addObjective() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Objective"));
    auto* form = new QFormLayout(&dialog);
    auto* sense = new QComboBox(&dialog);
    sense->addItems({tr("minimize"), tr("maximize")});
    auto* name = new QLineEdit(tr("Primary objective"), &dialog);
    QString defaultExpression = _responses.empty() ? QStringLiteral("response_value") : QString::fromStdString(_responses.front()->getName());
    auto* expression = new QLineEdit(defaultExpression, &dialog);
    auto* weight = new QDoubleSpinBox(&dialog);
    weight->setRange(0.0, 1000000.0);
    weight->setDecimals(6);
    weight->setValue(1.0);
    form->addRow(tr("Sense:"), sense);
    form->addRow(tr("Name:"), name);
    form->addRow(tr("Expression:"), expression);
    form->addRow(tr("Weight:"), weight);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int row = _objectivesTable->rowCount();
    _objectivesTable->insertRow(row);
    _objectivesTable->setItem(row, 0, checkableItem(true));
    _objectivesTable->setItem(row, 1, editableItem(sense->currentText()));
    _objectivesTable->setItem(row, 2, editableItem(name->text()));
    _objectivesTable->setItem(row, 3, editableItem(expression->text()));
    _objectivesTable->setItem(row, 4, editableItem(QString::number(weight->value(), 'g', 8)));
    refreshReport();
}

void OptimizerWindow::removeSelectedObjective() {
    if (_objectivesTable->currentRow() >= 0) {
        _objectivesTable->removeRow(_objectivesTable->currentRow());
        refreshReport();
    }
}

void OptimizerWindow::addConstraint() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Constraint"));
    auto* form = new QFormLayout(&dialog);
    auto* name = new QLineEdit(tr("New constraint"), &dialog);
    auto* expression = new QLineEdit(QStringLiteral("1"), &dialog);
    form->addRow(tr("Name:"), name);
    form->addRow(tr("Expression:"), expression);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int row = _constraintsTable->rowCount();
    _constraintsTable->insertRow(row);
    _constraintsTable->setItem(row, 0, checkableItem(true));
    _constraintsTable->setItem(row, 1, editableItem(name->text()));
    _constraintsTable->setItem(row, 2, editableItem(expression->text()));
    refreshReport();
}

void OptimizerWindow::removeSelectedConstraint() {
    if (_constraintsTable->currentRow() >= 0) {
        _constraintsTable->removeRow(_constraintsTable->currentRow());
        refreshReport();
    }
}

void OptimizerWindow::configureTechnique() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Optimization technique"));
    auto* form = new QFormLayout(&dialog);
    auto* technique = new QComboBox(&dialog);
    for (int i = 0; i < _techniqueCombo->count(); ++i) {
        technique->addItem(_techniqueCombo->itemText(i));
    }
    technique->setCurrentText(_techniqueCombo->currentText());
    auto* population = new QSpinBox(&dialog);
    population->setRange(_populationSizeSpin->minimum(), _populationSizeSpin->maximum());
    population->setValue(_populationSizeSpin->value());
    auto* mutation = new QDoubleSpinBox(&dialog);
    mutation->setRange(0.0, 1.0);
    mutation->setDecimals(4);
    mutation->setValue(_mutationRateSpin->value());
    auto* crossover = new QDoubleSpinBox(&dialog);
    crossover->setRange(0.0, 1.0);
    crossover->setDecimals(4);
    crossover->setValue(_crossoverRateSpin->value());
    form->addRow(tr("Technique:"), technique);
    form->addRow(tr("Population / neighborhood size:"), population);
    form->addRow(tr("Mutation / perturbation rate:"), mutation);
    form->addRow(tr("Crossover / recombination rate:"), crossover);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    _techniqueCombo->setCurrentText(technique->currentText());
    _populationSizeSpin->setValue(population->value());
    _mutationRateSpin->setValue(mutation->value());
    _crossoverRateSpin->setValue(crossover->value());
    _tabs->setCurrentIndex(5);
    refreshReport();
}

void OptimizerWindow::checkReadiness() {
    applyConfigurationToBackend();
    std::string message;
    const bool ready = _optimizer.checkReady(&message);
    statusBar()->showMessage(QString::fromStdString(message), 4000);
    if (!ready) {
        QMessageBox::information(this, tr("Optimizer readiness"), QString::fromStdString(message));
    }
    refreshRunState();
}

void OptimizerWindow::startOptimization() {
    applyConfigurationToBackend();
    _progressValues.clear();
    if (!_optimizer.start()) {
        std::string message;
        _optimizer.checkReady(&message);
        QMessageBox::warning(this, tr("Optimizer"), QString::fromStdString(message));
    } else {
        _tabs->setCurrentIndex(6);
    }
    refreshRunState();
}

void OptimizerWindow::stepOptimization() {
    const bool stepped = _optimizer.step();
    if (stepped) {
        const double x = static_cast<double>(_optimizer.getCurrentIteration());
        const double previewObjective = 100.0 / (1.0 + 0.08 * x) + 4.0 * std::sin(x * 0.3);
        _progressValues.append(previewObjective);
    }
    refreshRunState();
}

void OptimizerWindow::pauseOptimization() {
    _optimizer.pause();
    refreshRunState();
}

void OptimizerWindow::resumeOptimization() {
    _optimizer.resume();
    refreshRunState();
}

void OptimizerWindow::stopOptimization() {
    _optimizer.stop();
    refreshRunState();
}

void OptimizerWindow::showSkeletonMessage(const QString& featureName) {
    statusBar()->showMessage(tr("%1 is reserved for a later Optimizer iteration.").arg(featureName), 4000);
}

QString OptimizerWindow::formatNumber(double value) {
    if (!std::isfinite(value)) {
        return tr("n/a");
    }
    return QString::number(value, 'g', 8);
}

QString OptimizerWindow::stateText(Optimizer_if::ExecutionState state) {
    switch (state) {
        case Optimizer_if::ExecutionState::NOT_READY: return tr("Not ready");
        case Optimizer_if::ExecutionState::READY: return tr("Ready");
        case Optimizer_if::ExecutionState::RUNNING: return tr("Running");
        case Optimizer_if::ExecutionState::PAUSED: return tr("Paused");
        case Optimizer_if::ExecutionState::STOPPED: return tr("Stopped");
        case Optimizer_if::ExecutionState::FINISHED: return tr("Finished");
        case Optimizer_if::ExecutionState::ERROR: return tr("Error");
    }
    return tr("Unknown");
}

QString OptimizerWindow::techniqueDescription(const QString& technique) {
    if (technique.contains(QStringLiteral("Genetic"), Qt::CaseInsensitive)) {
        return tr("Genetic Algorithm preview: population, mutation and crossover settings are captured by the GUI. The current backend lifecycle is connected; actual chromosome evaluation remains in the future concrete technique layer.");
    }
    if (technique.contains(QStringLiteral("Hill"), Qt::CaseInsensitive)) {
        return tr("Hill Climbing preview: population size behaves as neighborhood size, mutation rate as perturbation intensity, and crossover is ignored by future local-search implementations.");
    }
    if (technique.contains(QStringLiteral("Swarm"), Qt::CaseInsensitive)) {
        return tr("Particle Swarm preview: population size represents particles. Mutation and crossover fields are placeholders for inertia, cognitive and social parameters.");
    }
    return tr("Multiobjective GA preview: objectives should keep separate weights and constraints. A future NSGA-II style implementation can feed Pareto-front views.");
}

bool OptimizerWindow::stringIsNumeric(const QString& value) {
    bool ok = false;
    const double numericValue = value.toDouble(&ok);
    return ok && std::isfinite(numericValue);
}

QTableWidgetItem* OptimizerWindow::readOnlyItem(const QString& text) {
    auto* item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

QTableWidgetItem* OptimizerWindow::editableItem(const QString& text) {
    return new QTableWidgetItem(text);
}

QTableWidgetItem* OptimizerWindow::checkableItem(bool checked, bool enabled) {
    auto* item = new QTableWidgetItem();
    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
    if (enabled) {
        flags |= Qt::ItemIsEnabled;
    }
    item->setFlags(flags);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    return item;
}
