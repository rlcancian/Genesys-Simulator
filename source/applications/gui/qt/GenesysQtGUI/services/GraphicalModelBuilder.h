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

// Rebuild graphical components and links from an already loaded kernel model.
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
