#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "../../../graphicals/NetworkBendGraphicsItem.h"
#include "../../../services/NetworkBendDiagramCodec.h"
#include "../../../services/GraphicalModelBuilder.h"

#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/TraceManager.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "kernel/util/Util.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QUndoCommand>
#include <QUndoStack>

#include <cmath>
#include <charconv>
#include <sstream>
#include <map>
#include <memory>
#include <limits>
#include <utility>
#include <unordered_set>

namespace {

constexpr auto kCpnExtensionId = "modal.cpn.gui";
using Marking = std::map<PetriPlace*, std::map<std::string, unsigned int>>;
void synchronizeCpnProjection(Simulator*, ModelGraphicsScene*);

Marking captureMarking(ColoredPetriNetNetwork* network) {
    Marking result;
    if (network == nullptr) return result;
    for (PetriPlace* place : *network->getPlaces()->list())
        if (place != nullptr && place->getAllTokens() != nullptr) result[place] = *place->getAllTokens();
    return result;
}

void restoreMarking(const Marking& marking) {
    for (const auto& [place, tokens] : marking) {
        if (place == nullptr) continue;
        place->clearTokens();
        for (const auto& [color, quantity] : tokens) place->setTokens(quantity, color);
    }
}

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

// This probe deliberately does not evaluate transition guards. The model parser
// can consume RNG or perform stateful evaluation, which must never be triggered
// just because Qt repaints a scene or opens an action menu.
struct CpnPreviewReadiness {
    bool tokenAndCapacityReady = false;
    bool guardPending = false;
    QString explanation;
};

CpnPreviewReadiness previewCpnReadiness(const ColoredPetriNetNetwork* network, CPNTransition* transition) {
    if (network == nullptr || transition == nullptr || network->getTransitions() == nullptr
        || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) {
        return {false, false, QObject::tr("Transition does not belong to this CPN Network")};
    }

    const auto inputs = network->getInputArcs(transition);
    const auto outputs = network->getOutputArcs(transition);
    if (inputs.empty() && outputs.empty()) {
        return {false, false, QObject::tr("No incident arcs: connect a place to this transition")};
    }

    using ColorCounts = std::map<std::string, unsigned long long>;
    std::map<PetriPlace*, ColorCounts> required;
    std::map<PetriPlace*, ColorCounts> produced;
    const auto accumulate = [](std::map<PetriPlace*, ColorCounts>& amounts,
                               const CPNArc* arc) -> bool {
        if (arc == nullptr || arc->getPlace() == nullptr) return false;
        for (const auto& [color, count] : arc->getInscriptions()) {
            unsigned long long& total = amounts[arc->getPlace()][color];
            if (total > std::numeric_limits<unsigned long long>::max() - count) return false;
            total += count;
        }
        return true;
    };

    for (CPNArc* arc : inputs)
        if (!accumulate(required, arc))
            return {false, false, QObject::tr("Invalid or overflowing input arc inscription")};
    for (CPNArc* arc : outputs)
        if (!accumulate(produced, arc))
            return {false, false, QObject::tr("Invalid or overflowing output arc inscription")};

    for (const auto& [place, colors] : required) {
        for (const auto& [color, count] : colors) {
            const unsigned int available = place->getTokens(color);
            if (count > available) {
                return {false, false,
                    QObject::tr("%1: color %2 requires %3 tokens, but only %4 are available")
                        .arg(QString::fromStdString(place->getName()),
                             QString::fromStdString(color))
                        .arg(count).arg(available)};
            }
        }
    }

    const auto maxTokens = std::numeric_limits<unsigned int>::max();
    for (const auto& [place, colors] : produced) {
        for (const auto& [color, count] : colors) {
            const unsigned long long available = place->getTokens(color);
            const unsigned long long consumed = required[place][color];
            if (count > maxTokens || available - consumed + count > maxTokens) {
                return {false, false,
                    QObject::tr("%1: output for color %2 would overflow token capacity")
                        .arg(QString::fromStdString(place->getName()),
                             QString::fromStdString(color))};
            }
        }
    }

    if (!transition->getGuardExpression().empty()) {
        return {true, true, QObject::tr(
            "Token prerequisites satisfied; guard NOT evaluated in GUI preview. "
            "The guard is evaluated only when firing is requested or simulation runs.")};
    }
    return {true, false, QObject::tr("Enabled by marking and topology (no guard expression)")};
}

void synchronizeCpnProjection(Simulator*, ModelGraphicsScene* scene) {
    if (scene == nullptr) return;
    std::unordered_set<CPNArc*> selectedArcs;
    QHash<QString, QPointF> previousBends;
    for (QGraphicsItem* item : scene->extensionOverlayItems(QString::fromLatin1(kCpnExtensionId))) {
        auto* graphic = dynamic_cast<NetworkBendGraphicsItem*>(item);
        if (graphic != nullptr && graphic->hasManualBendPoint())
            previousBends.insert(graphic->routeKey(), graphic->manualBendPoint());
        if (item != nullptr && item->isSelected() && item->data(1).isValid()) {
            auto* arc = reinterpret_cast<CPNArc*>(static_cast<quintptr>(item->data(1).toULongLong()));
            if (arc != nullptr) selectedArcs.insert(arc);
        }
    }
    scene->clearExtensionOverlayItems(QString::fromLatin1(kCpnExtensionId));
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(scene->networkEditorOwner());
    if (network == nullptr) return;

    for (PetriPlace* place : *network->getPlaces()->list()) {
        auto* graphical = scene->findGraphicalModelDataDefinition(place);
        if (graphical == nullptr) continue;
        const QRectF bounds = graphical->sceneBoundingRect();
        auto* badge = new QGraphicsEllipseItem(bounds.adjusted(-6, -6, 6, 6));
        badge->setPen(QPen(QColor(45, 105, 165), 2.0));
        badge->setBrush(Qt::NoBrush);
        badge->setZValue(-0.7);
        QStringList tokens;
        for (const auto& [color, quantity] : *place->getAllTokens())
            if (quantity > 0) tokens << QStringLiteral("%1:%2").arg(QString::fromStdString(color)).arg(quantity);
        badge->setAcceptedMouseButtons(Qt::NoButton);
        badge->setToolTip(QObject::tr("Current marking: %1")
            .arg(tokens.isEmpty() ? QObject::tr("empty") : tokens.join(QStringLiteral(", "))));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), badge);
        if (!tokens.isEmpty()) {
            auto* label = new QGraphicsSimpleTextItem(tokens.join(QStringLiteral("\n")));
            label->setAcceptedMouseButtons(Qt::NoButton);
            label->setBrush(QColor(20, 70, 120));
            label->setPos(bounds.topRight() + QPointF(3, -label->boundingRect().height() / 2.0));
            label->setZValue(-0.6);
            scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), label);
        }
    }
    for (CPNTransition* transition : *network->getTransitions()->list()) {
        auto* graphical = scene->findGraphicalModelDataDefinition(transition);
        if (graphical == nullptr) continue;
        auto* badge = new QGraphicsRectItem(graphical->sceneBoundingRect().adjusted(-5, -5, 5, 5));
        const bool fired = network->getLastFiredTransition() == transition;
        const CpnPreviewReadiness preview = previewCpnReadiness(network, transition);
        badge->setPen(QPen(fired ? QColor(225, 90, 35)
                                  : !preview.tokenAndCapacityReady ? QColor(110, 110, 110)
                                  : preview.guardPending ? QColor(200, 145, 45)
                                                         : QColor(45, 150, 90),
                              fired ? 4.0 : 2.0));
        badge->setBrush(Qt::NoBrush);
        badge->setZValue(-0.7);
        badge->setAcceptedMouseButtons(Qt::NoButton);
        badge->setToolTip(fired ? QObject::tr("Last fired CPN transition. %1").arg(preview.explanation)
                                : preview.explanation);
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), badge);
    }
    for (CPNArc* arc : *network->getArcs()->list()) {
        if (arc == nullptr || arc->getPlace() == nullptr || arc->getTransition() == nullptr) continue;
        auto* place = scene->findGraphicalModelDataDefinition(arc->getPlace());
        auto* transition = scene->findGraphicalModelDataDefinition(arc->getTransition());
        if (place == nullptr || transition == nullptr) continue;
        const QPointF placeCenter = place->sceneBoundingRect().center();
        const QPointF transitionCenter = transition->sceneBoundingRect().center();
        const QPointF start = arc->isInputArc() ? placeCenter : transitionCenter;
        const QPointF end = arc->isInputArc() ? transitionCenter : placeCenter;

        int rank = 0, count = 0;
        bool opposite = false;
        for (CPNArc* candidate : *network->getArcs()->list()) {
            if (candidate == nullptr || candidate->getPlace() != arc->getPlace()
                || candidate->getTransition() != arc->getTransition()) continue;
            if (candidate->getDirection() == arc->getDirection()) {
                if (candidate == arc) rank = count;
                ++count;
            } else {
                opposite = true;
            }
        }
        const QString key = NetworkBendDiagramCodec::relationKey(
            arc->getId(), QString::fromStdString(arc->getPlace()->getName()),
            QString::fromStdString(arc->getTransition()->getName()),
            QString::fromStdString(arc->getName()));
        auto* item = new NetworkBendGraphicsItem(QString::fromLatin1(kCpnExtensionId), key);
        item->setGeometry(start, end, rank, count, opposite, true);
        if (previousBends.contains(key)) item->setManualBendPoint(previousBends.value(key));
        const bool lastFiring = network->getLastFiredTransition() == arc->getTransition();
        item->setVisiblePen(QPen(lastFiring ? QColor(225, 90, 35) : QColor(90, 75, 115),
                                 lastFiring ? 3.5 : 2.0));
        item->setData(1, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(arc)));
        item->setToolTip(lastFiring
            ? QObject::tr("Last firing: %1").arg(QString::fromStdString(arc->show()))
            : QString::fromStdString(arc->show()));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), item);
        if (selectedArcs.contains(arc)) item->setSelected(true);

        QStringList inscriptions;
        for (const auto& [color, quantity] : arc->getInscriptions()) {
            if (quantity > 0) inscriptions.append(
                QStringLiteral("%1:%2").arg(QString::fromStdString(color)).arg(quantity));
        }
        if (!inscriptions.isEmpty()) {
            auto* label = new QGraphicsSimpleTextItem(inscriptions.join(QStringLiteral(", ")));
            label->setBrush(QColor(80, 50, 105));
            label->setPos(item->path().pointAtPercent(0.5) + QPointF(5, -14));
            label->setZValue(-0.8);
            label->setAcceptedMouseButtons(Qt::NoButton);
            scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), label);
        }
    }
}

bool acceptsCpnDrop(ModelDataDefinition*, const std::string& type, bool component) {
    return !component && (type == Util::TypeOf<PetriPlace>() || type == Util::TypeOf<CPNTransition>());
}

void addCpnNode(ModelDataDefinition* owner, ModelDataDefinition* dropped) {
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(owner);
    if (network == nullptr || dropped == nullptr) return;
    if (auto* place = dynamic_cast<PetriPlace*>(dropped)) network->addPlace(place);
    if (auto* transition = dynamic_cast<CPNTransition*>(dropped)) network->addTransition(transition);
}

CPNTransition* selectedTransition(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* transition = graphical != nullptr ? dynamic_cast<CPNTransition*>(graphical->getDataDefinition()) : nullptr;
        if (transition != nullptr && network->getTransitions()->find(transition) != network->getTransitions()->list()->end())
            return transition;
    }
    return nullptr;
}

class FireCpnTransitionUndoCommand final : public QUndoCommand {
public:
    FireCpnTransitionUndoCommand(ColoredPetriNetNetwork* network, CPNTransition* transition, ModelGraphicsScene* scene)
        : _network(network), _transition(transition), _scene(scene), _before(captureMarking(network)),
          _beforeLast(network != nullptr ? network->getLastFiredTransition() : nullptr) {
        setText(QObject::tr("Fire CPN transition"));
    }
    void redo() override {
        if (_network == nullptr || _transition == nullptr) { setObsolete(true); return; }
        if (_firstRedo) {
            _firstRedo = false;
            if (!_network->fire(_transition)) {
                restoreMarking(_before);
                _network->setLastFiredTransition(_beforeLast);
                setObsolete(true);
                return;
            }
            _after = captureMarking(_network);
        } else restoreMarking(_after);
        if (!_firstRedo) _network->setLastFiredTransition(_transition);
        synchronizeCpnProjection(nullptr, _scene);
    }
    void undo() override {
        restoreMarking(_before);
        _network->setLastFiredTransition(_beforeLast);
        synchronizeCpnProjection(nullptr, _scene);
    }
private:
    ColoredPetriNetNetwork* _network;
    CPNTransition* _transition;
    QPointer<ModelGraphicsScene> _scene;
    Marking _before;
    Marking _after;
    CPNTransition* _beforeLast;
    bool _firstRedo = true;
};

class ToggleCpnArcUndoCommand final : public QUndoCommand {
public:
    ToggleCpnArcUndoCommand(ColoredPetriNetNetwork* network, CPNArc* arc, ModelGraphicsScene* scene)
        : _network(network), _arc(arc), _scene(scene) { setText(QObject::tr("Add CPN arc")); }
    void redo() override {
        if (_network == nullptr || _arc == nullptr || !_network->addArc(_arc)) { setObsolete(true); return; }
        synchronizeCpnProjection(nullptr, _scene);
    }
    void undo() override {
        if (_network == nullptr || _arc == nullptr) { setObsolete(true); return; }
        _network->removeArc(_arc);
        synchronizeCpnProjection(nullptr, _scene);
    }
private:
    ColoredPetriNetNetwork* _network;
    CPNArc* _arc;
    QPointer<ModelGraphicsScene> _scene;
};

class RemoveCpnArcUndoCommand final : public QUndoCommand {
public:
    RemoveCpnArcUndoCommand(ColoredPetriNetNetwork* network, CPNArc* arc, ModelGraphicsScene* scene)
        : _network(network), _arc(arc), _scene(scene) { setText(QObject::tr("Remove CPN arc")); }
    void redo() override {
        if (_network == nullptr || _arc == nullptr || _network->getArcs()->find(_arc) == _network->getArcs()->list()->end()) {
            setObsolete(true); return;
        }
        _network->removeArc(_arc);
        synchronizeCpnProjection(nullptr, _scene);
    }
    void undo() override {
        if (_network == nullptr || _arc == nullptr || !_network->addArc(_arc)) { setObsolete(true); return; }
        synchronizeCpnProjection(nullptr, _scene);
    }
private:
    ColoredPetriNetNetwork* _network;
    CPNArc* _arc;
    QPointer<ModelGraphicsScene> _scene;
};

class EditCpnArcInscriptionUndoCommand final : public QUndoCommand {
public:
    EditCpnArcInscriptionUndoCommand(ColoredPetriNetNetwork* network, CPNArc* arc, ModelGraphicsScene* scene,
                                     std::map<std::string, unsigned int> after, std::string name = {})
        : _network(network), _arc(arc), _scene(scene), _before(arc->getInscriptions()), _after(std::move(after)),
          _beforeName(arc->getName()), _afterName(name.empty() ? arc->getName() : std::move(name)) {
        setText(QObject::tr("Edit CPN arc inscription"));
    }
    void redo() override { apply(_after, _afterName); }
    void undo() override { apply(_before, _beforeName); }
private:
    void apply(const std::map<std::string, unsigned int>& inscription, const std::string& name) {
        if (_network == nullptr || _arc == nullptr
            || _network->getArcs()->find(_arc) == _network->getArcs()->list()->end()) { setObsolete(true); return; }
        _arc->clearInscriptions();
        for (const auto& [color, quantity] : inscription) _arc->setInscription(color, quantity);
        _arc->setName(name);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    CPNArc* _arc;
    QPointer<ModelGraphicsScene> _scene;
    std::map<std::string, unsigned int> _before;
    std::map<std::string, unsigned int> _after;
    std::string _beforeName;
    std::string _afterName;
};

CPNArc* selectedArc(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        if (dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) continue;
        auto* arc = reinterpret_cast<CPNArc*>(static_cast<quintptr>(item->data(1).toULongLong()));
        if (arc != nullptr && network->getArcs()->find(arc) != network->getArcs()->list()->end()) return arc;
    }
    return nullptr;
}

void removeSelectedArc(const GuiExtensionRuntimeContext& context) {
    auto* arc = selectedArc(context);
    if (arc == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    context.graphicsScene->getUndoStack()->push(new RemoveCpnArcUndoCommand(network, arc, context.graphicsScene));
}

class SetCpnInitialMarkingUndoCommand final : public QUndoCommand {
public:
    SetCpnInitialMarkingUndoCommand(ColoredPetriNetNetwork* network, PetriPlace* place,
                                    ModelGraphicsScene* scene, std::map<std::string, unsigned int> after)
        : _network(network), _place(place), _scene(scene), _before(network->getInitialMarking(place)), _after(std::move(after)) {
        setText(QObject::tr("Edit CPN initial marking"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(const std::map<std::string, unsigned int>& marking) {
        if (_network == nullptr || _place == nullptr
            || _network->getPlaces()->find(_place) == _network->getPlaces()->list()->end()) { setObsolete(true); return; }
        _network->setInitialMarking(_place, marking);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network; PetriPlace* _place; QPointer<ModelGraphicsScene> _scene;
    std::map<std::string, unsigned int> _before, _after;
};

class EditCpnTransitionUndoCommand final : public QUndoCommand {
public:
    EditCpnTransitionUndoCommand(ColoredPetriNetNetwork* network, CPNTransition* transition,
                                 ModelGraphicsScene* scene, std::string guard, unsigned int priority)
        : _network(network), _transition(transition), _scene(scene), _beforeGuard(transition->getGuardExpression()),
          _beforePriority(transition->getPriority()), _afterGuard(std::move(guard)), _afterPriority(priority) {
        setText(QObject::tr("Edit CPN transition"));
    }
    void redo() override { apply(_afterGuard, _afterPriority); }
    void undo() override { apply(_beforeGuard, _beforePriority); }
private:
    void apply(const std::string& guard, unsigned int priority) {
        if (_network == nullptr || _transition == nullptr) { setObsolete(true); return; }
        _transition->setGuardExpression(guard);
        _transition->setPriority(priority);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    CPNTransition* _transition;
    QPointer<ModelGraphicsScene> _scene;
    std::string _beforeGuard;
    unsigned int _beforePriority;
    std::string _afterGuard;
    unsigned int _afterPriority;
};

class EditCpnNodeBasePropertiesUndoCommand final : public QUndoCommand {
public:
    struct Values { std::string name, label; std::uint32_t trace = 0; bool report = false; };
    EditCpnNodeBasePropertiesUndoCommand(ColoredPetriNetNetwork* network, ModelDataDefinition* node,
                                         bool isPlace, Values after)
        : _network(network), _node(node), _isPlace(isPlace), _before{node->getName(), node->getLabel(),
            static_cast<std::uint32_t>(node->getTraceLevelSpecific()), node->isReportStatistics()},
          _after(std::move(after)) { setText(QObject::tr("Edit CPN node")); }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(const Values& v) {
        if (_network == nullptr || _node == nullptr) { setObsolete(true); return; }
        bool member = false;
        if (_isPlace) { auto* p = dynamic_cast<PetriPlace*>(_node); member = p != nullptr && _network->getPlaces()->find(p) != _network->getPlaces()->list()->end(); }
        else { auto* t = dynamic_cast<CPNTransition*>(_node); member = t != nullptr && _network->getTransitions()->find(t) != _network->getTransitions()->list()->end(); }
        if (!member) { setObsolete(true); return; }
        _node->setName(v.name); _node->setLabel(v.label);
        _node->setTraceLevelSpecific(static_cast<TraceManager::Level>(v.trace));
        _node->setReportStatistics(v.report);
    }
    ColoredPetriNetNetwork* _network; ModelDataDefinition* _node; bool _isPlace;
    Values _before, _after;
};

PetriPlace* selectedPlace(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* place = graphical != nullptr ? dynamic_cast<PetriPlace*>(graphical->getDataDefinition()) : nullptr;
        if (place != nullptr && network->getPlaces()->find(place) != network->getPlaces()->list()->end()) return place;
    }
    return nullptr;
}

// Initial marking is editable independent of the current simulation marking.
class ResetCpnPlaceMarkingUndoCommand final : public QUndoCommand {
public:
    ResetCpnPlaceMarkingUndoCommand(ColoredPetriNetNetwork* network, PetriPlace* place,
                                   ModelGraphicsScene* scene)
        : _network(network), _place(place), _scene(scene),
          _before(*place->getAllTokens()), _after(network->getInitialMarking(place)) {
        setText(QObject::tr("Reset CPN place to initial marking"));
    }
    void undo() override { apply(_before); }
    void redo() override { apply(_after); }
private:
    void apply(const std::map<std::string, unsigned int>& marking) {
        if (_network == nullptr || _place == nullptr
            || _network->getPlaces()->find(_place) == _network->getPlaces()->list()->end()) {
            setObsolete(true); return;
        }
        _place->clearTokens();
        for (const auto& [color, count] : marking) _place->setTokens(count, color);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    PetriPlace* _place;
    QPointer<ModelGraphicsScene> _scene;
    std::map<std::string, unsigned int> _before, _after;
};

class SetCpnCurrentMarkingUndoCommand final : public QUndoCommand {
public:
    SetCpnCurrentMarkingUndoCommand(ColoredPetriNetNetwork* network, PetriPlace* place,
                                    ModelGraphicsScene* scene,
                                    std::map<std::string, unsigned int> after)
        : _network(network), _place(place), _scene(scene),
          _before(*place->getAllTokens()), _after(std::move(after)) {
        setText(QObject::tr("Edit CPN current marking"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(const std::map<std::string, unsigned int>& value) {
        if (_network == nullptr || _place == nullptr || _scene == nullptr
            || _scene->networkEditorOwner() != _network
            || _network->getPlaces()->find(_place) == _network->getPlaces()->list()->end()) {
            setObsolete(true);
            return;
        }
        _place->clearTokens();
        for (const auto& [color, count] : value) _place->setTokens(count, color);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    PetriPlace* _place;
    QPointer<ModelGraphicsScene> _scene;
    std::map<std::string, unsigned int> _before, _after;
};

bool canEditCpnPlaceMarking(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedPlace(context) != nullptr;
}

void editCpnPlaceMarking(const GuiExtensionRuntimeContext& context, bool initial) {
    if (!canEditCpnPlaceMarking(context)) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    PetriPlace* place = selectedPlace(context);
    const std::map<std::string, unsigned int> previous = initial
        ? network->getInitialMarking(place) : *place->getAllTokens();

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("CPN %1 marking — %2")
        .arg(initial ? QObject::tr("initial") : QObject::tr("current"),
             QString::fromStdString(place->getName())));
    dialog.resize(480, 360);
    QVBoxLayout layout(&dialog);
    auto* explanation = new QLabel(initial
        ? QObject::tr("Edit initial tokens by color. Current tokens do not change until reset or simulation restart.")
        : QObject::tr("Edit current tokens by color. The initial marking remains unchanged."), &dialog);
    explanation->setWordWrap(true);
    layout.addWidget(explanation);
    auto* table = new QTableWidget(0, 2, &dialog);
    table->setHorizontalHeaderLabels({QObject::tr("Color"), QObject::tr("Token count")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout.addWidget(table);
    const auto addRow = [table](const QString& color, unsigned int count) {
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(color));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
    };
    for (const auto& [color, count] : previous)
        addRow(QString::fromStdString(color), count);

    QHBoxLayout rowActions;
    auto* add = new QPushButton(QObject::tr("Add color"), &dialog);
    auto* remove = new QPushButton(QObject::tr("Remove selected"), &dialog);
    rowActions.addWidget(add);
    rowActions.addWidget(remove);
    layout.addLayout(&rowActions);
    QObject::connect(add, &QPushButton::clicked, &dialog,
                     [addRow]() { addRow(QString(), 0); });
    QObject::connect(remove, &QPushButton::clicked, &dialog, [table]() {
        if (table->currentRow() >= 0) table->removeRow(table->currentRow());
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        std::map<std::string, unsigned int> marking;
        for (int row = 0; row < table->rowCount(); ++row) {
            auto* colorItem = table->item(row, 0);
            auto* quantityItem = table->item(row, 1);
            const QString color = colorItem != nullptr ? colorItem->text().trimmed() : QString();
            const QByteArray quantity = quantityItem != nullptr
                ? quantityItem->text().trimmed().toLatin1() : QByteArray();
            unsigned int count = 0;
            const auto [end, error] = std::from_chars(quantity.constData(),
                quantity.constData() + quantity.size(), count);
            if (color.isEmpty() || color.contains(QLatin1Char('=')) || color.contains(QLatin1Char(';'))
                || error != std::errc{} || end != quantity.constData() + quantity.size()
                || !marking.emplace(color.toStdString(), count).second) {
                QMessageBox::warning(&dialog, QObject::tr("Invalid marking"),
                                     QObject::tr("Each row needs a unique color and a nonnegative integer token count."));
                return;
            }
        }
        if (marking != previous) {
            if (initial) {
                context.graphicsScene->getUndoStack()->push(
                    new SetCpnInitialMarkingUndoCommand(network, place, context.graphicsScene, std::move(marking)));
            } else {
                context.graphicsScene->getUndoStack()->push(
                    new SetCpnCurrentMarkingUndoCommand(network, place, context.graphicsScene, std::move(marking)));
            }
        }
        dialog.accept();
    });
    dialog.exec();
}

void editCpnPlaceInitialMarking(const GuiExtensionRuntimeContext& context) {
    editCpnPlaceMarking(context, true);
}

void editCpnPlaceCurrentMarking(const GuiExtensionRuntimeContext& context) {
    editCpnPlaceMarking(context, false);
}

bool canEditCpnArcInscription(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedArc(context) != nullptr;
}

void editCpnArcInscription(const GuiExtensionRuntimeContext& context) {
    if (!canEditCpnArcInscription(context)) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    CPNArc* arc = selectedArc(context);
    if (network == nullptr || arc == nullptr) return;

    QDialog dialog;
    dialog.setWindowTitle(QObject::tr("CPN arc inscription — %1")
        .arg(QString::fromStdString(arc->getName())));
    dialog.resize(480, 360);
    QVBoxLayout layout(&dialog);
    auto* table = new QTableWidget(0, 2, &dialog);
    table->setHorizontalHeaderLabels({QObject::tr("Color"), QObject::tr("Tokens per firing")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout.addWidget(table);
    const auto addRow = [table](const QString& color, unsigned int count) {
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(color));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
    };
    for (const auto& [color, quantity] : arc->getInscriptions())
        addRow(QString::fromStdString(color), quantity);

    QHBoxLayout actions;
    auto* add = new QPushButton(QObject::tr("Add color"), &dialog);
    auto* remove = new QPushButton(QObject::tr("Remove selected"), &dialog);
    actions.addWidget(add);
    actions.addWidget(remove);
    layout.addLayout(&actions);
    QObject::connect(add, &QPushButton::clicked, &dialog,
                     [addRow]() { addRow(QString(), 1u); });
    QObject::connect(remove, &QPushButton::clicked, &dialog, [table]() {
        if (table->currentRow() >= 0) table->removeRow(table->currentRow());
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout.addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        std::map<std::string, unsigned int> inscription;
        for (int row = 0; row < table->rowCount(); ++row) {
            const auto* colorItem = table->item(row, 0);
            const auto* quantityItem = table->item(row, 1);
            const QString color = colorItem != nullptr ? colorItem->text().trimmed() : QString();
            const QByteArray numeric = quantityItem != nullptr
                ? quantityItem->text().trimmed().toLatin1() : QByteArray();
            unsigned int value = 0u;
            const auto [end, ec] = std::from_chars(
                numeric.constData(), numeric.constData() + numeric.size(), value);
            if (color.isEmpty() || color.contains(QLatin1Char(';')) || color.contains(QLatin1Char('='))
                || ec != std::errc{} || end != numeric.constData() + numeric.size()
                || value == 0u || !inscription.emplace(color.toStdString(), value).second) {
                QMessageBox::warning(&dialog, QObject::tr("Invalid arc inscription"),
                    QObject::tr("Use unique colors and strictly positive integer quantities."));
                return;
            }
        }
        if (inscription != arc->getInscriptions()) {
            context.graphicsScene->getUndoStack()->push(
                new EditCpnArcInscriptionUndoCommand(network, arc, context.graphicsScene,
                                                       std::move(inscription)));
        }
        dialog.accept();
    });
    dialog.exec();
}

void resetCpnPlaceToInitialMarking(const GuiExtensionRuntimeContext& context) {
    if (!canEditCpnPlaceMarking(context)) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    auto* place = selectedPlace(context);
    if (*place->getAllTokens() == network->getInitialMarking(place)) return;
    context.graphicsScene->getUndoStack()->push(
        new ResetCpnPlaceMarkingUndoCommand(network, place, context.graphicsScene));
}

class RemoveCpnNodeUndoCommand final : public QUndoCommand {
public:
    RemoveCpnNodeUndoCommand(ColoredPetriNetNetwork* network, PetriPlace* place, ModelGraphicsScene* scene)
        : _network(network), _place(place), _node(place), _scene(scene),
          _oldLevel(place != nullptr ? place->getLevel() : 0),
          _initialMarking(network != nullptr ? network->getInitialMarking(place)
                                             : std::map<std::string, unsigned int>{}),
          _lastFired(network != nullptr ? network->getLastFiredTransition() : nullptr) {
        initialize();
    }

    RemoveCpnNodeUndoCommand(ColoredPetriNetNetwork* network, CPNTransition* transition, ModelGraphicsScene* scene)
        : _network(network), _transition(transition), _node(transition), _scene(scene),
          _oldLevel(transition != nullptr ? transition->getLevel() : 0),
          _lastFired(network != nullptr ? network->getLastFiredTransition() : nullptr) {
        initialize();
    }

    void redo() override {
        if (_scene == nullptr || _network == nullptr || _node == nullptr || !isMember()) {
            setObsolete(true);
            return;
        }
        for (const auto& [arc, ordinal] : _arcs) {
            (void)ordinal;
            _network->removeArc(arc);
            if (Model* model = _node->getParentModel()) model->remove(arc);
        }
        if (_place != nullptr) _network->removePlace(_place);
        else _network->removeTransition(_transition);
        if (Model* model = _node->getParentModel()) model->remove(_node);
        _node->setModelLevel(0);
        synchronizeScene(false);
    }

    void undo() override {
        if (_scene == nullptr || _network == nullptr || _node == nullptr || isMember()) {
            setObsolete(true);
            return;
        }
        if (Model* model = _node->getParentModel()) model->insert(_node);
        _node->setModelLevel(_oldLevel);
        if (_place != nullptr) {
            _network->addPlace(_place);
            _network->setInitialMarking(_place, _initialMarking);
        } else {
            _network->addTransition(_transition);
        }
        for (const auto& [arc, ordinal] : _arcs) {
            if (Model* model = _node->getParentModel()) model->insert(arc);
            arc->setModelLevel(_oldLevel);
            if (!_network->restoreArc(arc, ordinal)) { setObsolete(true); return; }
        }
        _network->setLastFiredTransition(_lastFired);
        synchronizeScene(true);
    }

private:
    bool isMember() const {
        if (_place != nullptr) return _network->getPlaces()->find(_place) != _network->getPlaces()->list()->end();
        return _network->getTransitions()->find(_transition) != _network->getTransitions()->list()->end();
    }

    void initialize() {
        if (_network == nullptr || _node == nullptr) return;
        unsigned int ordinal = 0;
        for (CPNArc* arc : *_network->getArcs()->list()) {
            if (arc != nullptr && (_place == arc->getPlace() || _transition == arc->getTransition()))
                _arcs.emplace_back(arc, ordinal);
            ++ordinal;
        }
        if (_scene != nullptr) {
            if (auto* item = _scene->findGraphicalModelDataDefinition(_node)) _position = item->pos();
        }
    }

    void synchronizeScene(bool restorePosition) {
        if (_scene == nullptr) return;
        GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_scene->getSimulator(), _scene);
        if (restorePosition) {
            if (auto* item = _scene->findGraphicalModelDataDefinition(_node)) {
                item->setPos(_position);
                item->setOldPosition(_position.x(), _position.y());
            }
        }
        synchronizeCpnProjection(_scene->getSimulator(), _scene);
    }

    ColoredPetriNetNetwork* _network;
    PetriPlace* _place = nullptr;
    CPNTransition* _transition = nullptr;
    ModelDataDefinition* _node;
    QPointer<ModelGraphicsScene> _scene;
    unsigned int _oldLevel = 0;
    std::map<std::string, unsigned int> _initialMarking;
    CPNTransition* _lastFired = nullptr;
    QPointF _position;
    std::vector<std::pair<CPNArc*, unsigned int>> _arcs;
};

bool canRemoveCpnNode(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr || context.graphicsScene->getUndoStack() == nullptr) return false;
    return selectedPlace(context) != nullptr || selectedTransition(context) != nullptr;
}

void removeSelectedCpnNode(const GuiExtensionRuntimeContext& context) {
    if (!canRemoveCpnNode(context)) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    if (PetriPlace* place = selectedPlace(context)) {
        context.graphicsScene->getUndoStack()->push(new RemoveCpnNodeUndoCommand(network, place, context.graphicsScene));
    } else if (CPNTransition* transition = selectedTransition(context)) {
        context.graphicsScene->getUndoStack()->push(
            new RemoveCpnNodeUndoCommand(network, transition, context.graphicsScene));
    }
}

bool canEditInitialTokens(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedPlace(context) != nullptr;
}

bool canValidateCpnNetwork(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr
        && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
}

void validateCpnNetwork(const GuiExtensionRuntimeContext& context) {
    auto* network = context.graphicsScene != nullptr
        ? dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) : nullptr;
    if (network == nullptr) return;
    std::string diagnostic;
    if (ModelDataDefinition::Check(network, diagnostic)) {
        QMessageBox::information(context.mainWindow, QObject::tr("CPN Validation"),
                                 QObject::tr("The colored Petri net is valid."));
    } else {
        QMessageBox::warning(context.mainWindow, QObject::tr("CPN Validation"),
                             QString::fromStdString(diagnostic));
    }
}

bool canCreateCpnArc(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr || context.graphicsScene->getUndoStack() == nullptr
        || dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) == nullptr) return false;
    int count = 0;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical != nullptr && (dynamic_cast<PetriPlace*>(graphical->getDataDefinition())
            || dynamic_cast<CPNTransition*>(graphical->getDataDefinition()))) ++count;
    }
    return count == 1;
}

void createCpnArc(const GuiExtensionRuntimeContext& context) {
    if (!canCreateCpnArc(context)) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(scene->networkEditorOwner());
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* place = graphical != nullptr ? dynamic_cast<PetriPlace*>(graphical->getDataDefinition()) : nullptr;
        auto* transition = graphical != nullptr ? dynamic_cast<CPNTransition*>(graphical->getDataDefinition()) : nullptr;
        if (graphical != nullptr && ((place != nullptr && network->getPlaces()->find(place) != network->getPlaces()->list()->end())
            || (transition != nullptr && network->getTransitions()->find(transition) != network->getTransitions()->list()->end()))) {
            scene->beginNetworkRelationCreation(graphical);
            return;
        }
    }
}

bool canFireSelectedTransition(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr || context.graphicsScene->getUndoStack() == nullptr) return false;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    auto* transition = selectedTransition(context);
    return previewCpnReadiness(network, transition).tokenAndCapacityReady;
}

void fireSelectedTransition(const GuiExtensionRuntimeContext& context) {
    if (!canFireSelectedTransition(context)) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    context.graphicsScene->getUndoStack()->push(
        new FireCpnTransitionUndoCommand(network, selectedTransition(context), context.graphicsScene));
}

class ColoredPetriNetGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return kCpnExtensionId; }
    std::string displayName() const override { return "Colored Petri Net Editor"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"ColoredPetriNetNetwork"}; }
    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        GuiDiagramProjectionContribution projection;
        projection.id = kCpnExtensionId;
        projection.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<ColoredPetriNetNetwork*>(definition) != nullptr;
        };
        projection.synchronize = synchronizeCpnProjection;
        projection.sectionName = "ColoredPetriNetBend";
        projection.captureLayout = [](Simulator*, ModelDataDefinition* owner, ModelGraphicsScene* scene) {
            return NetworkBendDiagramCodec::capture(owner, scene, QString::fromLatin1(kCpnExtensionId));
        };
        projection.restoreLayout = [](Simulator*, ModelDataDefinition* owner,
                                      ModelGraphicsScene* scene, const std::any& layout) {
            NetworkBendDiagramCodec::restore(owner, scene, layout, QString::fromLatin1(kCpnExtensionId));
        };
        projection.serializeLayouts = NetworkBendDiagramCodec::serialize;
        projection.parseLayouts = [](Model* model, const QStringList& lines) {
            return NetworkBendDiagramCodec::parse(model, lines, [](ModelDataDefinition* owner) {
                return dynamic_cast<ColoredPetriNetNetwork*>(owner) != nullptr;
            });
        };
        registry->addDiagramProjection(std::move(projection));
        GuiActionContribution validate;
        validate.id = "modal.cpn.validate-network";
        validate.text = "Validate Colored Petri Net...";
        validate.menuPath = "Model";
        validate.isVisible = canValidateCpnNetwork;
        validate.isEnabled = canValidateCpnNetwork;
        validate.trigger = validateCpnNetwork;
        registry->addAction(std::move(validate));
        GuiNetworkEditorContribution editor;
        editor.id = kCpnExtensionId;
        editor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<ColoredPetriNetNetwork*>(definition) != nullptr;
        };
        editor.ownsEditableDataDefinition = [](ModelDataDefinition* definition, ModelDataDefinition* candidate) {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            if (network == nullptr || candidate == nullptr) return false;
            if (auto* place = dynamic_cast<PetriPlace*>(candidate)) return network->getPlaces()->find(place) != network->getPlaces()->list()->end();
            if (auto* transition = dynamic_cast<CPNTransition*>(candidate)) return network->getTransitions()->find(transition) != network->getTransitions()->list()->end();
            return false;
        };
        editor.createDataDefinitionPropertyCommand = [](ModelDataDefinition* definition, ModelDataDefinition* candidate,
            const std::string& property, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            if (network == nullptr || candidate == nullptr) return nullptr;
            if (auto* transition = dynamic_cast<CPNTransition*>(candidate)) {
                if (network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return nullptr;
                EditCpnNodeBasePropertiesUndoCommand::Values base{transition->getName(), transition->getLabel(),
                    static_cast<std::uint32_t>(transition->getTraceLevelSpecific()), transition->isReportStatistics()};
                if (property == "Name") base.name = value;
                else if (property == "Label") base.label = value;
                else if (property == "Report Statistics") base.report = value == "true" || value == "1";
                else if (property == "Trace Level") {
                    const auto [end, ec] = std::from_chars(value.data(), value.data()+value.size(), base.trace);
                    if (ec != std::errc{} || end != value.data()+value.size()) return nullptr;
                } else {
                    if (property == "Guard Expression") return new EditCpnTransitionUndoCommand(network, transition, nullptr, value, transition->getPriority());
                    if (property == "Priority") {
                        unsigned int priority = 0; const auto [end, ec] = std::from_chars(value.data(), value.data()+value.size(), priority);
                        if (ec != std::errc{} || end != value.data()+value.size()) return nullptr;
                        return new EditCpnTransitionUndoCommand(network, transition, nullptr, transition->getGuardExpression(), priority);
                    }
                    return nullptr;
                }
                return new EditCpnNodeBasePropertiesUndoCommand(network, transition, false, std::move(base));
            }
            if (auto* place = dynamic_cast<PetriPlace*>(candidate)) {
                if (network->getPlaces()->find(place) == network->getPlaces()->list()->end()) return nullptr;
                if (property == "Initial Marking") {
                    std::map<std::string, unsigned int> marking;
                    std::istringstream input(value); std::string token;
                    while (std::getline(input, token, ';')) {
                        if (token.empty()) continue;
                        const auto eq = token.find('='); if (eq == std::string::npos || eq == 0 || eq + 1 == token.size()) return nullptr;
                        unsigned int count = 0; const std::string number = token.substr(eq + 1);
                        const auto [end, ec] = std::from_chars(number.data(), number.data()+number.size(), count);
                        if (ec != std::errc{} || end != number.data()+number.size()
                            || !marking.emplace(token.substr(0, eq), count).second) return nullptr;
                    }
                    return new SetCpnInitialMarkingUndoCommand(network, place, nullptr, std::move(marking));
                }
                EditCpnNodeBasePropertiesUndoCommand::Values base{place->getName(), place->getLabel(),
                    static_cast<std::uint32_t>(place->getTraceLevelSpecific()), place->isReportStatistics()};
                if (property == "Name") base.name = value;
                else if (property == "Label") base.label = value;
                else if (property == "Report Statistics") base.report = value == "true" || value == "1";
                else if (property == "Trace Level") {
                    const auto [end, ec] = std::from_chars(value.data(), value.data()+value.size(), base.trace);
                    if (ec != std::errc{} || end != value.data()+value.size()) return nullptr;
                } else return nullptr;
                return new EditCpnNodeBasePropertiesUndoCommand(network, place, true, std::move(base));
            }
            return nullptr;
        };
        editor.canStartRelationCreation = [](ModelDataDefinition* definition, QGraphicsItem* item) {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
            if (network == nullptr || graphical == nullptr) return false;
            auto* data = graphical->getDataDefinition();
            if (auto* place = dynamic_cast<PetriPlace*>(data)) return network->getPlaces()->find(place) != network->getPlaces()->list()->end();
            if (auto* transition = dynamic_cast<CPNTransition*>(data)) return network->getTransitions()->find(transition) != network->getTransitions()->list()->end();
            return false;
        };
        editor.createRelationCommand = [](ModelDataDefinition* definition, QGraphicsItem* sourceItem,
                                           QGraphicsItem* destinationItem) -> QUndoCommand* {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            auto* sourceGraphic = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* destinationGraphic = dynamic_cast<GraphicalModelDataDefinition*>(destinationItem);
            auto* sourcePlace = sourceGraphic != nullptr ? dynamic_cast<PetriPlace*>(sourceGraphic->getDataDefinition()) : nullptr;
            auto* destinationPlace = destinationGraphic != nullptr ? dynamic_cast<PetriPlace*>(destinationGraphic->getDataDefinition()) : nullptr;
            auto* sourceTransition = sourceGraphic != nullptr ? dynamic_cast<CPNTransition*>(sourceGraphic->getDataDefinition()) : nullptr;
            auto* destinationTransition = destinationGraphic != nullptr ? dynamic_cast<CPNTransition*>(destinationGraphic->getDataDefinition()) : nullptr;
            PetriPlace* place = sourcePlace != nullptr ? sourcePlace : destinationPlace;
            CPNTransition* transition = sourceTransition != nullptr ? sourceTransition : destinationTransition;
            if (network == nullptr || place == nullptr || transition == nullptr
                || (sourcePlace != nullptr && destinationPlace != nullptr)
                || (sourceTransition != nullptr && destinationTransition != nullptr)
                || network->getPlaces()->find(place) == network->getPlaces()->list()->end()
                || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return nullptr;
            const auto direction = sourcePlace != nullptr ? CPNArc::Direction::PlaceToTransition : CPNArc::Direction::TransitionToPlace;
            const std::string name = "A" + std::to_string(network->getArcs()->size());
            auto* arc = new CPNArc(network->getParentModel(), place, transition, direction, name);
            arc->setInscription("default", 1);
            return new ToggleCpnArcUndoCommand(network, arc, sourceGraphic->scene() != nullptr
                ? dynamic_cast<ModelGraphicsScene*>(sourceGraphic->scene()) : nullptr);
        };
        editor.elementProperties = [](ModelDataDefinition* definition, QGraphicsItem* item) {
            std::vector<GuiNetworkEditorContribution::ElementProperty> result;
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            if (network == nullptr || item == nullptr) return result;
            using P = GuiNetworkEditorContribution::ElementProperty;
            if (dynamic_cast<QGraphicsPathItem*>(item) != nullptr && item->data(1).isValid()) {
                auto* arc = reinterpret_cast<CPNArc*>(static_cast<quintptr>(item->data(1).toULongLong()));
                if (arc == nullptr || network->getArcs()->find(arc) == network->getArcs()->list()->end()) return result;
                std::ostringstream inscription; bool first = true;
                for (const auto& [color, quantity] : arc->getInscriptions()) { if (!first) inscription << ";"; first = false; inscription << color << "=" << quantity; }
                result.push_back(P{"Name", arc->getName(), {}, P::Kind::String, true});
                result.push_back(P{"Place", arc->getPlaceName(), {}, P::Kind::String, false});
                result.push_back(P{"Transition", arc->getTransitionName(), {}, P::Kind::String, false});
                result.push_back(P{"Direction", arc->isInputArc() ? "Place to transition" : "Transition to place", {}, P::Kind::String, false});
                result.push_back(P{"Inscription", inscription.str(), {}, P::Kind::String, true});
                return result;
            }
            auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
            auto* data = graphical != nullptr ? graphical->getDataDefinition() : nullptr;
            if (auto* transition = dynamic_cast<CPNTransition*>(data)) {
                if (network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return result;
                const CpnPreviewReadiness preview = previewCpnReadiness(network, transition);
                result.push_back(P{"Firing Readiness", preview.explanation.toStdString(), {},
                                   P::Kind::String, false});
                result.push_back(P{"Guard Expression", transition->getGuardExpression(), {}, P::Kind::String, true});
                result.push_back(P{"Priority", std::to_string(transition->getPriority()), {}, P::Kind::Integer, true});
            } else if (auto* place = dynamic_cast<PetriPlace*>(data)) {
                if (network->getPlaces()->find(place) == network->getPlaces()->list()->end()) return result;
                std::ostringstream marking; bool first = true;
                for (const auto& [color, quantity] : network->getInitialMarking(place)) {
                    if (!first) marking << ";"; first = false; marking << color << "=" << quantity;
                }
                result.push_back(P{"Initial Marking", marking.str(), {}, P::Kind::String, true});
                std::ostringstream current; bool firstCurrent = true;
                for (const auto& [color, quantity] : *place->getAllTokens()) {
                    if (!firstCurrent) current << ";";
                    firstCurrent = false;
                    current << color << "=" << quantity;
                }
                result.push_back(P{"Current Marking", current.str(), {}, P::Kind::String, false});
            }
            return result;
        };
        editor.createElementPropertyCommand = [](ModelDataDefinition* definition, QGraphicsItem* item,
            const std::string& property, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(definition);
            if (network == nullptr || item == nullptr || dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) return nullptr;
            auto* arc = reinterpret_cast<CPNArc*>(static_cast<quintptr>(item->data(1).toULongLong()));
            if (arc == nullptr || network->getArcs()->find(arc) == network->getArcs()->list()->end()
                || (property != "Inscription" && property != "Name")) return nullptr;
            std::map<std::string, unsigned int> inscription;
            if (property == "Name") inscription = arc->getInscriptions();
            else {
                std::istringstream input(value); std::string token;
                while (std::getline(input, token, ';')) {
                    if (token.empty()) continue;
                    const auto eq = token.find('='); if (eq == std::string::npos || eq == 0 || eq + 1 == token.size()) return nullptr;
                    unsigned int count = 0; const std::string number = token.substr(eq + 1);
                    const auto [end, ec] = std::from_chars(number.data(), number.data()+number.size(), count);
                    if (ec != std::errc{} || end != number.data()+number.size() || count == 0
                        || !inscription.emplace(token.substr(0, eq), count).second) return nullptr;
                }
            }
            return new EditCpnArcInscriptionUndoCommand(network, arc,
                dynamic_cast<ModelGraphicsScene*>(item->scene()), std::move(inscription),
                property == "Name" ? value : arc->getName());
        };
        editor.acceptsDrop = acceptsCpnDrop;
        editor.onDataDefinitionDropped = addCpnNode;
        editor.onDataDefinitionRemoved = [](ModelDataDefinition* owner, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<ColoredPetriNetNetwork*>(owner);
            if (network == nullptr || dropped == nullptr) return;
            if (auto* place = dynamic_cast<PetriPlace*>(dropped)) network->removePlace(place);
            if (auto* transition = dynamic_cast<CPNTransition*>(dropped)) network->removeTransition(transition);
        };
        registry->addNetworkEditor(std::move(editor));
        GuiActionContribution editMarking;
        editMarking.id = "modal.cpn.edit-initial-marking";
        editMarking.text = "Edit Selected CPN Place Initial Marking...";
        editMarking.menuPath = "Model";
        editMarking.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editMarking.isEnabled = canEditCpnPlaceMarking;
        editMarking.trigger = editCpnPlaceInitialMarking;
        registry->addAction(std::move(editMarking));

        GuiActionContribution editCurrentMarking;
        editCurrentMarking.id = "modal.cpn.edit-current-marking";
        editCurrentMarking.text = "Edit Selected CPN Place Current Marking...";
        editCurrentMarking.menuPath = "Model";
        editCurrentMarking.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editCurrentMarking.isEnabled = canEditCpnPlaceMarking;
        editCurrentMarking.trigger = editCpnPlaceCurrentMarking;
        registry->addAction(std::move(editCurrentMarking));

        GuiActionContribution editArcInscription;
        editArcInscription.id = "modal.cpn.edit-arc-inscription";
        editArcInscription.text = "Edit Selected CPN Arc Inscription...";
        editArcInscription.menuPath = "Model";
        editArcInscription.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editArcInscription.isEnabled = canEditCpnArcInscription;
        editArcInscription.trigger = editCpnArcInscription;
        registry->addAction(std::move(editArcInscription));

        GuiActionContribution resetMarking;
        resetMarking.id = "modal.cpn.reset-place-marking";
        resetMarking.text = "Reset Selected CPN Place to Initial Marking";
        resetMarking.menuPath = "Model";
        resetMarking.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        resetMarking.isEnabled = canEditCpnPlaceMarking;
        resetMarking.trigger = resetCpnPlaceToInitialMarking;
        registry->addAction(std::move(resetMarking));

        GuiActionContribution fire;
        fire.id = "modal.cpn.fire-transition";
        fire.text = "Fire Selected Enabled CPN Transition";
        fire.menuPath = "Model";
        fire.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        fire.isEnabled = canFireSelectedTransition;
        fire.trigger = fireSelectedTransition;
        registry->addAction(std::move(fire));

        GuiActionContribution addArc;
        addArc.id = "modal.cpn.create-arc";
        addArc.text = "Connect Selected Place and Transition...";
        addArc.menuPath = "Model";
        addArc.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        addArc.isEnabled = canCreateCpnArc;
        addArc.trigger = createCpnArc;
        registry->addAction(std::move(addArc));

        GuiActionContribution removeArc;
        removeArc.id = "modal.cpn.remove-arc";
        removeArc.text = "Remove Selected CPN Arc";
        removeArc.menuPath = "Model";
        removeArc.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeArc.isEnabled = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
                && selectedArc(context) != nullptr;
        };
        removeArc.trigger = removeSelectedArc;
        registry->addAction(std::move(removeArc));

        GuiActionContribution removeNode;
        removeNode.id = "modal.cpn.remove-node";
        removeNode.text = "Remove Selected CPN Place or Transition";
        removeNode.menuPath = "Model";
        removeNode.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeNode.isEnabled = canRemoveCpnNode;
        removeNode.trigger = removeSelectedCpnNode;
        registry->addAction(std::move(removeNode));
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(ColoredPetriNetGuiExtensionPlugin);
