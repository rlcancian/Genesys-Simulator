#include "EFSMNetworkEditorController.h"
#include "../../../mainwindow.h"
#include "../../../ui_mainwindow.h"
#include "../../../actions/EFSMTransitionUndoCommand.h"
#include "../../../actions/EFSMTransitionPropertiesUndoCommand.h"
#include "../../../actions/EFSMInitialStateUndoCommand.h"
#include "../../../actions/EFSMStateMembershipUndoCommand.h"
#include "../../../actions/EFSMStatePropertiesUndoCommand.h"
#include "../../../graphicals/EFSMTransitionGraphicsItem.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "../../../services/EFSMNetworkDiagramProjection.h"
#include "../../../services/GraphicalModelBuilder.h"
#include "../../../../plugins/components/ModalModel/FSMState.h"
#include "../../../../plugins/data/ModalModel/EFSMNetwork.h"
#include "../../../../kernel/simulator/model/ModelDataManager.h"
#include "../../../../kernel/simulator/model/ModelManager.h"
#include "../../../../kernel/simulator/model/Model.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/util/Util.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QUndoStack>

#include <algorithm>
#include <limits>
#include <memory>

EFSMNetworkEditorController::EFSMNetworkEditorController(MainWindow* host) : _host(host) {}

void EFSMNetworkEditorController::createState() {
    if (_host->ui == nullptr || _host->ui->graphicsView == nullptr || _host->ui->graphicsView->getScene() == nullptr
        || _host->simulator == nullptr || _host->simulator->getModelManager() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0) {
        return;
    }
    Model* model = _host->simulator->getModelManager()->current();
    auto* network = dynamic_cast<EFSMNetwork*>(model != nullptr && model->getDataManager() != nullptr
                                                    ? model->getDataManager()->getDataDefinition(
                                                          Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter())
                                                    : nullptr);
    if (network == nullptr) {
        return;
    }
    bool accepted = false;
    const QString name = QInputDialog::getText(_host, _host->tr("Create EFSM State"), _host->tr("State name:"),
                                               QLineEdit::Normal, _host->tr("State"), &accepted);
    if (!accepted || name.trimmed().isEmpty()) {
        return;
    }
    if (model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), name.toStdString()) != nullptr) {
        QMessageBox::warning(_host, _host->tr("Create EFSM State"), _host->tr("A state with that name already exists in this model."));
        return;
    }
    auto state = std::make_unique<FSMState>(model, name.toStdString());
    FSMState* statePointer = state.get();
    QUndoStack* undoStack = scene->getUndoStack();
    EFSMStateMembershipUndoCommand* command = nullptr;
    if (undoStack != nullptr) {
        command = new EFSMStateMembershipUndoCommand(model, network, std::move(state));
        undoStack->push(command);
    } else {
        network->addState(state.release());
    }
    const bool stateIsRegistered = model->getDataManager()->getDataDefinition(
        Util::TypeOf<FSMState>(), statePointer->getId()) == statePointer;
    const bool stateIsInNetwork = network->getStates()->find(statePointer) != network->getStates()->list()->end();
    if (!stateIsRegistered || !stateIsInNetwork) {
        if (undoStack != nullptr && command != nullptr) {
            command->setObsolete(true);
            undoStack->undo();
        } else {
            network->removeState(statePointer);
            model->getDataManager()->remove(statePointer);
            delete statePointer;
        }
        QMessageBox::warning(_host, _host->tr("Create EFSM State"), _host->tr("The state could not be registered in this Network."));
        return;
    }
    _host->_graphicalModelHasChanged = true;
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_host->simulator, scene);
    EFSMNetworkDiagramProjection::synchronize(_host->simulator, scene);
    _host->_actualizeActions();
}

void EFSMNetworkEditorController::createTransition() {
    if (_host == nullptr || _host->ui == nullptr || _host->ui->graphicsView == nullptr
        || _host->ui->graphicsView->getScene() == nullptr) return;
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    GraphicalModelDataDefinition* selectedState = nullptr;
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (auto* graphicalState = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
            if (dynamic_cast<FSMState*>(graphicalState->getDataDefinition()) != nullptr) selectedState = graphicalState;
        }
    }
    if (selectedState == nullptr) return;
    scene->beginNetworkRelationCreation(selectedState);
}

void EFSMNetworkEditorController::setInitialState() {
    if (_host->ui == nullptr || _host->ui->graphicsView == nullptr || _host->ui->graphicsView->getScene() == nullptr
        || _host->simulator == nullptr || _host->simulator->getModelManager() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0 || scene->getUndoStack() == nullptr) {
        return;
    }
    FSMState* selectedState = nullptr;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphicalDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* state = graphicalDefinition != nullptr
            ? dynamic_cast<FSMState*>(graphicalDefinition->getDataDefinition()) : nullptr;
        if (state == nullptr) {
            continue;
        }
        if (selectedState != nullptr) {
            return;
        }
        selectedState = state;
    }
    Model* model = _host->simulator->getModelManager()->current();
    auto* network = dynamic_cast<EFSMNetwork*>(model != nullptr && model->getDataManager() != nullptr
        ? model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter())
        : nullptr);
    if (network == nullptr || selectedState == nullptr
        || network->getStates()->find(selectedState) == network->getStates()->list()->end()) {
        return;
    }
    scene->getUndoStack()->push(new EFSMInitialStateUndoCommand(network, selectedState));
    EFSMNetworkDiagramProjection::synchronize(_host->simulator, scene);
    _host->_graphicalModelHasChanged = true;
    _host->_actualizeActions();
}

bool EFSMNetworkEditorController::hasNetworkContext() const {
    if (_host == nullptr || _host->ui == nullptr || _host->ui->graphicsView == nullptr
        || _host->ui->graphicsView->getScene() == nullptr || _host->simulator == nullptr
        || _host->simulator->getModelManager() == nullptr) return false;
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    Model* model = _host->simulator->getModelManager()->current();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0 || model == nullptr
        || model->getDataManager() == nullptr || model->getSimulation() == nullptr
        || model->getSimulation()->isRunning() || model->getSimulation()->isPaused()) return false;
    return model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter()) != nullptr;
}

bool EFSMNetworkEditorController::canCreateTransition() const {
    if (!hasNetworkContext()) return false;
    int selectedStateCount = 0;
    for (QGraphicsItem* item : _host->ui->graphicsView->getScene()->selectedItems()) {
        auto* definition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (definition != nullptr && dynamic_cast<FSMState*>(definition->getDataDefinition()) != nullptr) {
            ++selectedStateCount;
        }
    }
    return selectedStateCount == 1;
}

bool EFSMNetworkEditorController::canSetInitialState() const {
    if (!hasNetworkContext()) return false;
    int count = 0;
    for (QGraphicsItem* item : _host->ui->graphicsView->getScene()->selectedItems()) {
        auto* definition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (definition != nullptr && dynamic_cast<FSMState*>(definition->getDataDefinition()) != nullptr) ++count;
    }
    return count == 1;
}

bool EFSMNetworkEditorController::canCreateState() const {
    return hasNetworkContext();
}
