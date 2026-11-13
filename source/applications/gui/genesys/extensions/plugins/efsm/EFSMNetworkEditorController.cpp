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
    if (_host->ui == nullptr || _host->ui->graphicsView == nullptr || _host->ui->graphicsView->getScene() == nullptr
        || _host->simulator == nullptr || _host->simulator->getModelManager() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0) {
        return;
    }
    QList<FSMState*> selectedStates;
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (auto* graphicalState = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
            if (auto* state = dynamic_cast<FSMState*>(graphicalState->getDataDefinition())) {
                selectedStates.append(state);
            }
        }
    }
    if (selectedStates.size() < 1 || selectedStates.size() > 2) {
        QMessageBox::information(_host, _host->tr("Create EFSM Transition"),
                                 _host->tr("Select one state for a self-loop or two states for a directed transition."));
        return;
    }
    const bool selfLoopOnly = selectedStates.size() == 1;
    Model* model = _host->simulator->getModelManager()->current();
    auto* network = dynamic_cast<EFSMNetwork*>(model != nullptr && model->getDataManager() != nullptr
                                                    ? model->getDataManager()->getDataDefinition(
                                                          Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter())
                                                    : nullptr);
    if (network == nullptr || network->getStates()->find(selectedStates.at(0)) == network->getStates()->list()->end()
        || (!selfLoopOnly
            && network->getStates()->find(selectedStates.at(1)) == network->getStates()->list()->end())) {
        return;
    }
    QDialog dialog(_host);
    dialog.setWindowTitle(_host->tr("Create EFSM Transition"));
    auto* form = new QFormLayout(&dialog);
    auto* source = new QComboBox(&dialog);
    auto* destination = new QComboBox(&dialog);
    for (FSMState* state : selectedStates) {
        const QString stateName = QString::fromStdString(state->getName());
        source->addItem(stateName);
        destination->addItem(stateName);
    }
    source->setEnabled(!selfLoopOnly);
    destination->setEnabled(!selfLoopOnly);
    auto* name = new QLineEdit(&dialog);
    form->addRow(_host->tr("Source"), source);
    form->addRow(_host->tr("Destination"), destination);
    form->addRow(_host->tr("Name"), name);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const int sourceIndex = source->currentIndex();
    const int destinationIndex = destination->currentIndex();
    if (sourceIndex < 0 || sourceIndex >= selectedStates.size()
        || destinationIndex < 0 || destinationIndex >= selectedStates.size()) {
        return;
    }
    auto transition = std::make_unique<EFSMTransition>(selectedStates.at(sourceIndex),
                                                       selectedStates.at(destinationIndex),
                                                       name->text().toStdString());
    EFSMTransition* transitionPointer = transition.get();
    QUndoStack* undoStack = scene->getUndoStack();
    EFSMTransitionUndoCommand* undoCommand = nullptr;
    if (undoStack == nullptr) {
        network->addTransition(transition.release());
    } else {
        undoCommand = new EFSMTransitionUndoCommand(network, std::move(transition));
        undoStack->push(undoCommand);
    }
    const bool transitionIsRegistered = network->getTransitions()->find(transitionPointer)
        != network->getTransitions()->list()->end();
    if (!transitionIsRegistered) {
        if (undoStack != nullptr && undoCommand != nullptr) {
            undoCommand->setObsolete(true);
            undoStack->undo();
        } else {
            network->removeTransition(transitionPointer);
        }
        QMessageBox::warning(_host, _host->tr("Create EFSM Transition"),
                             _host->tr("The transition could not be registered in this Network."));
        return;
    }
    _host->_graphicalModelHasChanged = true;
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_host->simulator, scene);
    _host->_actualizeActions();
}

void EFSMNetworkEditorController::editTransition() {
    if (_host->ui == nullptr || _host->ui->graphicsView == nullptr || _host->ui->graphicsView->getScene() == nullptr
        || _host->simulator == nullptr || _host->simulator->getModelManager() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0 || scene->getUndoStack() == nullptr) {
        return;
    }
    EFSMTransitionGraphicsItem* selectedItem = nullptr;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* candidate = dynamic_cast<EFSMTransitionGraphicsItem*>(item);
        if (candidate == nullptr) {
            continue;
        }
        if (selectedItem != nullptr) {
            return;
        }
        selectedItem = candidate;
    }
    Model* model = _host->simulator->getModelManager()->current();
    auto* network = dynamic_cast<EFSMNetwork*>(model != nullptr && model->getDataManager() != nullptr
        ? model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter())
        : nullptr);
    if (network == nullptr || selectedItem == nullptr || selectedItem->transition() == nullptr
        || network->getTransitions()->find(selectedItem->transition()) == network->getTransitions()->list()->end()) {
        return;
    }

    EFSMTransition* transition = selectedItem->transition();
    QDialog dialog(_host);
    dialog.setWindowTitle(_host->tr("Edit EFSM Transition"));
    auto* form = new QFormLayout(&dialog);
    auto* name = new QLineEdit(QString::fromStdString(transition->getName()), &dialog);
    auto* guard = new QLineEdit(QString::fromStdString(transition->getGuardExpression()), &dialog);
    auto* output = new QLineEdit(QString::fromStdString(transition->getOutputExpression()), &dialog);
    auto* input = new QLineEdit(QString::fromStdString(transition->getInputEvent()), &dialog);
    auto* priority = new QSpinBox(&dialog);
    priority->setRange(0, std::numeric_limits<int>::max());
    priority->setValue(static_cast<int>(std::min<unsigned int>(transition->getPriority(), std::numeric_limits<int>::max())));
    auto* trigger = new QLineEdit(QString::fromStdString(transition->getTriggerEvent()), &dialog);
    auto* probability = new QLineEdit(QString::fromStdString(transition->getProbabilityExpression()), &dialog);
    form->addRow(_host->tr("Name"), name);
    form->addRow(_host->tr("Guard expression"), guard);
    form->addRow(_host->tr("Output expression"), output);
    form->addRow(_host->tr("Input event"), input);
    form->addRow(_host->tr("Priority"), priority);
    form->addRow(_host->tr("Trigger event"), trigger);
    form->addRow(_host->tr("Probability expression"), probability);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    EFSMTransitionPropertiesUndoCommand::Properties updated;
    updated.name = name->text().toStdString();
    updated.guardExpression = guard->text().toStdString();
    updated.outputExpression = output->text().toStdString();
    updated.inputEvent = input->text().toStdString();
    updated.priority = static_cast<unsigned int>(priority->value());
    updated.triggerEvent = trigger->text().toStdString();
    updated.probabilityExpression = probability->text().toStdString();
    scene->getUndoStack()->push(new EFSMTransitionPropertiesUndoCommand(network, transition, std::move(updated)));
    EFSMNetworkDiagramProjection::synchronize(_host->simulator, scene);
    _host->_graphicalModelHasChanged = true;
    _host->_actualizeActions();
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

void EFSMNetworkEditorController::editState() {
    if (_host->ui == nullptr || _host->ui->graphicsView == nullptr || _host->ui->graphicsView->getScene() == nullptr
        || _host->simulator == nullptr || _host->simulator->getModelManager() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = _host->ui->graphicsView->getScene();
    if (!scene->hasModelLevelFilter() || scene->modelLevelFilter() == 0 || scene->getUndoStack() == nullptr) {
        return;
    }
    FSMState* state = nullptr;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphicalDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* candidate = graphicalDefinition != nullptr
            ? dynamic_cast<FSMState*>(graphicalDefinition->getDataDefinition()) : nullptr;
        if (candidate == nullptr) {
            continue;
        }
        if (state != nullptr) {
            return;
        }
        state = candidate;
    }
    Model* model = _host->simulator->getModelManager()->current();
    auto* network = dynamic_cast<EFSMNetwork*>(model != nullptr && model->getDataManager() != nullptr
        ? model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter())
        : nullptr);
    if (network == nullptr || state == nullptr
        || network->getStates()->find(state) == network->getStates()->list()->end()) {
        return;
    }

    QDialog dialog(_host);
    dialog.setWindowTitle(_host->tr("Edit EFSM State"));
    auto* form = new QFormLayout(&dialog);
    auto* name = new QLineEdit(QString::fromStdString(state->getName()), &dialog);
    auto* entry = new QLineEdit(QString::fromStdString(state->getEntryActionExpression()), &dialog);
    auto* exit = new QLineEdit(QString::fromStdString(state->getExitActionExpression()), &dialog);
    auto* finalState = new QCheckBox(_host->tr("Final state"), &dialog);
    finalState->setChecked(state->isFinalNode());
    form->addRow(_host->tr("Name"), name);
    form->addRow(_host->tr("Entry action"), entry);
    form->addRow(_host->tr("Exit action"), exit);
    form->addRow(finalState);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const QString newName = name->text().trimmed();
    if (newName.isEmpty()) {
        QMessageBox::warning(_host, _host->tr("Edit EFSM State"), _host->tr("State name cannot be empty."));
        return;
    }
    ModelDataDefinition* sameName = model->getDataManager()->getDataDefinition(
        Util::TypeOf<FSMState>(), newName.toStdString());
    if (sameName != nullptr && sameName != state) {
        QMessageBox::warning(_host, _host->tr("Edit EFSM State"), _host->tr("A state with that name already exists."));
        return;
    }
    EFSMStatePropertiesUndoCommand::Properties updated;
    updated.name = newName.toStdString();
    updated.entryActionExpression = entry->text().toStdString();
    updated.exitActionExpression = exit->text().toStdString();
    updated.finalNode = finalState->isChecked();
    scene->getUndoStack()->push(new EFSMStatePropertiesUndoCommand(network, state, std::move(updated)));
    GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_host->simulator, scene);
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
    return selectedStateCount == 1 || selectedStateCount == 2;
}

bool EFSMNetworkEditorController::canEditTransition() const {
    if (!hasNetworkContext()) return false;
    int count = 0;
    for (QGraphicsItem* item : _host->ui->graphicsView->getScene()->selectedItems()) {
        if (dynamic_cast<EFSMTransitionGraphicsItem*>(item) != nullptr) ++count;
    }
    return count == 1;
}

bool EFSMNetworkEditorController::canEditState() const {
    return canSetInitialState();
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
