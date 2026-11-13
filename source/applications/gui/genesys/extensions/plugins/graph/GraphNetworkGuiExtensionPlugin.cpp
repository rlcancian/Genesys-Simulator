#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/ModelGraphicsScene.h"
#include "../../../services/GraphicalModelBuilder.h"

#include "plugins/data/ModalModel/GraphNetwork.h"
#include "plugins/data/ModalModel/GraphNode.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/TraceManager.h"
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
#include <charconv>
#include <cmath>
#include <iterator>
#include <optional>
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

class RemoveGraphNodeUndoCommand final : public QUndoCommand {
public:
    RemoveGraphNodeUndoCommand(GraphNetwork* network, GraphNode* node, ModelGraphicsScene* scene)
        : _network(network), _node(node), _scene(scene), _oldLevel(node != nullptr ? node->getLevel() : 0) {
        setText(QObject::tr("Remove graph node"));
        if (network == nullptr || node == nullptr) return;
        const auto edges = network->getEdges();
        for (std::size_t index = 0; index < edges.size(); ++index) {
            GraphEdge* edge = edges[index];
            if (edge != nullptr && (edge->getSource() == node || edge->getDestination() == node)) {
                _incidentEdges.emplace_back(edge, static_cast<unsigned int>(index));
            }
        }
        if (scene != nullptr) {
            if (auto* item = scene->findGraphicalModelDataDefinition(node)) _position = item->pos();
        }
    }

    void redo() override {
        if (_scene == nullptr || _network == nullptr || _node == nullptr || !_network->containsNode(_node)) {
            setObsolete(true);
            return;
        }
        if (!_network->removeNode(_node)) { setObsolete(true); return; }
        Model* model = _node->getParentModel();
        if (model != nullptr) {
            model->remove(_node);
            for (const auto& edge : _incidentEdges) model->remove(edge.first);
        }
        synchronizeScene(false);
    }

    void undo() override {
        if (_scene == nullptr || _network == nullptr || _node == nullptr || _network->containsNode(_node)) {
            setObsolete(true);
            return;
        }
        Model* model = _node->getParentModel();
        if (model != nullptr) model->insert(_node);
        _node->setModelLevel(_oldLevel);
        if (!_network->addNode(_node)) { setObsolete(true); return; }
        for (const auto& [edge, ordinal] : _incidentEdges) {
            if (model != nullptr) model->insert(edge);
            edge->setModelLevel(_oldLevel);
            if (!_network->restoreEdge(edge, ordinal)) { setObsolete(true); return; }
        }
        synchronizeScene(true);
    }

private:
    void synchronizeScene(bool restorePosition) {
        if (_scene == nullptr) return;
        GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(_scene->getSimulator(), _scene);
        if (restorePosition) {
            if (auto* item = _scene->findGraphicalModelDataDefinition(_node)) {
                item->setPos(_position);
                item->setOldPosition(_position.x(), _position.y());
            }
        }
        synchronizeGraphProjection(nullptr, _scene);
    }

    GraphNetwork* _network;
    GraphNode* _node;
    QPointer<ModelGraphicsScene> _scene;
    unsigned int _oldLevel = 0;
    QPointF _position;
    std::vector<std::pair<GraphEdge*, unsigned int>> _incidentEdges;
};

class EditGraphEdgePropertiesUndoCommand final : public QUndoCommand {
public:
    EditGraphEdgePropertiesUndoCommand(GraphNetwork* network, GraphEdge* edge, ModelGraphicsScene* scene,
                                       std::string name, std::optional<double> weight)
        : _network(network), _edge(edge), _scene(scene), _beforeName(edge != nullptr ? edge->getName() : ""),
          _afterName(std::move(name)), _beforeWeight(edge != nullptr && edge->hasWeight()
              ? std::optional<double>(edge->getWeight()) : std::nullopt), _afterWeight(weight) {
        setText(QObject::tr("Edit graph edge"));
    }
    void redo() override {
        _apply(_afterName, _afterWeight);
    }
    void undo() override {
        _apply(_beforeName, _beforeWeight);
    }
private:
    void _apply(const std::string& name, const std::optional<double>& weight) {
        if (_network == nullptr || _edge == nullptr || !_network->containsEdge(_edge)) {
            setObsolete(true);
            return;
        }
        _edge->setName(name);
        if (weight.has_value()) _edge->setWeight(*weight);
        else _edge->clearWeight();
        if (_scene != nullptr) synchronizeGraphProjection(nullptr, _scene);
    }
    GraphNetwork* _network;
    GraphEdge* _edge;
    QPointer<ModelGraphicsScene> _scene;
    std::string _beforeName;
    std::string _afterName;
    std::optional<double> _beforeWeight;
    std::optional<double> _afterWeight;
};

class EditGraphNodePropertiesUndoCommand final : public QUndoCommand {
public:
    struct Properties {
        std::string name;
        std::string label;
        std::uint32_t traceLevel = 0;
        bool reportStatistics = false;
    };
    EditGraphNodePropertiesUndoCommand(GraphNetwork* network, GraphNode* node, Properties after)
        : _network(network), _node(node), _before(_read(node)), _after(std::move(after)) {
        setText(QObject::tr("Edit graph node"));
    }
    void redo() override { _apply(_after); }
    void undo() override { _apply(_before); }
private:
    static Properties _read(GraphNode* node) {
        Properties result;
        if (node != nullptr) {
            result.name = node->getName();
            result.label = node->getLabel();
            result.traceLevel = static_cast<std::uint32_t>(node->getTraceLevelSpecific());
            result.reportStatistics = node->isReportStatistics();
        }
        return result;
    }
    void _apply(const Properties& properties) {
        if (_network == nullptr || _node == nullptr || !_network->containsNode(_node)) {
            setObsolete(true);
            return;
        }
        const std::string normalized = Util::StrReplace(properties.name, " ", "_");
        Model* model = _node->getParentModel();
        auto* manager = model != nullptr ? model->getDataManager() : nullptr;
        ModelDataDefinition* conflict = manager != nullptr
            ? manager->getDataDefinition(Util::TypeOf<GraphNode>(), normalized) : nullptr;
        if (normalized.empty() || (conflict != nullptr && conflict != _node)) {
            setObsolete(true);
            return;
        }
        _node->setName(properties.name);
        _node->setLabel(properties.label);
        _node->setTraceLevelSpecific(static_cast<TraceManager::Level>(properties.traceLevel));
        _node->setReportStatistics(properties.reportStatistics);
    }
    GraphNetwork* _network;
    GraphNode* _node;
    Properties _before;
    Properties _after;
};

GraphEdge* graphEdgeForItem(QGraphicsItem* item, GraphNetwork* network) {
    if (item == nullptr || network == nullptr || dynamic_cast<QGraphicsPathItem*>(item) == nullptr
        || !item->data(1).isValid()) return nullptr;
    auto* edge = reinterpret_cast<GraphEdge*>(static_cast<quintptr>(item->data(1).toULongLong()));
    return edge != nullptr && network->containsEdge(edge) ? edge : nullptr;
}


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
    return selectedNodes == 1;
}

void createEdge(const GuiExtensionRuntimeContext& context) {
    if (!canCreateEdge(context)) return;
    auto* scene = context.graphicsScene;
    for (QGraphicsItem* item : scene->selectedItems()) {
        auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(item);
        auto* node = graphical != nullptr ? dynamic_cast<GraphNode*>(graphical->getDataDefinition()) : nullptr;
        auto* network = dynamic_cast<GraphNetwork*>(scene->networkEditorOwner());
        if (node != nullptr && network != nullptr && network->containsNode(node)) {
            scene->beginNetworkRelationCreation(graphical);
            return;
        }
    }
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

bool canDeleteGraphNode(const GuiExtensionRuntimeContext& context) {
    return context.graphicsScene != nullptr && context.graphicsScene->getUndoStack() != nullptr
        && selectedGraphNode(context) != nullptr;
}

void deleteSelectedGraphNode(const GuiExtensionRuntimeContext& context) {
    if (!canDeleteGraphNode(context)) return;
    auto* network = dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner());
    GraphNode* node = selectedGraphNode(context);
    if (network != nullptr && node != nullptr) {
        context.graphicsScene->getUndoStack()->push(
            new RemoveGraphNodeUndoCommand(network, node, context.graphicsScene));
    }
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
        editor.ownsEditableDataDefinition = [](ModelDataDefinition* definition, ModelDataDefinition* candidate) {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* node = dynamic_cast<GraphNode*>(candidate);
            return network != nullptr && node != nullptr && network->containsNode(node);
        };
        editor.createDataDefinitionPropertyCommand = [](
            ModelDataDefinition* definition, ModelDataDefinition* candidate,
            const std::string& propertyName, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* node = dynamic_cast<GraphNode*>(candidate);
            if (network == nullptr || node == nullptr || !network->containsNode(node)) return nullptr;
            EditGraphNodePropertiesUndoCommand::Properties updated;
            updated.name = node->getName();
            updated.label = node->getLabel();
            updated.traceLevel = static_cast<std::uint32_t>(node->getTraceLevelSpecific());
            updated.reportStatistics = node->isReportStatistics();
            if (propertyName == "Name") updated.name = value;
            else if (propertyName == "Label") updated.label = value;
            else if (propertyName == "Report Statistics") updated.reportStatistics = value == "true" || value == "1";
            else if (propertyName == "Trace Level") {
                unsigned int parsed = 0;
                const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                if (error != std::errc{} || end != value.data() + value.size()) return nullptr;
                updated.traceLevel = parsed;
            } else return nullptr;
            return new EditGraphNodePropertiesUndoCommand(network, node, std::move(updated));
        };
        editor.canStartRelationCreation = [](ModelDataDefinition* definition, QGraphicsItem* sourceItem) {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* graphical = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* node = graphical != nullptr ? dynamic_cast<GraphNode*>(graphical->getDataDefinition()) : nullptr;
            return network != nullptr && node != nullptr && network->containsNode(node);
        };
        editor.createRelationCommand = [](
            ModelDataDefinition* definition, QGraphicsItem* sourceItem,
            QGraphicsItem* destinationItem) -> QUndoCommand* {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* sourceGraphic = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* destinationGraphic = dynamic_cast<GraphicalModelDataDefinition*>(destinationItem);
            auto* source = sourceGraphic != nullptr ? dynamic_cast<GraphNode*>(sourceGraphic->getDataDefinition()) : nullptr;
            auto* destination = destinationGraphic != nullptr
                ? dynamic_cast<GraphNode*>(destinationGraphic->getDataDefinition()) : nullptr;
            if (network == nullptr || source == nullptr || destination == nullptr
                || !network->containsNode(source) || !network->containsNode(destination)) return nullptr;
            if (dynamic_cast<DirectedAcyclicGraphNetwork*>(network) != nullptr
                && (source == destination || network->hasPath(destination, source))) return nullptr;
            std::size_t ordinal = network->getEdges().size();
            std::string name;
            bool inUse = false;
            do {
                name = "E" + std::to_string(ordinal++);
                inUse = false;
                for (GraphEdge* edge : network->getEdges()) {
                    if (edge != nullptr && edge->getName() == name) { inUse = true; break; }
                }
            } while (inUse);
            auto* edge = new GraphEdge(network->getParentModel(), source, destination, name);
            return new AddGraphEdgeUndoCommand(network, edge, sourceGraphic->scene() != nullptr
                ? dynamic_cast<ModelGraphicsScene*>(sourceGraphic->scene()) : nullptr);
        };
        editor.elementProperties = [](ModelDataDefinition* definition, QGraphicsItem* item) {
            std::vector<GuiNetworkEditorContribution::ElementProperty> properties;
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            GraphEdge* edge = graphEdgeForItem(item, network);
            if (edge == nullptr) return properties;
            using Property = GuiNetworkEditorContribution::ElementProperty;
            properties.push_back(Property{"Name", edge->getName(), {}, Property::Kind::String, true});
            properties.push_back(Property{"Source Node", edge->getSource() != nullptr
                ? edge->getSource()->getName() : "", {}, Property::Kind::String, false});
            properties.push_back(Property{"Destination Node", edge->getDestination() != nullptr
                ? edge->getDestination()->getName() : "", {}, Property::Kind::String, false});
            properties.push_back(Property{"Weight", edge->hasWeight() ? std::to_string(edge->getWeight()) : "",
                {}, Property::Kind::String, true});
            return properties;
        };
        editor.createElementPropertyCommand = [](ModelDataDefinition* definition, QGraphicsItem* item,
                                                   const std::string& propertyName,
                                                   const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            GraphEdge* edge = graphEdgeForItem(item, network);
            if (edge == nullptr) return nullptr;
            std::string name = edge->getName();
            std::optional<double> weight = edge->hasWeight()
                ? std::optional<double>(edge->getWeight()) : std::nullopt;
            if (propertyName == "Name") name = value;
            else if (propertyName == "Weight") {
                if (!value.empty()) {
                    double parsed = 0.0;
                    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                    if (error != std::errc{} || end != value.data() + value.size() || !std::isfinite(parsed)) return nullptr;
                    weight = parsed;
                } else weight.reset();
            } else return nullptr;
            auto* scene = item != nullptr ? dynamic_cast<ModelGraphicsScene*>(item->scene()) : nullptr;
            return new EditGraphEdgePropertiesUndoCommand(network, edge, scene, std::move(name), weight);
        };
        editor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<GraphNode>();
        };
        editor.onDataDefinitionDropped = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* node = dynamic_cast<GraphNode*>(dropped);
            if (network != nullptr && node != nullptr) network->addNode(node);
        };
        editor.onDataDefinitionRemoved = [](ModelDataDefinition* definition, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<GraphNetwork*>(definition);
            auto* node = dynamic_cast<GraphNode*>(dropped);
            if (network != nullptr && node != nullptr) network->removeNode(node);
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

        GuiActionContribution removeNode;
        removeNode.id = "modal.graph.remove-node";
        removeNode.text = "Remove Selected Graph Node";
        removeNode.menuPath = "Model";
        removeNode.isVisible = [](const GuiExtensionRuntimeContext& context) {
            return context.graphicsScene != nullptr
                && dynamic_cast<GraphNetwork*>(context.graphicsScene->networkEditorOwner()) != nullptr;
        };
        removeNode.isEnabled = canDeleteGraphNode;
        removeNode.trigger = deleteSelectedGraphNode;
        registry->addAction(std::move(removeNode));

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
