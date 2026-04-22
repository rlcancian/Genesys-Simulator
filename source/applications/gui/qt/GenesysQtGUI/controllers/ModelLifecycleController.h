#ifndef MODELLIFECYCLECONTROLLER_H
#define MODELLIFECYCLECONTROLLER_H

#include <functional>
#include <string>

class QFile;
class QString;
class QWidget;
class Simulator;
class Model;

namespace Ui {
class MainWindow;
}

// Document model lifecycle orchestration delegated by MainWindow compatibility wrappers.
/**
 * @brief Controller for model/application lifecycle flows delegated from MainWindow.
 *
 * This controller centralizes lifecycle actions (new/open/save/close/check/configure/exit)
 * while MainWindow remains the composition root and compatibility façade. It uses explicit
 * callback injection to invoke existing UI update and persistence routines without changing
 * signatures or ownership.
 *
 * Responsibilities:
 * - orchestrate user-triggered lifecycle actions and guard pending-change confirmations;
 * - coordinate model load/save paths through injected serializer callbacks;
 * - trigger UI/tab/action refreshes required after lifecycle transitions.
 *
 * Boundaries:
 * - it does not directly implement file formats (delegated to services);
 * - it does not own simulator/model objects;
 * - it does not manage scene tools, property editor internals, or simulation traces.
 */
class ModelLifecycleController {
public:
    // Group callback dependencies to keep constructor arguments focused and explicit.
    struct Callbacks {
        std::function<void(const std::string&)> insertCommandInConsole;
        std::function<void(Model*)> initUiForNewModel;
        std::function<void()> actualizeActions;
        std::function<void()> actualizeTabPanes;
        std::function<void(bool)> actualizeModelTextHasChanged;
        std::function<bool()> checkModel;
        std::function<bool()> setSimulationModelBasedOnText;
        std::function<void()> clearModelEditors;
        std::function<bool(QString)> saveGraphicalModel;
        std::function<bool(QFile*, QString)> saveTextModel;
        std::function<Model*(std::string)> loadGraphicalModel;
        std::function<void()> connectSceneSignals;
        std::function<void(const char*)> disconnectSceneSignals;
    };

    /** @brief Creates the lifecycle orchestration controller used by MainWindow wrappers. */
    ModelLifecycleController(QWidget* ownerWidget,
                             Simulator* simulator,
                             Ui::MainWindow* ui,
                             QString* modelFilename,
                             bool* textModelHasChanged,
                             bool* graphicalModelHasChanged,
                             bool* closingApproved,
                             bool* loaded,
                             bool& parallelizationEnabled,
                             int& parallelizationThreads,
                             int& parallelizationBatchSize,
                             Callbacks callbacks);

    /** @brief Starts the new-model lifecycle flow and reinitializes GUI/model state. */
    void onActionModelNewTriggered() const;
    /** @brief Starts model-open flow with persistence loading and UI synchronization. */
    void onActionModelOpenTriggered() const;
    /** @brief Opens a model from an explicit file path using the same lifecycle flow as the open action. */
    bool openModelFile(const QString& fileName) const;
    /** @brief Starts model-save flow for graphical/text representations. */
    void onActionModelSaveTriggered() const;
    /** @brief Closes current model after pending-change checks and cleanup orchestration. */
    void onActionModelCloseTriggered() const;
    /** @brief Opens model information dialog for the current model context. */
    void onActionModelInformationTriggered() const;
    /** @brief Runs model-check workflow as a delegated lifecycle operation. */
    void onActionModelCheckTriggered() const;
    /** @brief Opens simulation configuration workflow anchored to lifecycle façade. */
    void onActionSimulationConfigureTriggered() const;
    /** @brief Executes application-exit flow with pending-change confirmation semantics. */
    void onActionSimulatorExitTriggered() const;
    /** @brief Indicates whether current model/text state has unsaved changes. */
    bool hasPendingModelChanges() const;
    /** @brief Requests exit confirmation preserving compatibility prompt behavior. */
    bool confirmApplicationExit() const;

private:
    bool openModelFileInternal(const QString& fileName, bool showDialogs) const;

    // Keep QObject ownership separate while allowing dialog parenting.
    QWidget* _ownerWidget;
    Simulator* _simulator;
    Ui::MainWindow* _ui;
    QString* _modelFilename;
    bool* _textModelHasChanged;
    bool* _graphicalModelHasChanged;
    bool* _closingApproved;
    bool* _loaded;
    bool& _parallelizationEnabled;
    int& _parallelizationThreads;
    int& _parallelizationBatchSize;
    Callbacks _callbacks;
};

#endif // MODELLIFECYCLECONTROLLER_H
