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
    /**
     * @brief Creates the Graphviz exporter service used by MainWindow compatibility wrappers.
     */
    GraphvizModelExporter(Simulator* simulator,
                          QLabel* modelGraphicLabel,
                          QCheckBox* showStatistics,
                          QCheckBox* showEditable,
                          QCheckBox* showShared,
                          QCheckBox* showRecursive,
                          QCheckBox* showLevels,
                          std::function<bool()> ensureModelSynchronized);

    /**
     * @brief Normalizes identifiers to Graphviz-safe names.
     * @param name Raw label/name from model representation.
     * @return DOT-safe identifier preserving compatibility naming intent.
     */
    std::string adjustDotName(std::string name) const;

    /**
     * @brief Inserts a DOT line into the hierarchical rank/level map used for image generation.
     */
    void insertTextInDot(std::string text,
                         unsigned int compLevel,
                         unsigned int compRank,
                         std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap,
                         bool isNode = false) const;
    /**
     * @brief Recursively traverses model/data nodes to build DOT content fragments.
     */
    void recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData,
                                            std::list<ModelDataDefinition*>* visited,
                                            std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) const;

    /**
     * @brief Generates and displays the model image after synchronization preconditions.
     * @return true when DOT generation and image rendering complete successfully.
     */
    bool createModelImage() const;

private:
    Simulator* _simulator;
    QLabel* _modelGraphicLabel;
    QCheckBox* _showStatistics;
    QCheckBox* _showEditable;
    QCheckBox* _showShared;
    QCheckBox* _showRecursive;
    QCheckBox* _showLevels;
    // Keep synchronization dependency narrow and explicit.
    std::function<bool()> _ensureModelSynchronized;
};

#endif // GRAPHVIZMODELEXPORTER_H
