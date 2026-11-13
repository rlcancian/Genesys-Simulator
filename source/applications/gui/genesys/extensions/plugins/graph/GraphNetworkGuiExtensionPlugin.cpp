#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"

#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/GraphNode.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"
#include "kernel/util/Util.h"

#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainterPath>
#include <QPointer>
#include <QPen>
#include <QVBoxLayout>
#include <QUndoCommand>
#include <QUndoStack>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace {

constexpr auto kGraphExtensionId = "modal.graph.gui";
constexpr auto kGraphAnalysisOverlayId = "modal.graph.analysis";
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

class EditGraphEdgeWeightUndoCommand final : public QUndoCommand {
public:
    EditGraphEdgeWeightUndoCommand(GraphEdge* edge, ModelGraphicsScene* scene, double weight)
        : _edge(edge), _scene(scene), _beforeHasWeight(edge != nullptr && edge->hasWeight()),
          _beforeWeight(edge != nullptr ? edge->getWeight() : 1.0), _afterWeight(weight) {
        setText(QObject::tr("Edit graph edge weight"));
    }
    void redo() override {
        if (_edge == nullptr) { setObsolete(true); return; }
        _edge->setWeight(_afterWeight);
        if (_scene != nullptr) synchronizeGraphProjection(nullptr, _scene);
    }
    void undo() override {
        if (_edge == nullptr) { setObsolete(true); return; }
        if (_beforeHasWeight) _edge->setWeight(_beforeWeight);
        else _edge->clearWeight();
        if (_scene != nullptr) synchronizeGraphProjection(nullptr, _scene);
    }
private:
    GraphEdge* _edge;
    QPointer<ModelGraphicsScene> _scene;
    bool _beforeHasWeight;
    double _beforeWeight;
    double _afterWeight;
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
    std::vector<GraphNode*> selectedNodes;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* node = graphical != nullptr ? dynamic_cast<GraphNode*>(graphical->getDataDefinition()) : nullptr;
        if (node != nullptr && network != nullptr && network->containsNode(node)) selectedNodes.push_back(node);
    }
    if (network == nullptr || selectedNodes.size() != 2 || network->getParentModel() == nullptr) return;

    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Create Graph Edge"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* sourceChoice = new QComboBox(&dialog);
    auto* destinationChoice = new QComboBox(&dialog);
    for (GraphNode* node : selectedNodes) {
        const QString label = QString::fromStdString(node->getName());
        const QVariant identity = QVariant::fromValue<qulonglong>(reinterpret_cast<quintptr>(node));
        sourceChoice->addItem(label, identity);
        destinationChoice->addItem(label, identity);
    }
    layout->addWidget(new QLabel(QObject::tr("Source node:"), &dialog));
    layout->addWidget(sourceChoice);
    layout->addWidget(new QLabel(QObject::tr("Destination node:"), &dialog));
    layout->addWidget(destinationChoice);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return;
    auto* source = reinterpret_cast<GraphNode*>(static_cast<quintptr>(sourceChoice->currentData().toULongLong()));
    auto* destination = reinterpret_cast<GraphNode*>(static_cast<quintptr>(destinationChoice->currentData().toULongLong()));
    if (source == nullptr || destination == nullptr) return;

    if (dynamic_cast<DirectedAcyclicGraphNetwork*>(network) != nullptr
        && (source == destination || network->hasPath(destination, source))) {
        QMessageBox::warning(context.mainWindow, QObject::tr("Invalid DAG Edge"),
                             QObject::tr("This edge would create a cycle in the directed acyclic graph."));
        return;
    }
    auto* edge = new GraphEdge(network->getParentModel(), source, destination);
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

GraphEdge* selectedEdge(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr) return nullptr;
    auto* network = dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner());
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        if (dynamic_cast<QGraphicsPathItem*>(item) == nullptr || !item->data(1).isValid()) continue;
        auto* edge = reinterpret_cast<GraphEdge*>(static_cast<quintptr>(item->data(1).toULongLong()));
        if (edge != nullptr && network->containsEdge(edge)) return edge;
    }
    return nullptr;
}

bool canEditEdgeWeight(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedEdge(context) != nullptr;
}

void editSelectedEdgeWeight(const GuiExtensionRuntimeContext& context) {
    GraphEdge* edge = selectedEdge(context);
    if (edge == nullptr || context.graphicsScene->getUndoStack() == nullptr) return;
    bool accepted = false;
    const double weight = QInputDialog::getDouble(
        context.mainWindow, QObject::tr("Graph Edge Weight"), QObject::tr("Weight:"), edge->getWeight(),
        -1.0e12, 1.0e12, 6, &accepted);
    if (accepted && std::isfinite(weight) && (!edge->hasWeight() || edge->getWeight() != weight)) {
        context.graphicsScene->getUndoStack()->push(
            new EditGraphEdgeWeightUndoCommand(edge, context.graphicsScene, weight));
    }
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

GraphNode* selectedGraphNode(const GuiExtensionRuntimeContext& context) {
    if (context.graphicsScene == nullptr) return nullptr;
    auto* network = dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner());
    if (network == nullptr) return nullptr;
    for (QGraphicsItem* item : context.graphicsScene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* node = graphical != nullptr ? dynamic_cast<GraphNode*>(graphical->getDataDefinition()) : nullptr;
        if (node != nullptr && network->containsNode(node)) return node;
    }
    return nullptr;
}

bool canAnalyzeGraph(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr
        && selectedGraphNode(context) != nullptr;
}

void analyzeGraph(const GuiExtensionRuntimeContext& context) {
    if (!canAnalyzeGraph(context)) return;
    auto* scene = context.graphicsScene;
    auto* network = dynamic_cast<GraphNetwork*>(scene->networkEditorOwner());
    GraphNode* source = selectedGraphNode(context);
    QDialog dialog(context.mainWindow);
    dialog.setWindowTitle(QObject::tr("Graph Structure Analysis"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* algorithm = new QComboBox(&dialog);
    algorithm->addItem(QObject::tr("Breadth-first search (BFS)"), QStringLiteral("bfs"));
    algorithm->addItem(QObject::tr("Depth-first search (DFS)"), QStringLiteral("dfs"));
    layout->addWidget(new QLabel(QObject::tr("Analysis from %1:").arg(QString::fromStdString(source->getName())), &dialog));
    layout->addWidget(algorithm);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return;

    scene->clearExtensionOverlayItems(QString::fromLatin1(kGraphAnalysisOverlayId));
    const bool breadthFirst = algorithm->currentData().toString() == QStringLiteral("bfs");
    const GraphTraversalResult result = breadthFirst
        ? network->breadthFirstSearch(source) : network->depthFirstSearch(source);
    for (std::size_t index = 0; index < result.visitOrder.size(); ++index) {
        GraphNode* node = result.visitOrder[index];
        auto* graphical = scene->findGraphicalModelDataDefinition(node);
        if (graphical == nullptr) continue;
        auto* ring = new QGraphicsEllipseItem(graphical->sceneBoundingRect().adjusted(-9, -9, 9, 9));
        ring->setPen(QPen(QColor(220, 105, 35), 2.5));
        ring->setBrush(Qt::NoBrush);
        ring->setZValue(-0.4);
        ring->setToolTip(QStringLiteral("%1 visit %2: %3")
            .arg(breadthFirst ? QStringLiteral("BFS") : QStringLiteral("DFS"))
            .arg(index + 1).arg(QString::fromStdString(node->getName())));
        scene->addExtensionOverlayItem(QString::fromLatin1(kGraphAnalysisOverlayId), ring);
        auto* order = new QGraphicsSimpleTextItem(QString::number(index + 1));
        order->setBrush(QColor(180, 70, 20));
        order->setPos(graphical->sceneBoundingRect().topLeft() + QPointF(-8, -20));
        order->setZValue(-0.3);
        scene->addExtensionOverlayItem(QString::fromLatin1(kGraphAnalysisOverlayId), order);
    }
    for (const auto& entry : result.tree) {
        if (entry.predecessor == nullptr || entry.edge == nullptr) continue;
        auto* parent = scene->findGraphicalModelDataDefinition(entry.predecessor);
        auto* child = scene->findGraphicalModelDataDefinition(entry.node);
        if (parent == nullptr || child == nullptr) continue;
        QPainterPath path(parent->sceneBoundingRect().center());
        path.lineTo(child->sceneBoundingRect().center());
        auto* treeEdge = new QGraphicsPathItem(path);
        treeEdge->setPen(QPen(QColor(235, 125, 35), 4.0, Qt::SolidLine, Qt::RoundCap));
        treeEdge->setZValue(-1.2);
        treeEdge->setToolTip(QObject::tr("%1 tree edge: %2 → %3")
            .arg(breadthFirst ? QStringLiteral("BFS") : QStringLiteral("DFS"),
                 QString::fromStdString(entry.predecessor->getName()), QString::fromStdString(entry.node->getName())));
        scene->addExtensionOverlayItem(QString::fromLatin1(kGraphAnalysisOverlayId), treeEdge);
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

        GuiActionContribution editEdgeWeight;
        editEdgeWeight.id = "modal.graph.edit-edge-weight";
        editEdgeWeight.text = "Edit Selected Graph Edge Weight...";
        editEdgeWeight.menuPath = "Model";
        editEdgeWeight.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        editEdgeWeight.isEnabled = canEditEdgeWeight;
        editEdgeWeight.trigger = editSelectedEdgeWeight;
        registry->addAction(std::move(editEdgeWeight));

        GuiActionContribution analyze;
        analyze.id = "modal.graph.analyze-traversal";
        analyze.text = "Show Graph BFS/DFS Analysis...";
        analyze.menuPath = "Model";
        analyze.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        analyze.isEnabled = canAnalyzeGraph;
        analyze.trigger = analyzeGraph;
        registry->addAction(std::move(analyze));
    }
};

} // namespace

REGISTER_GUI_EXTENSION_PLUGIN(GraphNetworkGuiExtensionPlugin);
