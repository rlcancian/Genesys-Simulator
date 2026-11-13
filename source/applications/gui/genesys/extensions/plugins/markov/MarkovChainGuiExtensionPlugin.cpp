#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include "plugins/data/ModalModel/MarkovChainNetwork.h"
#include "plugins/data/ModalModel/MarkovState.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QVBoxLayout>
#include <QUndoCommand>
#include <QUndoStack>

#include <algorithm>
#include <cmath>
#include <memory>
#include <iterator>
#include <utility>
#include <vector>

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

class EditMarkovProbabilityUndoCommand final : public QUndoCommand {
public:
    EditMarkovProbabilityUndoCommand(MarkovChainNetwork::MarkovTransition* transition,
                                     ModelGraphicsScene* scene, double value)
        : _transition(transition), _scene(scene), _before(transition->getProbability()), _after(value) {
        setText(QObject::tr("Edit Markov transition probability"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(double value) {
        if (_transition == nullptr) { setObsolete(true); return; }
        _transition->setProbability(value);
        synchronizeMarkovProjection(nullptr, _scene);
    }
    MarkovChainNetwork::MarkovTransition* _transition;
    QPointer<ModelGraphicsScene> _scene;
    double _before;
    double _after;
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
    return count == 2;
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

void editSelectedProbability(const GuiExtensionRuntimeContext& context) {
    auto* transition = selectedTransition(context);
    if (transition == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    bool accepted = false;
    const double value = QInputDialog::getDouble(context.mainWindow, QObject::tr("Markov Transition Probability"),
        QObject::tr("Probability:"), transition->getProbability(), 0.0, 1.0, 6, &accepted);
    if (accepted && std::isfinite(value) && value != transition->getProbability())
        context.graphicsScene->getUndoStack()->push(
            new EditMarkovProbabilityUndoCommand(transition, context.graphicsScene, value));
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

void setSelectedInitialState(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    auto* state = selectedMarkovState(context);
    if (network != nullptr && state != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && network->getStates()->find(state) != network->getStates()->list()->end())
        context.graphicsScene->getUndoStack()->push(
            new SetMarkovInitialStateUndoCommand(network, context.graphicsScene, state));
}

void createTransition(const GuiExtensionRuntimeContext& context) {
    if (!canCreateTransition(context)) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<MarkovChainNetwork*>(scene->networkEditorOwner());
    std::vector<MarkovState*> states;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* state = graphical != nullptr ? dynamic_cast<MarkovState*>(graphical->getDataDefinition()) : nullptr;
        if (state != nullptr && network->getStates()->find(state) != network->getStates()->list()->end()) states.push_back(state);
    }
    if (states.size() != 2) return;

    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Create Markov Transition"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* sourceChoice = new QComboBox(&dialog);
    auto* destinationChoice = new QComboBox(&dialog);
    for (MarkovState* state : states) {
        const QVariant identity = QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(state));
        const QString name = QString::fromStdString(state->getName());
        sourceChoice->addItem(name, identity);
        destinationChoice->addItem(name, identity);
    }
    layout->addWidget(new QLabel(QObject::tr("Source state:"), &dialog));
    layout->addWidget(sourceChoice);
    layout->addWidget(new QLabel(QObject::tr("Destination state:"), &dialog));
    layout->addWidget(destinationChoice);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return;
    auto* source = reinterpret_cast<MarkovState*>(static_cast<quintptr>(sourceChoice->currentData().toULongLong()));
    auto* destination = reinterpret_cast<MarkovState*>(static_cast<quintptr>(destinationChoice->currentData().toULongLong()));
    if (source == nullptr || destination == nullptr) return;
    bool accepted = false;
    const double probability = QInputDialog::getDouble(context.mainWindow, QObject::tr("Markov Transition Probability"),
        QObject::tr("Probability:"), 1.0, 0.0, 1.0, 6, &accepted);
    if (!accepted || !std::isfinite(probability)) return;
    auto transition = std::make_unique<MarkovChainNetwork::MarkovTransition>(source, destination, probability);
    const unsigned int ordinal = static_cast<unsigned int>(network->getTransitions()->size());
    scene->getUndoStack()->push(new AddMarkovTransitionUndoCommand(network, std::move(transition), ordinal, scene));
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

        GuiNetworkEditorContribution editor;
        editor.id = kMarkovExtensionId;
        editor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<MarkovChainNetwork*>(definition) != nullptr;
        };
        editor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<MarkovState>();
        };
        editor.onDataDefinitionDropped = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<MarkovChainNetwork*>(definition);
            auto* state = dynamic_cast<MarkovState*>(dropped);
            if (network != nullptr && state != nullptr) network->addState(state);
        };
        registry->addNetworkEditor(std::move(editor));

        GuiActionContribution transition;
        transition.id = "modal.markov.create-transition";
        transition.text = "Create Markov Transition from Selected States...";
        transition.menuPath = "Model";
        transition.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        transition.isEnabled = canCreateTransition;
        transition.trigger = createTransition;
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

        GuiActionContribution editProbability;
        editProbability.id = "modal.markov.edit-transition-probability";
        editProbability.text = "Edit Selected Markov Transition Probability...";
        editProbability.menuPath = "Model";
        editProbability.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<MarkovChainNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editProbability.isEnabled = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
                && selectedTransition(context) != nullptr;
        };
        editProbability.trigger = editSelectedProbability;
        registry->addAction(std::move(editProbability));

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
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(MarkovChainGuiExtensionPlugin);
