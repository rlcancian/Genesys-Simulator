#ifndef GRAPHVIZMODELEXPORTER_H
#define GRAPHVIZMODELEXPORTER_H

#include <functional>
#include <list>
#include <map>
#include <string>

class QLabel;
class QCheckBox;
class Simulator;
class ModelDataDefinition;

// This Phase-1 service encapsulates Graphviz DOT generation and PNG rendering for model representation.
class GraphvizModelExporter {
public:
    // MainWindow provides explicit dependencies once, keeping wrappers thin and stable.
    GraphvizModelExporter(Simulator* simulator,
                          QLabel* modelGraphicLabel,
                          QCheckBox* showInternals,
                          QCheckBox* showElements,
                          QCheckBox* showRecursive,
                          QCheckBox* showLevels);

    std::string adjustDotName(std::string name) const;
    void insertTextInDot(std::string text,
                         unsigned int compLevel,
                         unsigned int compRank,
                         std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap,
                         bool isNode = false) const;
    void recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData,
                                            std::list<ModelDataDefinition*>* visited,
                                            std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) const;
    bool createModelImage(const std::function<bool()>& setSimulationModelBasedOnText) const;

private:
    Simulator* _simulator;
    QLabel* _modelGraphicLabel;
    QCheckBox* _showInternals;
    QCheckBox* _showElements;
    QCheckBox* _showRecursive;
    QCheckBox* _showLevels;
};

#endif // GRAPHVIZMODELEXPORTER_H
