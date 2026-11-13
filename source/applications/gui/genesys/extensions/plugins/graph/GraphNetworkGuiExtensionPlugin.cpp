#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/GraphNode.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "kernel/util/Util.h"

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QUndoCommand>
#include <QUndoStack>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace {

constexpr auto kGraphExtensionId = "modal.graph.gui";
void synchronizeGraphProjection(Simulator*, ModelGraphicsScene*);

class AddGraphEdgeUndoCommand final : public QUndoCommand {
public:
    AddGraphEdgeUndoCommand(GraphNetwork* network, GraphEdge* edge, ModelGraphicsScene* scene)
        : _network(network), _edge(edge), _scene(scene) {
        setText(QObject::tr("Add graph edge"));
    }

    void redo() override {
        if (_network == nullptr || _edge == nullptr || !_network->addEdge(_edge)) {
            setObsolete(true);
            return;
        }
        synchronize();
    }

    void undo() override {
        if (_network == nullptr || _edge == nullptr || !_network->removeEdge(_edge)) {
            setObsolete(true);
            return;
        }
        synchronize();
    }

private:
    void synchronize() {
        if (_scene != nullptr) synchronizeGraphProjection(nullptr, _scene);
    }

    GraphNetwork* _network;
    GraphEdge* _edge;
    QPointer<ModelGraphicsScene> _scene;
};

void synchronizeGraphProjection(Simulator*, ModelGraphicsScene* scene) {
    if (scene == nullptr) return;
    scene->clearExtensionOverlayItems(QString::fromLatin1(kGraphExtensionId));
    auto* network = dynamic_cast<GraphNetwork*>(scene->networkEditorOwner());
    if (network == nullptr) return;

    const auto edges = network->getEdges();
    for (std::size_t index = 0; index < edges.size(); ++index) {
        GraphEdge* edge = edges[index];
        if (edge == nullptr || edge->getSource() == nullptr || edge->getDestination() == nullptr) continue;
        GraphicalModelDataDefinition* source = scene->findGraphicalModelDataDefinition(edge->getSource());
        GraphicalModelDataDefinition* destination = scene->findGraphicalModelDataDefinition(edge->getDestination());
        if (source == nullptr || destination == nullptr) continue;

        const QPointF start = source->sceneBoundingRect().center();
        const QPointF end = destination->sceneBoundingRect().center();
        QPainterPath path(start);
        if (edge->getSource() == edge->getDestination()) {
            const QRectF nodeBounds = source->sceneBoundingRect();
            const QPointF top = QPointF(nodeBounds.center().x(), nodeBounds.top());
            path.moveTo(top);
            path.cubicTo(top + QPointF(-35, -55), top + QPointF(35, -55), top);
        } else {
            const auto parallel = network->getEdgesBetween(edge->getSource(), edge->getDestination());
            const auto edgePosition = std::find(parallel.begin(), parallel.end(), edge);
            const std::size_t ordinal = edgePosition == parallel.end()
                ? 0u : static_cast<std::size_t>(std::distance(parallel.begin(), edgePosition));
            if (parallel.size() > 1) {
                const QPointF delta = end - start;
                const qreal length = std::max<qreal>(1.0, std::hypot(delta.x(), delta.y()));
                const QPointF normal(-delta.y() / length, delta.x() / length);
                const qreal offset = (static_cast<qreal>(ordinal) - (parallel.size() - 1) / 2.0) * 22.0;
                path.quadTo((start + end) / 2.0 + normal * offset, end);
            } else {
                path.lineTo(end);
            }
        }

        auto* item = new QGraphicsPathItem(path);
        QPen pen(QColor(65, 75, 90), 2.0);
        item->setPen(pen);
        item->setBrush(Qt::NoBrush);
        item->setZValue(-1.0);
        item->setData(0, QString::fromStdString(edge->getName()));
        item->setData(1, QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(edge)));
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        QString tip = QString::fromStdString(edge->getName());
        if (edge->hasWeight()) tip += QStringLiteral(" (weight=%1)").arg(edge->getWeight());
        item->setToolTip(tip);
        if (network->isDirected()) {
            const QPointF tangent = path.pointAtPercent(0.98) - path.pointAtPercent(0.94);
            const qreal angle = std::atan2(tangent.y(), tangent.x());
            const QPointF tipPoint = path.pointAtPercent(1.0);
            QPainterPath arrow(tipPoint);
            constexpr qreal arrowLength = 10.0;
            constexpr qreal arrowAngle = 0.45;
            arrow.lineTo(tipPoint - QPointF(std::cos(angle - arrowAngle) * arrowLength,
                                           std::sin(angle - arrowAngle) * arrowLength));
            arrow.moveTo(tipPoint);
            arrow.lineTo(tipPoint - QPointF(std::cos(angle + arrowAngle) * arrowLength,
                                           std::sin(angle + arrowAngle) * arrowLength));
            item->setPath(path.united(arrow));
        }
        scene->addExtensionOverlayItem(QString::fromLatin1(kGraphExtensionId), item);
    }
}

class RemoveGraphEdgeUndoCommand final : public QUndoCommand {
public:
    RemoveGraphEdgeUndoCommand(GraphNetwork* network, GraphEdge* edge, ModelGraphicsScene* scene)
        : _network(network), _edge(edge), _scene(scene) { setText(QObject::tr("Remove graph edge")); }
    void redo() override {
        if (_network == nullptr || _edge == nullptr || !_network->removeEdge(_edge)) { setObsolete(true); return; }
        synchronizeGraphProjection(nullptr, _scene);
    }
    void undo() override {
        if (_network == nullptr || _edge == nullptr || !_network->addEdge(_edge)) { setObsolete(true); return; }
        synchronizeGraphProjection(nullptr, _scene);
    }
private:
    GraphNetwork* _network;
    GraphEdge* _edge;
    QPointer<ModelGraphicsScene> _scene;
};


bool canCreateEdge(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr
        || context.graphicsScene->getUndoStack() == nullptr
        || dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) == nullptr) return false;
    int selectedNodes = 0;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (graphical != nullptr && dynamic_cast<GraphNode*>(graphical->getDataDefinition()) != nullptr) {
            ++selectedNodes;
        }
    }
    return selectedNodes == 2;
}

void createEdge(const GuiExtensionRuntimeContext& context) {
    if (!canCreateEdge(context) || context.simulator == nullptr) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<GraphNetwork*>(scene->networkEditorOwner());
    std::vector<GraphNode*> nodes;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* node = graphical != nullptr ? dynamic_cast<GraphNode*>(graphical->getDataDefinition()) : nullptr;
        if (node != nullptr) nodes.push_back(node);
    }
    if (network == nullptr || nodes.size() != 2 || network->getParentModel() == nullptr) return;
    auto* edge = new GraphEdge(network->getParentModel(), nodes[0], nodes[1]);
    scene->getUndoStack()->push(new AddGraphEdgeUndoCommand(network, edge, scene));
}

bool canDeleteEdge(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr || context.graphicsScene->getUndoStack() == nullptr
        || dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) == nullptr) return false;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        if (dynamic_cast<QGraphicsPathItem*>(item) != nullptr
            && item->data(1).isValid()) return true;
    }
    return false;
}

void deleteSelectedEdge(const GuiExtensionRuntimeContext& context) {
    if (!canDeleteEdge(context)) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<GraphNetwork*>(scene->networkEditorOwner());
    for (QGraphicsItem* item : scene->selectedItems()) {
        if (dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) continue;
        auto* edge = reinterpret_cast<GraphEdge*>(static_cast<quintptr>(item->data(1).toULongLong()));
        if (edge != nullptr && network->containsEdge(edge)) {
            scene->getUndoStack()->push(new RemoveGraphEdgeUndoCommand(network, edge, scene));
            return;
        }
    }
}


class GraphNetworkGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return kGraphExtensionId; }
    std::string displayName() const override { return "Graph Network Editor"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"GraphNetwork"}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        GuiDiagramProjectionContribution projection;
        projection.id = kGraphExtensionId;
        projection.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<GraphNetwork*>(definition) != nullptr;
        };
        projection.synchronize = synchronizeGraphProjection;
        registry->addDiagramProjection(std::move(projection));

        GuiNetworkEditorContribution editor;
        editor.id = kGraphExtensionId;
        editor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<GraphNetwork*>(definition) != nullptr;
        };
        editor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<GraphNode>();
        };
        editor.onDataDefinitionDropped = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* node = dynamic_cast<GraphNode*>(dropped);
            if (network != nullptr && node != nullptr) network->addNode(node);
        };
        registry->addNetworkEditor(std::move(editor));

        GuiActionContribution addEdge;
        addEdge.id = "modal.graph.create-edge";
        addEdge.text = "Connect Selected Graph Nodes";
        addEdge.menuPath = "Model";
        addEdge.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        addEdge.isEnabled = canCreateEdge;
        addEdge.trigger = createEdge;
        registry->addAction(std::move(addEdge));

        GuiActionContribution removeEdge;
        removeEdge.id = "modal.graph.remove-edge";
        removeEdge.text = "Remove Selected Graph Edge";
        removeEdge.menuPath = "Model";
        removeEdge.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeEdge.isEnabled = canDeleteEdge;
        removeEdge.trigger = deleteSelectedEdge;
        registry->addAction(std::move(removeEdge));
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(GraphNetworkGuiExtensionPlugin);
