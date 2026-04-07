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
    // Receive a narrow callback to preserve text-to-model sync precondition before image export.
    GraphvizModelExporter(Simulator* simulator,
                          QLabel* modelGraphicLabel,
                          QCheckBox* showInternals,
                          QCheckBox* showElements,
                          QCheckBox* showRecursive,
                          QCheckBox* showLevels,
                          std::function<bool()> ensureModelSynchronized);

    std::string adjustDotName(std::string name) const;
    void insertTextInDot(std::string text,
                         unsigned int compLevel,
                         unsigned int compRank,
                         std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap,
                         bool isNode = false) const;
    void recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData,
                                            std::list<ModelDataDefinition*>* visited,
                                            std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) const;
    // Keep image generation API wrapper-friendly while internally invoking synchronization callback.
    bool createModelImage() const;

private:
    Simulator* _simulator;
    QLabel* _modelGraphicLabel;
    QCheckBox* _showInternals;
    QCheckBox* _showElements;
    QCheckBox* _showRecursive;
    QCheckBox* _showLevels;
    // Keep synchronization dependency narrow and explicit.
    std::function<bool()> _ensureModelSynchronized;
};

#endif // GRAPHVIZMODELEXPORTER_H
