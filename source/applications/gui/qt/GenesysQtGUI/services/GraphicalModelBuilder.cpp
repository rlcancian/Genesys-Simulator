#include "GraphicalModelBuilder.h"

#include "../TraitsGUI.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/simulator/ModelManager.h"
#include "../../../../../kernel/simulator/Model.h"
#include "../../../../../kernel/simulator/ComponentManager.h"
#include "../../../../../kernel/simulator/ConnectionManager.h"
#include "../../../../../kernel/simulator/PluginManager.h"
#include "../../../../../kernel/simulator/Plugin.h"
#include "../../../../../kernel/simulator/SourceModelComponent.h"
#include "../../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../../kernel/simulator/ModelDataDefinition.h"

#include <QTextEdit>
#include <QSet>
#include <QDebug>

namespace {
QPointF stableComponentPosition(GraphicalModelComponent* component) {
    if (component == nullptr) {
        return QPointF();
    }
    const QPointF oldPosition = component->getOldPosition();
    if (!qFuzzyIsNull(oldPosition.x()) || !qFuzzyIsNull(oldPosition.y())) {
        return oldPosition;
    }
    if (component->scene() != nullptr) {
        return component->scenePos();
    }
    return component->pos();
}

QPointF stableDataDefinitionPosition(GraphicalModelDataDefinition* dataDefinition) {
    if (dataDefinition == nullptr) {
        return QPointF();
    }
    const QPointF oldPosition = dataDefinition->getOldPosition();
    if (!qFuzzyIsNull(oldPosition.x()) || !qFuzzyIsNull(oldPosition.y())) {
        return oldPosition;
    }
    if (dataDefinition->scene() != nullptr) {
        return dataDefinition->scenePos();
    }
    return dataDefinition->pos();
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
      _console(console) {}

// Preserve recursive layout traversal and connection restoration behavior.
void GraphicalModelBuilder::recursivalyGenerateGraphicalModelFromModel(ModelComponent* component,
                                                                        List<ModelComponent*>* visited,
                                                                        std::map<ModelComponent*, GraphicalModelComponent*>* map,
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
            } else {
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
                                                   destinyGmc->getGraphicalInputPorts().at(connectionMap.second->channel.portNumber),
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
void GraphicalModelBuilder::rebuildGraphicalDataDefinitionsLayer(std::map<ModelComponent*, GraphicalModelComponent*>* componentMap) {
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
        std::map<ModelComponent*, GraphicalModelComponent*>* map = new std::map<ModelComponent*, GraphicalModelComponent*>();

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
        } while (foundNotVisited);

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

    // Exit early when no model data definitions exist, avoiding unnecessary scene scans and geometry reads.
    bool hasAnyModelDataDefinition = false;
    for (const std::string& dataTypename : dataManager->getDataDefinitionClassnames()) {
        List<ModelDataDefinition*>* modelDataDefinitionList = dataManager->getDataDefinitionList(dataTypename);
        if (modelDataDefinitionList != nullptr && !modelDataDefinitionList->list()->empty()) {
            hasAnyModelDataDefinition = true;
            break;
        }
    }
    if (!hasAnyModelDataDefinition) {
        scene->clearGraphicalDiagramConnections();
        scene->clearGraphicalModelDataDefinitions();
        scene->setDiagramLayerState(false, false);
        return;
    }

    std::map<ModelComponent*, GraphicalModelComponent*> componentMap;
    for (GraphicalModelComponent* gmc : *scene->getAllComponents()) {
        if (gmc == nullptr || gmc->getComponent() == nullptr) {
            continue;
        }
        componentMap[gmc->getComponent()] = gmc;
    }

    // Build the live graphical data-definition snapshot strictly from scene-owned items.
    std::map<ModelDataDefinition*, GraphicalModelDataDefinition*> existingDataDefinitions;
    const QList<QGraphicsItem*> liveItems = scene->items();
    for (QGraphicsItem* item : liveItems) {
        if (auto* gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
            qInfo() << "synchronizeGraphicalDataDefinitionsLayer: ignoring GraphicalModelComponent in data-definition snapshot"
                    << (gmc->getComponent() != nullptr ? QString::fromStdString(gmc->getComponent()->getName()) : QString("<null>"));
            continue;
        }
        GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
            continue;
        }
        existingDataDefinitions[gmdd->getDataDefinition()] = gmdd;
    }

    PluginManager* pluginManager = simulator->getPluginManager();
    QColor purple(128, 0, 128);
    QColor grey(220, 220, 220);

    std::map<ModelDataDefinition*, GraphicalModelDataDefinition*> dataDefinitionMap;
    QSet<ModelDataDefinition*> seenInModel;
    QSet<ModelDataDefinition*> newDataDefinitions;

    for (const std::string& dataTypename : dataManager->getDataDefinitionClassnames()) {
        std::list<ModelDataDefinition*>* listDataDefinitions = dataManager->getDataDefinitionList(dataTypename)->list();
        for (ModelDataDefinition* dataDefinition : *listDataDefinitions) {
            if (dataDefinition == nullptr) {
                continue;
            }
            seenInModel.insert(dataDefinition);

            auto existingIt = existingDataDefinitions.find(dataDefinition);
            if (existingIt != existingDataDefinitions.end()) {
                dataDefinitionMap[dataDefinition] = existingIt->second;
                continue;
            }

            Plugin* plugin = pluginManager->find(dataDefinition->getClassname());
            if (plugin == nullptr) {
                continue;
            }
            GraphicalModelDataDefinition* graphicalDataDefinition = scene->addGraphicalModelDataDefinition(plugin, dataDefinition, QPointF(0, 0), grey);
            if (graphicalDataDefinition != nullptr) {
                dataDefinitionMap[dataDefinition] = graphicalDataDefinition;
                newDataDefinitions.insert(dataDefinition);
            }
        }
    }

    QList<GraphicalModelDataDefinition*> staleGraphicalDataDefinitions;
    for (auto it = existingDataDefinitions.begin(); it != existingDataDefinitions.end(); ++it) {
        if (!seenInModel.contains(it->first) && it->second != nullptr) {
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

    for (const auto& componentEntry : componentMap) {
        ModelComponent* component = componentEntry.first;
        GraphicalModelComponent* graphicalComponent = componentEntry.second;
        if (component == nullptr || graphicalComponent == nullptr) {
            continue;
        }
        qreal yInternal = 0.0;
        qreal yAttached = 0.0;
        QPointF componentPosition;
        bool hasComponentPosition = false;

        for (const auto& attachedData : *component->getAttachedData()) {
            auto attachedIt = dataDefinitionMap.find(attachedData.second);
            if (attachedIt == dataDefinitionMap.end()) {
                continue;
            }
            GraphicalModelDataDefinition* gdd = attachedIt->second;
            if (gdd == nullptr) {
                continue;
            }
            if (newDataDefinitions.contains(attachedData.second)) {
                // Read component geometry only when a newly created attached data definition needs placement.
                if (!hasComponentPosition) {
                    componentPosition = stableComponentPosition(graphicalComponent);
                    yInternal = componentPosition.y();
                    yAttached = componentPosition.y();
                    hasComponentPosition = true;
                }
                yAttached -= 150;
                gdd->setParentItem(nullptr);
                gdd->setPos(componentPosition.x(), yAttached);
                gdd->setOldPosition(componentPosition.x(), yAttached);
                gdd->setColor(purple);
            }
            scene->addGraphicalDiagramConnection(gdd, graphicalComponent, GraphicalDiagramConnection::ConnectionType::ATTACHED);
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
            if (newDataDefinitions.contains(internalData.second)) {
                // Read component geometry only when a newly created internal data definition needs placement.
                if (!hasComponentPosition) {
                    componentPosition = stableComponentPosition(graphicalComponent);
                    yInternal = componentPosition.y();
                    yAttached = componentPosition.y();
                    hasComponentPosition = true;
                }
                yInternal += 150;
                gdd->setPos(componentPosition.x(), yInternal);
                gdd->setOldPosition(componentPosition.x(), yInternal);
                scene->ensureInitialInternalDataDefinitionGrouping(gdd, graphicalComponent);
            }
            scene->addGraphicalDiagramConnection(gdd, graphicalComponent, GraphicalDiagramConnection::ConnectionType::INTERNAL);
        }
    }

    for (const auto& dataDefinitionEntry : dataDefinitionMap) {
        ModelDataDefinition* parentDefinition = dataDefinitionEntry.first;
        GraphicalModelDataDefinition* parentGraphicalDefinition = dataDefinitionEntry.second;
        if (parentDefinition == nullptr || parentGraphicalDefinition == nullptr) {
            continue;
        }
        QPointF parentPosition;
        qreal x = 0.0;
        bool hasParentPosition = false;
        for (const auto& internalData : *parentDefinition->getInternalData()) {
            auto childIt = dataDefinitionMap.find(internalData.second);
            if (childIt == dataDefinitionMap.end() || childIt->second == nullptr) {
                continue;
            }
            GraphicalModelDataDefinition* childGraphicalDefinition = childIt->second;
            if (newDataDefinitions.contains(internalData.second)) {
                // Read parent geometry only when placing newly created child data definitions.
                if (!hasParentPosition) {
                    parentPosition = stableDataDefinitionPosition(parentGraphicalDefinition);
                    x = parentPosition.x();
                    hasParentPosition = true;
                }
                x -= 200;
                childGraphicalDefinition->setParentItem(nullptr);
                childGraphicalDefinition->setPos(x, parentPosition.y());
                childGraphicalDefinition->setOldPosition(x, parentPosition.y());
            }
            scene->addGraphicalDiagramConnection(childGraphicalDefinition, parentGraphicalDefinition, GraphicalDiagramConnection::ConnectionType::INTERNAL);
        }
    }

    const bool hasDataDefinitions = !dataDefinitionMap.empty();
    scene->setDiagramLayerState(hasDataDefinitions, hasDataDefinitions);
    if (hasDataDefinitions) {
        scene->showDiagrams();
    }
}
