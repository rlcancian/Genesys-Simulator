#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "kernel/util/Util.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QLineEdit>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QUndoCommand>
#include <QUndoStack>

#include <cmath>
#include <map>
#include <memory>
#include <utility>

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
        item->setPen(QPen(QColor(90, 75, 115), 2.0));
        item->setBrush(Qt::NoBrush);
        item->setZValue(-1.0);
        item->setData(1, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(arc)));
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        item->setToolTip(QString::fromStdString(arc->show()));
        scene->addExtensionOverlayItem(QString::fromLatin1(kCpnExtensionId), item);
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
                                     std::map<std::string, unsigned int> after)
        : _network(network), _arc(arc), _scene(scene), _before(arc->getInscriptions()), _after(std::move(after)) {
        setText(QObject::tr("Edit CPN arc inscription"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(const std::map<std::string, unsigned int>& inscription) {
        if (_network == nullptr || _arc == nullptr) { setObsolete(true); return; }
        _arc->clearInscriptions();
        for (const auto& [color, quantity] : inscription) _arc->setInscription(color, quantity);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    CPNArc* _arc;
    QPointer<ModelGraphicsScene> _scene;
    std::map<std::string, unsigned int> _before;
    std::map<std::string, unsigned int> _after;
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

void editSelectedArcInscription(const GuiExtensionRuntimeContext& context) {
    auto* arc = selectedArc(context);
    if (arc == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    QString color = QStringLiteral("default");
    unsigned int quantity = 1;
    if (!arc->getInscriptions().empty()) {
        color = QString::fromStdString(arc->getInscriptions().begin()->first);
        quantity = arc->getInscriptions().begin()->second;
    }
    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Edit CPN Arc Inscription"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* colorInput = new QLineEdit(color, &dialog);
    colorInput->setObjectName(QStringLiteral("cpnInscriptionColor"));
    auto* quantityInput = new QSpinBox(&dialog);
    quantityInput->setRange(1, 1000000);
    quantityInput->setValue(static_cast<int>(quantity));
    layout->addWidget(new QLabel(QObject::tr("Token color:"), &dialog));
    layout->addWidget(colorInput);
    layout->addWidget(new QLabel(QObject::tr("Quantity:"), &dialog));
    layout->addWidget(quantityInput);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted || colorInput->text().trimmed().isEmpty()) return;
    std::map<std::string, unsigned int> inscription;
    inscription[colorInput->text().trimmed().toStdString()] = static_cast<unsigned int>(quantityInput->value());
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    context.graphicsScene->getUndoStack()->push(new EditCpnArcInscriptionUndoCommand(
        network, arc, context.graphicsScene, std::move(inscription)));
}

class SetCpnInitialTokensUndoCommand final : public QUndoCommand {
public:
    SetCpnInitialTokensUndoCommand(ColoredPetriNetNetwork* network, PetriPlace* place,
                                   ModelGraphicsScene* scene, std::string color, unsigned int value)
        : _network(network), _place(place), _scene(scene), _color(std::move(color)),
          _before(network->getInitialTokens(place, _color)), _after(value) {
        setText(QObject::tr("Edit CPN initial marking"));
    }
    void redo() override { apply(_after); }
    void undo() override { apply(_before); }
private:
    void apply(unsigned int value) {
        if (_network == nullptr || _place == nullptr) { setObsolete(true); return; }
        _network->setInitialTokens(_place, _color, value);
        synchronizeCpnProjection(nullptr, _scene);
    }
    ColoredPetriNetNetwork* _network;
    PetriPlace* _place;
    QPointer<ModelGraphicsScene> _scene;
    std::string _color;
    unsigned int _before;
    unsigned int _after;
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

bool canEditInitialTokens(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedPlace(context) != nullptr;
}

void editInitialTokens(const GuiExtensionRuntimeContext& context) {
    auto* place = selectedPlace(context);
    if (place == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Edit CPN Initial Marking"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* color = new QLineEdit(QStringLiteral("default"), &dialog);
    color->setObjectName(QStringLiteral("cpnInitialColor"));
    auto* quantity = new QSpinBox(&dialog);
    quantity->setRange(0, 1000000);
    quantity->setValue(static_cast<int>(network->getInitialTokens(place, "default")));
    layout->addWidget(new QLabel(QObject::tr("Token color:"), &dialog));
    layout->addWidget(color);
    layout->addWidget(new QLabel(QObject::tr("Initial quantity:"), &dialog));
    layout->addWidget(quantity);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() == QDialog::Accepted && !color->text().trimmed().isEmpty())
        context.graphicsScene->getUndoStack()->push(new SetCpnInitialTokensUndoCommand(
            network, place, context.graphicsScene, color->text().trimmed().toStdString(),
            static_cast<unsigned int>(quantity->value())));
}

bool canEditCpnTransition(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedTransition(context) != nullptr;
}

void editCpnTransition(const GuiExtensionRuntimeContext& context) {
    auto* transition = selectedTransition(context);
    if (transition == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Edit CPN Transition"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* guard = new QLineEdit(QString::fromStdString(transition->getGuardExpression()), &dialog);
    guard->setObjectName(QStringLiteral("cpnTransitionGuard"));
    auto* priority = new QSpinBox(&dialog);
    priority->setRange(0, 1000000);
    priority->setValue(static_cast<int>(transition->getPriority()));
    layout->addWidget(new QLabel(QObject::tr("Guard expression:"), &dialog));
    layout->addWidget(guard);
    layout->addWidget(new QLabel(QObject::tr("Priority:"), &dialog));
    layout->addWidget(priority);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() == QDialog::Accepted) {
        auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
        context.graphicsScene->getUndoStack()->push(new EditCpnTransitionUndoCommand(
            network, transition, context.graphicsScene, guard->text().toStdString(),
            static_cast<unsigned int>(priority->value())));
    }
}

bool canCreateCpnArc(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr || context.graphicsScene->getUndoStack() == nullptr
        || dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) == nullptr) return false;
    PetriPlace* place = nullptr;
    CPNTransition* transition = nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical == nullptr) continue;
        if (auto* candidate = dynamic_cast<PetriPlace*>(graphical->getDataDefinition())) place = candidate;
        if (auto* candidate = dynamic_cast<CPNTransition*>(graphical->getDataDefinition())) transition = candidate;
    }
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner());
    return place != nullptr && transition != nullptr
        && network->getPlaces()->find(place) != network->getPlaces()->list()->end()
        && network->getTransitions()->find(transition) != network->getTransitions()->list()->end();
}

void createCpnArc(const GuiExtensionRuntimeContext& context) {
    if (!canCreateCpnArc(context) || context.simulator == nullptr) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<ColoredPetriNetNetwork*>(scene->networkEditorOwner());
    PetriPlace* place = nullptr;
    CPNTransition* transition = nullptr;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical == nullptr) continue;
        if (auto* candidate = dynamic_cast<PetriPlace*>(graphical->getDataDefinition())) place = candidate;
        if (auto* candidate = dynamic_cast<CPNTransition*>(graphical->getDataDefinition())) transition = candidate;
    }
    if (network == nullptr || place == nullptr || transition == nullptr || network->getParentModel() == nullptr) return;
    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Create Colored Petri Net Arc"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* direction = new QComboBox(&dialog);
    direction->addItem(QObject::tr("Place to transition (input)"), static_cast<int>(CPNArc::Direction::PlaceToTransition));
    direction->addItem(QObject::tr("Transition to place (output)"), static_cast<int>(CPNArc::Direction::TransitionToPlace));
    auto* color = new QLineEdit(QStringLiteral("default"), &dialog);
    color->setObjectName(QStringLiteral("cpnArcColor"));
    auto* quantity = new QSpinBox(&dialog);
    quantity->setRange(1, 1000000);
    quantity->setValue(1);
    layout->addWidget(new QLabel(QObject::tr("Direction:"), &dialog));
    layout->addWidget(direction);
    layout->addWidget(new QLabel(QObject::tr("Token color:"), &dialog));
    layout->addWidget(color);
    layout->addWidget(new QLabel(QObject::tr("Inscription quantity:"), &dialog));
    layout->addWidget(quantity);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted || color->text().trimmed().isEmpty()) return;
    const auto arcDirection = static_cast<CPNArc::Direction>(direction->currentData().toInt());
    auto* arc = new CPNArc(network->getParentModel(), place, transition, arcDirection);
    arc->setInscription(color->text().trimmed().toStdString(), static_cast<unsigned int>(quantity->value()));
    scene->getUndoStack()->push(new ToggleCpnArcUndoCommand(network, arc, scene));
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
        GuiNetworkEditorContribution editor;
        editor.id = kCpnExtensionId;
        editor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<ColoredPetriNetNetwork*>(definition) != nullptr;
        };
        editor.acceptsDrop = acceptsCpnDrop;
        editor.onDataDefinitionDropped = addCpnNode;
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

        GuiActionContribution editMarking;
        editMarking.id = "modal.cpn.edit-initial-marking";
        editMarking.text = "Edit Selected Place Initial Marking...";
        editMarking.menuPath = "Model";
        editMarking.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editMarking.isEnabled = canEditInitialTokens;
        editMarking.trigger = editInitialTokens;
        registry->addAction(std::move(editMarking));

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

        GuiActionContribution editArc;
        editArc.id = "modal.cpn.edit-arc-inscription";
        editArc.text = "Edit Selected CPN Arc Inscription...";
        editArc.menuPath = "Model";
        editArc.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editArc.isEnabled = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
                && selectedArc(context) != nullptr;
        };
        editArc.trigger = editSelectedArcInscription;
        registry->addAction(std::move(editArc));

        GuiActionContribution editTransition;
        editTransition.id = "modal.cpn.edit-transition";
        editTransition.text = "Edit Selected CPN Transition Guard and Priority...";
        editTransition.menuPath = "Model";
        editTransition.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<ColoredPetriNetNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editTransition.isEnabled = canEditCpnTransition;
        editTransition.trigger = editCpnTransition;
        registry->addAction(std::move(editTransition));
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(ColoredPetriNetGuiExtensionPlugin);
