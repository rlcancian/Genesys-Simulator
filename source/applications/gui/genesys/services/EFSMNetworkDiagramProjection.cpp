#include "EFSMNetworkDiagramProjection.h"

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

} // namespace EFSMNetworkDiagramProjection
