#include "mainwindow.h"
#include "ui_mainwindow.h"
// Include the Phase 3 controller interface required by compatibility wrappers.
#include "controllers/ModelInspectorController.h"
// Include the Phase 5 controller interface required by plugin-tree wrappers.
#include "controllers/PluginCatalogController.h"
// Include the Phase 7 controller interface required by lifecycle compatibility wrappers.
#include "controllers/ModelLifecycleController.h"
// Include the Phase 8 controller interface required by simulation-command compatibility wrappers.
#include "controllers/SimulationCommandController.h"
// Include the Phase 9 controller interface required by edit-command compatibility wrappers.
#include "controllers/EditCommandController.h"
// Include the Phase 10 controller interface required by scene-tool compatibility wrappers.
#include "controllers/SceneToolController.h"
// Include the Phase 11 controller interface required by dialog-utility compatibility wrappers.
#include "controllers/DialogUtilityController.h"

#include "dialogs/dialogBreakpoint.h"
#include "dialogs/Dialogmodelinformation.h"
#include "dialogs/dialogsimulationconfigure.h"
#include "dialogs/dialogpluginmanager.h"
#include "dialogs/dialogsystempreferences.h"
#include "dialogs/DialogFind.h"
#include "controllers/SimulationController.h"

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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 8 refactor.
    if (_simulationCommandController != nullptr) {
        _simulationCommandController->onActionSimulationStopTriggered();
    }
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 8 refactor.
    if (_simulationCommandController != nullptr) {
        _simulationCommandController->onActionSimulationStartTriggered(_modelCheked);
    }
}

/**
 * @brief Executes one simulation step after readiness validation.
 *
 * Uses the same precondition pipeline as start command.
 *
 * @todo Consolidate duplicated animation-toggle code between start and step.
 */
void MainWindow::on_actionSimulationStep_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 8 refactor.
    if (_simulationCommandController != nullptr) {
        _simulationCommandController->onActionSimulationStepTriggered(_modelCheked);
    }
}

/**
 * @brief Pauses running simulation.
 *
 * @todo Add explicit feedback when pause is requested in invalid state.
 */
void MainWindow::on_actionSimulationPause_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 8 refactor.
    if (_simulationCommandController != nullptr) {
        _simulationCommandController->onActionSimulationPauseTriggered();
    }
}

/**
 * @brief Resumes simulation execution after readiness validation.
 *
 * @todo Introduce a dedicated `resume()` API in kernel side when available
 *       to avoid semantic coupling with `start()`.
 */
void MainWindow::on_actionSimulationResume_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 8 refactor.
    if (_simulationCommandController != nullptr) {
        _simulationCommandController->onActionSimulationResumeTriggered(_modelCheked);
    }
}


void MainWindow::on_actionAboutAbout_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionAboutAboutTriggered();
    }
}


void MainWindow::on_actionAboutLicence_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionAboutLicenceTriggered();
    }
}


void MainWindow::on_actionAboutGetInvolved_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionAboutGetInvolvedTriggered();
    }
}


void MainWindow::on_actionEditUndo_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditUndoTriggered();
    }
}


void MainWindow::on_actionEditRedo_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditRedoTriggered();
    }
}


void MainWindow::on_actionEditFind_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionEditFindTriggered();
    }
}



// void MainWindow::on_actionReplace_triggered() {
//     _showMessageNotImplemented();
// }
void MainWindow::on_actionEditReplace_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionEditReplaceTriggered();
    }
}



void MainWindow::on_actionEditCut_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditCutTriggered();
    }
}

void MainWindow::on_actionEditCopy_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditCopyTriggered();
    }
}

void MainWindow::on_actionEditPaste_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditPasteTriggered();
    }
}


void MainWindow::on_actionShowGrid_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowGridTriggered();
    }
}


void MainWindow::on_actionShowRule_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowRuleTriggered();
    }
}


void MainWindow::on_actionShowGuides_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowGuidesTriggered();
    }
}


void MainWindow::on_actionZoom_In_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionZoomInTriggered();
    }
}


void MainWindow::on_actionZoom_Out_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionZoomOutTriggered();
    }
}


void MainWindow::on_actionZoom_All_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionZoomAllTriggered();
    }
}


void MainWindow::on_actionDrawLine_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDrawLineTriggered();
    }
}


void MainWindow::on_actionDrawRectangle_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDrawRectangleTriggered();
    }
}


void MainWindow::on_actionDrawEllipse_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDrawEllipseTriggered();
    }
}

void MainWindow::on_actionDrawText_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDrawTextTriggered();
    }
}

void MainWindow::on_actionDrawPoligon_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDrawPoligonTriggered();
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditDeleteTriggered();
    }
}


void MainWindow::on_actionSimulatorPreferences_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionSimulatorPreferencesTriggered();
    }
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

void MainWindow::on_actionAnimateCounter_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionAnimateCounterTriggered();
    }
}

void MainWindow::on_actionAnimateVariable_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionAnimateVariableTriggered();
    }
}

void MainWindow::on_actionAnimateSimulatedTime_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionAnimateSimulatedTimeTriggered();
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditGroupTriggered();
    }
}


void MainWindow::on_actionEditUngroup_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionEditUngroupTriggered();
    }
}


void MainWindow::on_actionToolsParserGrammarChecker_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionToolsParserGrammarCheckerTriggered();
    }
}



void MainWindow::on_actionToolsExperimentation_triggered()
{
    // Legacy slot: action is not exposed in current mainwindow.ui.
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsOptimizator_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionToolsOptimizatorTriggered();
    }
}



void MainWindow::on_actionToolsDataAnalyzer_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionToolsDataAnalyzerTriggered();
    }
}



void MainWindow::on_actionAnimatePlot_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionViewConfigure_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionViewConfigureTriggered();
    }
}


//void MainWindow::on_actionConfigure_triggered() {//?????????????????????????
//}
//void MainWindow::on_actionOpen_triggered() {//?????????????????????????
//}



void MainWindow::on_actionModelNew_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelNewTriggered();
}

void MainWindow::on_actionModelOpen_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelOpenTriggered();
}


void MainWindow::on_actionModelSave_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelSaveTriggered();
}



void MainWindow::on_actionModelClose_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelCloseTriggered();
}


void MainWindow::on_actionModelInformation_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelInformationTriggered();
}


void MainWindow::on_actionModelCheck_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionModelCheckTriggered();
}

void MainWindow::on_actionSimulatorExit_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionSimulatorExitTriggered();
}

bool MainWindow::_hasPendingModelChanges() const {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    return _modelLifecycleController->hasPendingModelChanges();
}

bool MainWindow::_confirmApplicationExit() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    return _modelLifecycleController->confirmApplicationExit();
}


void MainWindow::on_actionSimulationConfigure_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 7 refactor.
    _modelLifecycleController->onActionSimulationConfigureTriggered();
}


void MainWindow::on_treeWidgetDataDefnitions_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 3 refactor.
    _modelInspectorController->beginDataDefinitionNameEdit(item, column);
}

void MainWindow::on_treeWidgetDataDefnitions_itemChanged(QTreeWidgetItem *item, int column)
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 3 refactor.
    _modelInspectorController->applyDataDefinitionNameChange(item, column);
}


void MainWindow::on_actionShowSnap_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowSnapTriggered();
    }
}

void MainWindow::on_actionViewGroup_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionViewGroupTriggered();
    }
}


void MainWindow::on_actionViewUngroup_triggered()
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->onActionViewUngroupTriggered();
    }
}

void MainWindow::on_actionArranjeLeft_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeLeftTriggered();
    }
}


void MainWindow::on_actionArranjeCenter_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeCenterTriggered();
    }
}


void MainWindow::on_actionArranjeRight_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeRightTriggered();
    }
}


void MainWindow::on_actionArranjeTop_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeTopTriggered();
    }
}


void MainWindow::on_actionArranjeMiddle_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeMiddleTriggered();
    }
}


void MainWindow::on_actionArranjeBototm_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionArranjeBototmTriggered();
    }
}

void MainWindow::on_actionGModelShowConnect_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionGModelShowConnectTriggered();
    }
}

void MainWindow::on_actionSimulatorsPluginManager_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionSimulatorsPluginManagerTriggered();
    }
}


//void MainWindow::on_actionteste_triggered()
//{
//    _graphicalSimulation = (!_graphicalSimulation);
//}


void MainWindow::on_actionActivateGraphicalSimulation_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionActivateGraphicalSimulationTriggered();
    }
}


void MainWindow::on_horizontalSliderAnimationSpeed_valueChanged(int value) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onHorizontalSliderAnimationSpeedValueChanged(value);
    }
}


void MainWindow::on_actionDiagrams_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionDiagramsTriggered();
    }
}


void MainWindow::on_actionSelectAll_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionSelectAllTriggered();
    }
}

void MainWindow::on_actionParallelization_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onActionParallelizationTriggered();
    }
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onCheckBoxShowElementsStateChanged(arg1);
    }
}

void MainWindow::on_checkBox_ShowInternals_stateChanged(int arg1) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onCheckBoxShowInternalsStateChanged(arg1);
    }
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onCheckBoxShowRecursiveStateChanged(arg1);
    }
}

void MainWindow::on_checkBox_ShowLevels_stateChanged(int arg1) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onCheckBoxShowLevelsStateChanged(arg1);
    }
}

void MainWindow::on_tabWidget_Debug_currentChanged(int index) {
    _actualizeActions();
}

void MainWindow::on_pushButton_Breakpoint_Insert_clicked() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onPushButtonBreakpointInsertClicked();
    }
}


void MainWindow::on_pushButton_Breakpoint_Remove_clicked() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onPushButtonBreakpointRemoveClicked();
    }
}


void MainWindow::on_tabWidgetCentral_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetCentral_tabBarClicked(int index) {
}

void MainWindow::on_treeWidget_Plugins_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 5 refactor.
    _pluginCatalogController->handlePluginItemDoubleClicked(item, column);
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onHorizontalSliderZoomGraphicalValueChanged(value);
    }
}

void MainWindow::on_actionConnect_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionConnectTriggered();
    }
}

void MainWindow::on_pushButton_Export_clicked() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 11 refactor.
    if (_dialogUtilityController != nullptr) {
        _dialogUtilityController->onPushButtonExportClicked();
    }
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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowInternalElementsTriggered();
    }
}

void MainWindow::on_actionShowAttachedElements_triggered() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 10 refactor.
    if (_sceneToolController != nullptr) {
        _sceneToolController->onActionShowAttachedElementsTriggered();
    }
}

void MainWindow::on_treeWidgetComponents_itemSelectionChanged() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 3 refactor.
    _modelInspectorController->syncSelectedComponentTreeItemToScene();
}

void MainWindow::on_treeWidget_Plugins_itemClicked(QTreeWidgetItem *item, int column) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 5 refactor.
    _pluginCatalogController->handlePluginItemClicked(item, column);
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
