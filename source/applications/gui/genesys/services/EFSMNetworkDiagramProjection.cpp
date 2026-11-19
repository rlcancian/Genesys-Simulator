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
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <QBrush>
#include <QPolygonF>
#include <QSet>
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
    // Preserve manual routes across domain/selection refreshes that recreate overlay objects.
    QHash<EFSMTransition*, QPointF> existingBends;
    QSet<EFSMTransition*> selectedTransitions;
    for (QGraphicsItem* graphic : scene->extensionOverlayItems(QStringLiteral("efsm"))) {
        if (auto* edge = dynamic_cast<EFSMTransitionGraphicsItem*>(graphic); edge != nullptr) {
            if (edge->hasManualBendPoint())
                existingBends.insert(edge->transition(), edge->manualBendPoint());
            if (edge->isSelected()) selectedTransitions.insert(edge->transition());
        }
    }
    scene->clearExtensionOverlayItems(QStringLiteral("efsm"));
    scene->clearExtensionOverlayItems(QStringLiteral("efsm.state.markers"));
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
        if (existingBends.contains(transition))
            item->setManualBendPoint(existingBends.value(transition));
        if (selectedTransitions.contains(transition)) item->setSelected(true);
        item->setFired(network->getLastTransition() == transition);
        scene->addExtensionOverlayItem(QStringLiteral("efsm"), item);
    }

    // Highlight initial and final states in the projection only; no domain property is changed.
    for (FSMState* state : *network->getStates()->list()) {
        if (GraphicalModelDataDefinition* item = scene->findGraphicalModelDataDefinition(state)) {
            const bool initial = state == network->getInitialState();
            const bool final = state->isFinalNode();
            const bool current = state == network->getCurrentState();
            item->setColor(current ? QColor(105, 165, 240)
                              : initial && final ? QColor(111, 190, 188)
                              : initial ? QColor(135, 210, 155)
                              : final ? QColor(245, 195, 125)
                                       : QColor(220, 220, 220));
            // Presentation-only markers are kept separate from selectable relations.
            const QRectF bounds = item->sceneBoundingRect();
            if (final) {
                auto* doubleBorder = new QGraphicsEllipseItem(bounds.adjusted(-7.0, -7.0, 7.0, 7.0));
                doubleBorder->setPen(QPen(QColor(55, 65, 85), 2.0));
                doubleBorder->setBrush(Qt::NoBrush);
                doubleBorder->setZValue(-0.5);
                doubleBorder->setAcceptedMouseButtons(Qt::NoButton);
                doubleBorder->setToolTip(QObject::tr("Final EFSM state"));
                scene->addExtensionOverlayItem(QStringLiteral("efsm.state.markers"), doubleBorder);
            }
            if (initial) {
                const qreal centerY = bounds.center().y();
                auto* arrow = new QGraphicsPolygonItem(QPolygonF()
                    << QPointF(bounds.left() - 23.0, centerY - 7.0)
                    << QPointF(bounds.left() - 9.0, centerY)
                    << QPointF(bounds.left() - 23.0, centerY + 7.0));
                arrow->setPen(QPen(QColor(30, 115, 75), 1.5));
                arrow->setBrush(QBrush(QColor(30, 115, 75)));
                arrow->setZValue(-0.5);
                arrow->setAcceptedMouseButtons(Qt::NoButton);
                arrow->setToolTip(QObject::tr("Initial EFSM state"));
                scene->addExtensionOverlayItem(QStringLiteral("efsm.state.markers"), arrow);
            }
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

    // An EFSM transition is not a ModelDataDefinition: retain the established
    // sequence ordinal plus source/destination/name integrity checks.
    if (network->getTransitions() != nullptr) {
        QVector<EFSMTransition*> transitions;
        for (EFSMTransition* transition : *network->getTransitions()->list())
            if (transition != nullptr) transitions.append(transition);
        for (QGraphicsItem* overlay : scene->extensionOverlayItems(QStringLiteral("efsm"))) {
            auto* item = dynamic_cast<EFSMTransitionGraphicsItem*>(overlay);
            if (item == nullptr || !item->hasManualBendPoint()) continue;
            const int ordinal = item->ordinal();
            if (ordinal < 0 || ordinal >= transitions.size()
                || transitions.at(ordinal) != item->transition()) continue;
            EFSMTransition* transition = item->transition();
            EFSMNetworkDiagramLayout::TransitionLayout route;
            route.ordinal = ordinal;
            route.sourceName = transition->getSource() != nullptr
                ? QString::fromStdString(transition->getSource()->getName()) : QString();
            route.destinationName = transition->getDestination() != nullptr
                ? QString::fromStdString(transition->getDestination()->getName()) : QString();
            route.transitionName = QString::fromStdString(transition->getName());
            route.controlPoints.append(item->manualBendPoint());
            captured.transitions.append(std::move(route));
        }
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

    QVector<EFSMTransition*> transitions;
    if (network->getTransitions() != nullptr) {
        for (EFSMTransition* transition : *network->getTransitions()->list())
            if (transition != nullptr) transitions.append(transition);
    }
    for (const auto& route : layout.transitions) {
        if (route.controlPoints.size() != 1 || route.ordinal < 0
            || route.ordinal >= transitions.size()) continue;
        EFSMTransition* transition = transitions.at(route.ordinal);
        if (transition == nullptr
            || (transition->getSource() != nullptr ? QString::fromStdString(transition->getSource()->getName()) : QString()) != route.sourceName
            || (transition->getDestination() != nullptr ? QString::fromStdString(transition->getDestination()->getName()) : QString()) != route.destinationName
            || QString::fromStdString(transition->getName()) != route.transitionName) continue;
        const QPointF bend = route.controlPoints.front();
        if (!std::isfinite(bend.x()) || !std::isfinite(bend.y())) continue;
        for (QGraphicsItem* overlay : scene->extensionOverlayItems(QStringLiteral("efsm"))) {
            auto* item = dynamic_cast<EFSMTransitionGraphicsItem*>(overlay);
            if (item != nullptr && item->transition() == transition && item->ordinal() == route.ordinal) {
                item->setManualBendPoint(bend);
                break;
            }
        }
    }
}

} // namespace EFSMNetworkDiagramProjection
