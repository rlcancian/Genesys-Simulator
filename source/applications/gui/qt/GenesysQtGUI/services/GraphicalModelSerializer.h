#ifndef GRAPHICALMODELSERIALIZER_H
#define GRAPHICALMODELSERIALIZER_H

#include <functional>
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
                             QAction* actionShowAttachedElements,
                             QAction* actionDiagrams,
                             QTextEdit* console,
                             QString* modelFilename,
                             std::function<void()> clearModelEditors,
                             std::function<void()> rebuildGraphicalModelFromModel,
                             std::function<void()> applyShowInternalElements,
                             std::function<void()> applyShowAttachedElements,
                             std::function<void()> applyDiagramsVisibility);

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
    QAction* _actionShowAttachedElements;
    QAction* _actionDiagrams;
    QTextEdit* _console;
    QString* _modelFilename;
    std::function<void()> _clearModelEditors;
    std::function<void()> _rebuildGraphicalModelFromModel;
    std::function<void()> _applyShowInternalElements;
    std::function<void()> _applyShowAttachedElements;
    std::function<void()> _applyDiagramsVisibility;
};

#endif // GRAPHICALMODELSERIALIZER_H
