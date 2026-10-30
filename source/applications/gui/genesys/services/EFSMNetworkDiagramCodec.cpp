#include "EFSMNetworkDiagramCodec.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/util/List.h"
#include "kernel/util/Util.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <QLocale>
#include <QRegularExpression>
#include <QTextStream>
#include <QUrl>

#include <cmath>

namespace {

QString encodeToken(const QString& text) {
    return QString::fromUtf8(QUrl::toPercentEncoding(text));
}

QString decodeToken(const QString& text) {
    return QUrl::fromPercentEncoding(text.toUtf8());
}

double parseDouble(const QString& text, bool* ok) {
    return QLocale::c().toDouble(text.trimmed(), ok);
}

QString formatPoint(const QPointF& point) {
    return QString("(%1,%2)").arg(point.x(), 0, 'f', 4).arg(point.y(), 0, 'f', 4);
}

bool parsePoint(const QString& text, QPointF* out) {
    static const QRegularExpression pointRegex("^\\(([^,]+),([^\\)]+)\\)$");
    const QRegularExpressionMatch match = pointRegex.match(text.trimmed());
    if (!match.hasMatch() || out == nullptr) {
        return false;
    }
    bool okX = false;
    bool okY = false;
    const double x = parseDouble(match.captured(1), &okX);
    const double y = parseDouble(match.captured(2), &okY);
    if (!okX || !okY || !std::isfinite(x) || !std::isfinite(y)) {
        return false;
    }
    *out = QPointF(x, y);
    return true;
}

QString formatControlPoints(const QVector<QPointF>& points) {
    QStringList encoded;
    encoded.reserve(points.size());
    for (const QPointF& point : points) {
        encoded << QString("%1,%2").arg(point.x(), 0, 'f', 4).arg(point.y(), 0, 'f', 4);
    }
    return encoded.join(";");
}

QVector<QPointF> parseControlPoints(const QString& text) {
    QVector<QPointF> points;
    const QStringList pairs = text.split(';', Qt::SkipEmptyParts);
    for (const QString& pair : pairs) {
        const QStringList coords = pair.split(',');
        if (coords.size() != 2) {
            continue;
        }
        bool okX = false;
        bool okY = false;
        const double x = parseDouble(coords[0], &okX);
        const double y = parseDouble(coords[1], &okY);
        if (okX && okY && std::isfinite(x) && std::isfinite(y)) {
            points.append(QPointF(x, y));
        }
    }
    return points;
}

// key=value tokenizer matching the existing tab-separated `.gui` line convention.
QHash<QString, QString> tokenizeFields(const QStringList& tokens, int startIndex) {
    QHash<QString, QString> fields;
    for (int i = startIndex; i < tokens.size(); ++i) {
        const QString token = tokens[i].trimmed();
        const int eq = token.indexOf('=');
        if (eq <= 0) {
            continue;
        }
        fields.insert(token.left(eq), token.mid(eq + 1));
    }
    return fields;
}

// EFSMTransition has no persistent id. Its global position is nevertheless
// stable across the existing .gen save/load contract: EFSMNetwork writes and
// reconstructs transitions by this same sequence index. The graphical codec
// also validates source/destination and the exact persisted name before
// accepting the reference, so topology changes fail closed instead of being
// guessed.
QVector<EFSMTransition*> liveTransitions(EFSMNetwork* network) {
    QVector<EFSMTransition*> transitions;
    if (network == nullptr || network->getTransitions() == nullptr) {
        return transitions;
    }
    for (EFSMTransition* transition : *network->getTransitions()->list()) {
        if (transition == nullptr) {
            continue;
        }
        transitions.append(transition);
    }
    return transitions;
}

EFSMNetwork* findNetworkById(Model* model, unsigned long id) {
    if (model == nullptr || model->getDataManager() == nullptr) {
        return nullptr;
    }
    ModelDataDefinition* found = model->getDataManager()->getDataDefinition(Util::TypeOf<EFSMNetwork>(), id);
    return dynamic_cast<EFSMNetwork*>(found);
}

EFSMNetwork* findUniqueNetworkByName(Model* model, const QString& name) {
    if (model == nullptr || model->getDataManager() == nullptr || name.isEmpty()) {
        return nullptr;
    }
    EFSMNetwork* match = nullptr;
    List<ModelDataDefinition*>* definitions =
        model->getDataManager()->getDataDefinitionList(Util::TypeOf<EFSMNetwork>());
    if (definitions == nullptr) {
        return nullptr;
    }
    for (ModelDataDefinition* definition : *definitions->list()) {
        auto* candidate = dynamic_cast<EFSMNetwork*>(definition);
        if (candidate == nullptr || QString::fromStdString(candidate->getName()) != name) {
            continue;
        }
        if (match != nullptr) {
            return nullptr;
        }
        match = candidate;
    }
    return match;
}

FSMState* findStateById(EFSMNetwork* network, unsigned long id) {
    if (network == nullptr || network->getStates() == nullptr) {
        return nullptr;
    }
    for (FSMState* state : *network->getStates()->list()) {
        if (state != nullptr && state->getId() == id) {
            return state;
        }
    }
    return nullptr;
}

FSMState* findUniqueStateByName(EFSMNetwork* network, const QString& name) {
    if (network == nullptr || network->getStates() == nullptr) {
        return nullptr;
    }
    FSMState* match = nullptr;
    for (FSMState* state : *network->getStates()->list()) {
        if (state != nullptr && QString::fromStdString(state->getName()) == name) {
            if (match != nullptr) {
                return nullptr;
            }
            match = state;
        }
    }
    return match;
}

} // namespace

namespace EFSMNetworkDiagramCodec {

void writeSection(QTextStream& out, Simulator* simulator,
                   const QHash<unsigned long, EFSMNetworkDiagramLayout>& layouts) {
    if (simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }

    // Deterministic write order: ascending network id.
    QVector<unsigned long> networkIds = layouts.keys().toVector();
    std::sort(networkIds.begin(), networkIds.end());

    for (unsigned long networkId : networkIds) {
        const EFSMNetworkDiagramLayout& layout = layouts.value(networkId);
        if (layout.isEmpty()) {
            continue;
        }
        EFSMNetwork* network = findNetworkById(model, networkId);
        if (network == nullptr) {
            continue;
        }

        out << QString("%1\tEFSMNetworkDiagram\tclassname=%2\tname=%3\tversion=%4\n")
                   .arg(networkId)
                   .arg(QString::fromStdString(network->getClassname()))
                   .arg(encodeToken(QString::fromStdString(network->getName())))
                   .arg(kSupportedSchemaVersion);

        // States: deterministic order by id, resolved/skipped individually.
        QVector<EFSMNetworkDiagramLayout::StatePosition> states = layout.states;
        std::sort(states.begin(), states.end(),
                  [](const auto& a, const auto& b) { return a.stateId < b.stateId; });
        for (const auto& statePosition : states) {
            FSMState* state = findStateById(network, statePosition.stateId);
            if (state == nullptr) {
                continue;
            }
            out << QString("%1\tEFSMState\tid=%2\tname=%3\tposition=%4\n")
                       .arg(networkId)
                       .arg(statePosition.stateId)
                       .arg(encodeToken(QString::fromStdString(state->getName())))
                       .arg(formatPoint(statePosition.position));
        }

        const QVector<EFSMTransition*> transitions = liveTransitions(network);
        for (const auto& transitionLayout : layout.transitions) {
            if (transitionLayout.ordinal < 0 || transitionLayout.ordinal >= transitions.size()) {
                continue;
            }
            EFSMTransition* transition = transitions.at(transitionLayout.ordinal);
            const QString source = transition->getSource() != nullptr
                                       ? QString::fromStdString(transition->getSource()->getName())
                                       : QString();
            const QString destination = transition->getDestination() != nullptr
                                            ? QString::fromStdString(transition->getDestination()->getName())
                                            : QString();
            const QString name = QString::fromStdString(transition->getName());
            if (source != transitionLayout.sourceName || destination != transitionLayout.destinationName
                || name != transitionLayout.transitionName) {
                continue;
            }
            QString line = QString("%1\tEFSMTransition\tsource=%2\tdestination=%3\tname=%4\tordinal=%5")
                                .arg(networkId)
                                .arg(encodeToken(source))
                                .arg(encodeToken(destination))
                                .arg(encodeToken(name))
                                .arg(transitionLayout.ordinal);
            if (!transitionLayout.controlPoints.isEmpty()) {
                line += "\tcontrolpoints=" + formatControlPoints(transitionLayout.controlPoints);
            }
            if (transitionLayout.hasLabelPosition) {
                line += "\tlabel=" + formatPoint(transitionLayout.labelPosition);
            }
            out << line << "\n";
        }
    }
}

QHash<unsigned long, EFSMNetworkDiagramLayout> parseSection(const QStringList& lines, Model* model) {
    QHash<unsigned long, EFSMNetworkDiagramLayout> result;
    if (model == nullptr) {
        return result;
    }

    QHash<unsigned long, int> declaredVersionByNetworkId;
    QHash<unsigned long, bool> validHeaderByNetworkId;
    QHash<unsigned long, EFSMNetworkDiagramLayout> rawByNetworkId;

    // First pass: collect declared schema versions so state/transition lines
    // belonging to an unsupported or missing version can be rejected as a group.
    for (const QString& rawLine : lines) {
        if (rawLine.trimmed().isEmpty()) {
            continue;
        }
        const QStringList tokens = rawLine.split('\t', Qt::SkipEmptyParts);
        if (tokens.size() < 2 || tokens[1].trimmed() != "EFSMNetworkDiagram") {
            continue;
        }
        bool idOk = false;
        const unsigned long networkId = tokens[0].trimmed().toULong(&idOk);
        if (!idOk) {
            continue;
        }
        const QHash<QString, QString> fields = tokenizeFields(tokens, 2);
        const QString expectedClassname = QString::fromStdString(Util::TypeOf<EFSMNetwork>());
        if (!fields.contains("classname") || fields.value("classname") != expectedClassname
            || !fields.contains("name")) {
            validHeaderByNetworkId.insert(networkId, false);
            continue;
        }
        bool versionOk = false;
        const int version = fields.value("version").toInt(&versionOk);
        if (!versionOk) {
            validHeaderByNetworkId.insert(networkId, false);
            continue;
        }
        if (validHeaderByNetworkId.contains(networkId)) {
            const EFSMNetworkDiagramLayout& existing = rawByNetworkId.value(networkId);
            if (!validHeaderByNetworkId.value(networkId) || declaredVersionByNetworkId.value(networkId) != version
                || existing.networkName != decodeToken(fields.value("name"))) {
                validHeaderByNetworkId.insert(networkId, false);
                continue;
            }
        }
        declaredVersionByNetworkId.insert(networkId, version);
        validHeaderByNetworkId.insert(networkId, true);
        EFSMNetworkDiagramLayout layout;
        layout.networkId = networkId;
        layout.networkName = decodeToken(fields.value("name"));
        rawByNetworkId.insert(networkId, layout);
    }

    for (const QString& rawLine : lines) {
        if (rawLine.trimmed().isEmpty()) {
            continue;
        }
        const QStringList tokens = rawLine.split('\t', Qt::SkipEmptyParts);
        if (tokens.size() < 2) {
            continue;
        }
        const QString kind = tokens[1].trimmed();
        if (kind == "EFSMNetworkDiagram") {
            continue; // already handled above
        }

        bool idOk = false;
        const unsigned long networkId = tokens[0].trimmed().toULong(&idOk);
        if (!idOk) {
            continue;
        }
        auto versionIt = declaredVersionByNetworkId.find(networkId);
        if (versionIt == declaredVersionByNetworkId.end() || versionIt.value() != kSupportedSchemaVersion
            || !validHeaderByNetworkId.value(networkId, false)) {
            // Unknown/unsupported/missing version for this network: reject
            // its lines in a controlled way instead of guessing a mapping.
            continue;
        }
        auto layoutIt = rawByNetworkId.find(networkId);
        if (layoutIt == rawByNetworkId.end()) {
            continue;
        }

        const QHash<QString, QString> fields = tokenizeFields(tokens, 2);

        if (kind == "EFSMState") {
            if (!fields.contains("id") || !fields.contains("name") || !fields.contains("position")) {
                continue; // the v1 state identity and position are mandatory
            }
            bool stateIdOk = false;
            const unsigned long stateId = fields.value("id").toULong(&stateIdOk);
            QPointF position;
            const bool hasPosition = parsePoint(fields.value("position"), &position);
            if (!stateIdOk || !hasPosition) {
                continue; // malformed: skip this entry only
            }
            EFSMNetworkDiagramLayout::StatePosition statePosition;
            statePosition.stateId = stateId;
            statePosition.stateName = decodeToken(fields.value("name"));
            statePosition.position = position;
            layoutIt->states.append(statePosition);
        } else if (kind == "EFSMTransition") {
            if (!fields.contains("source") || !fields.contains("destination") || !fields.contains("name")
                || !fields.contains("ordinal")) {
                continue; // malformed: skip this entry only
            }
            bool ordinalOk = false;
            const int ordinal = fields.value("ordinal").toInt(&ordinalOk);
            if (!ordinalOk || ordinal < 0) {
                continue;
            }
            EFSMNetworkDiagramLayout::TransitionLayout transitionLayout;
            transitionLayout.sourceName = decodeToken(fields.value("source"));
            transitionLayout.destinationName = decodeToken(fields.value("destination"));
            transitionLayout.transitionName = decodeToken(fields.value("name"));
            transitionLayout.ordinal = ordinal;
            if (fields.contains("controlpoints")) {
                transitionLayout.controlPoints = parseControlPoints(fields.value("controlpoints"));
            }
            if (fields.contains("label")) {
                QPointF label;
                if (parsePoint(fields.value("label"), &label)) {
                    transitionLayout.hasLabelPosition = true;
                    transitionLayout.labelPosition = label;
                }
            }
            layoutIt->transitions.append(transitionLayout);
        }
        // Unknown line kinds are ignored rather than treated as fatal.
    }

    // Resolve against the live model: keep only entries whose network/state
    // still exist, and whose global transition index and integrity fields still
    // resolve. Ambiguous/unresolved entries are dropped.
    for (auto it = rawByNetworkId.begin(); it != rawByNetworkId.end(); ++it) {
        const unsigned long networkId = it.key();
        EFSMNetworkDiagramLayout& layout = it.value();
        EFSMNetwork* network = findNetworkById(model, networkId);
        if (network != nullptr && QString::fromStdString(network->getName()) != layout.networkName) {
            continue; // ID/name disagreement must never select the wrong Network
        }
        if (network == nullptr && !layout.networkName.isEmpty()) {
            network = findUniqueNetworkByName(model, layout.networkName);
        }
        if (network == nullptr) {
            continue; // network no longer exists: drop its whole layout
        }
        if (QString::fromStdString(network->getClassname())
            != QString::fromStdString(Util::TypeOf<EFSMNetwork>())) {
            continue;
        }

        EFSMNetworkDiagramLayout resolved;
        resolved.networkId = static_cast<unsigned long>(network->getId());
        resolved.networkName = QString::fromStdString(network->getName());

        for (const auto& statePosition : layout.states) {
            FSMState* state = findStateById(network, statePosition.stateId);
            if (state == nullptr && !statePosition.stateName.isEmpty()) {
                state = findUniqueStateByName(network, statePosition.stateName);
            }
            if (state == nullptr) {
                continue; // unresolved state reference: skip this entry only
            }
            if (QString::fromStdString(state->getName()) != statePosition.stateName) {
                continue; // id/name mismatch: do not attach to an approximate state
            }
            EFSMNetworkDiagramLayout::StatePosition resolvedState;
            resolvedState.stateId = static_cast<unsigned long>(state->getId());
            resolvedState.stateName = QString::fromStdString(state->getName());
            resolvedState.position = statePosition.position;
            resolved.states.append(resolvedState);
        }

        const QVector<EFSMTransition*> transitions = liveTransitions(network);
        for (const auto& transitionLayout : layout.transitions) {
            if (transitionLayout.ordinal < 0 || transitionLayout.ordinal >= transitions.size()) {
                continue; // no live transition at this persisted sequence index
            }
            EFSMTransition* transition = transitions.at(transitionLayout.ordinal);
            const QString source = transition->getSource() != nullptr
                                       ? QString::fromStdString(transition->getSource()->getName())
                                       : QString();
            const QString destination = transition->getDestination() != nullptr
                                            ? QString::fromStdString(transition->getDestination()->getName())
                                            : QString();
            const QString name = QString::fromStdString(transition->getName());
            if (source != transitionLayout.sourceName || destination != transitionLayout.destinationName
                || name != transitionLayout.transitionName) {
                continue; // stale or ambiguous reference: skip this entry only
            }
            resolved.transitions.append(transitionLayout);
        }

        if (!resolved.isEmpty()) {
            result.insert(resolved.networkId, resolved);
        }
    }

    return result;
}

} // namespace EFSMNetworkDiagramCodec
