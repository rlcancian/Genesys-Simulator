#ifndef GRAPHVIZMODELEXPORTER_H
#define GRAPHVIZMODELEXPORTER_H

#include <functional>
#include <list>
#include <map>
#include <string>

class Simulator;
class ModelDataDefinition;

namespace Ui {
class MainWindow;
}

// This service encapsulates Graphviz DOT generation and PNG rendering for model representation.
class GraphvizModelExporter {
public:
    // This method normalizes identifiers so they are DOT-compatible while preserving current naming rules.
    std::string adjustDotName(std::string name) const;

    // This method inserts generated DOT text into the ranked map structure used by the legacy algorithm.
    void insertTextInDot(std::string text,
                         unsigned int compLevel,
                         unsigned int compRank,
                         std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap,
                         bool isNode = false) const;

    // This method traverses model components and data definitions recursively to build DOT structures.
    void recursiveCreateModelGraphicPicture(Simulator* simulator,
                                            Ui::MainWindow* ui,
                                            ModelDataDefinition* componentOrData,
                                            std::list<ModelDataDefinition*>* visited,
                                            std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) const;

    // This method creates the model image by orchestrating model synchronization and DOT rendering.
    bool createModelImage(Simulator* simulator,
                          Ui::MainWindow* ui,
                          const std::function<bool()>& setSimulationModelBasedOnText) const;
};

#endif // GRAPHVIZMODELEXPORTER_H
