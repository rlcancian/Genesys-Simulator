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

#include <QTextEdit>

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

        delete map;
        delete visited;
        _graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
    }
}
