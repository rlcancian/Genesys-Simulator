#ifndef MODELLANGUAGESYNCHRONIZER_H
#define MODELLANGUAGESYNCHRONIZER_H

#include <functional>

class QWidget;
class QPlainTextEdit;
class Simulator;

// Document the service boundary for textual model representation synchronization.
/**
 * @brief Service layer that synchronizes textual model language with kernel model state.
 *
 * This service was extracted from MainWindow representation code to isolate text/model
 * synchronization concerns. MainWindow remains the composition root and delegates wrapper
 * methods to this class.
 *
 * Responsibilities:
 * - generate/update the textual model representation from current Simulator model;
 * - parse/apply textual edits back into simulator model state;
 * - notify MainWindow via callback when model creation/loading side effects are required.
 *
 * Boundaries:
 * - it does not own simulator or editor widgets;
 * - it does not handle graphical persistence, Graphviz export, or C++ code export;
 * - it is a synchronization service, not a controller.
 */
class ModelLanguageSynchronizer {
public:
    /**
     * @brief Creates the synchronization service used by MainWindow compatibility wrappers.
     */
    ModelLanguageSynchronizer(Simulator* simulator,
                              QPlainTextEdit* modelTextEditor,
                              bool* textModelHasChangedFlag,
                              QWidget* ownerWidget,
                              std::function<void()> onModelCreatedOrLoaded);

    /**
     * @brief Rebuilds textual model representation from the current kernel model.
     *
     * This is part of the synchronization bridge used after lifecycle, property, and scene updates.
     */
    void actualizeModelSimLanguage() const;

    /**
     * @brief Parses text editor content and applies it to the simulator model.
     * @return true when synchronization succeeds and model state is valid for downstream workflows.
     */
    bool setSimulationModelBasedOnText() const;

private:
    Simulator* _simulator;
    QPlainTextEdit* _modelTextEditor;
    bool* _textModelHasChangedFlag;
    QWidget* _ownerWidget;
    std::function<void()> _onModelCreatedOrLoaded;
};

#endif // MODELLANGUAGESYNCHRONIZER_H
