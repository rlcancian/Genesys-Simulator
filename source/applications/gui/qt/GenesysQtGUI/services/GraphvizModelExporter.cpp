#include "services/GraphvizModelExporter.h"

// This include gives access to generated Qt widgets consumed by the exporter.
// These includes provide kernel/model types required by DOT generation.
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/ModelDataDefinition.h"
#include "../../../../kernel/simulator/ModelComponent.h"
#include "../../../../kernel/simulator/ConnectionManager.h"
#include "../../../../kernel/simulator/SourceModelComponent.h"
#include "../../../../kernel/simulator/SinkModelComponent.h"

// This include provides utility string helpers used by legacy DOT-name normalization.
#include "../../../../kernel/util/Util.h"

// These includes provide Qt classes used by image generation and UI updates.
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QRegularExpression>
#include <QStringList>

// These includes provide STL facilities used by traversal and file persistence.
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <fstream>
// This include provides std::move used to store constructor callbacks.
#include <utility>

// This helper keeps legacy label escaping semantics for Graphviz output.
static std::string _escapeDotLabelText(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
            case '\r':
                escaped += " ";
                break;
            default:
                escaped += c;
        }
    }
    return escaped;
}

// Keep the Graphviz representation aligned with the graphics-view data-definition categories.
static bool _isStatisticsDataDefinition(ModelDataDefinition* dataDefinition) {
    if (dataDefinition == nullptr) {
        return false;
    }
    const std::string classname = dataDefinition->getClassname();
    return classname == "Counter" || classname == "StatisticsCollector";
}


// Capture only Graphviz-related UI controls required by the exporter.
GraphvizModelExporter::GraphvizModelExporter(Simulator* simulator,
                                             QLabel* modelGraphicLabel,
                                             QCheckBox* showStatistics,
                                             QCheckBox* showEditable,
                                             QCheckBox* showShared,
                                             QCheckBox* showRecursive,
                                             QCheckBox* showLevels,
                                             std::function<bool()> ensureModelSynchronized)
    : _simulator(simulator)
    , _modelGraphicLabel(modelGraphicLabel)
    , _showStatistics(showStatistics)
    , _showEditable(showEditable)
    , _showShared(showShared)
    , _showRecursive(showRecursive)
    , _showLevels(showLevels)
    // Keep model-text synchronization callback local to this service dependency set.
    , _ensureModelSynchronized(std::move(ensureModelSynchronized)) {
}
std::string GraphvizModelExporter::adjustDotName(std::string name) const {
    // This block preserves identifier normalization behavior used by current DOT output.
    std::string text = Util::StrReplace(name, "[", "_");
    text = Util::StrReplace(text, "]", "");
    text = Util::StrReplace(text, ".", "_");
    for (char& c : text) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
            c = '_';
        }
    }
    if (!text.empty() && std::isdigit(static_cast<unsigned char>(text.front()))) {
        text = "_" + text;
    }
    return text;
}

void GraphvizModelExporter::insertTextInDot(std::string text,
                                            unsigned int compLevel,
                                            unsigned int compRank,
                                            std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap,
                                            bool isNode) const {
    // This block preserves the legacy ranked DOT-map insertion strategy.
    if (dotmap->find(compLevel) == dotmap->end()) {
        dotmap->insert({compLevel, new std::map<unsigned int, std::list<std::string>*>()});
    }
    std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotPair = (*dotmap->find(compLevel));
    if (dotPair.second->find(compRank) == dotPair.second->end()) {
        dotPair.second->insert({compRank, new std::list<std::string>()});
    }
    std::pair<unsigned int, std::list<std::string>*> dotPair2 = *(dotPair.second->find(compRank));
    if (isNode) {
        dotPair2.second->insert(dotPair2.second->begin(), text);
    } else {
        dotPair2.second->insert(dotPair2.second->end(), text);
    }
}

void GraphvizModelExporter::recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData,
                                                               std::list<ModelDataDefinition*>* visited,
                                                               std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) const {
    // This local style bundle preserves the existing visual styling and ranking semantics.
    const struct DOT_STYLES {
        std::string nodeComponent = "shape=Mrecord, fontsize=11, fontname=\"Helvetica\", fontcolor=\"#1f2937\", color=\"#334155\", penwidth=1.2, style=\"rounded,filled\", fillcolor=\"bisque\"";
        std::string edgeComponent = "style=solid, arrowhead=\"vee\", color=\"#334155\", fontcolor=\"#334155\", fontsize=8, penwidth=1.1";
        std::string nodeDataDefInternal = "shape=Mrecord, fontsize=9, fontname=\"Helvetica\", color=\"#64748b\", fontcolor=\"#334155\", style=\"rounded,filled\", fillcolor=\"#e2e8f0\"";
        std::string nodeDataDefAttached = "shape=Mrecord, fontsize=9, fontname=\"Helvetica\", color=\"#475569\", fontcolor=\"#1e293b\", style=\"rounded,filled\", fillcolor=\"#dcfce7\"";
        std::string edgeDataDefInternal = "style=dashed, arrowhead=\"diamond\", color=\"#64748b\", fontcolor=\"#64748b\", fontsize=7";
        std::string edgeDataDefAttached = "style=dashed, arrowhead=\"odiamond\", color=\"#475569\", fontcolor=\"#475569\", fontsize=7";
        unsigned int rankSource = 0;
        unsigned int rankSink = 1;
        unsigned int rankComponent = 99;
        unsigned int rankComponentOtherLevel = 99;
        unsigned int rankDataDefInternal = 99;
        unsigned int rankDataDefAttached = 99;
        unsigned int rankEdge = 99;
    } DOT;

    // This block preserves traversal bookkeeping to avoid duplicate DOT generation.
    visited->insert(visited->end(), componentOrData);
    std::string text;
    unsigned int modellevel = _simulator->getModelManager()->current()->getLevel();
    std::list<ModelDataDefinition*>::iterator visitedIt;
    ModelComponent* parentComponentSuperLevel = nullptr;
    unsigned int level = componentOrData->getLevel();

    // This block emits model component nodes according to level visibility flags.
    if (dynamic_cast<ModelComponent*> (componentOrData) != nullptr) {
        if (level != modellevel && !_showLevels->isChecked()) {
            parentComponentSuperLevel = _simulator->getModelManager()->current()->getComponentManager()->find(level);
            assert(parentComponentSuperLevel != nullptr);
            visitedIt = std::find(visited->begin(), visited->end(), parentComponentSuperLevel);
            if (visitedIt == visited->end()) {
                text = "  " + adjustDotName(parentComponentSuperLevel->getName()) + " [" + DOT.nodeComponent + ", label=\"" + _escapeDotLabelText(parentComponentSuperLevel->getClassname()) + "|" + _escapeDotLabelText(parentComponentSuperLevel->getName()) + "\"]" + ";\n";
                insertTextInDot(text, level, DOT.rankComponentOtherLevel, dotmap, true);
            }
        } else {
            text = "  " + adjustDotName(componentOrData->getName()) + " [" + DOT.nodeComponent + ", label=\"" + _escapeDotLabelText(componentOrData->getClassname()) + "|" + _escapeDotLabelText(componentOrData->getName()) + "\"]" + ";\n";
            if (dynamic_cast<SourceModelComponent*> (componentOrData) != nullptr) {
                insertTextInDot(text, level, DOT.rankSource, dotmap, true);
            } else if (dynamic_cast<SinkModelComponent*> (componentOrData) != nullptr) {
                insertTextInDot(text, level, DOT.rankSink, dotmap, true);
            } else {
                insertTextInDot(text, level, DOT.rankComponent, dotmap, true);
            }
        }
    }

    // This block emits attached/internal data definitions and related edges.
    std::string dataname;
    std::string componentName = parentComponentSuperLevel != nullptr ? parentComponentSuperLevel->getName() : componentOrData->getName();
    for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getInternalData()) {
        const bool isStatistics = _isStatisticsDataDefinition(dataPair.second);
        const bool categoryVisible = isStatistics
                                         ? _showStatistics->isChecked()
                                         : _showEditable->isChecked();
        if (!categoryVisible) {
            continue;
        }
        dataname = adjustDotName(dataPair.second->getName());
        level = dataPair.second->getLevel();
        visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
        if (visitedIt == visited->end()) {
            if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                text = "  " + dataname + " [" + DOT.nodeDataDefInternal + ", label=\"" + _escapeDotLabelText(dataPair.second->getClassname()) + "|" + _escapeDotLabelText(dataPair.second->getName()) + "\"]" + ";\n";
                insertTextInDot(text, level, DOT.rankDataDefInternal, dotmap, true);
                if (_showRecursive->isChecked()) {
                    recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
                }
            }
        }
        if (dataPair.second->getLevel() == modellevel || _showLevels->isChecked()) {
            text = "    " + dataname + "->" + adjustDotName(componentName) + " [" + DOT.edgeDataDefInternal + ", label=\"" + _escapeDotLabelText(dataPair.first) + "\"];\n";
            insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
        }
    }
    for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getAttachedData()) {
        const bool isStatistics = _isStatisticsDataDefinition(dataPair.second);
        const bool categoryVisible = isStatistics
                                         ? _showStatistics->isChecked()
                                         : _showShared->isChecked();
        if (!categoryVisible) {
            continue;
        }
        dataname = adjustDotName(dataPair.second->getName());
        level = dataPair.second->getLevel();
        visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
        if (visitedIt == visited->end()) {
            if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                text = "  " + dataname + " [" + DOT.nodeDataDefAttached + ", label=\"" + _escapeDotLabelText(dataPair.second->getClassname()) + "|" + _escapeDotLabelText(dataPair.second->getName()) + "\"]" + ";\n";
                insertTextInDot(text, level, DOT.rankDataDefAttached, dotmap, true);
            }
            if (_showRecursive->isChecked()) {
                recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
            }
        }
        text = "    " + dataname + "->" + adjustDotName(componentName) + " [" + DOT.edgeDataDefAttached + ", label=\"" + _escapeDotLabelText(dataPair.first) + "\"];\n";
        insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
    }

    // This block emits component-to-component connections and continues traversal.
    ModelComponent* component = dynamic_cast<ModelComponent*> (componentOrData);
    if (component != nullptr) {
        level = component->getLevel();
        for (unsigned short i = 0; i < component->getConnectionManager()->size(); i++) {
            Connection* connection = component->getConnectionManager()->getConnectionAtPort(i);
            visitedIt = std::find(visited->begin(), visited->end(), connection->component);
            if (visitedIt == visited->end()) {
                recursiveCreateModelGraphicPicture(connection->component, visited, dotmap);
            }
            if (connection->component->getLevel() == modellevel || _showLevels->isChecked()) {
                text = "    " + adjustDotName(componentName) + "->" + adjustDotName(connection->component->getName()) + "[" + DOT.edgeComponent + "];\n";
                insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
            }
        }
    }
}

bool GraphvizModelExporter::createModelImage() const {
    // This block preserves model synchronization precondition prior to DOT generation.
    bool res = _ensureModelSynchronized ? _ensureModelSynchronized() : false;
    if (!res || _simulator == nullptr || _modelGraphicLabel == nullptr || _showStatistics == nullptr || _showEditable == nullptr || _showShared == nullptr || _showRecursive == nullptr || _showLevels == nullptr || _simulator->getModelManager()->current() == nullptr) {
        return false;
    }

    // This block preserves DOT graph construction orchestration and traversal order.
    std::string dot = "digraph G {\n";
    dot += "  compound=true; rankdir=LR; \n";
    std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap = new std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>();

    std::list<ModelDataDefinition*>* visited = new std::list<ModelDataDefinition*>();
    for (SourceModelComponent* source : *_simulator->getModelManager()->current()->getComponentManager()->getSourceComponents()) {
        if (std::find(visited->begin(), visited->end(), source) == visited->end()) {
            recursiveCreateModelGraphicPicture(source, visited, dotmap);
        }
    }
    for (ModelComponent* transfer : *_simulator->getModelManager()->current()->getComponentManager()->getTransferInComponents()) {
        if (std::find(visited->begin(), visited->end(), transfer) == visited->end()) {
            recursiveCreateModelGraphicPicture(transfer, visited, dotmap);
        }
    }
    for (ModelComponent* comp : *_simulator->getModelManager()->current()->getComponentManager()->getAllComponents()) {
        if (std::find(visited->begin(), visited->end(), comp) == visited->end()) {
            recursiveCreateModelGraphicPicture(comp, visited, dotmap);
        }
    }

    // This block preserves aggregation of level subgraphs into final DOT content.
    unsigned int modelLevel = _simulator->getModelManager()->current()->getLevel();
    for (std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotpair : *dotmap) {
        if (dotpair.first == modelLevel) {
            dot += "\n  // model level\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0) dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1) dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10) dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
        } else if (_showLevels->isChecked()) {
            dot += "\n\n // submodel level  " + std::to_string(dotpair.first) + "\n";
            dot += " subgraph cluster_level_" + std::to_string(dotpair.first) + " {\n";
            dot += "   graph[style=filled; fillcolor=mistyrose2] label=\"" + _simulator->getModelManager()->current()->getComponentManager()->find(dotpair.first)->getName() + "\";\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0) dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1) dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10) dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
            dot += "\n }\n";
        }
    }
    dot += "}\n";

    // This block preserves file writing and external Graphviz invocation behavior.
    std::string basefilename = "./.tempFixedGraphicalModelRepresentation";
    std::string dotfilename = basefilename + ".dot";
    std::string pngfilename = basefilename + ".png";
    try {
        std::ofstream savefile;
        savefile.open(dotfilename, std::ofstream::out);
        QStringList strList = QString::fromStdString(dot).split(QRegularExpression("[\n]"), Qt::SkipEmptyParts);
        for (int i = 0; i < strList.size(); i++) {
            savefile << strList.at(i).toStdString() << std::endl;
        }
        savefile.close();

        try {
            std::remove(pngfilename.c_str());
        } catch (...) {
        }

        try {
            std::string command = "dot -Tpng " + dotfilename + " -o " + pngfilename;
            system(command.c_str());
            QPixmap pm(QString::fromStdString(pngfilename));
                        _modelGraphicLabel->setPixmap(pm);
            _modelGraphicLabel->setScaledContents(false);
            return true;
        } catch (...) {
        }
    } catch (...) {
    }

    // This return preserves current failure behavior when image generation is not successful.
    return false;
}
