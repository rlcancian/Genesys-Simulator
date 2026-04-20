#include "GraphicalModelBuilder.h"

#include "GraphicalDataDefinitionLayout.h"
#include "../TraitsGUI.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/ConnectionManager.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/SourceModelComponent.h"
#include "kernel/simulator/GenesysPropertyIntrospection.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelDataDefinition.h"

#include <QTextEdit>
#include <QSet>
#include <QDebug>
#include <functional>
#include <cctype>
#include <set>

namespace {
    constexpr qreal minimumAutomaticLayoutCoordinate = 20.0;
    constexpr qreal automaticLayoutCollisionPadding = 12.0;
    constexpr int maximumAutomaticLayoutCollisionSteps = 10;

    enum class DataDefinitionDisplayCategory {
        Statistics,
        Editable,
        Shared
    };

    QPointF visibleAutomaticPosition(GraphicalModelDataDefinition* dataDefinition, const QPointF& requestedPosition) {
        QPointF position = requestedPosition;
        QRectF bounds;
        if (dataDefinition != nullptr) {
            bounds = dataDefinition->boundingRect();
        }

        const qreal itemLeft = position.x() + bounds.left();
        if (itemLeft < minimumAutomaticLayoutCoordinate) {
            position.setX(position.x() + minimumAutomaticLayoutCoordinate - itemLeft);
        }

        const qreal itemTop = position.y() + bounds.top();
        if (itemTop < minimumAutomaticLayoutCoordinate) {
            position.setY(position.y() + minimumAutomaticLayoutCoordinate - itemTop);
        }

        return position;
    }

    QRectF paddedLayoutRect(const QRectF& rect) {
        return rect.adjusted(-automaticLayoutCollisionPadding,
                             -automaticLayoutCollisionPadding,
                             automaticLayoutCollisionPadding,
                             automaticLayoutCollisionPadding);
    }

    QRectF dataDefinitionLayoutRect(GraphicalModelDataDefinition* dataDefinition, const QPointF& position) {
        if (dataDefinition == nullptr) {
            return QRectF();
        }
        return dataDefinition->boundingRect().translated(position);
    }

    bool intersectsOccupiedLayout(const QRectF& candidate, const QList<QRectF>& occupiedRects) {
        if (!candidate.isValid()) {
            return false;
        }
        for (const QRectF& occupied : occupiedRects) {
            if (occupied.isValid() && candidate.intersects(occupied)) {
                return true;
            }
        }
        return false;
    }

    void setAutomaticPosition(GraphicalModelDataDefinition* dataDefinition, const QPointF& position) {
        if (dataDefinition == nullptr) {
            return;
        }
        dataDefinition->setPos(position);
        dataDefinition->setOldPosition(position.x(), position.y());
    }

    bool isStatisticsDataDefinition(ModelDataDefinition* dataDefinition) {
        if (dataDefinition == nullptr) {
            return false;
        }
        const std::string classname = dataDefinition->getClassname();
        return classname == "Counter"
               || classname == "StatisticsCollector";
    }

    DataDefinitionDisplayCategory internalDataDefinitionCategory(ModelDataDefinition* dataDefinition) {
        return isStatisticsDataDefinition(dataDefinition)
                   ? DataDefinitionDisplayCategory::Statistics
                   : DataDefinitionDisplayCategory::Editable;
    }

    bool categoryIsVisible(ModelGraphicsScene* scene, DataDefinitionDisplayCategory category) {
        if (scene == nullptr) {
            return false;
        }
        switch (category) {
        case DataDefinitionDisplayCategory::Statistics:
            return scene->showStatisticsDataDefinitions();
        case DataDefinitionDisplayCategory::Editable:
            return scene->showEditableDataDefinitions();
        case DataDefinitionDisplayCategory::Shared:
            return scene->showSharedDataDefinitions();
        }
        return false;
    }

    QColor categoryColor(DataDefinitionDisplayCategory category) {
        switch (category) {
        case DataDefinitionDisplayCategory::Statistics:
            return QColor(80, 145, 205);
        case DataDefinitionDisplayCategory::Editable:
            return QColor(105, 105, 105);
        case DataDefinitionDisplayCategory::Shared:
            return QColor(128, 0, 128);
        }
        return QColor(220, 220, 220);
    }

    void collectEditableReferencedDataDefinitions(List<SimulationControl*>* controls,
                                                  QSet<ModelDataDefinition*>* editableDefinitions,
                                                  std::set<const SimulationControl*>* recursionPath,
                                                  int depth = 0) {
        if (controls == nullptr || editableDefinitions == nullptr || recursionPath == nullptr || depth > 10) {
            return;
        }

        for (SimulationControl* control : *controls->list()) {
            if (control == nullptr || recursionPath->find(control) != recursionPath->end()) {
                continue;
            }

            const GenesysPropertyDescriptor descriptor = GenesysPropertyIntrospection::describe(control);
            if (descriptor.isModelDataDefinitionReference && !descriptor.readOnly) {
                ModelDataDefinition* referenced = control->getReferencedModelDataDefinition();
                if (referenced != nullptr && !isStatisticsDataDefinition(referenced)) {
                    editableDefinitions->insert(referenced);
                }
            }

            recursionPath->insert(control);
            if (descriptor.supportsListEditor && descriptor.isClass) {
                const int itemCount = static_cast<int>(descriptor.choices.size());
                for (int index = 0; index < itemCount; ++index) {
                    ModelDataDefinition* listElement = control->getListElementModelDataDefinition(index);
                    if (listElement != nullptr && !isStatisticsDataDefinition(listElement)) {
                        // The list member itself is an editable model object. This matters for
                        // nested data-definition owners such as Set::ElementSet, where Resource
                        // members should remain controlled by the Editable Elements visibility
                        // category even though they are attached to another GMDD instead of a GMC.
                        editableDefinitions->insert(listElement);
                    }
                    collectEditableReferencedDataDefinitions(control->getProperties(index),
                                                             editableDefinitions,
                                                             recursionPath,
                                                             depth + 1);
                }
            } else if (descriptor.supportsInlineExpansion && control->hasObjectInstance()) {
                collectEditableReferencedDataDefinitions(control->getProperties(),
                                                         editableDefinitions,
                                                         recursionPath,
                                                         depth + 1);
            }
            recursionPath->erase(control);
        }
    }

    bool isPositiveIndexSuffix(const std::string& suffix) {
        if (suffix.empty()) {
            return false;
        }

        std::string indexText = suffix;
        if (indexText.size() > 2 && indexText.front() == '[' && indexText.back() == ']') {
            indexText = indexText.substr(1, indexText.size() - 2);
        }

        if (indexText.empty()) {
            return false;
        }

        for (const char character : indexText) {
            if (!std::isdigit(static_cast<unsigned char>(character))) {
                return false;
            }
        }
        return true;
    }

    bool attachmentNameMatchesEditableControl(const std::string& attachmentName,
                                              const GenesysPropertyDescriptor& descriptor) {
        if (attachmentName.empty() || descriptor.readOnly || !descriptor.isClass) {
            return false;
        }

        if (attachmentName == descriptor.displayName || attachmentName == descriptor.technicalTypeName) {
            return true;
        }

        if (descriptor.isList
            && !descriptor.technicalTypeName.empty()
            && attachmentName.rfind(descriptor.technicalTypeName, 0) == 0) {
            return isPositiveIndexSuffix(attachmentName.substr(descriptor.technicalTypeName.size()));
        }

        return false;
    }

    void collectEditableAttachedDataDefinitionsFromComponent(ModelComponent* component,
                                                             QSet<ModelDataDefinition*>* editableDefinitions) {
        if (component == nullptr || editableDefinitions == nullptr
            || component->getProperties() == nullptr || component->getAttachedData() == nullptr) {
            return;
        }

        for (SimulationControl* control : *component->getProperties()->list()) {
            const GenesysPropertyDescriptor descriptor = GenesysPropertyIntrospection::describe(control);
            for (const auto& attachedData : *component->getAttachedData()) {
                ModelDataDefinition* dataDefinition = attachedData.second;
                if (dataDefinition == nullptr || isStatisticsDataDefinition(dataDefinition)) {
                    continue;
                }
                if (attachmentNameMatchesEditableControl(attachedData.first, descriptor)) {
                    editableDefinitions->insert(dataDefinition);
                }
            }
        }
    }

    QSet<ModelDataDefinition*> editableDataDefinitionsForModel(Model* model) {
        QSet<ModelDataDefinition*> editableDefinitions;
        if (model == nullptr) {
            return editableDefinitions;
        }

        std::set<const SimulationControl*> recursionPath;
        if (model->getComponentManager() != nullptr) {
            for (ModelComponent* component : *model->getComponentManager()->getAllComponents()) {
                if (component != nullptr) {
                    collectEditableReferencedDataDefinitions(component->getProperties(),
                                                             &editableDefinitions,
                                                             &recursionPath);
                    collectEditableAttachedDataDefinitionsFromComponent(component, &editableDefinitions);
                }
            }
        }

        if (model->getDataManager() != nullptr) {
            for (const std::string& dataTypename : model->getDataManager()->getDataDefinitionClassnames()) {
                List<ModelDataDefinition*>* definitions = model->getDataManager()->getDataDefinitionList(dataTypename);
                if (definitions == nullptr) {
                    continue;
                }
                for (ModelDataDefinition* dataDefinition : *definitions->list()) {
                    if (dataDefinition != nullptr) {
                        collectEditableReferencedDataDefinitions(dataDefinition->getProperties(),
                                                                 &editableDefinitions,
                                                                 &recursionPath);
                    }
                }
            }
        }

        return editableDefinitions;
    }

    bool dataDefinitionIsEditableInPropertyEditor(
        ModelDataDefinition* dataDefinition,
        const QSet<ModelDataDefinition*>& editableDefinitions) {
        return dataDefinition != nullptr
               && !isStatisticsDataDefinition(dataDefinition)
               && editableDefinitions.contains(dataDefinition);
    }

    DataDefinitionDisplayCategory attachedDataDefinitionCategory(
        ModelDataDefinition* dataDefinition,
        const QSet<ModelDataDefinition*>& editableDefinitions) {
        if (isStatisticsDataDefinition(dataDefinition)) {
            return DataDefinitionDisplayCategory::Statistics;
        }
        return dataDefinitionIsEditableInPropertyEditor(dataDefinition, editableDefinitions)
                   ? DataDefinitionDisplayCategory::Editable
                   : DataDefinitionDisplayCategory::Shared;
    }

    struct DataDefinitionLayoutLink {
        ModelDataDefinition* dataDefinition = nullptr;
        GraphicalModelDataDefinition* graphicalDefinition = nullptr;
        GraphicalDiagramConnection::ConnectionType connectionType = GraphicalDiagramConnection::ConnectionType::ATTACHED;
    };

    QPointF dataDefinitionArcPosition(const QRectF& anchorBounds,
                                      GraphicalModelDataDefinition* child,
                                      int index,
                                      int count,
                                      bool upperArc,
                                      int radialLayer = 0) {
        if (child == nullptr) {
            return QPointF();
        }

        const QRectF childBounds = child->boundingRect();
        return GraphicalDataDefinitionLayout::arcPosition(anchorBounds,
                                                          childBounds.size(),
                                                          index,
                                                          count,
                                                          upperArc,
                                                          radialLayer);
    }

    void positionNewDataDefinitionsInArc(const QRectF& anchorBounds,
                                         const QList<DataDefinitionLayoutLink>& links,
                                         bool upperArc,
                                         const QSet<ModelDataDefinition*>& newDataDefinitions,
                                         QSet<ModelDataDefinition*>* positionedDataDefinitions,
                                         QList<QRectF>* occupiedLayoutRects,
                                         int baseRadialLayer = 0) {
        if (positionedDataDefinitions == nullptr) {
            return;
        }

        QList<DataDefinitionLayoutLink> newLinks;
        for (const DataDefinitionLayoutLink& link : links) {
            if (link.dataDefinition == nullptr || link.graphicalDefinition == nullptr) {
                continue;
            }
            if (!newDataDefinitions.contains(link.dataDefinition)
                || positionedDataDefinitions->contains(link.dataDefinition)) {
                continue;
            }
            newLinks.append(link);
        }

        const int count = newLinks.size();
        for (int index = 0; index < count; ++index) {
            GraphicalModelDataDefinition* child = newLinks.at(index).graphicalDefinition;
            child->setParentItem(nullptr);
            QPointF selectedPosition = visibleAutomaticPosition(
                child,
                dataDefinitionArcPosition(anchorBounds, child, index, count, upperArc, baseRadialLayer));
            QRectF selectedRect = paddedLayoutRect(dataDefinitionLayoutRect(child, selectedPosition));

            if (occupiedLayoutRects != nullptr) {
                for (int radialLayer = baseRadialLayer + 1;
                     radialLayer <= baseRadialLayer + maximumAutomaticLayoutCollisionSteps
                     && intersectsOccupiedLayout(selectedRect, *occupiedLayoutRects);
                     ++radialLayer) {
                    selectedPosition = visibleAutomaticPosition(
                        child,
                        dataDefinitionArcPosition(anchorBounds, child, index, count, upperArc, radialLayer));
                    selectedRect = paddedLayoutRect(dataDefinitionLayoutRect(child, selectedPosition));
                }
            }

            setAutomaticPosition(child, selectedPosition);
            positionedDataDefinitions->insert(newLinks.at(index).dataDefinition);
            if (occupiedLayoutRects != nullptr) {
                occupiedLayoutRects->append(selectedRect);
            }
        }
    }
}

// Build the graphical reconstruction service with explicit dependencies.
GraphicalModelBuilder::GraphicalModelBuilder(Simulator* simulator,
                                             ModelGraphicsView* graphicsView,
                                             ModelGraphicsScene* scene,
                                             std::map<std::string, QColor>* pluginCategoryColor,
                                             QTextEdit* console)
    : _simulator(simulator),
      _graphicsView(graphicsView),
      _scene(scene),
      _pluginCategoryColor(pluginCategoryColor),
      _console(console) {
}

// Preserve recursive layout traversal and connection restoration behavior.
void GraphicalModelBuilder::recursivalyGenerateGraphicalModelFromModel(ModelComponent* component,
                                                                       List<ModelComponent*>* visited,
                                                                       std::map<ModelComponent*, GraphicalModelComponent
                                                                           *>* map,
                                                                       int* x,
                                                                       int* y,
                                                                       int* ymax,
                                                                       int sequenceInLine) {
    PluginManager* pm = _simulator->getPluginManager();
    Plugin* plugin = pm->find(component->getClassname());
    if (plugin == nullptr) {
        _console->append(QString::fromStdString(
            "Warning: Skipping component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ") because plugin \""
            + component->getClassname() + "\" is unavailable."));
        return;
    }

    QColor color = Qt::white;
    auto colorIt = _pluginCategoryColor->find(plugin->getPluginInfo()->getCategory());
    if (colorIt != _pluginCategoryColor->end()) {
        color = colorIt->second;
    }

    GraphicalModelComponent* gmc = _scene->addGraphicalModelComponent(plugin, component, QPoint(*x, *y), color);
    if (gmc == nullptr) {
        _console->append(QString::fromStdString(
            "Warning: Failed to draw component \"" + component->getName() + "\" (id="
            + std::to_string(component->getId()) + ")."));
        return;
    }

    map->insert({component, gmc});
    visited->insert(component);

    int yIni = *y;
    int xIni = *x;
    const int deltaY = TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 1.5;
    for (auto connectionMap : *component->getConnectionManager()->connections()) {
        if (connectionMap.second == nullptr || connectionMap.second->component == nullptr) {
            continue;
        }

        ModelComponent* nextComp = connectionMap.second->component;
        if (visited->find(nextComp) == visited->list()->end()) {
            if (++sequenceInLine == 6) {
                *x -= 5 * TraitsGUI<GModelComponent>::width * 1.5;
                *y += deltaY;
                sequenceInLine = 0;
            }
            else {
                *x += TraitsGUI<GModelComponent>::width * 1.5;
            }
            if (*y > *ymax) {
                *ymax = *y;
            }

            recursivalyGenerateGraphicalModelFromModel(nextComp, visited, map, x, y, ymax, sequenceInLine);
            auto destinyIt = map->find(nextComp);
            if (destinyIt != map->end()) {
                GraphicalModelComponent* destinyGmc = destinyIt->second;
                if (connectionMap.first < gmc->getGraphicalOutputPorts().size()
                    && connectionMap.second->channel.portNumber < destinyGmc->getGraphicalInputPorts().size()) {
                    _scene->addGraphicalConnection(gmc->getGraphicalOutputPorts().at(connectionMap.first),
                                                   destinyGmc->getGraphicalInputPorts().at(
                                                       connectionMap.second->channel.portNumber),
                                                   connectionMap.first,
                                                   connectionMap.second->channel.portNumber);
                }
            }

            *x = xIni;
            *y += deltaY;
            sequenceInLine--;
        }
    }

    *y = yIni;
}

// Rebuild graphical data definitions and diagram links as part of the main model regeneration flow.
void GraphicalModelBuilder::rebuildGraphicalDataDefinitionsLayer(
    std::map<ModelComponent*, GraphicalModelComponent*>* componentMap) {
    Q_UNUSED(componentMap);
    synchronizeGraphicalDataDefinitionsLayer(_simulator, _scene);
}

// Preserve the existing full-model generation flow and visitation semantics.
void GraphicalModelBuilder::generateGraphicalModelFromModel() {
    Model* m = _simulator->getModelManager()->current();
    if (m != nullptr) {
        _graphicsView->setCanNotifyGraphicalModelEventHandlers(false);
        int x = TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter * 0.8;
        int y = TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter * 0.8;
        int ymax = y;

        ComponentManager* cm = m->getComponentManager();
        if (cm == nullptr) {
            _graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
            return;
        }

        List<ModelComponent*>* visited = new List<ModelComponent*>();
        std::map<ModelComponent*, GraphicalModelComponent*>* map = new std::map<
            ModelComponent*, GraphicalModelComponent*>();

        for (SourceModelComponent* source : *cm->getSourceComponents()) {
            recursivalyGenerateGraphicalModelFromModel(source, visited, map, &x, &y, &ymax, 0);
            y = ymax + TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 3;
        }

        bool foundNotVisited;
        do {
            foundNotVisited = false;
            for (ModelComponent* comp : *cm->getAllComponents()) {
                if (visited->find(comp) == visited->list()->end()) {
                    foundNotVisited = true;
                    visited->insert(comp);
                }
            }
        }
        while (foundNotVisited);

        rebuildGraphicalDataDefinitionsLayer(map);

        delete map;
        delete visited;
        _graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
    }
}

void GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer(Simulator* simulator, ModelGraphicsScene* scene) {
    if (simulator == nullptr || scene == nullptr) {
        return;
    }

    class ScopedConnectionGeometryBlocker {
    public:
        explicit ScopedConnectionGeometryBlocker(ModelGraphicsScene* guardedScene) : _guardedScene(guardedScene) {
            if (_guardedScene != nullptr) {
                _guardedScene->setConnectionGeometryUpdatesBlocked(true);
            }
        }

        ~ScopedConnectionGeometryBlocker() {
            if (_guardedScene != nullptr) {
                _guardedScene->setConnectionGeometryUpdatesBlocked(false);
            }
        }

    private:
        ModelGraphicsScene* _guardedScene = nullptr;
    };

    ScopedConnectionGeometryBlocker scopedConnectionGeometryBlocker(scene);

    // Sanitize scene helper containers before any diff logic to avoid stale bookkeeping pointers.
    scene->sanitizeGraphicalDataDefinitionsBookkeeping();

    Model* model = simulator->getModelManager()->current();
    if (model == nullptr || model->getDataManager() == nullptr) {
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->setDiagramLayerState(false, false);
        return;
    }
    ModelDataManager* dataManager = model->getDataManager();
    const QSet<ModelDataDefinition*> editablePropertyDataDefinitions = editableDataDefinitionsForModel(model);

    std::map<ModelComponent*, GraphicalModelComponent*> componentMap;
    const QList<GraphicalModelComponent*> liveGraphicalComponents = scene->graphicalModelComponentItems();
    for (GraphicalModelComponent* gmc : liveGraphicalComponents) {
        if (gmc == nullptr || gmc->getComponent() == nullptr) {
            continue;
        }
        componentMap[gmc->getComponent()] = gmc;
    }

    QSet<ModelDataDefinition*> visibleDataDefinitions;
    QSet<ModelDataDefinition*> expandedDataDefinitions;
    std::map<ModelDataDefinition*, DataDefinitionDisplayCategory> visibleCategories;

    std::function<void(ModelDataDefinition*)> expandVisibleDataDefinition =
        [&](ModelDataDefinition* owner) {
        if (owner == nullptr || !scene->showRecursiveDataDefinitions() || expandedDataDefinitions.contains(owner)) {
            return;
        }
        expandedDataDefinitions.insert(owner);

        for (const auto& attachedData : *owner->getAttachedData()) {
            ModelDataDefinition* child = attachedData.second;
            if (child == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = attachedDataDefinitionCategory(
                child,
                editablePropertyDataDefinitions);
            if (!categoryIsVisible(scene, category)) {
                continue;
            }
            visibleDataDefinitions.insert(child);
            visibleCategories[child] = category;
            expandVisibleDataDefinition(child);
        }

        for (const auto& internalData : *owner->getInternalData()) {
            ModelDataDefinition* child = internalData.second;
            if (child == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = internalDataDefinitionCategory(child);
            if (!categoryIsVisible(scene, category)) {
                continue;
            }
            visibleDataDefinitions.insert(child);
            visibleCategories[child] = category;
            expandVisibleDataDefinition(child);
        }
    };

    for (const auto& componentEntry : componentMap) {
        ModelComponent* component = componentEntry.first;
        if (component == nullptr) {
            continue;
        }

        for (const auto& attachedData : *component->getAttachedData()) {
            ModelDataDefinition* dataDefinition = attachedData.second;
            if (dataDefinition == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = attachedDataDefinitionCategory(
                dataDefinition,
                editablePropertyDataDefinitions);
            if (!categoryIsVisible(scene, category)) {
                continue;
            }
            visibleDataDefinitions.insert(dataDefinition);
            visibleCategories[dataDefinition] = category;
            expandVisibleDataDefinition(dataDefinition);
        }

        for (const auto& internalData : *component->getInternalData()) {
            ModelDataDefinition* dataDefinition = internalData.second;
            if (dataDefinition == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = internalDataDefinitionCategory(dataDefinition);
            if (!categoryIsVisible(scene, category)) {
                continue;
            }
            visibleDataDefinitions.insert(dataDefinition);
            visibleCategories[dataDefinition] = category;
            expandVisibleDataDefinition(dataDefinition);
        }
    }

    if (visibleDataDefinitions.isEmpty()) {
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->setDiagramLayerState(false, false);
        return;
    }

    // Build the live graphical data-definition snapshot strictly from scene-owned items.
    std::map<ModelDataDefinition*, GraphicalModelDataDefinition*> existingDataDefinitions;
    const QList<QGraphicsItem*> liveItems = scene->items();
    for (QGraphicsItem* item : liveItems) {
        if (auto* gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
            qInfo() <<
                "synchronizeGraphicalDataDefinitionsLayer: ignoring GraphicalModelComponent in data-definition snapshot"
                << (gmc->getComponent() != nullptr
                        ? QString::fromStdString(gmc->getComponent()->getName())
                        : QString("<null>"));
            continue;
        }
        GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
            continue;
        }
        existingDataDefinitions[gmdd->getDataDefinition()] = gmdd;
    }

    PluginManager* pluginManager = simulator->getPluginManager();
    QColor grey(220, 220, 220);

    std::map<ModelDataDefinition*, GraphicalModelDataDefinition*> dataDefinitionMap;
    QSet<ModelDataDefinition*> newDataDefinitions;

    const auto ensureVisibleGraphicalDataDefinition = [&](ModelDataDefinition* dataDefinition) {
        if (dataDefinition == nullptr || !visibleDataDefinitions.contains(dataDefinition)) {
            return;
        }
        if (dataDefinitionMap.find(dataDefinition) != dataDefinitionMap.end()) {
            return;
        }

        auto existingIt = existingDataDefinitions.find(dataDefinition);
        if (existingIt != existingDataDefinitions.end()) {
            dataDefinitionMap[dataDefinition] = existingIt->second;
            const auto categoryIt = visibleCategories.find(dataDefinition);
            if (categoryIt != visibleCategories.end() && existingIt->second != nullptr) {
                existingIt->second->setColor(categoryColor(categoryIt->second));
                existingIt->second->setEditableInPropertyEditor(
                    dataDefinitionIsEditableInPropertyEditor(dataDefinition, editablePropertyDataDefinitions));
            }
            return;
        }

        Plugin* plugin = pluginManager->find(dataDefinition->getClassname());
        if (plugin == nullptr) {
            return;
        }
        const auto categoryIt = visibleCategories.find(dataDefinition);
        const QColor color = categoryIt != visibleCategories.end()
                                 ? categoryColor(categoryIt->second)
                                 : grey;
        GraphicalModelDataDefinition* graphicalDataDefinition = scene->addGraphicalModelDataDefinition(
            plugin, dataDefinition, QPointF(0, 0), color);
        if (graphicalDataDefinition != nullptr) {
            dataDefinitionMap[dataDefinition] = graphicalDataDefinition;
            graphicalDataDefinition->setEditableInPropertyEditor(
                dataDefinitionIsEditableInPropertyEditor(dataDefinition, editablePropertyDataDefinitions));
            newDataDefinitions.insert(dataDefinition);
        }
    };

    for (const std::string& dataTypename : dataManager->getDataDefinitionClassnames()) {
        std::list<ModelDataDefinition*>* listDataDefinitions = dataManager->getDataDefinitionList(dataTypename)->list();
        for (ModelDataDefinition* dataDefinition : *listDataDefinitions) {
            ensureVisibleGraphicalDataDefinition(dataDefinition);
        }
    }
    for (ModelDataDefinition* dataDefinition : visibleDataDefinitions) {
        ensureVisibleGraphicalDataDefinition(dataDefinition);
    }

    QList<GraphicalModelDataDefinition*> staleGraphicalDataDefinitions;
    for (auto it = existingDataDefinitions.begin(); it != existingDataDefinitions.end(); ++it) {
        if (!visibleDataDefinitions.contains(it->first) && it->second != nullptr) {
            staleGraphicalDataDefinitions.append(it->second);
        }
    }
    for (GraphicalModelDataDefinition* stale : staleGraphicalDataDefinitions) {
        const QString staleName = (stale->getDataDefinition() != nullptr)
                                      ? QString::fromStdString(stale->getDataDefinition()->getName())
                                      : QString("<null>");
        qInfo() << "synchronizeGraphicalDataDefinitionsLayer: stale data definition removal"
            << staleName;
        scene->removeGraphicalModelDataDefinition(stale);
    }

    scene->clearGraphicalDiagramConnections();

    QSet<ModelDataDefinition*> positionedDataDefinitions;
    QList<QRectF> occupiedLayoutRects;
    for (const auto& componentEntry : componentMap) {
        GraphicalModelComponent* graphicalComponent = componentEntry.second;
        if (graphicalComponent != nullptr) {
            occupiedLayoutRects.append(paddedLayoutRect(graphicalComponent->sceneBoundingRect()));
        }
    }
    for (const auto& dataDefinitionEntry : dataDefinitionMap) {
        ModelDataDefinition* dataDefinition = dataDefinitionEntry.first;
        GraphicalModelDataDefinition* graphicalDefinition = dataDefinitionEntry.second;
        if (dataDefinition == nullptr || graphicalDefinition == nullptr
            || newDataDefinitions.contains(dataDefinition)) {
            continue;
        }
        occupiedLayoutRects.append(paddedLayoutRect(graphicalDefinition->sceneBoundingRect()));
    }

    for (const auto& componentEntry : componentMap) {
        ModelComponent* component = componentEntry.first;
        GraphicalModelComponent* graphicalComponent = componentEntry.second;
        if (component == nullptr || graphicalComponent == nullptr) {
            continue;
        }

        QList<DataDefinitionLayoutLink> upperLinks;
        QList<DataDefinitionLayoutLink> lowerStatisticsLinks;
        QList<DataDefinitionLayoutLink> lowerSharedLinks;

        for (const auto& attachedData : *component->getAttachedData()) {
            auto attachedIt = dataDefinitionMap.find(attachedData.second);
            if (attachedIt == dataDefinitionMap.end()) {
                continue;
            }
            GraphicalModelDataDefinition* gdd = attachedIt->second;
            if (gdd == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = visibleCategories[attachedData.second];
            const bool editable = dataDefinitionIsEditableInPropertyEditor(attachedData.second,
                                                                           editablePropertyDataDefinitions);
            gdd->setEditableInPropertyEditor(editable);
            gdd->setColor(categoryColor(category));
            const DataDefinitionLayoutLink link{
                attachedData.second,
                gdd,
                GraphicalDiagramConnection::ConnectionType::ATTACHED};
            if (editable) {
                upperLinks.append(link);
            } else if (category == DataDefinitionDisplayCategory::Statistics) {
                lowerStatisticsLinks.append(link);
            } else {
                lowerSharedLinks.append(link);
            }
        }

        for (const auto& internalData : *component->getInternalData()) {
            auto internalIt = dataDefinitionMap.find(internalData.second);
            if (internalIt == dataDefinitionMap.end()) {
                continue;
            }
            GraphicalModelDataDefinition* gdd = internalIt->second;
            if (gdd == nullptr) {
                continue;
            }
            const DataDefinitionDisplayCategory category = visibleCategories[internalData.second];
            const bool editable = dataDefinitionIsEditableInPropertyEditor(internalData.second,
                                                                           editablePropertyDataDefinitions);
            gdd->setEditableInPropertyEditor(editable);
            gdd->setColor(categoryColor(category));
            const DataDefinitionLayoutLink link{
                internalData.second,
                gdd,
                GraphicalDiagramConnection::ConnectionType::INTERNAL};
            if (editable) {
                upperLinks.append(link);
            } else if (category == DataDefinitionDisplayCategory::Statistics) {
                lowerStatisticsLinks.append(link);
            } else {
                lowerSharedLinks.append(link);
            }
        }

        const QRectF componentBounds = graphicalComponent->sceneBoundingRect();
        positionNewDataDefinitionsInArc(componentBounds,
                                        upperLinks,
                                        true,
                                        newDataDefinitions,
                                        &positionedDataDefinitions,
                                        &occupiedLayoutRects);
        positionNewDataDefinitionsInArc(componentBounds,
                                        lowerStatisticsLinks,
                                        false,
                                        newDataDefinitions,
                                        &positionedDataDefinitions,
                                        &occupiedLayoutRects);
        positionNewDataDefinitionsInArc(componentBounds,
                                        lowerSharedLinks,
                                        false,
                                        newDataDefinitions,
                                        &positionedDataDefinitions,
                                        &occupiedLayoutRects,
                                        1);

        const auto addComponentConnections = [&](const QList<DataDefinitionLayoutLink>& links) {
            for (const DataDefinitionLayoutLink& link : links) {
                scene->addGraphicalDiagramConnection(link.graphicalDefinition,
                                                     graphicalComponent,
                                                     link.connectionType);
            }
        };
        addComponentConnections(upperLinks);
        addComponentConnections(lowerStatisticsLinks);
        addComponentConnections(lowerSharedLinks);
    }

    for (const auto& dataDefinitionEntry : dataDefinitionMap) {
        ModelDataDefinition* parentDefinition = dataDefinitionEntry.first;
        GraphicalModelDataDefinition* parentGraphicalDefinition = dataDefinitionEntry.second;
        if (parentDefinition == nullptr || parentGraphicalDefinition == nullptr) {
            continue;
        }
        QList<DataDefinitionLayoutLink> childLinks;
        QList<DataDefinitionLayoutLink> lowerStatisticsChildLinks;
        QList<DataDefinitionLayoutLink> lowerSharedChildLinks;

        for (const auto& attachedData : *parentDefinition->getAttachedData()) {
            auto childIt = dataDefinitionMap.find(attachedData.second);
            if (childIt == dataDefinitionMap.end() || childIt->second == nullptr) {
                continue;
            }
            GraphicalModelDataDefinition* childGraphicalDefinition = childIt->second;
            const DataDefinitionDisplayCategory category = visibleCategories[attachedData.second];
            childGraphicalDefinition->setEditableInPropertyEditor(
                dataDefinitionIsEditableInPropertyEditor(attachedData.second, editablePropertyDataDefinitions));
            childGraphicalDefinition->setColor(categoryColor(category));
            const DataDefinitionLayoutLink link{attachedData.second,
                                                childGraphicalDefinition,
                                                GraphicalDiagramConnection::ConnectionType::ATTACHED};
            childLinks.append(link);
            if (category == DataDefinitionDisplayCategory::Statistics) {
                lowerStatisticsChildLinks.append(link);
            } else {
                lowerSharedChildLinks.append(link);
            }
        }

        for (const auto& internalData : *parentDefinition->getInternalData()) {
            auto childIt = dataDefinitionMap.find(internalData.second);
            if (childIt == dataDefinitionMap.end() || childIt->second == nullptr) {
                continue;
            }
            GraphicalModelDataDefinition* childGraphicalDefinition = childIt->second;
            const DataDefinitionDisplayCategory category = visibleCategories[internalData.second];
            childGraphicalDefinition->setEditableInPropertyEditor(
                dataDefinitionIsEditableInPropertyEditor(internalData.second, editablePropertyDataDefinitions));
            childGraphicalDefinition->setColor(categoryColor(category));
            const DataDefinitionLayoutLink link{internalData.second,
                                                childGraphicalDefinition,
                                                GraphicalDiagramConnection::ConnectionType::INTERNAL};
            childLinks.append(link);
            if (category == DataDefinitionDisplayCategory::Statistics) {
                lowerStatisticsChildLinks.append(link);
            } else {
                lowerSharedChildLinks.append(link);
            }
        }

        const bool parentUsesUpperArc = parentGraphicalDefinition->isEditableInPropertyEditor();
        const QRectF parentBounds = parentGraphicalDefinition->sceneBoundingRect();
        if (parentUsesUpperArc) {
            positionNewDataDefinitionsInArc(parentBounds,
                                            childLinks,
                                            true,
                                            newDataDefinitions,
                                            &positionedDataDefinitions,
                                            &occupiedLayoutRects);
        } else {
            positionNewDataDefinitionsInArc(parentBounds,
                                            lowerStatisticsChildLinks,
                                            false,
                                            newDataDefinitions,
                                            &positionedDataDefinitions,
                                            &occupiedLayoutRects);
            positionNewDataDefinitionsInArc(parentBounds,
                                            lowerSharedChildLinks,
                                            false,
                                            newDataDefinitions,
                                            &positionedDataDefinitions,
                                            &occupiedLayoutRects,
                                            1);
        }

        for (const DataDefinitionLayoutLink& link : childLinks) {
            scene->addGraphicalDiagramConnection(link.graphicalDefinition,
                                                 parentGraphicalDefinition,
                                                 link.connectionType);
        }
    }

    const bool hasDataDefinitions = !dataDefinitionMap.empty();
    scene->setDiagramLayerState(hasDataDefinitions, hasDataDefinitions);
    if (hasDataDefinitions) {
        scene->showDiagrams();
    }
}
