#include "EFSMNetworkDiagramProjection.h"

#include "EFSMNetworkDiagramLayout.h"

#include "../graphicals/EFSMTransitionGraphicsItem.h"
#include "../graphicals/GraphicalModelDataDefinition.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/util/List.h"
#include "kernel/util/Util.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <QPointF>
#include <algorithm>
#include <cmath>

namespace {

EFSMNetwork* contextNetwork(Simulator* simulator, ModelGraphicsScene* scene) {
    if (simulator == nullptr || scene == nullptr || !scene->hasModelLevelFilter()
        || scene->modelLevelFilter() == 0 || simulator->getModelManager() == nullptr) {
        return nullptr;
    }
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr || model->getDataManager() == nullptr) {
        return nullptr;
    }
    ModelDataDefinition* definition = model->getDataManager()->getDataDefinition(
        Util::TypeOf<EFSMNetwork>(), scene->modelLevelFilter());
    return dynamic_cast<EFSMNetwork*>(definition);
}

QPointF stateCenter(ModelGraphicsScene* scene, FSMState* state) {
    GraphicalModelDataDefinition* item = scene->findGraphicalModelDataDefinition(state);
    return item != nullptr ? item->sceneBoundingRect().center() : QPointF();
}

} // namespace

namespace EFSMNetworkDiagramProjection {

void synchronize(Simulator* simulator, ModelGraphicsScene* scene) {
    if (scene == nullptr) {
        return;
    }
    scene->clearEFSMNetworkDiagramItems();
    EFSMNetwork* network = contextNetwork(simulator, scene);
    if (network == nullptr || network->getTransitions() == nullptr) {
        return;
    }

    QVector<EFSMTransition*> transitions;
    for (EFSMTransition* transition : *network->getTransitions()->list()) {
        if (transition != nullptr) {
            transitions.append(transition);
        }
    }
    for (int ordinal = 0; ordinal < transitions.size(); ++ordinal) {
        EFSMTransition* transition = transitions.at(ordinal);
        FSMState* source = dynamic_cast<FSMState*>(transition->getSource());
        FSMState* destination = dynamic_cast<FSMState*>(transition->getDestination());
        if (source == nullptr || destination == nullptr
            || scene->findGraphicalModelDataDefinition(source) == nullptr
            || scene->findGraphicalModelDataDefinition(destination) == nullptr) {
            continue;
        }

        int rank = 0;
        int count = 0;
        for (int candidateOrdinal = 0; candidateOrdinal < transitions.size(); ++candidateOrdinal) {
            EFSMTransition* candidate = transitions.at(candidateOrdinal);
            if (candidate->getSource() == source && candidate->getDestination() == destination) {
                if (candidateOrdinal < ordinal) {
                    ++rank;
                }
                ++count;
            }
        }

        auto* item = new EFSMTransitionGraphicsItem(transition, ordinal);
        item->setGeometry(stateCenter(scene, source), stateCenter(scene, destination), rank, count);
        scene->addEFSMNetworkDiagramItem(item);
    }

    // Highlight initial and final states in the projection only; no domain property is changed.
    for (FSMState* state : *network->getStates()->list()) {
        if (GraphicalModelDataDefinition* item = scene->findGraphicalModelDataDefinition(state)) {
            const bool initial = state == network->getInitialState();
            const bool final = state->isFinalNode();
            item->setColor(initial && final ? QColor(111, 190, 188)
                              : initial ? QColor(135, 210, 155)
                              : final ? QColor(245, 195, 125)
                                       : QColor(220, 220, 220));
        }
    }
}

bool captureLayout(EFSMNetwork* network, ModelGraphicsScene* scene, EFSMNetworkDiagramLayout* layout) {
    if (network == nullptr || scene == nullptr || layout == nullptr || !scene->hasModelLevelFilter()
        || scene->modelLevelFilter() != static_cast<unsigned int>(network->getId())
        || network->getStates() == nullptr) {
        return false;
    }

    EFSMNetworkDiagramLayout captured;
    captured.networkId = static_cast<unsigned long>(network->getId());
    captured.networkName = QString::fromStdString(network->getName());
    for (FSMState* state : *network->getStates()->list()) {
        if (state == nullptr) {
            continue;
        }
        GraphicalModelDataDefinition* item = scene->findGraphicalModelDataDefinition(state);
        if (item == nullptr) {
            return false;
        }
        const QPointF position = item->pos();
        if (!std::isfinite(position.x()) || !std::isfinite(position.y())) {
            return false;
        }
        captured.states.append({static_cast<unsigned long>(state->getId()),
                                QString::fromStdString(state->getName()), position});
    }

    *layout = std::move(captured);
    return true;
}

void restoreLayout(Simulator* simulator, EFSMNetwork* network, ModelGraphicsScene* scene,
                   const EFSMNetworkDiagramLayout& layout) {
    if (network == nullptr || scene == nullptr || !scene->hasModelLevelFilter()
        || scene->modelLevelFilter() != static_cast<unsigned int>(network->getId())
        || layout.networkId != static_cast<unsigned long>(network->getId())
        || network->getStates() == nullptr
        || (!layout.networkName.isEmpty() && layout.networkName != QString::fromStdString(network->getName()))) {
        return;
    }

    for (const auto& statePosition : layout.states) {
        if (!std::isfinite(statePosition.position.x()) || !std::isfinite(statePosition.position.y())) {
            continue;
        }
        FSMState* matchingState = nullptr;
        for (FSMState* state : *network->getStates()->list()) {
            if (state != nullptr && static_cast<unsigned long>(state->getId()) == statePosition.stateId
                && QString::fromStdString(state->getName()) == statePosition.stateName) {
                matchingState = state;
                break;
            }
        }
        if (matchingState == nullptr) {
            continue;
        }
        if (GraphicalModelDataDefinition* item = scene->findGraphicalModelDataDefinition(matchingState)) {
            item->setPos(statePosition.position);
            item->setOldPosition(statePosition.position.x(), statePosition.position.y());
        }
    }

    synchronize(simulator, scene);
}

} // namespace EFSMNetworkDiagramProjection
