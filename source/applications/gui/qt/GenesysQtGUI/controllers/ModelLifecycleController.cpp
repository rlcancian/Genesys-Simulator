#include "ModelLifecycleController.h"

// Use the generated UI header through the project include path for portability across build folders.
#include "ui_mainwindow.h"
#include "../dialogs/Dialogmodelinformation.h"
#include "../dialogs/dialogsimulationconfigure.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../systempreferences.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <utility>

// Store the lifecycle dependencies for Phase 7 delegation from MainWindow.
ModelLifecycleController::ModelLifecycleController(QWidget* ownerWidget,
                                                   Simulator* simulator,
                                                   Ui::MainWindow* ui,
                                                   QString* modelFilename,
                                                   bool* textModelHasChanged,
                                                   bool* graphicalModelHasChanged,
                                                   bool* closingApproved,
                                                   bool* loaded,
                                                   bool& parallelizationEnabled,
                                                   int& parallelizationThreads,
                                                   int& parallelizationBatchSize,
                                                   Callbacks callbacks)
    : _ownerWidget(ownerWidget),
      _simulator(simulator),
      _ui(ui),
      _modelFilename(modelFilename),
      _textModelHasChanged(textModelHasChanged),
      _graphicalModelHasChanged(graphicalModelHasChanged),
      _closingApproved(closingApproved),
      _loaded(loaded),
      _parallelizationEnabled(parallelizationEnabled),
      _parallelizationThreads(parallelizationThreads),
      _parallelizationBatchSize(parallelizationBatchSize),
      _callbacks(std::move(callbacks)) {
}

// Move model creation orchestration out of MainWindow while preserving prompts and console flow.
void ModelLifecycleController::onActionModelNewTriggered() const {
    _callbacks.insertCommandInConsole("new");
    Model* m = _simulator->getModelManager()->newModel();
    _callbacks.initUiForNewModel(m);
}

// Move model open orchestration out of MainWindow while preserving file-dialog defaults and messages.
void ModelLifecycleController::onActionModelOpenTriggered() const {
    // Preserve the existing default directory behavior that points to the models folder.
    QString currentDirectory = QDir::currentPath();
    QDir parentDir(currentDirectory);
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cd("models");
    QString initialDirectory = parentDir.absolutePath();

    QString fileName = QFileDialog::getOpenFileName(
        _ownerWidget, "Open Model", initialDirectory,
        QObject::tr("Genesys Model (*.gen);;Genesys Graphical User Interface (*.gui);;XML Files (*.xml);;JSON Files (*.json);;C++ Files (*.cpp)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName == "") {
        return;
    }
    openModelFileInternal(fileName, true);
}

bool ModelLifecycleController::openModelFile(const QString& fileName) const {
    return openModelFileInternal(fileName, false);
}

bool ModelLifecycleController::openModelFileInternal(const QString& fileName, bool showDialogs) const {
    if (fileName.trimmed().isEmpty()) {
        return false;
    }

    _callbacks.insertCommandInConsole("load " + fileName.toStdString());
    Model* model = _callbacks.loadGraphicalModel(fileName.toStdString());
    if (model != nullptr) {
        *_loaded = true;
        _callbacks.initUiForNewModel(model);
        _callbacks.actualizeModelTextHasChanged(false);
        if (_graphicalModelHasChanged != nullptr) {
            *_graphicalModelHasChanged = false;
        }
        model->setHasChanged(false);
        SystemPreferences::pushRecentModelFile(fileName.toStdString());
        SystemPreferences::save();
        if (showDialogs) {
            QMessageBox::information(_ownerWidget, "Open Model", "Model successfully oppened");
        }
    } else {
        if (showDialogs) {
            QMessageBox::warning(_ownerWidget, "Open Model", "Error while opening model");
        }
        _callbacks.actualizeActions();
        _callbacks.actualizeTabPanes();
    }
    _ui->graphicsView->getScene()->getUndoStack()->clear();
    return model != nullptr;
}

// Move model save orchestration out of MainWindow while preserving .gen/.gui persistence and checks.
void ModelLifecycleController::onActionModelSaveTriggered() const {
    QString fileName = QFileDialog::getSaveFileName(_ownerWidget,
                                                    QObject::tr("Save Model"), *_modelFilename,
                                                    QObject::tr("Genesys Model (*.gen)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) {
        return;
    } else {
        _callbacks.insertCommandInConsole("save " + fileName.toStdString());
        QString finalFileName = fileName + ".gen";
        QFile saveFile(finalFileName);

        if (!saveFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(_ownerWidget, QObject::tr("Unable to access file to save"),
                                     saveFile.errorString());
            return;
        } else {
            if (!_callbacks.saveTextModel(&saveFile, _ui->TextCodeEditor->toPlainText())) {
                saveFile.close();
                QMessageBox::warning(_ownerWidget, "Save Model", "Error while saving model text.");
                return;
            }
            saveFile.close();
        }
        if (!_callbacks.saveGraphicalModel(fileName + ".gui")) {
            QMessageBox::warning(_ownerWidget, "Save Model", "Error while saving graphical model.");
            return;
        }
        *_modelFilename = fileName;
        if (!_callbacks.setSimulationModelBasedOnText()) {
            QMessageBox::warning(_ownerWidget, "Save Model", "Model was saved, but the simulation model could not be synchronized.");
            return;
        }
        _callbacks.actualizeModelTextHasChanged(false);
        if (_graphicalModelHasChanged != nullptr) {
            *_graphicalModelHasChanged = false;
        }
        if (Model* currentModel = _simulator->getModelManager()->current()) {
            // A successful save makes the current in-memory model state the new clean baseline.
            currentModel->setHasChanged(false);
        }
        SystemPreferences::setLastModelFilename((fileName + ".gui").toStdString());
        SystemPreferences::pushRecentModelFile((fileName + ".gui").toStdString());
        SystemPreferences::save();
        QMessageBox::information(_ownerWidget, "Save Model", "Model successfully saved");
    }
    _callbacks.actualizeActions();
    _ui->graphicsView->getScene()->getUndoStack()->clear();
}

// Move model close orchestration out of MainWindow while preserving signal wiring and cleanup sequence.
void ModelLifecycleController::onActionModelCloseTriggered() const {
    _callbacks.disconnectSceneSignals("on_actionModelClose_triggered(begin)");
    if (hasPendingModelChanges()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Close ModelSyS");
        msgBox.setText("Model has changed. Do you want to save it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int reply = msgBox.exec();

        if (reply == QMessageBox::Yes) {
            onActionModelSaveTriggered();
        }
    }
    _callbacks.insertCommandInConsole("close");

    // Preserve scene clearing order to avoid regressions in existing UI flows.
    _ui->graphicsView->getScene()->grid()->clear();
    _ui->actionShowGrid->setChecked(false);
    _ui->graphicsView->getScene()->getUndoStack()->clear();
    _ui->graphicsView->getScene()->clearAnimationsQueue();
    _ui->graphicsView->getScene()->getGraphicalModelComponents()->clear();
    _ui->graphicsView->getScene()->getGraphicalConnections()->clear();
    // Reset data-definition diagram bookkeeping alongside component/connection cleanup.
    _ui->graphicsView->getScene()->getGraphicalModelDataDefinitions()->clear();
    _ui->graphicsView->getScene()->getGraphicalDiagramsConnections()->clear();
    _ui->graphicsView->getScene()->getAllComponents()->clear();
    _ui->graphicsView->getScene()->getAllConnections()->clear();
    _ui->graphicsView->getScene()->getAllDataDefinitions()->clear();
    _ui->graphicsView->getScene()->getAllGraphicalDiagramsConnections()->clear();
    _ui->graphicsView->getScene()->clearAnimations();
    _ui->graphicsView->getScene()->clear();
    _ui->graphicsView->clear();

    // Preserve property-editor selection reset without changing Phase 6 hardening behavior.
    _ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();

    // Preserve kernel/model cleanup sequence and UI reset behavior.
    _simulator->getModelManager()->current()->getComponentManager()->getAllComponents()->clear();
    _simulator->getModelManager()->current()->getComponentManager()->clear();
    _ui->progressBarSimulation->setValue(0);
    _simulator->getModelManager()->remove(_simulator->getModelManager()->current());
    _ui->actionActivateGraphicalSimulation->setChecked(false);

    _callbacks.clearModelEditors();
    _callbacks.connectSceneSignals();
    _callbacks.actualizeActions();
    _callbacks.actualizeTabPanes();
}

// Move model-information dialog trigger out of MainWindow while keeping dialog behavior unchanged.
void ModelLifecycleController::onActionModelInformationTriggered() const {
    DialogModelInformation* diag = new DialogModelInformation(_ownerWidget);
    // Edit the information object owned by the currently open model.
    diag->setModelInfo(_simulator->getModelManager()->current()->getInfos());
    diag->show();
}

// Move model-check trigger out of MainWindow while preserving check callback behavior.
void ModelLifecycleController::onActionModelCheckTriggered() const {
    _callbacks.checkModel();
}

// Move simulation-configure dialog trigger out of MainWindow while preserving simulator wiring.
void ModelLifecycleController::onActionSimulationConfigureTriggered() const {
    DialogSimulationConfigure* dialog = new DialogSimulationConfigure(_ownerWidget);
    // Edit the simulation object owned by the currently open model.
    dialog->setModelSimulation(_simulator->getModelManager()->current()->getSimulation());
    dialog->setExperimentManager(_simulator->getExperimentManager());
    dialog->setParallelizationSettings(&_parallelizationEnabled, &_parallelizationThreads, &_parallelizationBatchSize);
    dialog->show();
}

// Move simulator-exit trigger out of MainWindow while preserving closing-approval semantics.
void ModelLifecycleController::onActionSimulatorExitTriggered() const {
    if (!confirmApplicationExit()) {
        return;
    }

    *_closingApproved = true;
    QCoreApplication::quit();
}

// Move pending-model-change detection out of MainWindow while preserving exact criteria.
bool ModelLifecycleController::hasPendingModelChanges() const {
    Model* currentModel = _simulator->getModelManager()->current();
    if (currentModel == nullptr) {
        return false;
    }

    return *_textModelHasChanged
        || (_graphicalModelHasChanged != nullptr && *_graphicalModelHasChanged)
        || currentModel->hasChanged();
}

// Move application-exit confirmation out of MainWindow while preserving save/confirm ordering.
bool ModelLifecycleController::confirmApplicationExit() const {
    if (hasPendingModelChanges()) {
        QMessageBox::StandardButton saveReply = QMessageBox::question(
            _ownerWidget,
            "Exit GenESyS",
            "Model has changed. Do you want to save it?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);

        if (saveReply == QMessageBox::Cancel) {
            return false;
        }

        if (saveReply == QMessageBox::Yes) {
            onActionModelSaveTriggered();
            if (hasPendingModelChanges()) {
                return false;
            }
        }
    }

    QMessageBox::StandardButton exitReply = QMessageBox::question(
        _ownerWidget,
        "Exit GenESyS",
        "Do you want to exit GenESyS?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    return exitReply == QMessageBox::Yes;
}
