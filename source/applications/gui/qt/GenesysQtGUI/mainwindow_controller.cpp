#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialogs/dialogBreakpoint.h"
#include "dialogs/Dialogmodelinformation.h"
#include "dialogs/dialogsimulationconfigure.h"
#include "dialogs/dialogpluginmanager.h"
#include "dialogs/dialogsystempreferences.h"
#include "dialogs/DialogFind.h"
#include "controllers/SimulationController.h"

#include "actions/DeleteUndoCommand.h"
#include "actions/PasteUndoCommand.h"

// std
#include <string>
#include <fstream>
#include <memory>
#include <algorithm>
#include <cmath>
//#include <sstream>
#include <cstdlib>
//#include <streambuf>

// QT
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
// #include <qt5/QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsitem.h>
#include <QGraphicsScene>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSignalBlocker>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QStatusBar>
#include <QImage>
#include <QPainter>
#include <QFileInfo>
#include <QCoreApplication>
#include "../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../tools/SolverDefaultImpl1.h"


//-------------------------
// PRIVATE SLOTS
//-------------------------

// -------------------------------------------------
//  menu actions
// -------------------------------------------------

/**
 * @brief Stops the active simulation execution.
 *
 * Guarded by SimulationController to avoid dereferencing null model/simulation.
 *
 * @todo Move command execution details (animation flags + console command) into SimulationController.
 */
void MainWindow::on_actionSimulationStop_triggered() {
    if (!_simulationController || !_simulationController->hasCurrentModelSimulation()) {
        return;
    }
    ModelSimulation* simulation = _simulationController->currentSimulation();
    if (simulation == nullptr) {
        return;
    }

    AnimationTransition::setRunning(false);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("stop");

    simulation->stop();

    _actualizeActions();
}

/**
 * @brief Starts simulation after readiness validation.
 *
 * Preconditions:
 * - Current model/simulation must exist;
 * - Model may be checked when needed;
 * - Textual model representation must be synchronized.
 *
 * @todo Replace lambda callbacks by explicit command objects for better unit testing.
 */
void MainWindow::on_actionSimulationStart_triggered() {
    if (!_simulationController || !_simulationController->ensureReady(
            true,
            _modelCheked,
            [this]() { return _check(false); },
            [this]() { return _setSimulationModelBasedOnText(); })) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    if (simulation == nullptr) {
        return;
    }

    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("start");
    simulation->start();
}

/**
 * @brief Executes one simulation step after readiness validation.
 *
 * Uses the same precondition pipeline as start command.
 *
 * @todo Consolidate duplicated animation-toggle code between start and step.
 */
void MainWindow::on_actionSimulationStep_triggered() {
    if (!_simulationController || !_simulationController->ensureReady(
            true,
            _modelCheked,
            [this]() { return _check(false); },
            [this]() { return _setSimulationModelBasedOnText(); })) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    if (simulation == nullptr) {
        return;
    }

    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("step");
    simulation->step();
}

/**
 * @brief Pauses running simulation.
 *
 * @todo Add explicit feedback when pause is requested in invalid state.
 */
void MainWindow::on_actionSimulationPause_triggered() {
    if (!_simulationController || !_simulationController->hasCurrentModelSimulation()) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    if (simulation == nullptr) {
        return;
    }

    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(true);

    _insertCommandInConsole("pause");
    simulation->pause();
}

/**
 * @brief Resumes simulation execution after readiness validation.
 *
 * @todo Introduce a dedicated `resume()` API in kernel side when available
 *       to avoid semantic coupling with `start()`.
 */
void MainWindow::on_actionSimulationResume_triggered() {
    if (!_simulationController || !_simulationController->ensureReady(
            false,
            _modelCheked,
            [this]() { return _check(false); },
            [this]() { return _setSimulationModelBasedOnText(); })) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    if (simulation == nullptr) {
        return;
    }

    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("resume");
    simulation->start();
}


void MainWindow::on_actionAboutAbout_triggered() {
    QMessageBox::about(this, "About Genesys", "Genesys is a result of teaching and research activities of Professor Dr. Ing Rafael Luiz Cancian. It began in early 2002 as a way to teach students the basics and simulation techniques of systems implemented by other comercial simulation tools, such as Arena. In Genesys development he replicated all the SIMAN language, used by Arena software, and Genesys has become a clone of that tool, including its graphical interface. Genesys allowed the inclusion of new simulation components through dynamic link libraries and also the parallel execution of simulation models in a distributed environment. The development of Genesys continued until 2009, when the professor stopped teaching systems simulation classes. Ten years later the professor starts again to teach systems simulation classes and to carry out scientific research in the area. So in 2019 Genesys is reborn, with new language and programming techniques, and even more ambitious goals.");
}

void MainWindow::on_actionAboutLicence_triggered() {
    LicenceManager* licman = simulator->getLicenceManager();
    std::string text = licman->showLicence() + "\n";
    text += licman->showLimits() + "\n";
    text += licman->showActivationCode();
    QMessageBox::about(this, "About Licence", QString::fromStdString(text));
}

void MainWindow::on_actionAboutGetInvolved_triggered() {
    QMessageBox::about(this, "Get Involved", "Genesys is a free open-source simulator (and tools) available at 'https://github.com/rlcancian/Genesys-Simulator'. Help us by submiting your pull requests containing code improvements. Contact: rafael.cancian@ufsc.br");
}

void MainWindow::on_actionEditUndo_triggered() {
    if (ui->graphicsView->getScene()->getUndoStack()) {
        ui->graphicsView->getScene()->getUndoStack()->undo();
    }
}


void MainWindow::on_actionEditRedo_triggered() {
    if (ui->graphicsView->getScene()->getUndoStack()) {
        ui->graphicsView->getScene()->getUndoStack()->redo();
    }
}


void MainWindow::on_actionEditFind_triggered() {
    // Cria um novo diálogo para Buscar componentes
    DialogFind *find = new DialogFind(this, ui->graphicsView->getScene());

    // Mostra esse dialogo na tela
    find->show();

    if (find->exec() == QDialog::Accepted) find->setFocus();
}


// void MainWindow::on_actionReplace_triggered() {
//     _showMessageNotImplemented();
// }
void MainWindow::on_actionEditReplace_triggered() {
    if (ui->TextCodeEditor == nullptr) {
        return;
    }

    auto* editor = ui->TextCodeEditor;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Replace"));
    dialog.setModal(false);

    auto* layout = new QVBoxLayout(&dialog);
    auto* formLayout = new QFormLayout();
    auto* findLine = new QLineEdit(&dialog);
    auto* replaceLine = new QLineEdit(&dialog);
    auto* caseSensitive = new QCheckBox(tr("Case sensitive"), &dialog);
    auto* statusLabel = new QLabel(&dialog);
    statusLabel->setWordWrap(true);
    statusLabel->setText(tr("Ready."));

    static QString lastFindText;
    static QString lastReplaceText;
    findLine->setText(lastFindText);
    replaceLine->setText(lastReplaceText);

    formLayout->addRow(tr("Find:"), findLine);
    formLayout->addRow(tr("Replace with:"), replaceLine);

    auto* buttonFindNext = new QPushButton(tr("Find Next"), &dialog);
    auto* buttonReplace = new QPushButton(tr("Replace"), &dialog);
    auto* buttonReplaceAll = new QPushButton(tr("Replace All"), &dialog);
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

    auto findNext = [&]() -> bool {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(tr("Enter text to find."));
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
            statusLabel->setText(tr("Occurrence selected."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            return true;
        }

        statusLabel->setText(tr("No occurrences found."));
        return false;
    };

    connect(buttonFindNext, &QPushButton::clicked, &dialog, [&]() {
        findNext();
    });

    connect(buttonReplace, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(tr("Enter text to find."));
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
            statusLabel->setText(tr("Occurrence replaced."));
            lastFindText = findText;
            lastReplaceText = replaceLine->text();
            findNext();
        }
    });

    connect(buttonReplaceAll, &QPushButton::clicked, &dialog, [&]() {
        const QString findText = findLine->text();
        if (findText.isEmpty()) {
            statusLabel->setText(tr("Enter text to find."));
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
        statusLabel->setText(tr("%1 occurrence(s) replaced.").arg(replacements));
    });

    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    findLine->setFocus();
    dialog.exec();
}


void MainWindow::on_actionEditCut_triggered() {
    _gmc_copies->clear();
    _ports_copies->clear();
    _group_copy->clear();
    _draw_copy->clear();

    QList<QGraphicsItem *> selecteds =  ui->graphicsView->scene()->selectedItems();

    // Verifica se tem itens selecionados
    if (selecteds.size() > 0) {

        // Pega a cena
        ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->getScene());

        // Seta o cut
        _cut = true;

        // Adiciona na lista de cópias (conexões, componentes e desenhos)
        foreach (QGraphicsItem *item , ui->graphicsView->scene()->selectedItems()) {
            QList<GraphicalModelComponent*> groupComponents  = QList<GraphicalModelComponent*>();
            QList<GraphicalConnection*> * connGroup = new QList<GraphicalConnection*>();

            // Tenta transformar em um componente gráfico de modelo
            if (GraphicalModelComponent *gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                // Adiciona em uma lista de cópias de componentes
                _gmc_copies->append(gmc);
            }
            else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(group->childItems().at(i));

                    if (!component->getGraphicalInputPorts().empty() && !component->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                        for (int j = 0; j < component->getGraphicalInputPorts().at(0)->getConnections()->size(); ++j) {
                            connGroup->append(component->getGraphicalInputPorts().at(0)->getConnections()->at(j));
                        }
                    }

                    for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
                        GraphicalComponentPort *port = component->getGraphicalOutputPorts().at(j);

                        if (!port->getConnections()->empty()) {
                            connGroup->append(port->getConnections()->at(0));
                        }
                    }

                    _gmc_copies->append(component);
                    groupComponents.append(component);
                }
                saveItemForCopy(&groupComponents, connGroup);

                _group_copy->append(group);
                ui->graphicsView->getScene()->insertComponentGroup(group, groupComponents);
                for (unsigned int k = 0; k < (unsigned int) connGroup->size(); k++) {
                    _ports_copies->append(connGroup->at(k));
                }
            } else if (GraphicalConnection *port = dynamic_cast<GraphicalConnection*>(item)) {
                _ports_copies->append(port);
            } else {
                _draw_copy->append(item);
            }

            delete connGroup;
        }

        // Removendo as conexoes do modelo e graficamente
        // Só não é removido a conexão quando todos os itens estão selecionados
        // (2x componentes e a conexão (similar ao arena)
        saveItemForCopy(_gmc_copies, _ports_copies);

        QUndoCommand *deleteUndoCommand = new DeleteUndoCommand(selecteds, scene);
        scene->getUndoStack()->push(deleteUndoCommand);

    }
}

void MainWindow::on_actionEditCopy_triggered() {
    _gmc_copies->clear();
    _ports_copies->clear();
    _group_copy->clear();
    _draw_copy->clear();

    QList<QGraphicsItem*> selected = ui->graphicsView->scene()->selectedItems();
    QList<GraphicalModelComponent *> gmc_copies_copy = QList<GraphicalModelComponent *>();

    // verifica se tem itens selecionados
    if (selected.size() > 0) {

        // seta o cut
        _cut = false;

        // adiciona na lista de cópias (conexões, componentes e desenhos)
        foreach (QGraphicsItem *item , ui->graphicsView->scene()->selectedItems()) {
            // verifica se é um componente gráfico
            if (GraphicalModelComponent *gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                // Adiciona em uma lista de cópias de componentes
                gmc->setSelected(false);
                _gmc_copies->append(gmc);
                gmc_copies_copy.append(gmc);
            }
            // verifica se é uma conexão gráfica
            else if (GraphicalConnection *conn = dynamic_cast<GraphicalConnection*>(item)) {
                conn->setSelected(false);
                _ports_copies->append(conn);
            }
            // verifica se é um grupo
            else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                group->setSelected(false);
                _group_copy->append(group);

                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(group->childItems().at(i));

                    gmc_copies_copy.append(component);
                }
            }
            // se não for nenhum deles é um desenho na tela
            else {
                item->setSelected(false);
                _draw_copy->append(item);
            }
        }

        // removendo as conexões em que os seus componentes não foram selecionados
        saveItemForCopy(&gmc_copies_copy, _ports_copies);

        // limpa a lista auxiliar
        gmc_copies_copy.clear();
    }

}

void MainWindow::on_actionEditPaste_triggered() {

    // se tiver componente copiados
    if (_gmc_copies->size() > 0 || _draw_copy->size() > 0 || _group_copy->size() > 0) {

        // pega a cena
        ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->getScene());

        // se não for ação de recorte chama o auxiliar do copy
        if (!_cut) {
            _helpCopy();
        }

        // cola na cena o que foi copiado
        QUndoCommand *pasteUndoCommand = new PasteUndoCommand(_gmc_copies, _ports_copies, _group_copy, _draw_copy, scene);
        scene->getUndoStack()->push(pasteUndoCommand);

        // limpa todas as listas
        _gmc_copies->clear();
        _ports_copies->clear();
        _draw_copy->clear();
        _group_copy->clear();
        _cut = false;
    }
}


void MainWindow::on_actionShowGrid_triggered() {
    // Aplica o estado de grid da action diretamente na cena para evitar inversão por toggle.
    ui->graphicsView->getScene()->setGridVisible(ui->actionShowGrid->isChecked());
}


void MainWindow::on_actionShowRule_triggered() {
    // Applies the requested ruler state and then mirrors the effective backend state into the QAction.
    const bool requestedVisible = ui->actionShowRule->isChecked();
    ui->graphicsView->setRuleVisible(requestedVisible);
    ui->actionShowRule->setChecked(ui->graphicsView->isRuleVisible());
}


void MainWindow::on_actionShowGuides_triggered() {
    // Applies the requested guides state and then mirrors the effective backend state into the QAction.
    const bool requestedVisible = ui->actionShowGuides->isChecked();
    ui->graphicsView->setGuidesVisible(requestedVisible);
    ui->actionShowGuides->setChecked(ui->graphicsView->isGuidesVisible());
}


void MainWindow::on_actionZoom_In_triggered() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value+TraitsGUI<GMainWindow>::zoomButtonChange);
}


void MainWindow::on_actionZoom_Out_triggered() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value-TraitsGUI<GMainWindow>::zoomButtonChange);

}


void MainWindow::on_actionZoom_All_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (scene == nullptr || scene->items().isEmpty()) {
        return;
    }

    const QRectF bounds = scene->itemsBoundingRect();
    if (!bounds.isValid() || bounds.isEmpty()) {
        return;
    }

    ui->graphicsView->resetTransform();
    ui->graphicsView->fitInView(bounds.adjusted(-20.0, -20.0, 20.0, 20.0), Qt::KeepAspectRatio);
    {
        QSignalBlocker blocker(ui->horizontalSlider_ZoomGraphical);
        _zoomValue = ui->horizontalSlider_ZoomGraphical->maximum() / 2;
        ui->horizontalSlider_ZoomGraphical->setValue(_zoomValue);
    }
    ui->graphicsView->centerOn(bounds.center());
}


void MainWindow::on_actionDrawLine_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawLine->isChecked()) {
        ui->graphicsView->setCursor(Qt::SizeHorCursor);
        ui->actionDrawLine->setChecked(true);
        // Ative a ferramenta de desenho de linha
        scene->setAction(ui->actionDrawLine);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::LINE); // Enumeração que representa o modo de desenho de linha
    } else {
        unselectDrawIcons();
    }
}


void MainWindow::on_actionDrawRectangle_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    // Ative a ferramenta de desenho de retangulo
    if (!checkSelectedDrawIcons() && ui->actionDrawRectangle->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->actionDrawRectangle->setChecked(true);
        scene->setAction(ui->actionDrawRectangle);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::RECTANGLE);
    } else {
        unselectDrawIcons();
    }
}


void MainWindow::on_actionDrawEllipse_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    // Ative a ferramenta de desenho de ellipse
    if (!checkSelectedDrawIcons() && ui->actionDrawEllipse->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->actionDrawEllipse->setChecked(true);
        scene->setAction(ui->actionDrawEllipse);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::ELLIPSE);
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionDrawText_triggered()
{
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawText->isChecked()) {
        ui->actionDrawText->setChecked(true);
        scene->setAction(ui->actionDrawText);
        // Ative a ferramenta de desenho do texto
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::TEXT);
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionDrawPoligon_triggered()
{
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawPoligon->isChecked()) {
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        ui->actionDrawPoligon->setChecked(true);
        scene->setAction(ui->actionDrawPoligon);
        // Ative a ferramenta de desenho do polygon
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::POLYGON);
    } else {
        unselectDrawIcons();
    }
}




void MainWindow::on_actionAnimateExpression_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateResource_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateQueue_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateStation_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditDelete_triggered()
{
    QList<QGraphicsItem *> selecteds = ui->graphicsView->scene()->selectedItems();
    if (selecteds.isEmpty()) {
        return;
    }

    ModelGraphicsScene *scene = ui->graphicsView->getScene();
    QUndoCommand *deleteUndoCommand = new DeleteUndoCommand(selecteds, scene);
    scene->getUndoStack()->push(deleteUndoCommand);
    _actualizeActions();
}


void MainWindow::on_actionSimulatorPreferences_triggered()
{
    DialogSystemPreferences* dialog = new DialogSystemPreferences(this);
    dialog->show();
}


void MainWindow::on_actionAlignMiddle_triggered()
{
    // Legacy slot kept for compatibility with old .ui action names.
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignTop_triggered()
{
    // Legacy slot kept for compatibility with old .ui action names.
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignRight_triggered()
{
    // Legacy slot kept for compatibility with old .ui action names.
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignCenter_triggered()
{
    // Legacy slot kept for compatibility with old .ui action names.
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignLeft_triggered()
{
    // Legacy slot kept for compatibility with old .ui action names.
    _showMessageNotImplemented();
}

void MainWindow::on_actionAnimateCounter_triggered()
{
    if (!checkSelectedDrawIcons() && ui->actionAnimateCounter->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateCounter);
        myScene()->drawingCounter();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateVariable_triggered() {
    if (!checkSelectedDrawIcons() && ui->actionAnimateVariable->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateVariable);
        myScene()->drawingVariable();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateSimulatedTime_triggered()
{
    if (!checkSelectedDrawIcons() && ui->actionAnimateSimulatedTime->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateSimulatedTime);
        myScene()->drawingTimer();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateEntity_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateEvent_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateAttribute_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateStatistics_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditGroup_triggered()
{
    on_actionViewGroup_triggered();
}


void MainWindow::on_actionEditUngroup_triggered()
{
    on_actionViewUngroup_triggered();
}


void MainWindow::on_actionToolsParserGrammarChecker_triggered()
{
    // Opens a parser checker dialog and validates expressions through the model parser integration.
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        QMessageBox::information(this, tr("Parser Grammar Checker"), tr("Open or create a model before checking parser expressions."));
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Parser Grammar Checker"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* expressionEditor = new QPlainTextEdit(&dialog);
    expressionEditor->setPlaceholderText(tr("Type an expression to validate, e.g. UNIF(1,5) + 2."));
    if (ui->TextCodeEditor != nullptr) {
        const QString selectedText = ui->TextCodeEditor->textCursor().selectedText().trimmed();
        if (!selectedText.isEmpty()) {
            expressionEditor->setPlainText(selectedText);
        }
    }
    auto* resultLabel = new QLabel(tr("Provide an expression and click Check."), &dialog);
    resultLabel->setWordWrap(true);
    auto* checkButton = new QPushButton(tr("Check"), &dialog);
    auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);

    layout->addWidget(new QLabel(tr("Expression:"), &dialog));
    layout->addWidget(expressionEditor);
    layout->addWidget(resultLabel);
    layout->addWidget(checkButton);
    layout->addWidget(closeButtons);

    // Executes parser validation and reports success/failure without closing the checker dialog.
    connect(checkButton, &QPushButton::clicked, &dialog, [this, model, expressionEditor, resultLabel]() {
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
            QMessageBox::information(this, QObject::tr("Parser Grammar Checker"), QObject::tr("Expression is valid.\nEvaluated value: %1").arg(value));
        } else {
            const QString errorText = QString::fromStdString(errorMessage.isEmpty() ? std::string("Unknown parser error.") : errorMessage);
            resultLabel->setText(QObject::tr("Parse failed: %1").arg(errorText));
            QMessageBox::warning(this, QObject::tr("Parser Grammar Checker"), QObject::tr("Expression is invalid.\nError: %1").arg(errorText));
        }
    });
    connect(closeButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}


void MainWindow::on_actionToolsExperimentation_triggered()
{
    // Legacy slot: action is not exposed in current mainwindow.ui.
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsOptimizator_triggered()
{
    // Provides a minimal optimization preparation dialog based on numeric solver parameters.
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Optimizator"));
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
    auto* resultLabel = new QLabel(tr("Run to evaluate ∫x² dx in the informed interval."), &dialog);
    resultLabel->setWordWrap(true);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addRow(tr("Precision"), precisionInput);
    layout->addRow(tr("Max steps"), maxStepsInput);
    layout->addRow(tr("Integral min"), minInput);
    layout->addRow(tr("Integral max"), maxInput);
    layout->addRow(resultLabel);
    layout->addRow(buttons);

    // Persists optimizer parameters and runs a sample numeric integration to validate the configured setup.
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, precisionInput, maxStepsInput, minInput, maxInput, resultLabel]() {
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
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}


void MainWindow::on_actionToolsDataAnalyzer_triggered()
{
    // Opens a dataset workflow that loads numeric values and reports descriptive statistics with summary export.
    const QString initialPath = _lastDataAnalyzerPath.isEmpty() ? QDir::currentPath() : _lastDataAnalyzerPath;
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Dataset"),
        initialPath,
        tr("Data files (*.csv *.txt *.dat);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) {
        return;
    }
    // Persists the last visited dataset folder for the next analyzer execution.
    _lastDataAnalyzerPath = QFileInfo(fileName).absolutePath();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Data Analyzer"), tr("Could not open selected file."));
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

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Data Analyzer"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* summaryLabel = new QLabel(&dialog);
    summaryLabel->setWordWrap(true);
    auto* preview = new QTextEdit(&dialog);
    preview->setReadOnly(true);
    auto* closeButtons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto* exportSummaryButton = new QPushButton(tr("Save Summary"), &dialog);

    if (numericValues.isEmpty()) {
        summaryLabel->setText(tr("No numeric values were detected in the selected file."));
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
        // Computes variance and standard deviation to provide a concrete analysis output.
        double squaredDiffSum = 0.0;
        for (double value : numericValues) {
            const double diff = value - mean;
            squaredDiffSum += diff * diff;
        }
        const double standardDeviation = std::sqrt(squaredDiffSum / static_cast<double>(numericValues.size()));
        summaryLabel->setText(tr("File: %1\nNumeric samples: %2\nMin: %3\nMax: %4\nMean: %5\nStd Dev: %6")
                              .arg(fileName)
                              .arg(numericValues.size())
                              .arg(minValue)
                              .arg(maxValue)
                              .arg(mean)
                              .arg(standardDeviation));
    }
    preview->setPlainText(previewLines.join("\n"));

    layout->addWidget(summaryLabel);
    layout->addWidget(new QLabel(tr("Preview (first 10 lines):"), &dialog));
    layout->addWidget(preview);
    layout->addWidget(exportSummaryButton);
    layout->addWidget(closeButtons);
    // Saves the current analysis summary into a text file selected by the user.
    connect(exportSummaryButton, &QPushButton::clicked, &dialog, [this, summaryLabel]() {
        const QString exportPath = QFileDialog::getSaveFileName(
            this,
            tr("Save Data Analyzer Summary"),
            QDir::currentPath() + "/data-analyzer-summary.txt",
            tr("Text files (*.txt);;All files (*.*)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);
        if (exportPath.isEmpty()) {
            return;
        }
        QFile outputFile(exportPath);
        if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Data Analyzer"), tr("Could not save summary file."));
            return;
        }
        QTextStream out(&outputFile);
        out << summaryLabel->text() << "\n";
        outputFile.close();
        statusBar()->showMessage(tr("Data analysis summary saved to %1").arg(exportPath), 4000);
    });
    connect(closeButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}


void MainWindow::on_actionAnimatePlot_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionViewConfigure_triggered()
{
    // Opens a compact view configuration dialog initialized from the effective scene and view backend states.
    ModelGraphicsScene* scene = myScene();
    if (scene == nullptr) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Configure View"));
    auto* layout = new QFormLayout(&dialog);
    auto* showGrid = new QCheckBox(tr("Show grid"), &dialog);
    auto* showRule = new QCheckBox(tr("Show ruler"), &dialog);
    auto* showGuides = new QCheckBox(tr("Show guides"), &dialog);
    auto* snapToGrid = new QCheckBox(tr("Snap to grid"), &dialog);
    auto* gridInterval = new QSpinBox(&dialog);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    showGrid->setChecked(scene->isGridVisible());
    showRule->setChecked(ui->graphicsView->isRuleVisible());
    showGuides->setChecked(ui->graphicsView->isGuidesVisible());
    snapToGrid->setChecked(ui->actionShowSnap->isChecked());
    gridInterval->setRange(5, 200);
    gridInterval->setValue(static_cast<int>(scene->grid()->interval));

    layout->addRow(showGrid);
    layout->addRow(showRule);
    layout->addRow(showGuides);
    layout->addRow(snapToGrid);
    layout->addRow(tr("Grid interval"), gridInterval);
    layout->addRow(buttons);

    // Applies each configured option through existing action/back-end paths to keep menu and view synchronized.
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, scene, showGrid, showRule, showGuides, snapToGrid, gridInterval, &dialog]() {
        scene->grid()->interval = static_cast<unsigned int>(gridInterval->value());
        if (scene->isGridVisible()) {
            scene->setGridVisible(false);
            scene->setGridVisible(true);
        }
        ui->actionShowGrid->setChecked(showGrid->isChecked());
        on_actionShowGrid_triggered();
        ui->actionShowRule->setChecked(showRule->isChecked());
        on_actionShowRule_triggered();
        ui->actionShowGuides->setChecked(showGuides->isChecked());
        on_actionShowGuides_triggered();
        ui->actionShowSnap->setChecked(snapToGrid->isChecked());
        scene->setSnapToGrid(snapToGrid->isChecked());
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

//void MainWindow::on_actionConfigure_triggered() {//?????????????????????????
//}
//void MainWindow::on_actionOpen_triggered() {//?????????????????????????
//}



void MainWindow::on_actionModelNew_triggered() {
    Model* m;
    if ((m = simulator->getModelManager()->current()) != nullptr) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "New Model", "There is a model already oppened. Do you want to close it and to create new model?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        } else {
            this->on_actionModelClose_triggered();
            //return; //@TODO Ceck if needed (since will remove bellow)
        }
    }
    _insertCommandInConsole("new");
    if (m != nullptr) {
        simulator->getModelManager()->remove(m);
    }
    m = simulator->getModelManager()->newModel();
    _initUiForNewModel(m);
}

void MainWindow::on_actionModelOpen_triggered()
{
    Model *m;
    if ((m = simulator->getModelManager()->current()) != nullptr) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("New Model");
        msgBox.setText("There is a model already opened. Do you want to close it and create a new model?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int reply = msgBox.exec();

        if (reply == QMessageBox::No) return;
        else on_actionModelClose_triggered();
    }

    // Obtém o diretório atual
    QString currentDirectory = QDir::currentPath();

    // Navega para o diretório desejado
    QDir parentDir(currentDirectory);

    // Sobe 5 pastas
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();

    // Entra na pasta models
    parentDir.cd("models");

    // Define o diretório inicial como a pasta "models"
    QString initialDirectory = parentDir.absolutePath();

    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Model", initialDirectory,
        tr("Genesys Model (*.gen);;Genesys Graphical User Interface (*.gui);;XML Files (*.xml);;JSON Files (*.json);;C++ Files (*.cpp)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName == "") {
        return;
    }
    _insertCommandInConsole("load " + fileName.toStdString());
    // load Model (in the simulator)
    Model *model = this->_loadGraphicalModel(fileName.toStdString());
    if (model != nullptr) {
        _loaded = true;
        _initUiForNewModel(model);

        QMessageBox::information(this, "Open Model", "Model successfully oppened");
    } else {
        QMessageBox::warning(this, "Open Model", "Error while opening model");
        _actualizeActions();
        _actualizeTabPanes();
    }
    ui->graphicsView->getScene()->getUndoStack()->clear();

}


void MainWindow::on_actionModelSave_triggered()
{
    QObject* triggerSender = sender();
    qInfo() << "on_actionModelSave_triggered sender="
            << (triggerSender ? triggerSender->metaObject()->className() : "nullptr")
            << " objectName=" << (triggerSender ? triggerSender->objectName() : QString())
            << " senderPtr=" << triggerSender
            << " scenePtr=" << (ui && ui->graphicsView ? ui->graphicsView->scene() : nullptr)
            << " modelPtr=" << simulator->getModelManager()->current();
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Model"), _modelfilename,
                                                    tr("Genesys Model (*.gen)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;
    else {
        _insertCommandInConsole("save " + fileName.toStdString());
        QString finalFileName = fileName + ".gen";
        QFile saveFile(finalFileName);

        if (!saveFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to access file to save"),
                                     saveFile.errorString());
            return;
        } else {
            _saveTextModel(&saveFile, ui->TextCodeEditor->toPlainText());
            saveFile.close();
        }
        _saveGraphicalModel(fileName + ".gui");
        _modelfilename = fileName;
        QMessageBox::information(this, "Save Model", "Model successfully saved");
        // convert text info Model
        _setSimulationModelBasedOnText();
        //
        _actualizeModelTextHasChanged(false);
    }
    _actualizeActions();
    ui->graphicsView->getScene()->getUndoStack()->clear();
}



void MainWindow::on_actionModelClose_triggered()
{
    qInfo() << "on_actionModelClose_triggered scenePtr="
            << (ui && ui->graphicsView ? ui->graphicsView->scene() : nullptr)
            << " modelPtr=" << simulator->getModelManager()->current();
    _disconnectSceneSignals("on_actionModelClose_triggered(begin)");
    if (_textModelHasChanged || simulator->getModelManager()->current()->hasChanged()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Close ModelSyS");
        msgBox.setText("Model has changed. Do you want to save it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int reply = msgBox.exec();

        if (reply == QMessageBox::Yes) {
            this->on_actionModelSave_triggered();
        }
    }
    _insertCommandInConsole("close");

    // quando a cena é fechada, limpo o grid associado a ela
    ui->graphicsView->getScene()->grid()->clear();
    // volto o botao de grid para "não clicado"
    ui->actionShowGrid->setChecked(false);

    // limpando tudo a que se refere à cena
    ui->graphicsView->getScene()->getUndoStack()->clear();
    ui->graphicsView->getScene()->clearAnimationsQueue();
    ui->graphicsView->getScene()->getGraphicalModelComponents()->clear();
    ui->graphicsView->getScene()->getGraphicalConnections()->clear();
    ui->graphicsView->getScene()->getAllComponents()->clear();
    ui->graphicsView->getScene()->getAllConnections()->clear();
    ui->graphicsView->getScene()->clearAnimations();
    ui->graphicsView->getScene()->clear();
    ui->graphicsView->clear();

    // limpando referencia do ultimo elemento selecionado em property editor
    ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();

    // limpando tudo a que se refere ao modelo
    simulator->getModelManager()->current()->getComponentManager()->getAllComponents()->clear();
    simulator->getModelManager()->current()->getComponentManager()->clear();
    ui->progressBarSimulation->setValue(0); // Seta o progresso da simulação para zero
    simulator->getModelManager()->remove(simulator->getModelManager()->current());

    ui->actionActivateGraphicalSimulation->setChecked(false);

    _clearModelEditors();

    _connectSceneSignals();
    _actualizeActions();
    _actualizeTabPanes();
    //QMessageBox::information(this, "Close Model", "Model successfully closed");
}


void MainWindow::on_actionModelInformation_triggered()
{
    DialogModelInformation* diag = new DialogModelInformation(this);
    diag->show();
}


void MainWindow::on_actionModelCheck_triggered()
{
    _check();
}

void MainWindow::on_actionSimulatorExit_triggered()
{
    if (!_confirmApplicationExit()) {
        return;
    }

    _closingApproved = true;
    QCoreApplication::quit();
}

bool MainWindow::_hasPendingModelChanges() const {
    Model* currentModel = simulator->getModelManager()->current();
    if (currentModel == nullptr) {
        return false;
    }

    return _textModelHasChanged || currentModel->hasChanged();
}

bool MainWindow::_confirmApplicationExit() {
    if (_hasPendingModelChanges()) {
        QMessageBox::StandardButton saveReply = QMessageBox::question(
                this,
                "Exit GenESyS",
                "Model has changed. Do you want to save it?",
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);

        if (saveReply == QMessageBox::Cancel) {
            return false;
        }

        if (saveReply == QMessageBox::Yes) {
            this->on_actionModelSave_triggered();
            if (_hasPendingModelChanges()) {
                return false;
            }
        }
    }

    QMessageBox::StandardButton exitReply = QMessageBox::question(
            this,
            "Exit GenESyS",
            "Do you want to exit GenESyS?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    return exitReply == QMessageBox::Yes;
}


void MainWindow::on_actionSimulationConfigure_triggered()
{
    DialogSimulationConfigure * dialog = new DialogSimulationConfigure(this);
    dialog->setSimulator(simulator);
    dialog->previousConfiguration();
    dialog->show();

}


void MainWindow::on_treeWidgetDataDefnitions_itemDoubleClicked(QTreeWidgetItem *item, int column)
{

    // Check if the column index is 2 (Name column)
    if (column == 2) {

        // Set the Qt::ItemIsEditable flag to enable editing for the specific item
        // It's required to set the flag here because otherwise all the other fields could changed too.
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        // Initiate the editing of the specified item in the specified column in the QTreeWidget
        ui->treeWidgetDataDefnitions->editItem(item, column);

        // Reset the Qt::ItemIsEditable flag to disable further editing after the edit operation
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    }
}

void MainWindow::on_treeWidgetDataDefnitions_itemChanged(QTreeWidgetItem *item, int column)
{

    // Check if the column index is 2 (Name column)
    if (column == 2) {

        // Get the changes
        QString after = item->text(column);
        Model * m = simulator->getModelManager()->current();

        // Save in the model
        for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {

                QString id = QString::fromStdString(Util::StrIndex(comp->getId()));

                if (id.contains(item->text(0)))
                    comp->setName(after.toStdString());
            }
        }
    }

}


void MainWindow::on_actionShowSnap_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    // Sincroniza o snap com o estado checkado da action de forma determinística.
    scene->setSnapToGrid(ui->actionShowSnap->isChecked());
}

void MainWindow::on_actionViewGroup_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->groupComponents(false);
}


void MainWindow::on_actionViewUngroup_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->ungroupComponents();
}

void MainWindow::on_actionArranjeLeft_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(0);
}


void MainWindow::on_actionArranjeCenter_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(4);
}


void MainWindow::on_actionArranjeRight_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(1);
}


void MainWindow::on_actionArranjeTop_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(2);
}


void MainWindow::on_actionArranjeMiddle_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(5);
}


void MainWindow::on_actionArranjeBototm_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(3);
}

void MainWindow::on_actionGModelShowConnect_triggered()
{
    if (!ui->actionGModelShowConnect->isChecked() && !_firstClickShowConnection) {
        ui->actionGModelShowConnect->setChecked(false);
        ui->graphicsView->getScene()->setConnectingStep(0);
        ui->graphicsView->setCursor(Qt::ArrowCursor);
    } else {
        ui->actionGModelShowConnect->setChecked(true);
        ui->graphicsView->getScene()->setConnectingStep(1);
        _firstClickShowConnection = false;
    }
}

void MainWindow::on_actionSimulatorsPluginManager_triggered()
{
    DialogPluginManager* dialog = new DialogPluginManager(this);
    dialog->show();
}

//void MainWindow::on_actionteste_triggered()
//{
//    _graphicalSimulation = (!_graphicalSimulation);
//}


void MainWindow::on_actionActivateGraphicalSimulation_triggered()
{
    bool visivible = true;

    if (!ui->actionActivateGraphicalSimulation->isChecked()) {
        AnimationTransition::setRunning(false);
        visivible = false;
    } else {
        AnimationTransition::setRunning(true);
    }

    // Esconde ou exibe animação de fila
    QList<QGraphicsItem *> *componentes = myScene()->getGraphicalModelComponents();

    for (QGraphicsItem* item : *componentes) {
        if (GraphicalModelComponent *component = dynamic_cast<GraphicalModelComponent *>(item)) {
            component->visivibleImageQueue(visivible);
        }
    }
}


void MainWindow::on_horizontalSliderAnimationSpeed_valueChanged(int value)
{
    double newValue = ((double) value) / 2;

    AnimationTransition::setTimeExecution(newValue);
}


void MainWindow::on_actionDiagrams_triggered()
{
    // Uses the active scene as the single source of truth for persisted diagram state.
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    if (ui->actionDiagrams->isChecked()) {
        // Creates diagram structures on demand when load restored diagrams=1 but none exist yet.
        if (!scene->existDiagram()) scene->createDiagrams();
        // Shows diagrams only after creation to fully restore persisted visibility.
        scene->showDiagrams();
    } else {
        // Hides diagrams only when they already exist to avoid side effects during load.
        if (scene->existDiagram()) scene->hideDiagrams();
    }
    // Re-syncs QAction with effective visibility to keep UI and scene flags coherent.
    const bool diagramsVisible = scene->existDiagram() && scene->visibleDiagram();
    if (ui->actionDiagrams->isChecked() != diagramsVisible) {
        ui->actionDiagrams->setChecked(diagramsVisible);
    }
}


void MainWindow::on_actionSelectAll_triggered()
{
    QList<QGraphicsItem *> itensToScene = myScene()->items();

    foreach (QGraphicsItem* item, itensToScene) {
        item->setSelected(true);
    }
}

void MainWindow::on_actionParallelization_triggered()
{
    // Opens a minimal parallelization configuration flow persisted in the GUI session.
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Parallelization"));
    auto* layout = new QFormLayout(&dialog);
    auto* enabled = new QCheckBox(tr("Enable parallel execution preparation"), &dialog);
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
    layout->addRow(tr("Worker threads"), threads);
    layout->addRow(tr("Batch size"), batchSize);
    layout->addRow(statusLabel);
    layout->addRow(buttons);

    // Persists parallelization preparation settings and publishes the applied values in the status bar.
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [this, enabled, threads, batchSize, statusLabel, &dialog]() {
        _parallelizationEnabled = enabled->isChecked();
        _parallelizationThreads = threads->value();
        _parallelizationBatchSize = batchSize->value();
        statusLabel->setText(QObject::tr("Configuration saved: enabled=%1, threads=%2, batch size=%3")
                             .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                             .arg(_parallelizationThreads)
                             .arg(_parallelizationBatchSize));
        statusBar()->showMessage(QObject::tr("Parallelization prepared: enabled=%1, threads=%2, batch size=%3")
                                 .arg(_parallelizationEnabled ? QObject::tr("true") : QObject::tr("false"))
                                 .arg(_parallelizationThreads)
                                 .arg(_parallelizationBatchSize), 5000);
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

void MainWindow::on_horizontalSlider_ZoomGraphical_actionTriggered(int action)
{

}



// -------------------------------------
// on Widgets
// -------------------------------------

void MainWindow::on_tabWidget_Model_tabBarClicked(int index) {

}

void MainWindow::on_checkBox_ShowElements_stateChanged(int arg1) {
    ui->actionShowAttachedElements->setChecked(arg1 == Qt::Checked);
    bool result = _createModelImage();
}

void MainWindow::on_checkBox_ShowInternals_stateChanged(int arg1) {
    ui->actionShowInternalElements->setChecked(arg1 == Qt::Checked);
    bool result = _createModelImage();
}

void MainWindow::on_horizontalSlider_Zoom_valueChanged(int value) {
    double factor = ((double) value / 100.0)*(2 - 0.5) + 0.5;
    double scaleFactor = 1.0;
    // @TODO: Qt5 -- Q_ASSERT(ui->label_ModelGraphic->pixmap());
    scaleFactor *= factor;
    ui->label_ModelGraphic->resize(scaleFactor * ui->label_ModelGraphic->pixmap().size());
    //adjustScrollBar(ui->scrollArea_Graphic->horizontalScrollBar(), factor);
    //adjustScrollBar(ui->scrollArea_Graphic->verticalScrollBar(), factor);

    //void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor){
    //    scrollBar->setValue(int(factor * scrollBar->value()
    //                            + ((factor - 1) * scrollBar->pageStep()/2)));
    //}

    //zoomInAct->setEnabled(scaleFactor < 3.0);
    //zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void MainWindow::on_checkBox_ShowRecursive_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_checkBox_ShowLevels_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_tabWidget_Debug_currentChanged(int index) {
    _actualizeActions();
}

void MainWindow::on_pushButton_Breakpoint_Insert_clicked() {
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    dialogBreakpoint dialog;
    dialog.setModal(true);
    dialog.setMVCModel(simulator);
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

void MainWindow::on_pushButton_Breakpoint_Remove_clicked() {
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }
    ModelSimulation* sim = model->getSimulation();
    if (sim == nullptr) {
        return;
    }

    int row = ui->tableWidget_Breakpoints->currentRow();
    if (row < 0 && ui->tableWidget_Breakpoints->selectionModel() != nullptr) {
        const QModelIndexList selectedRows = ui->tableWidget_Breakpoints->selectionModel()->selectedRows();
        if (!selectedRows.isEmpty()) {
            row = selectedRows.first().row();
        }
    }
    if (row < 0) {
        return;
    }
    QTableWidgetItem* typeItem = ui->tableWidget_Breakpoints->item(row, 1);
    QTableWidgetItem* onItem = ui->tableWidget_Breakpoints->item(row, 2);
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

void MainWindow::on_tabWidgetCentral_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetCentral_tabBarClicked(int index) {
}

void MainWindow::on_treeWidget_Plugins_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (ui->TextCodeEditor->isEnabled()) { // add text to modelsimulation
        /*
        if (item->toolTip(0).contains("DataDefinition")) {
            QTextCursor cursor = ui->TextCodeEditor->textCursor();
            QTextCursor cursorSaved = cursor;
            cursor.movePosition(QTextCursor::Start);
            ui->TextCodeEditor->setTextCursor(cursor);
            if (ui->TextCodeEditor->find("# Model Components")) {
                ui->TextCodeEditor->moveCursor(QTextCursor::MoveOperation::Left, QTextCursor::MoveMode::MoveAnchor);
                ui->TextCodeEditor->moveCursor(QTextCursor::MoveOperation::Up, QTextCursor::MoveMode::MoveAnchor);
                ui->TextCodeEditor->insertPlainText(item->statusTip(0) + "\n");
            } else {
                ui->TextCodeEditor->appendPlainText(item->statusTip(0));
            }
        } else {
            ui->TextCodeEditor->appendPlainText(item->statusTip(0));
        }
         */
    } else {
        // treeRoot? Always?
        for (int i = 0; i < ui->treeWidget_Plugins->topLevelItemCount(); i++) {
            //if (ui->treeWidget_Plugins->topLevelItem(i) != item) {
            ui->treeWidget_Plugins->topLevelItem(i)->setExpanded(false);
            //} else {
            //	ui->treeWidget_Plugins->expandItem(item);
            //	//ui->treeWidget_Plugins->topLevelItem(i)->setExpanded(true);
            //}
        }
        //ui->treeWidget_Plugins->setAnimated(true);
        ui->treeWidget_Plugins->expandItem(item);
    }
}

void MainWindow::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint) {
    // Reports rubber-band geometry during drag and final selected-item count when selection is completed.
    if (ui->graphicsView->scene() == nullptr) {
        return;
    }
    if (viewportRect.isNull()) {
        const int selectedCount = ui->graphicsView->scene()->selectedItems().size();
        statusBar()->showMessage(tr("Selection completed: %1 item(s) selected").arg(selectedCount), 3000);
        return;
    }
    const QRectF sceneRect = QRectF(fromScenePoint, toScenePoint).normalized();
    statusBar()->showMessage(tr("Selection area: %1 x %2 | Scene origin: (%3, %4)")
                             .arg(viewportRect.width())
                             .arg(viewportRect.height())
                             .arg(sceneRect.left(), 0, 'f', 1)
                             .arg(sceneRect.top(), 0, 'f', 1));
}

void MainWindow::on_horizontalSlider_ZoomGraphical_valueChanged(int value) {
    double factor = (value - _zoomValue)*0.002;
    _zoomValue = value;
    _gentle_zoom(1.0 + factor);
}

void MainWindow::on_actionConnect_triggered() {
    ((ModelGraphicsView*) ui->graphicsView)->beginConnection();
}

void MainWindow::on_pushButton_Export_clicked() {
    QPixmap modelPixmap = ui->label_ModelGraphic->pixmap();
    if (modelPixmap.isNull()) {
        _createModelImage();
        modelPixmap = ui->label_ModelGraphic->pixmap();
    }

    if (modelPixmap.isNull()) {
        ModelGraphicsScene* scene = ui->graphicsView->getScene();
        if (scene == nullptr || scene->items().isEmpty()) {
            QMessageBox::information(this, tr("Export Diagram"), tr("There is no diagram/image available to export."));
            return;
        }

        QRectF bounds = scene->itemsBoundingRect();
        if (!bounds.isValid() || bounds.isEmpty()) {
            QMessageBox::information(this, tr("Export Diagram"), tr("There is no diagram/image available to export."));
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
    const QString filters = tr("PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;Bitmap Image (*.bmp)");
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Diagram"), defaultName, filters, &selectedFilter, QFileDialog::DontUseNativeDialog);
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
        QMessageBox::warning(this, tr("Export Diagram"), tr("Could not export diagram to file."));
        return;
    }

    QMessageBox::information(this, tr("Export Diagram"), tr("Diagram exported successfully."));
}

void MainWindow::on_tabWidgetModelLanguages_currentChanged(int index) {
    if (index == CONST.TabModelSimLangIndex) {
        if (_graphicalModelHasChanged) {
            _actualizeModelSimLanguage();
        }
    } else if (index == CONST.TabModelCppCodeIndex) {
        _actualizeModelCppCode();
    }
    _actualizeActions();
}

void MainWindow::on_actionGModelComponentBreakpoint_triggered() {
    if (ui->graphicsView->selectedItems().size() == 1) {
        QGraphicsItem* gi = ui->graphicsView->selectedItems().at(0);
        GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*> (gi);
        if (gmc != nullptr) {
            ModelComponent* mc = gmc->getComponent();
            ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
            if (sim->getBreakpointsOnComponent()->find(mc) == sim->getBreakpointsOnComponent()->list()->end()) {
                sim->getBreakpointsOnComponent()->insert(mc);
            } else {
                sim->getBreakpointsOnComponent()->remove(mc);
            }
        }
        _actualizeDebugBreakpoints(false);
    }
}

void MainWindow::on_actionShowInternalElements_triggered() {
    const bool checked = ui->actionShowInternalElements->isChecked();
    if (ui->checkBox_ShowInternals->isChecked() != checked) {
        ui->checkBox_ShowInternals->setChecked(checked);
    } else {
        _createModelImage();
    }
}

void MainWindow::on_actionShowAttachedElements_triggered() {
    const bool checked = ui->actionShowAttachedElements->isChecked();
    if (ui->checkBox_ShowElements->isChecked() != checked) {
        ui->checkBox_ShowElements->setChecked(checked);
    } else {
        _createModelImage();
    }
}

void MainWindow::on_treeWidgetComponents_itemSelectionChanged() {
    QList<QTreeWidgetItem*> selectedItems = ui->treeWidgetComponents->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    bool ok = false;
    const Util::identification compId = selectedItems.first()->text(0).toULongLong(&ok);
    if (!ok) {
        return;
    }

    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (scene == nullptr) {
        return;
    }

    GraphicalModelComponent* gmc = scene->findGraphicalModelComponent(compId);
    if (gmc == nullptr) {
        return;
    }

    if (scene->selectedItems().size() == 1 && scene->selectedItems().first() == gmc) {
        ui->graphicsView->ensureVisible(gmc);
        return;
    }

    scene->clearSelection();
    gmc->setSelected(true);
    ui->graphicsView->ensureVisible(gmc);
    ui->graphicsView->centerOn(gmc);
}

void MainWindow::on_treeWidget_Plugins_itemClicked(QTreeWidgetItem *item, int column) {
    //showMessageNotImplemented();
}

void MainWindow::on_TextCodeEditor_textChanged() {
    this->_actualizeModelTextHasChanged(true);
}

void MainWindow::on_tabWidgetModel_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetSimulation_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetReports_currentChanged(int index) {
    _actualizeTabPanes();
}
