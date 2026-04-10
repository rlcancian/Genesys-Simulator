#ifndef GRAPHICALMODELBUILDER_H
#define GRAPHICALMODELBUILDER_H

#include <map>
#include <string>

#include <QColor>

class Simulator;
class QTextEdit;
class ModelGraphicsView;
class ModelGraphicsScene;
class ModelComponent;
class GraphicalModelComponent;
template<typename T> class List;

// Document the service that rebuilds graphical scene artifacts from kernel models.
/**
 * @brief Service that reconstructs graphical components/connections from kernel model data.
 *
 * This builder is used after model loading to recreate the scene representation while keeping
 * MainWindow as a compatibility façade. It acts as a model-representation bridge between the
 * kernel component graph and GUI graphical items.
 *
 * Responsibilities:
 * - recursively create graphical nodes/connections for component branches;
 * - generate a full scene representation from model source components;
 * - apply plugin-category visual metadata needed during reconstruction.
 *
 * Boundaries:
 * - it does not persist files or parse textual model language;
 * - it does not manage selection/property editor/simulation command flows;
 * - it operates as a reconstruction service, not a controller.
 */
class GraphicalModelBuilder {
public:
    // Keep builder dependencies narrow and explicit for Phase 2 extraction.
    GraphicalModelBuilder(Simulator* simulator,
                          ModelGraphicsView* graphicsView,
                          ModelGraphicsScene* scene,
                          std::map<std::string, QColor>* pluginCategoryColor,
                          QTextEdit* console);

    // Recursively create graphical items and connections from one component branch.
    void recursivalyGenerateGraphicalModelFromModel(ModelComponent* component,
                                                    List<ModelComponent*>* visited,
                                                    std::map<ModelComponent*, GraphicalModelComponent*>* map,
                                                    int* x,
                                                    int* y,
                                                    int* ymax,
                                                    int sequenceInLine);

    // Generate the full graphical model from all source components.
    void generateGraphicalModelFromModel();

private:
    Simulator* _simulator;
    ModelGraphicsView* _graphicsView;
    ModelGraphicsScene* _scene;
    std::map<std::string, QColor>* _pluginCategoryColor;
    QTextEdit* _console;
};

#endif // GRAPHICALMODELBUILDER_H
