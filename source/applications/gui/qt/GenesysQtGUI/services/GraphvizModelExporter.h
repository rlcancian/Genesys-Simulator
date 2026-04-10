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

// Document the service used to export graphical model representations via Graphviz.
/**
 * @brief Service that builds DOT content and renders model images for the GUI.
 *
 * This service encapsulates Graphviz-oriented model representation logic extracted from
 * MainWindow. It keeps image-generation behavior stable while MainWindow exposes compatibility
 * wrappers used by actions and tabs.
 *
 * Responsibilities:
 * - normalize identifiers for DOT compatibility;
 * - build hierarchical DOT fragments from model/data-definition traversal;
 * - generate and display model images after ensuring text/model synchronization.
 *
 * Boundaries:
 * - it does not own model synchronization policy (uses injected callback);
 * - it does not manage simulation commands, traces, or lifecycle actions;
 * - it provides export/representation service behavior, not controller orchestration.
 */
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
