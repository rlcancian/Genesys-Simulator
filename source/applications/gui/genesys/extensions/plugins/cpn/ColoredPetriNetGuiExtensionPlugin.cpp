#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "../../../services/GraphicalModelBuilder.h"

#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/TraceManager.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "kernel/util/Util.h"

#include <QDialog>
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

void synchronizeCpnProjection(Simulator*, ModelGraphicsScene* scene) {
    if (scene == nullptr) return;
    std::unordered_set<CPNArc*> selectedArcs;
    for (QGraphicsItem* item : scene->extensionOverlayItems(QString::fromLatin1(kCpnExtensionId))) {
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
        badge->setToolTip(QObject::tr("Current marking: %1")
            .arg(tokens.isEmpty() ? QObject::tr("empty") : tokens.join(QStringLiteral(", "))));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), badge);
        if (!tokens.isEmpty()) {
            auto* label = new QGraphicsSimpleTextItem(tokens.join(QStringLiteral("\n")));
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
        badge->setPen(QPen(fired ? QColor(225, 90, 35)
                                  : network->isEnabled(transition) ? QColor(45, 150, 90) : QColor(110, 110, 110),
                              fired ? 4.0 : 2.0));
        badge->setBrush(Qt::NoBrush);
        badge->setZValue(-0.7);
        badge->setToolTip(fired ? QObject::tr("Last fired CPN transition")
                                : network->isEnabled(transition) ? QObject::tr("Enabled CPN transition")
                                                                 : QObject::tr("Disabled CPN transition"));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), badge);
    }
    for (CPNArc* arc : *network->getArcs()->list()) {
        if (arc == nullptr) continue;
        auto* place = scene->findGraphicalModelDataDefinition(arc->getPlace());
        auto* transition = scene->findGraphicalModelDataDefinition(arc->getTransition());
        if (place == nullptr || transition == nullptr) continue;
        const QPointF placePoint = place->sceneBoundingRect().center();
        const QPointF transitionPoint = transition->sceneBoundingRect().center();
        const QPointF start = arc->isInputArc() ? placePoint : transitionPoint;
        const QPointF end = arc->isInputArc() ? transitionPoint : placePoint;
        QPainterPath path(start);
        path.lineTo(end);
        addArrow(path, end);
        auto* item = new QGraphicsPathItem(path);
        const bool participatedInLastFiring = network->getLastFiredTransition() == arc->getTransition();
        item->setPen(QPen(participatedInLastFiring ? QColor(225, 90, 35) : QColor(90, 75, 115),
                          participatedInLastFiring ? 3.5 : 2.0));
        item->setBrush(Qt::NoBrush);
        item->setZValue(-1.0);
        item->setData(1, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(arc)));
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setToolTip(participatedInLastFiring
            ? QObject::tr("Last firing: %1").arg(QString::fromStdString(arc->show()))
            : QString::fromStdString(arc->show()));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), item);
        if (selectedArcs.contains(arc)) item->setSelected(true);
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
    return network != nullptr && transition != nullptr && network->isEnabled(transition);
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
                result.push_back(P{"Guard Expression", transition->getGuardExpression(), {}, P::Kind::String, true});
                result.push_back(P{"Priority", std::to_string(transition->getPriority()), {}, P::Kind::Integer, true});
            } else if (auto* place = dynamic_cast<PetriPlace*>(data)) {
                if (network->getPlaces()->find(place) == network->getPlaces()->list()->end()) return result;
                std::ostringstream marking; bool first = true;
                for (const auto& [color, quantity] : network->getInitialMarking(place)) {
                    if (!first) marking << ";"; first = false; marking << color << "=" << quantity;
                }
                result.push_back(P{"Initial Marking", marking.str(), {}, P::Kind::String, true});
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
