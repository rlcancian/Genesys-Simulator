#ifndef EFSMNETWORKDIAGRAMLAYOUT_H
#define EFSMNETWORKDIAGRAMLAYOUT_H

#include <QPointF>
#include <QHash>
#include <QString>
#include <QVector>

class Model;

/**
 * @brief GUI-only presentation layout for one EFSMNetwork diagram.
 *
 * This is purely visual/presentation data: node positions and optional
 * transition control/label points. It never carries mathematical state
 * (current/initial state, guards, actions, priorities, probabilities) --
 * that remains the exclusive responsibility of `EFSMNetwork`/`FSMState`/
 * `EFSMTransition`. A missing or empty layout is valid; the future viewer
 * falls back to deterministic automatic positioning.
 *
 * `EFSMTransition` has no persistent identity in the kernel (unlike
 * `FSMState`, which is a `ModelDataDefinition`). The graphical entry uses
 * `ordinal` as the zero-based index in the complete `getTransitions()` list.
 * That sequence is written and reconstructed by the EFSM `.gen` persistence
 * contract and distinguishes parallel and unnamed transitions without using
 * display names. Source/destination names remain integrity checks. The
 * identity is stable only while transition order is unchanged.
 */
struct EFSMNetworkDiagramLayout {
    struct StatePosition {
        unsigned long stateId = 0;
        QString stateName;
        QPointF position;
    };

    struct TransitionLayout {
        QString sourceName;
        QString destinationName;
        QString transitionName;
        int ordinal = 0; // global EFSMNetwork transition-list index
        QVector<QPointF> controlPoints;
        bool hasLabelPosition = false;
        QPointF labelPosition;
    };

    unsigned long networkId = 0;
    QString networkName;
    QVector<StatePosition> states;
    QVector<TransitionLayout> transitions;

    bool isEmpty() const { return states.isEmpty() && transitions.isEmpty(); }
};

using EFSMNetworkDiagramLayoutsByModel =
    QHash<Model*, QHash<unsigned long, EFSMNetworkDiagramLayout>>;

#endif // EFSMNETWORKDIAGRAMLAYOUT_H
