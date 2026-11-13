#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "../../../services/GraphicalModelBuilder.h"

#include "plugins/data/ModalModel/MarkovChainNetwork.h"
#include "plugins/data/ModalModel/MarkovState.h"
#include "kernel/simulator/model/Model.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QUndoCommand>
#include <QUndoStack>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <memory>
#include <iterator>
#include <utility>
#include <unordered_set>
#include <vector>
#include <optional>

namespace {

constexpr auto kMarkovExtensionId = "modal.markov.gui";
void synchronizeMarkovProjection(Simulator*, ModelGraphicsScene*);

class AddMarkovTransitionUndoCommand final : public QUndoCommand {
public:
    AddMarkovTransitionUndoCommand(MarkovChainNetwork* network,
                                   std::unique_ptr<MarkovChainNetwork::MarkovTransition> transition,
                                   unsigned int ordinal, ModelGraphicsScene* scene)
        : _network(network), _transition(std::move(transition)), _ordinal(ordinal), _scene(scene) {
        setText(QObject::tr("Add Markov transition"));
    }

    void redo() override {
        if (_network == nullptr || !_transition
            || !_network->restoreTransition(_transition.get(), _ordinal)) {
            setObsolete(true);
            return;
        }
        _attached = _transition.get();
        _transition.release();
        _everAttached = true;
        if (_scene != nullptr) synchronizeMarkovProjection(nullptr, _scene);
    }

    void undo() override {
        if (_network == nullptr || !_everAttached || _attached == nullptr) {
            setObsolete(true);
            return;
        }
        _transition = _network->detachTransition(_attached, &_ordinal);
        if (!_transition) { setObsolete(true); return; }
        if (_scene != nullptr) synchronizeMarkovProjection(nullptr, _scene);
    }

private:
    MarkovChainNetwork* _network;
    std::unique_ptr<MarkovChainNetwork::MarkovTransition> _transition;
    unsigned int _ordinal;
    QPointer<ModelGraphicsScene> _scene;
    MarkovChainNetwork::MarkovTransition* _attached = nullptr;
    bool _everAttached = false;
};

class RemoveMarkovTransitionUndoCommand final : public QUndoCommand {
public:
    RemoveMarkovTransitionUndoCommand(MarkovChainNetwork* network,
                                      MarkovChainNetwork::MarkovTransition* transition,
                                      ModelGraphicsScene* scene)
        : _network(network), _transition(transition), _scene(scene) { setText(QObject::tr("Remove Markov transition")); }
    void redo() override {
        if (_network == nullptr || _transition == nullptr) { setObsolete(true); return; }
        _owned = _network->detachTransition(_transition, &_ordinal);
        if (!_owned) { setObsolete(true); return; }
        synchronizeMarkovProjection(nullptr, _scene);
    }
    void undo() override {
        if (_network == nullptr || !_owned || !_network->restoreTransition(_owned.get(), _ordinal)) { setObsolete(true); return; }
        _owned.release();
        synchronizeMarkovProjection(nullptr, _scene);
    }
private:
    MarkovChainNetwork* _network;
    MarkovChainNetwork::MarkovTransition* _transition;
    QPointer<ModelGraphicsScene> _scene;
    unsigned int _ordinal = 0;
    std::unique_ptr<MarkovChainNetwork::MarkovTransition> _owned;
};

class EditMarkovStatePropertiesUndoCommand final : public QUndoCommand {
public:
    struct Values { std::string name, label; std::uint32_t traceLevel = 0; bool report = false; };
    EditMarkovStatePropertiesUndoCommand(MarkovChainNetwork* network, MarkovState* state, Values after)
        : _network(network), _state(state), _before{state->getName(), state->getLabel(),
            static_cast<std::uint32_t>(state->getTraceLevelSpecific()), state->isReportStatistics()}, _after(std::move(after)) {
        setText(QObject::tr("Edit Markov state"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(const Values& v) {
        if (_network == nullptr || _state == nullptr
            || _network->getStates()->find(_state) == _network->getStates()->list()->end()) { setObsolete(true); return; }
        _state->setName(v.name); _state->setLabel(v.label);
        _state->setTraceLevelSpecific(static_cast<TraceManager::Level>(v.traceLevel));
        _state->setReportStatistics(v.report);
    }
    MarkovChainNetwork* _network; MarkovState* _state; Values _before, _after;
};

class EditMarkovTransitionPropertiesUndoCommand final : public QUndoCommand {
public:
    EditMarkovTransitionPropertiesUndoCommand(MarkovChainNetwork* network,
        MarkovChainNetwork::MarkovTransition* transition, ModelGraphicsScene* scene,
        std::string name, double probability)
        : _network(network), _transition(transition), _scene(scene), _beforeName(transition->getName()),
          _afterName(std::move(name)), _beforeProbability(transition->getProbability()), _afterProbability(probability) {
        setText(QObject::tr("Edit Markov transition"));
    }
    void redo() override { apply(_afterName, _afterProbability); }
    void undo() override { apply(_beforeName, _beforeProbability); }
private:
    void apply(const std::string& name, double probability) {
        if (_network == nullptr || _transition == nullptr
            || _network->getTransitions()->find(_transition) == _network->getTransitions()->list()->end()) {
            setObsolete(true); return;
        }
        _transition->setName(name); _transition->setProbability(probability);
        synchronizeMarkovProjection(nullptr, _scene);
    }
    MarkovChainNetwork* _network; MarkovChainNetwork::MarkovTransition* _transition;
    QPointer<ModelGraphicsScene> _scene; std::string _beforeName, _afterName;
    double _beforeProbability, _afterProbability;
};

class SetMarkovInitialStateUndoCommand final : public QUndoCommand {
public:
    SetMarkovInitialStateUndoCommand(MarkovChainNetwork* network, ModelGraphicsScene* scene, MarkovState* after)
        : _network(network), _scene(scene), _before(network->getInitialState()), _after(after) {
        setText(QObject::tr("Set Markov initial state"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(MarkovState* state) {
        if (_network == nullptr) { setObsolete(true); return; }
        _network->setInitialState(state);
        synchronizeMarkovProjection(nullptr, _scene);
    }
    MarkovChainNetwork* _network;
    QPointer<ModelGraphicsScene> _scene;
    MarkovState* _before;
    MarkovState* _after;
};

class RemoveMarkovStateUndoCommand final : public QUndoCommand {
public:
    RemoveMarkovStateUndoCommand(MarkovChainNetwork* network, MarkovState* state, ModelGraphicsScene* scene)
        : _network(network), _state(state), _scene(scene), _oldLevel(state != nullptr ? state->getLevel() : 0),
          _previousInitial(network != nullptr ? network->getInitialState() : nullptr),
          _previousCurrent(network != nullptr ? network->getCurrentState() : nullptr) {
        setText(QObject::tr("Remove Markov state"));
        if (network == nullptr || state == nullptr) return;
        unsigned int ordinal = 0;
        for (MarkovChainNetwork::MarkovTransition* transition : *network->getTransitions()->list()) {
            if (transition != nullptr && (transition->getSource() == state || transition->getDestination() == state)) {
                _transitions.push_back({transition, ordinal, {}});
            }
            ++ordinal;
        }
        if (scene != nullptr) {
            if (auto* item = scene->findGraphicalModelDataDefinition(state)) _position = item->pos();
        }
    }

    void redo() override {
        if (_scene == nullptr || _network == nullptr || _state == nullptr
            || _network->getStates()->find(_state) == _network->getStates()->list()->end()) {
            setObsolete(true);
            return;
        }
        for (auto& record : _transitions) {
            unsigned int detachedOrdinal = 0;
            record.owned = _network->detachTransition(record.transition, &detachedOrdinal);
            if (!record.owned) { setObsolete(true); return; }
        }
        _network->removeState(_state);
        if (Model* model = _state->getParentModel()) model->remove(_state);
        _state->setModelLevel(0);
        synchronizeScene(false);
    }

    void undo() override {
        if (_scene == nullptr || _network == nullptr || _state == nullptr
            || _network->getStates()->find(_state) != _network->getStates()->list()->end()) {
            setObsolete(true);
            return;
        }
        if (Model* model = _state->getParentModel()) model->insert(_state);
        _state->setModelLevel(_oldLevel);
        _network->addState(_state);
        if (_previousInitial != nullptr
            && _network->getStates()->find(_previousInitial) != _network->getStates()->list()->end())
            _network->setInitialState(_previousInitial);
        if (_previousCurrent != nullptr
            && _network->getStates()->find(_previousCurrent) != _network->getStates()->list()->end())
            _network->setCurrentState(_previousCurrent);
        for (auto& record : _transitions) {
            if (!record.owned || !_network->restoreTransition(record.owned.get(), record.ordinal)) {
                setObsolete(true);
                return;
            }
            record.owned.release();
        }
        synchronizeScene(true);
    }

private:
    struct TransitionRecord {
        MarkovChainNetwork::MarkovTransition* transition;
        unsigned int ordinal;
        std::unique_ptr<MarkovChainNetwork::MarkovTransition> owned;
    };

    void synchronizeScene(bool restorePosition) {
        if (_scene == nullptr) return;
        GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_scene->getSimulator(), _scene);
        if (restorePosition) {
            if (auto* item = _scene->findGraphicalModelDataDefinition(_state)) {
                item->setPos(_position);
                item->setOldPosition(_position.x(), _position.y());
            }
        }
        synchronizeMarkovProjection(_scene->getSimulator(), _scene);
    }

    MarkovChainNetwork* _network;
    MarkovState* _state;
    QPointer<ModelGraphicsScene> _scene;
    unsigned int _oldLevel;
    MarkovState* _previousInitial;
    MarkovState* _previousCurrent;
    QPointF _position;
    std::vector<TransitionRecord> _transitions;
};

void addArrow(QPainterPath& path, const QPointF& tip) {
    const QPointF tangent = path.pointAtPercent(0.98) - path.pointAtPercent(0.94);
    const qreal angle = std::atan2(tangent.y(), tangent.x());
    constexpr qreal length = 9.0;
    constexpr qreal spread = 0.45;
    path.moveTo(tip);
    path.lineTo(tip - QPointF(std::cos(angle - spread) * length, std::sin(angle - spread) * length));
    path.moveTo(tip);
    path.lineTo(tip - QPointF(std::cos(angle + spread) * length, std::sin(angle + spread) * length));
}

void synchronizeMarkovProjection(Simulator*, ModelGraphicsScene* scene) {
    if (scene == nullptr) return;
    std::unordered_set<MarkovChainNetwork::MarkovTransition*> selectedTransitions;
    for (QGraphicsItem* item : scene->extensionOverlayItems(QString::fromLatin1(kMarkovExtensionId))) {
        if (item != nullptr && item->isSelected() && item->data(1).isValid()) {
            auto* transition = reinterpret_cast<MarkovChainNetwork::MarkovTransition*>(
                static_cast<quintptr>(item->data(1).toULongLong()));
            if (transition != nullptr) selectedTransitions.insert(transition);
        }
    }
    scene->clearExtensionOverlayItems(QString::fromLatin1(kMarkovExtensionId));
    auto* network = dynamic_cast<MarkovChainNetwork*>(scene->networkEditorOwner());
    if (network == nullptr) return;

    for (MarkovState* state : *network->getStates()->list()) {
        GraphicalModelDataDefinition* graphical = scene->findGraphicalModelDataDefinition(state);
        if (graphical == nullptr) continue;
        if (network->getInitialState() == state) {
            auto* marker = new QGraphicsEllipseItem(graphical->sceneBoundingRect().adjusted(-5, -5, 5, 5));
            marker->setPen(QPen(QColor(40, 145, 90), 2.0, Qt::DashLine));
            marker->setBrush(Qt::NoBrush);
            marker->setZValue(-0.5);
            marker->setToolTip(QObject::tr("Initial Markov state"));
            scene->addExtensionOverlayItem(QString::fromLatin1(kMarkovExtensionId), marker);
        }
        if (network->getCurrentState() == state) {
            auto* marker = new QGraphicsEllipseItem(graphical->sceneBoundingRect().adjusted(-9, -9, 9, 9));
            marker->setPen(QPen(QColor(40, 100, 205), 2.0));
            marker->setBrush(Qt::NoBrush);
            marker->setZValue(-0.6);
            marker->setToolTip(QObject::tr("Current Markov state"));
            scene->addExtensionOverlayItem(QString::fromLatin1(kMarkovExtensionId), marker);
        }
    }

    const auto transitions = network->getTransitions()->list();
    for (auto it = transitions->begin(); it != transitions->end(); ++it) {
        auto* transition = *it;
        if (transition == nullptr || transition->getSource() == nullptr || transition->getDestination() == nullptr) continue;
        auto* source = scene->findGraphicalModelDataDefinition(transition->getSource());
        auto* destination = scene->findGraphicalModelDataDefinition(transition->getDestination());
        if (source == nullptr || destination == nullptr) continue;

        const QPointF start = source->sceneBoundingRect().center();
        const QPointF end = destination->sceneBoundingRect().center();
        QPainterPath path(start);
        if (transition->getSource() == transition->getDestination()) {
            const QPointF top(source->sceneBoundingRect().center().x(), source->sceneBoundingRect().top());
            path.moveTo(top);
            path.cubicTo(top + QPointF(-34, -52), top + QPointF(34, -52), top);
        } else {
            std::vector<MarkovChainNetwork::MarkovTransition*> parallel;
            for (auto* candidate : *transitions) {
                if (candidate != nullptr && candidate->getSource() == transition->getSource()
                    && candidate->getDestination() == transition->getDestination()) parallel.push_back(candidate);
            }
            const auto position = std::find(parallel.begin(), parallel.end(), transition);
            const std::size_t ordinal = position == parallel.end() ? 0u : std::distance(parallel.begin(), position);
            if (parallel.size() > 1u) {
                const QPointF delta = end - start;
                const qreal length = std::max<qreal>(1.0, std::hypot(delta.x(), delta.y()));
                const QPointF normal(-delta.y() / length, delta.x() / length);
                const qreal offset = (static_cast<qreal>(ordinal) - (parallel.size() - 1) / 2.0) * 22.0;
                path.quadTo((start + end) / 2.0 + normal * offset, end);
            } else {
                path.lineTo(end);
            }
        }
        addArrow(path, path.pointAtPercent(1.0));
        auto* edge = new QGraphicsPathItem(path);
        const bool sampled = network->getLastTransition() == transition;
        edge->setPen(QPen(sampled ? QColor(225, 90, 35) : QColor(65, 75, 90), sampled ? 4.0 : 2.0));
        edge->setBrush(Qt::NoBrush);
        edge->setZValue(-1.0);
        edge->setData(1, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(transition)));
        edge->setFlag(QGraphicsItem::ItemIsSelectable, true);
        edge->setToolTip(QStringLiteral("%1 → %2  p=%3")
                             .arg(QString::fromStdString(transition->getSource()->getName()),
                                  QString::fromStdString(transition->getDestination()->getName()))
                             .arg(transition->getProbability(), 0, 'g', 8));
        scene->addExtensionOverlayItem(QString::fromLatin1(kMarkovExtensionId), edge);
        if (selectedTransitions.contains(transition)) edge->setSelected(true);
        auto* label = new QGraphicsSimpleTextItem(QString::number(transition->getProbability(), 'g', 5));
        label->setBrush(QColor(35, 45, 60));
        label->setPos(path.pointAtPercent(0.5) + QPointF(4, -14));
        label->setZValue(-0.8);
        scene->addExtensionOverlayItem(QString::fromLatin1(kMarkovExtensionId), label);
    }
}

bool canCreateTransition(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr || context.graphicsScene->getUndoStack() == nullptr
        || dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) == nullptr) return false;
    int count = 0;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical != nullptr && dynamic_cast<MarkovState*>(graphical->getDataDefinition()) != nullptr) ++count;
    }
    return count == 1;
}

MarkovChainNetwork::MarkovTransition* selectedTransition(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr) return nullptr;
    auto* network = dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner());
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        if (dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) continue;
        auto* transition = reinterpret_cast<MarkovChainNetwork::MarkovTransition*>(
            static_cast<quintptr>(item->data(1).toULongLong()));
        if (transition != nullptr && network->getTransitions()->find(transition) != network->getTransitions()->list()->end())
            return transition;
    }
    return nullptr;
}

void removeSelectedTransition(const GuiExtensionRuntimeContext& context) {
    auto* transition = selectedTransition(context);
    if (transition == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    auto* network = dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner());
    context.graphicsScene->getUndoStack()->push(
        new RemoveMarkovTransitionUndoCommand(network, transition, context.graphicsScene));
}


MarkovState* selectedMarkovState(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* state = graphical != nullptr ? dynamic_cast<MarkovState*>(graphical->getDataDefinition()) : nullptr;
        if (state != nullptr) return state;
    }
    return nullptr;
}

bool canRemoveMarkovState(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    auto* state = selectedMarkovState(context);
    return network != nullptr && context.graphicsScene->getUndoStack() != nullptr && state != nullptr
        && network->getStates()->find(state) != network->getStates()->list()->end();
}

void removeSelectedMarkovState(const GuiExtensionRuntimeContext& context) {
    if (!canRemoveMarkovState(context)) return;
    auto* network = dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner());
    context.graphicsScene->getUndoStack()->push(
        new RemoveMarkovStateUndoCommand(network, selectedMarkovState(context), context.graphicsScene));
}

void setSelectedInitialState(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    auto* state = selectedMarkovState(context);
    if (network != nullptr && state != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && network->getStates()->find(state) != network->getStates()->list()->end())
        context.graphicsScene->getUndoStack()->push(
            new SetMarkovInitialStateUndoCommand(network, context.graphicsScene, state));
}

bool canValidateMarkovNetwork(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr
        && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
}

void validateMarkovNetwork(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr) return;
    std::string diagnostic;
    if (ModelDataDefinition::Check(network, diagnostic)) {
        QMessageBox::information(context.mainWindow, QObject::tr("Markov Network Validation"),
                                 QObject::tr("The network is valid."));
    } else {
        QMessageBox::warning(context.mainWindow, QObject::tr("Markov Network Validation"),
                             QString::fromStdString(diagnostic));
    }
}


class MarkovChainGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return kMarkovExtensionId; }
    std::string displayName() const override { return "Markov Chain Network Editor"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"MarkovChainNetwork"}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        GuiDiagramProjectionContribution projection;
        projection.id = kMarkovExtensionId;
        projection.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<MarkovChainNetwork*>(definition) != nullptr;
        };
        projection.synchronize = synchronizeMarkovProjection;
        registry->addDiagramProjection(std::move(projection));

        GuiActionContribution validate;
        validate.id = "modal.markov.validate-network";
        validate.text = "Validate Markov Network Probabilities...";
        validate.menuPath = "Model";
        validate.isVisible = canValidateMarkovNetwork;
        validate.isEnabled = canValidateMarkovNetwork;
        validate.trigger = validateMarkovNetwork;
        registry->addAction(std::move(validate));

        GuiNetworkEditorContribution editor;
        editor.id = kMarkovExtensionId;
        editor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<MarkovChainNetwork*>(definition) != nullptr;
        };
        editor.ownsEditableDataDefinition = [](ModelDataDefinition* definition, ModelDataDefinition* candidate) {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* state = dynamic_cast<MarkovState*>(candidate);
            return network != nullptr && state != nullptr
                && network->getStates()->find(state) != network->getStates()->list()->end();
        };
        editor.createDataDefinitionPropertyCommand = [](ModelDataDefinition* definition, ModelDataDefinition* candidate,
            const std::string& property, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* state = dynamic_cast<MarkovState*>(candidate);
            if (network == nullptr || state == nullptr
                || network->getStates()->find(state) == network->getStates()->list()->end()) return nullptr;
            EditMarkovStatePropertiesUndoCommand::Values v{state->getName(), state->getLabel(),
                static_cast<std::uint32_t>(state->getTraceLevelSpecific()), state->isReportStatistics()};
            if (property == "Name") v.name = value;
            else if (property == "Label") v.label = value;
            else if (property == "Report Statistics") v.report = value == "true" || value == "1";
            else if (property == "Trace Level") {
                unsigned int n = 0; const auto [end, ec] = std::from_chars(value.data(), value.data()+value.size(), n);
                if (ec != std::errc{} || end != value.data()+value.size()) return nullptr; v.traceLevel = n;
            } else return nullptr;
            return new EditMarkovStatePropertiesUndoCommand(network, state, std::move(v));
        };
        editor.canStartRelationCreation = [](ModelDataDefinition* definition, QGraphicsItem* item) {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
            auto* state = graphical != nullptr ? dynamic_cast<MarkovState*>(graphical->getDataDefinition()) : nullptr;
            return network != nullptr && state != nullptr
                && network->getStates()->find(state) != network->getStates()->list()->end();
        };
        editor.createRelationCommand = [](ModelDataDefinition* definition, QGraphicsItem* sourceItem,
                                           QGraphicsItem* destinationItem) -> QUndoCommand* {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* sourceGraphic = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* destinationGraphic = dynamic_cast<GraphicalModelDataDefinition*>(destinationItem);
            auto* source = sourceGraphic != nullptr ? dynamic_cast<MarkovState*>(sourceGraphic->getDataDefinition()) : nullptr;
            auto* destination = destinationGraphic != nullptr ? dynamic_cast<MarkovState*>(destinationGraphic->getDataDefinition()) : nullptr;
            if (network == nullptr || source == nullptr || destination == nullptr
                || network->getStates()->find(source) == network->getStates()->list()->end()
                || network->getStates()->find(destination) == network->getStates()->list()->end()) return nullptr;
            const unsigned int ordinal = static_cast<unsigned int>(network->getTransitions()->size());
            std::string name = "M" + std::to_string(ordinal);
            auto transition = std::make_unique<MarkovChainNetwork::MarkovTransition>(source, destination, 1.0, name);
            return new AddMarkovTransitionUndoCommand(network, std::move(transition), ordinal,
                sourceGraphic->scene() != nullptr ? dynamic_cast<ModelGraphicsScene*>(sourceGraphic->scene()) : nullptr);
        };
        editor.elementProperties = [](ModelDataDefinition* definition, QGraphicsItem* item) {
            std::vector<GuiNetworkEditorContribution::ElementProperty> result;
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            if (network == nullptr || item == nullptr) return result;
            if (auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
                auto* state = dynamic_cast<MarkovState*>(graphical->getDataDefinition());
                if (state != nullptr && network->getStates()->find(state) != network->getStates()->list()->end()) {
                    using P = GuiNetworkEditorContribution::ElementProperty;
                    result.push_back(P{"Initial State", network->getInitialState() == state ? "true" : "false", {}, P::Kind::Boolean, true});
                }
                return result;
            }
            if (dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) return result;
            auto* transition = reinterpret_cast<MarkovChainNetwork::MarkovTransition*>(
                static_cast<quintptr>(item->data(1).toULongLong()));
            if (transition == nullptr || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return result;
            using P = GuiNetworkEditorContribution::ElementProperty;
            result.push_back(P{"Name", transition->getName(), {}, P::Kind::String, true});
            result.push_back(P{"Source State", transition->getSource() ? transition->getSource()->getName() : "", {}, P::Kind::String, false});
            result.push_back(P{"Destination State", transition->getDestination() ? transition->getDestination()->getName() : "", {}, P::Kind::String, false});
            result.push_back(P{"Probability", std::to_string(transition->getProbability()), {}, P::Kind::Double, true});
            return result;
        };
        editor.createElementPropertyCommand = [](ModelDataDefinition* definition, QGraphicsItem* item,
            const std::string& property, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            if (network == nullptr || item == nullptr) return nullptr;
            if (auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
                auto* state = dynamic_cast<MarkovState*>(graphical->getDataDefinition());
                if (property != "Initial State" || state == nullptr
                    || network->getStates()->find(state) == network->getStates()->list()->end()) return nullptr;
                const bool selected = value == "true" || value == "1";
                if (selected) return new SetMarkovInitialStateUndoCommand(network,
                    dynamic_cast<ModelGraphicsScene*>(item->scene()), state);
                if (network->getInitialState() != state) return nullptr;
                return new SetMarkovInitialStateUndoCommand(network,
                    dynamic_cast<ModelGraphicsScene*>(item->scene()), nullptr);
            }
            if (!item->data(1).isValid()) return nullptr;
            auto* transition = reinterpret_cast<MarkovChainNetwork::MarkovTransition*>(
                static_cast<quintptr>(item->data(1).toULongLong()));
            if (transition == nullptr || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return nullptr;
            std::string name = transition->getName(); double probability = transition->getProbability();
            if (property == "Name") name = value;
            else if (property == "Probability") {
                const auto [end, ec] = std::from_chars(value.data(), value.data()+value.size(), probability);
                if (ec != std::errc{} || end != value.data()+value.size() || !std::isfinite(probability)
                    || probability < 0.0 || probability > 1.0) return nullptr;
            } else return nullptr;
            return new EditMarkovTransitionPropertiesUndoCommand(network, transition,
                dynamic_cast<ModelGraphicsScene*>(item->scene()), std::move(name), probability);
        };
        editor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<MarkovState>();
        };
        editor.onDataDefinitionDropped = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* state = dynamic_cast<MarkovState*>(dropped);
            if (network != nullptr && state != nullptr) network->addState(state);
        };
        editor.onDataDefinitionRemoved = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* state = dynamic_cast<MarkovState*>(dropped);
            if (network != nullptr && state != nullptr) network->removeState(state);
        };
        registry->addNetworkEditor(std::move(editor));

        GuiActionContribution transition;
        transition.id = "modal.markov.create-transition";
        transition.text = "Connect Selected Markov State";
        transition.menuPath = "Model";
        transition.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        transition.isEnabled = canCreateTransition;
        transition.trigger = [](const GuiExtensionRuntimeContext& context) {
            auto* scene = context.graphicsScene;
            if (scene == nullptr) return;
            for (QGraphicsItem* item : scene->selectedItems()) {
                auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
                auto* state = graphical != nullptr ? dynamic_cast<MarkovState*>(graphical->getDataDefinition()) : nullptr;
                auto* network = dynamic_cast<MarkovChainNetwork*>(scene->networkEditorOwner());
                if (network != nullptr && state != nullptr
                    && network->getStates()->find(state) != network->getStates()->list()->end()) {
                    scene->beginNetworkRelationCreation(graphical); return;
                }
            }
        };
        registry->addAction(std::move(transition));

        GuiActionContribution removeTransition;
        removeTransition.id = "modal.markov.remove-transition";
        removeTransition.text = "Remove Selected Markov Transition";
        removeTransition.menuPath = "Model";
        removeTransition.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeTransition.isEnabled = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
                && selectedTransition(context) != nullptr;
        };
        removeTransition.trigger = removeSelectedTransition;
        registry->addAction(std::move(removeTransition));


        GuiActionContribution setInitial;
        setInitial.id = "modal.markov.set-initial-state";
        setInitial.text = "Set Selected Markov State as Initial";
        setInitial.menuPath = "Model";
        setInitial.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        setInitial.isEnabled = [](const GuiExtensionRuntimeContext& context) {
            auto* network = context.graphicsScene != nullptr
                ? dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
            auto* state = selectedMarkovState(context);
            return network != nullptr && context.graphicsScene->getUndoStack() != nullptr && state != nullptr
                && state != network->getInitialState()
                && network->getStates()->find(state) != network->getStates()->list()->end();
        };
        setInitial.trigger = setSelectedInitialState;
        registry->addAction(std::move(setInitial));

        GuiActionContribution removeState;
        removeState.id = "modal.markov.remove-state";
        removeState.text = "Remove Selected Markov State";
        removeState.menuPath = "Model";
        removeState.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeState.isEnabled = canRemoveMarkovState;
        removeState.trigger = removeSelectedMarkovState;
        registry->addAction(std::move(removeState));
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(MarkovChainGuiExtensionPlugin);
