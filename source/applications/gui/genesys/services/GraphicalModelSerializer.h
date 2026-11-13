#ifndef GRAPHICALMODELSERIALIZER_H
#define GRAPHICALMODELSERIALIZER_H

#include "../extensions/GuiExtensionContracts.h"

#include <QHash>

#include <functional>
#include <any>
#include <memory>
#include <string>

class Simulator;
class Model;
class QWidget;
class QFile;
class QString;
class QPlainTextEdit;
class QTextEdit;
class QSlider;
class QAction;
class ModelGraphicsView;

// Document persistence responsibilities for textual and graphical GUI model data.
/**
 * @brief Persistence service for saving/loading graphical and textual model representations.
 *
 * This Phase-2 extraction removes serialization concerns from MainWindow while preserving file
 * compatibility through wrapper delegation. It bridges GUI state (scene/view/actions/editors)
 * and kernel model loading/saving workflows.
 *
 * Responsibilities:
 * - save textual model content with the existing line-based format;
 * - save full graphical .gui state sections used by legacy compatibility;
 * - load persisted model files and restore graphical/editor UI state via callbacks.
 *
 * Boundaries:
 * - it does not own widgets or simulator lifetime;
 * - it does not run simulation commands or trace rendering;
 * - it coordinates persistence/serialization only.
 */
class GraphicalModelSerializer {
public:
    /** @brief Creates the persistence service that backs MainWindow lifecycle wrappers. */
    GraphicalModelSerializer(Simulator* simulator,
                             QWidget* ownerWidget,
                             QPlainTextEdit* modelTextEditor,
                             ModelGraphicsView* graphicsView,
                             QSlider* zoomSlider,
                             QAction* actionShowGrid,
                             QAction* actionShowRule,
                             QAction* actionShowSnap,
                             QAction* actionShowGuides,
                             QAction* actionShowInternalElements,
                             QAction* actionShowEditableElements,
                             QAction* actionShowAttachedElements,
                             QAction* actionShowRecursiveElements,
                             QTextEdit* console,
                             QString* modelFilename,
                             std::function<void()> clearModelEditors,
                             std::function<void()> rebuildGraphicalModelFromModel,
                             std::function<void()> applyShowInternalElements,
                             std::function<void()> applyShowEditableElements,
                             std::function<void()> applyShowAttachedElements,
                             std::shared_ptr<GuiDiagramLayoutsByModel> diagramLayoutsByModel = nullptr,
                             std::shared_ptr<GuiUnknownDiagramSectionsByModel> unknownDiagramSectionsByModel = nullptr);

    /**
     * @brief Saves textual model language using the established compatibility format.
     * @return true when write succeeds.
     */
    bool saveTextModel(QFile* saveFile, const QString& data) const;
    /**
     * @brief Saves full graphical `.gui` persistence sections without format changes.
     * @return true when serialization succeeds.
     */
    bool saveGraphicalModel(const QString& filename) const;
    /**
     * @brief Loads `.gui`/`.gen` model files and restores persisted graphical/UI state.
     * @return Loaded kernel model pointer or nullptr when loading fails.
     */
    Model* loadGraphicalModel(const std::string& filename) const;

    /** @brief Stores a GUI-only formalism layout payload for a model, extension and Network ID. */
    void setNetworkDiagramLayout(Model* model, const std::string& extensionId,
                                 unsigned long networkId, std::any layout);
    /** @brief Returns one opaque GUI-only layout payload, or an empty value when absent. */
    std::any networkDiagramLayout(Model* model, const std::string& extensionId,
                                  unsigned long networkId) const;
    /** @brief Drops cached extension layouts when their owning root model is closed. */
    void forgetNetworkDiagramLayouts(Model* model);

private:
    // Encode free text fields into a tab-safe persisted value.
    static QString encodeGuiText(const QString& text);
    // Decode tab-safe persisted text into original UTF-8 value.
    static QString decodeGuiText(const QString& text);

private:
    Simulator* _simulator;
    QWidget* _ownerWidget;
    QPlainTextEdit* _modelTextEditor;
    ModelGraphicsView* _graphicsView;
    QSlider* _zoomSlider;
    QAction* _actionShowGrid;
    QAction* _actionShowRule;
    QAction* _actionShowSnap;
    QAction* _actionShowGuides;
    QAction* _actionShowInternalElements;
    QAction* _actionShowEditableElements;
    QAction* _actionShowAttachedElements;
    QAction* _actionShowRecursiveElements;
    QTextEdit* _console;
    QString* _modelFilename;
    std::function<void()> _clearModelEditors;
    std::function<void()> _rebuildGraphicalModelFromModel;
    std::function<void()> _applyShowInternalElements;
    std::function<void()> _applyShowEditableElements;
    std::function<void()> _applyShowAttachedElements;

    // Type-erased extension layouts keyed by root model, extension ID and Network ID.
    // Shared because serializer instances are rebuilt when the active view changes.
    std::shared_ptr<GuiDiagramLayoutsByModel> _diagramLayoutsByModel;
    std::shared_ptr<GuiUnknownDiagramSectionsByModel> _unknownDiagramSectionsByModel;
};

#endif // GRAPHICALMODELSERIALIZER_H
