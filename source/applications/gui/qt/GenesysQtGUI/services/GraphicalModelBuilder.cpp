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

    // Sanitize scene helper containers before any diff logic to avoid stale bookkeeping pointers.
    scene->sanitizeGraphicalDataDefinitionsBookkeeping();

    Model* model = simulator->getModelManager()->current();
    if (model == nullptr || model->getDataManager() == nullptr) {
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
        GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (gmdd == nullptr || gmdd->getDataDefinition() == nullptr) {
            continue;
        }
        existingDataDefinitions[gmdd->getDataDefinition()] = gmdd;
    }

    PluginManager* pluginManager = simulator->getPluginManager();
    ModelDataManager* dataManager = model->getDataManager();
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
        scene->removeGraphicalModelDataDefinition(stale);
    }

    scene->clearGraphicalDiagramConnections();

    for (const auto& componentEntry : componentMap) {
        ModelComponent* component = componentEntry.first;
        GraphicalModelComponent* graphicalComponent = componentEntry.second;
        if (component == nullptr || graphicalComponent == nullptr) {
            continue;
        }

        QPointF componentPosition = graphicalComponent->scenePos();
        qreal yInternal = componentPosition.y();
        qreal yAttached = componentPosition.y();

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

        QPointF parentPosition = parentGraphicalDefinition->scenePos();
        qreal x = parentPosition.x();
        for (const auto& internalData : *parentDefinition->getInternalData()) {
            auto childIt = dataDefinitionMap.find(internalData.second);
            if (childIt == dataDefinitionMap.end() || childIt->second == nullptr) {
                continue;
            }
            GraphicalModelDataDefinition* childGraphicalDefinition = childIt->second;
            if (newDataDefinitions.contains(internalData.second)) {
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
