#include "DialogUtilityController.h"

#include "../mainwindow.h"
#include "ui_mainwindow.h"
#include "../dialogs/DialogFind.h"
#include "../dialogs/dialogBreakpoint.h"
#include "../dialogs/dialogpluginmanager.h"
#include "../dialogs/dialogsystempreferences.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"

#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../../kernel/simulator/ModelComponent.h"
#include "../../../../../kernel/simulator/LicenceManager.h"
#include "../../../../../plugins/data/Entity.h"
#include "../../../../../tools/SolverDefaultImpl1.h"

#include <QCheckBox>
#include <QDialog>
#include <QSize>
#include <QPixmap>
#include <QPointF>
#include <QModelIndex>
#include <QDir>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <Qt>
#include <algorithm>
#include <cmath>
#include <memory>

// Initialize the Phase 11 controller with narrow dependencies and persisted GUI state references.
DialogUtilityController::DialogUtilityController(MainWindow* ownerWidget,
                                                 Simulator* simulator,
                                                 Ui::MainWindow* ui,
                                                 ModelGraphicsView* graphicsView,
                                                 std::function<void()> showMessageNotImplemented,
                                                 std::function<void(bool)> actualizeDebugBreakpoints,
                                                 std::function<bool()> createModelImage,
                                                 std::function<void()> actualizeActions,
                                                 std::function<void()> actualizeTabPanes,
                                                 std::function<ModelGraphicsScene*()> currentScene,
                                                 double& optimizerPrecision,
                                                 unsigned int& optimizerMaxSteps,
                                                 bool& parallelizationEnabled,
                                                 int& parallelizationThreads,
                                                 int& parallelizationBatchSize,
                                                 QString& lastDataAnalyzerPath)
    : _ownerWidget(ownerWidget),
      _simulator(simulator),
      _ui(ui),
      _graphicsView(graphicsView),
      _showMessageNotImplemented(std::move(showMessageNotImplemented)),
      _actualizeDebugBreakpoints(std::move(actualizeDebugBreakpoints)),
      _createModelImage(std::move(createModelImage)),
      _actualizeActions(std::move(actualizeActions)),
      _actualizeTabPanes(std::move(actualizeTabPanes)),
      _currentScene(std::move(currentScene)),
      _optimizerPrecision(optimizerPrecision),
      _optimizerMaxSteps(optimizerMaxSteps),
      _parallelizationEnabled(parallelizationEnabled),
      _parallelizationThreads(parallelizationThreads),
      _parallelizationBatchSize(parallelizationBatchSize),
      _lastDataAnalyzerPath(lastDataAnalyzerPath) {
}

// Preserve the existing About dialog text and parenting behavior.
void DialogUtilityController::onActionAboutAboutTriggered() {
    QMessageBox::about(_ownerWidget, "About Genesys", "Genesys is a result of teaching and research activities of Professor Dr. Ing Rafael Luiz Cancian. It began in early 2002 as a way to teach students the basics and simulation techniques of systems implemented by other comercial simulation tools, such as Arena. In Genesys development he replicated all the SIMAN language, used by Arena software, and Genesys has become a clone of that tool, including its graphical interface. Genesys allowed the inclusion of new simulation components through dynamic link libraries and also the parallel execution of simulation models in a distributed environment. The development of Genesys continued until 2009, when the professor stopped teaching systems simulation classes. Ten years later the professor starts again to teach systems simulation classes and to carry out scientific research in the area. So in 2019 Genesys is reborn, with new language and programming techniques, and even more ambitious goals.");
}

// Preserve the existing license dialog flow and text composition.
void DialogUtilityController::onActionAboutLicenceTriggered() {
    LicenceManager* licman = _simulator->getLicenceManager();
    std::string text = licman->showLicence() + "\n";
    text += licman->showLimits() + "\n";
    text += licman->showActivationCode();
    QMessageBox::about(_ownerWidget, "About Licence", QString::fromStdString(text));
}

// Preserve the existing get-involved message content and parent window.
void DialogUtilityController::onActionAboutGetInvolvedTriggered() {
    QMessageBox::about(_ownerWidget, "Get Involved", "Genesys is a free open-source simulator (and tools) available at 'https://github.com/rlcancian/Genesys-Simulator'. Help us by submiting your pull requests containing code improvements. Contact: rafael.cancian@ufsc.br");
}

// Preserve the existing find dialog workflow using the current scene.
void DialogUtilityController::onActionEditFindTriggered() {
    DialogFind* find = new DialogFind(_ownerWidget, _graphicsView->getScene());
    find->show();
    if (find->exec() == QDialog::Accepted) {
        find->setFocus();
    }
}

// Preserve the existing replace dialog and selection-based replace workflow.
void DialogUtilityController::onActionEditReplaceTriggered() {
    if (_ui->TextCodeEditor == nullptr) {
        return;
    }

    auto* editor = _ui->TextCodeEditor;

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Replace"));
    dialog.setModal(false);

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    auto* findLine = new QLineEdit(&dialog);
    auto* replaceLine = new QLineEdit(&dialog);
    auto* caseSensitive = new QCheckBox(QObject::tr("Case sensitive"), &dialog);
    auto* statusLabel = new QLabel(&dialog);
    statusLabel->setWordWrap(true);
    statusLabel->setText(QObject::tr("Ready."));

    static QString lastFindText;
    static QString lastReplaceText;
    findLine->setText(lastFindText);
    replaceLine->setText(lastReplaceText);

    formLayout->addRow(QObject::tr("Find:"), findLine);
    formLayout->addRow(QObject::tr("Replace with:"), replaceLine);

    auto* buttonFindNext = new QPushButton(QObject::tr("Find Next"), &dialog);
    auto* buttonReplace = new QPushButton(QObject::tr("Replace"), &dialog);
    auto* buttonReplaceAll = new QPushButton(QObject::tr("Replace All"), &dialog);
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);

    auto* actionLayout = new QHBoxLayout();
    actionLayout->addWidget(buttonFindNext);
    actionLayout->addWidget(buttonReplace);
    actionLayout->addWidget(buttonReplaceAll);

    layout->addLayout(formLayout);
    layout->addWidget(caseSensitive);
    layout->addLayout(actionLayout);
    layout->addWidget(statusLabel);
    layout->addWidget(buttonBox);

    // Keep the same looping find behavior that wraps to the document start.
    auto findNext = [&]() -> bool {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return false;
        }

        QTextDocument::FindFlags flags;
        if (caseSensitive->isChecked()) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        bool found = editor->find(findText, flags);
        if (!found) {
            QTextCursor cursor = editor->textCursor();
            cursor.movePosition(QTextCursor::Start);
            editor->setTextCursor(cursor);
            found = editor->find(findText, flags);
        }

        if (found) {
            statusLabel->setText(QObject::tr("Occurrence selected."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            return true;
        }

        statusLabel->setText(QObject::tr("No occurrences found."));
        return false;
    };

    // Keep existing button wiring and status updates for find next.
    QObject::connect(buttonFindNext, &QPushButton::clicked, &dialog, [&]() {
        findNext();
    });

    // Keep existing replace-one behavior and immediate next search.
    QObject::connect(buttonReplace, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return;
        }

        QTextCursor cursor = editor->textCursor();
        const bool matchesSelection = cursor.hasSelection() &&
                ((caseSensitive->isChecked() && cursor.selectedText() == findText) ||
                 (!caseSensitive->isChecked() && cursor.selectedText().compare(findText, Qt::CaseInsensitive) == 0));

        if (!matchesSelection && !findNext()) {
            return;
        }

        cursor = editor->textCursor();
        if (cursor.hasSelection()) {
            cursor.insertText(replaceLine->text());
            editor->setTextCursor(cursor);
            statusLabel->setText(QObject::tr("Occurrence replaced."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            findNext();
        }
    });

    // Keep existing replace-all scan semantics and edit block batching.
    QObject::connect(buttonReplaceAll, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(QObject::tr("Enter text to find."));
            return;
        }

        QTextDocument::FindFlags flags;
        if (caseSensitive->isChecked()) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        QTextCursor scanCursor(editor->document());
        scanCursor.movePosition(QTextCursor::Start);
        int replacements = 0;

        scanCursor.beginEditBlock();
        while (true) {
            QTextCursor found = editor->document()->find(findText, scanCursor, flags);
            if (found.isNull()) {
                break;
            }
            found.insertText(replaceLine->text());
            scanCursor = found;
            replacements++;
        }
        scanCursor.endEditBlock();

        editor->setFocus();
        lastFindText = findText;
        lastReplaceText = replaceLine->text();
        statusLabel->setText(QObject::tr("%1 occurrence(s) replaced.").arg(replacements));
    });

    // Keep existing close behavior and modal execution path.
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    findLine->setFocus();
    dialog.exec();
}

// Preserve the parser checker dialog flow and parser success/failure reporting.
void DialogUtilityController::onActionToolsParserGrammarCheckerTriggered() {
    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr) {
        QMessageBox::information(_ownerWidget, QObject::tr("Parser Grammar Checker"), QObject::tr("Open or create a model before checking parser expressions."));
        return;
    }

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Parser Grammar Checker"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* expressionEditor = new QPlainTextEdit(&dialog);
    expressionEditor->setPlaceholderText(QObject::tr("Type an expression to validate, e.g. UNIF(1,5) + 2."));
    if (_ui->TextCodeEditor != nullptr) {
        const QString selectedText = _ui->TextCodeEditor->textCursor().selectedText().trimmed();
        if (!selectedText.isEmpty()) {
            expressionEditor->setPlainText(selectedText);
        }
    }
    auto* resultLabel = new QLabel(QObject::tr("Provide an expression and click Check."), &dialog);
    resultLabel->setWordWrap(true);
    auto* checkButton = new QPushButton(QObject::tr("Check"), &dialog);
    auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);

    layout->addWidget(new QLabel(QObject::tr("Expression:"), &dialog));
    layout->addWidget(expressionEditor);
    layout->addWidget(resultLabel);
    layout->addWidget(checkButton);
    layout->addWidget(closeButtons);

    // Keep parser validation callback behavior and message-box semantics unchanged.
    QObject::connect(checkButton, &QPushButton::clicked, &dialog, [this, model, expressionEditor, resultLabel]() {
        const std::string expression = expressionEditor->toPlainText().trimmed().toStdString();
        if (expression.empty()) {
            resultLabel->setText(QObject::tr("Please enter an expression before checking."));
            return;
        }
        bool success = false;
        std::string errorMessage;
        const double value = model->parseExpression(expression, success, errorMessage);
        if (success) {
            resultLabel->setText(QObject::tr("Parse success. Evaluated value: %1").arg(value));
            QMessageBox::information(_ownerWidget, QObject::tr("Parser Grammar Checker"), QObject::tr("Expression is valid.\nEvaluated value: %1").arg(value));
        } else {
            const QString errorText = QString::fromStdString(errorMessage.empty() ? std::string("Unknown parser error.") : errorMessage);
            resultLabel->setText(QObject::tr("Parse failed: %1").arg(errorText));
            QMessageBox::warning(_ownerWidget, QObject::tr("Parser Grammar Checker"), QObject::tr("Expression is invalid.\nError: %1").arg(errorText));
        }
    });
    QObject::connect(closeButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

// Preserve optimizer setup dialog behavior and persisted precision/max-step fields.
void DialogUtilityController::onActionToolsOptimizatorTriggered() {
    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Optimizator"));
    auto* layout = new QFormLayout(&dialog);
    auto* precisionInput = new QDoubleSpinBox(&dialog);
    precisionInput->setDecimals(10);
    precisionInput->setRange(1e-10, 1.0);
    precisionInput->setValue(_optimizerPrecision);
    auto* maxStepsInput = new QSpinBox(&dialog);
    maxStepsInput->setRange(10, 10000000);
    maxStepsInput->setValue(static_cast<int>(_optimizerMaxSteps));
    auto* minInput = new QDoubleSpinBox(&dialog);
    minInput->setRange(-1e6, 1e6);
    minInput->setValue(0.0);
    auto* maxInput = new QDoubleSpinBox(&dialog);
    maxInput->setRange(-1e6, 1e6);
    maxInput->setValue(1.0);
    auto* resultLabel = new QLabel(QObject::tr("Run to evaluate ∫x² dx in the informed interval."), &dialog);
    resultLabel->setWordWrap(true);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addRow(QObject::tr("Precision"), precisionInput);
    layout->addRow(QObject::tr("Max steps"), maxStepsInput);
    layout->addRow(QObject::tr("Integral min"), minInput);
    layout->addRow(QObject::tr("Integral max"), maxInput);
    layout->addRow(resultLabel);
    layout->addRow(buttons);

    // Keep accepted callback behavior that persists settings and computes sample integral.
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, precisionInput, maxStepsInput, minInput, maxInput, resultLabel]() {
        if (maxInput->value() <= minInput->value()) {
            resultLabel->setText(QObject::tr("Integral max must be greater than min."));
            return;
        }
        _optimizerPrecision = precisionInput->value();
        _optimizerMaxSteps = static_cast<unsigned int>(maxStepsInput->value());
        SolverDefaultImpl1 solver(_optimizerPrecision, _optimizerMaxSteps);
        auto quadratic = [](double x, double) { return x * x; };
        const double result = solver.integrate(minInput->value(), maxInput->value(), quadratic, 0.0);
        resultLabel->setText(QObject::tr("Configuration saved. Integral result: %1").arg(result));
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

// Preserve dataset analyzer flow, persisted last path, and summary export/status-bar behavior.
void DialogUtilityController::onActionToolsDataAnalyzerTriggered() {
    const QString initialPath = _lastDataAnalyzerPath.isEmpty() ? QDir::currentPath() : _lastDataAnalyzerPath;
    const QString fileName = QFileDialog::getOpenFileName(
        _ownerWidget,
        QObject::tr("Open Dataset"),
        initialPath,
        QObject::tr("Data files (*.csv *.txt *.dat);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) {
        return;
    }
    _lastDataAnalyzerPath = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(_ownerWidget, QObject::tr("Data Analyzer"), QObject::tr("Could not open selected file."));
        return;
    }

    QTextStream stream(&file);
    QList<double> numericValues;
    QStringList previewLines;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (previewLines.size() < 10) {
            previewLines << line;
        }
        const QStringList tokens = line.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
        for (const QString& token : tokens) {
            bool ok = false;
            const double value = token.toDouble(&ok);
            if (ok) {
                numericValues << value;
            }
        }
    }
    file.close();

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Data Analyzer"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* summaryLabel = new QLabel(&dialog);
    summaryLabel->setWordWrap(true);
    auto* preview = new QTextEdit(&dialog);
    preview->setReadOnly(true);
    auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto* exportSummaryButton = new QPushButton(QObject::tr("Save Summary"), &dialog);

    if (numericValues.isEmpty()) {
        summaryLabel->setText(QObject::tr("No numeric values were detected in the selected file."));
    } else {
        double minValue = numericValues.first();
        double maxValue = numericValues.first();
        double sum = 0.0;
        for (double value : numericValues) {
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
            sum += value;
        }
        const double mean = sum / static_cast<double>(numericValues.size());
        double squaredDiffSum = 0.0;
        for (double value : numericValues) {
            const double diff = value - mean;
            squaredDiffSum += diff * diff;
        }
        const double standardDeviation = std::sqrt(squaredDiffSum / static_cast<double>(numericValues.size()));
        summaryLabel->setText(QObject::tr("File: %1\nNumeric samples: %2\nMin: %3\nMax: %4\nMean: %5\nStd Dev: %6")
                              .arg(fileName)
                              .arg(numericValues.size())
                              .arg(minValue)
                              .arg(maxValue)
                              .arg(mean)
                              .arg(standardDeviation));
    }
    preview->setPlainText(previewLines.join("\n"));

    layout->addWidget(summaryLabel);
    layout->addWidget(new QLabel(QObject::tr("Preview (first 10 lines):"), &dialog));
    layout->addWidget(preview);
    layout->addWidget(exportSummaryButton);
    layout->addWidget(closeButtons);

    // Keep summary save flow and status-bar message semantics unchanged.
    QObject::connect(exportSummaryButton, &QPushButton::clicked, &dialog, [this, summaryLabel]() {
        const QString exportPath = QFileDialog::getSaveFileName(
            _ownerWidget,
            QObject::tr("Save Data Analyzer Summary"),
            QDir::currentPath() + "/data-analyzer-summary.txt",
            QObject::tr("Text files (*.txt);;All files (*.*)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);
        if (exportPath.isEmpty()) {
            return;
        }
        QFile outputFile(exportPath);
        if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(_ownerWidget, QObject::tr("Data Analyzer"), QObject::tr("Could not save summary file."));
            return;
        }
        QTextStream out(&outputFile);
        out << summaryLabel->text() << "\n";
        outputFile.close();
        _ownerWidget->statusBar()->showMessage(QObject::tr("Data analysis summary saved to %1").arg(exportPath), 4000);
    });
    QObject::connect(closeButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

// Preserve view-configuration workflow and existing action-based synchronization path.
void DialogUtilityController::onActionViewConfigureTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Configure View"));
    auto* layout = new QFormLayout(&dialog);
    auto* showGrid = new QCheckBox(QObject::tr("Show grid"), &dialog);
    auto* showRule = new QCheckBox(QObject::tr("Show ruler"), &dialog);
    auto* showGuides = new QCheckBox(QObject::tr("Show guides"), &dialog);
    auto* snapToGrid = new QCheckBox(QObject::tr("Snap to grid"), &dialog);
    auto* gridInterval = new QSpinBox(&dialog);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    showGrid->setChecked(scene->isGridVisible());
    showRule->setChecked(_graphicsView->isRuleVisible());
    showGuides->setChecked(_graphicsView->isGuidesVisible());
    snapToGrid->setChecked(_ui->actionShowSnap->isChecked());
    gridInterval->setRange(5, 200);
    gridInterval->setValue(static_cast<int>(scene->grid()->interval));

    layout->addRow(showGrid);
    layout->addRow(showRule);
    layout->addRow(showGuides);
    layout->addRow(snapToGrid);
    layout->addRow(QObject::tr("Grid interval"), gridInterval);
    layout->addRow(buttons);

    // Keep the same application path by toggling existing QAction handlers.
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, scene, showGrid, showRule, showGuides, snapToGrid, gridInterval, &dialog]() {
        scene->grid()->interval = static_cast<unsigned int>(gridInterval->value());
        if (scene->isGridVisible()) {
            scene->setGridVisible(false);
            scene->setGridVisible(true);
        }
        _ui->actionShowGrid->setChecked(showGrid->isChecked());
        _ui->actionShowGrid->trigger();
        _ui->actionShowRule->setChecked(showRule->isChecked());
        _ui->actionShowRule->trigger();
        _ui->actionShowGuides->setChecked(showGuides->isChecked());
        _ui->actionShowGuides->trigger();
        _ui->actionShowSnap->setChecked(snapToGrid->isChecked());
        scene->setSnapToGrid(snapToGrid->isChecked());
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

// Preserve simulator preferences dialog parenting and non-blocking show behavior.
void DialogUtilityController::onActionSimulatorPreferencesTriggered() {
    DialogSystemPreferences* dialog = new DialogSystemPreferences(_ownerWidget);
    dialog->show();
}

// Preserve plugin manager dialog parenting and non-blocking show behavior.
void DialogUtilityController::onActionSimulatorsPluginManagerTriggered() {
    DialogPluginManager* dialog = new DialogPluginManager(_ownerWidget);
    dialog->show();
}

// Preserve breakpoint insertion flow and post-update refresh behavior.
void DialogUtilityController::onPushButtonBreakpointInsertClicked() {
    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    dialogBreakpoint dialog;
    dialog.setModal(true);
    dialog.setMVCModel(_simulator);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    std::unique_ptr<dialogBreakpoint::MVCResult> result(dialog.getMVCResult());
    if (!result) {
        return;
    }

    if (result->type == "Time") {
        const double onTime = QString::fromStdString(result->on).toDouble();
        if (sim->getBreakpointsOnTime()->find(onTime) == sim->getBreakpointsOnTime()->list()->end()) {
            sim->getBreakpointsOnTime()->insert(onTime);
        }
    } else if (result->type == "Entity") {
        ModelDataDefinition* dataDef = model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), result->on);
        Entity* entity = dynamic_cast<Entity*> (dataDef);
        if (entity != nullptr && sim->getBreakpointsOnEntity()->find(entity) == sim->getBreakpointsOnEntity()->list()->end()) {
            sim->getBreakpointsOnEntity()->insert(entity);
        }
    } else if (result->type == "Component") {
        ModelComponent* comp = model->getComponentManager()->find(result->on);
        if (comp != nullptr && sim->getBreakpointsOnComponent()->find(comp) == sim->getBreakpointsOnComponent()->list()->end()) {
            sim->getBreakpointsOnComponent()->insert(comp);
        }
    }

    _actualizeDebugBreakpoints(true);
}

// Preserve breakpoint removal workflow based on current selection and typed breakpoint target.
void DialogUtilityController::onPushButtonBreakpointRemoveClicked() {
    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    int row = _ui->tableWidget_Breakpoints->currentRow();
    if (row < 0 && _ui->tableWidget_Breakpoints->selectionModel() != nullptr) {
        const QModelIndexList selectedRows = _ui->tableWidget_Breakpoints->selectionModel()->selectedRows();
        if (!selectedRows.isEmpty()) {
            row = selectedRows.first().row();
        }
    }
    if (row < 0) {
        return;
    }
    QTableWidgetItem* typeItem = _ui->tableWidget_Breakpoints->item(row, 1);
    QTableWidgetItem* onItem = _ui->tableWidget_Breakpoints->item(row, 2);
    if (typeItem == nullptr || onItem == nullptr) {
        return;
    }

    const std::string type = typeItem->text().toStdString();
    const std::string on = onItem->text().toStdString();
    if (type == "Time") {
        sim->getBreakpointsOnTime()->remove(QString::fromStdString(on).toDouble());
    } else if (type == "Entity") {
        ModelDataDefinition* dataDef = model->getDataManager()->getDataDefinition(Util::TypeOf<Entity>(), on);
        Entity* entity = dynamic_cast<Entity*> (dataDef);
        if (entity != nullptr) {
            sim->getBreakpointsOnEntity()->remove(entity);
        }
    } else if (type == "Component") {
        ModelComponent* comp = model->getComponentManager()->find(on);
        if (comp != nullptr) {
            sim->getBreakpointsOnComponent()->remove(comp);
        }
    }

    _actualizeDebugBreakpoints(true);
}

// Preserve export workflow, including image fallback render and save-format handling.
void DialogUtilityController::onPushButtonExportClicked() {
    QPixmap modelPixmap = _ui->label_ModelGraphic->pixmap();
    if (modelPixmap.isNull()) {
        _createModelImage();
        modelPixmap = _ui->label_ModelGraphic->pixmap();
    }

    if (modelPixmap.isNull()) {
        ModelGraphicsScene* scene = _graphicsView->getScene();
        if (scene == nullptr || scene->items().isEmpty()) {
            QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("There is no diagram/image available to export."));
            return;
        }

        QRectF bounds = scene->itemsBoundingRect();
        if (!bounds.isValid() || bounds.isEmpty()) {
            QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("There is no diagram/image available to export."));
            return;
        }

        QImage image(bounds.size().toSize() + QSize(20, 20), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.translate(-bounds.topLeft() + QPointF(10.0, 10.0));
        scene->render(&painter);
        painter.end();
        modelPixmap = QPixmap::fromImage(image);
    }

    const QString defaultName = QDir::currentPath() + "/model-diagram.png";
    const QString filters = QObject::tr("PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;Bitmap Image (*.bmp)");
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(_ownerWidget, QObject::tr("Export Diagram"), defaultName, filters, &selectedFilter, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) {
        return;
    }

    QString format = "PNG";
    if (selectedFilter.contains("*.jpg") || selectedFilter.contains("*.jpeg")) {
        format = "JPG";
    } else if (selectedFilter.contains("*.bmp")) {
        format = "BMP";
    }

    if (QFileInfo(fileName).suffix().isEmpty()) {
        fileName += "." + format.toLower();
    }

    if (!modelPixmap.save(fileName, format.toStdString().c_str())) {
        QMessageBox::warning(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("Could not export diagram to file."));
        return;
    }

    QMessageBox::information(_ownerWidget, QObject::tr("Export Diagram"), QObject::tr("Diagram exported successfully."));
}

// Preserve parallelization dialog flow and persisted in-session settings behavior.
void DialogUtilityController::onActionParallelizationTriggered() {
    QDialog dialog(_ownerWidget);
    dialog.setWindowTitle(QObject::tr("Parallelization"));
    auto* layout = new QFormLayout(&dialog);
    auto* enabled = new QCheckBox(QObject::tr("Enable parallel execution preparation"), &dialog);
    auto* threads = new QSpinBox(&dialog);
    auto* batchSize = new QSpinBox(&dialog);
    auto* statusLabel = new QLabel(&dialog);
    statusLabel->setWordWrap(true);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    enabled->setChecked(_parallelizationEnabled);
    threads->setRange(1, 256);
    threads->setValue(_parallelizationThreads);
    batchSize->setRange(1, 1000000);
    batchSize->setValue(_parallelizationBatchSize);

    layout->addRow(enabled);
    layout->addRow(QObject::tr("Worker threads"), threads);
    layout->addRow(QObject::tr("Batch size"), batchSize);
    layout->addRow(statusLabel);
    layout->addRow(buttons);

    // Keep accepted callback behavior and status-bar messaging unchanged.
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, enabled, threads, batchSize, statusLabel, &dialog]() {
        _parallelizationEnabled = enabled->isChecked();
        _parallelizationThreads = threads->value();
        _parallelizationBatchSize = batchSize->value();
        statusLabel->setText(QObject::tr("Configuration saved: enabled=%1, threads=%2, batch size=%3")
                             .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                             .arg(_parallelizationThreads)
                             .arg(_parallelizationBatchSize));
        _ownerWidget->statusBar()->showMessage(QObject::tr("Parallelization prepared: enabled=%1, threads=%2, batch size=%3")
                                               .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                                               .arg(_parallelizationThreads)
                                               .arg(_parallelizationBatchSize), 5000);
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}
