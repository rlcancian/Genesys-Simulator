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

// Encapsulate Phase 7 model/application lifecycle orchestration with narrow dependencies.
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

    // Inject only the state and callbacks required by lifecycle flows.
    ModelLifecycleController(QWidget* ownerWidget,
                             Simulator* simulator,
                             Ui::MainWindow* ui,
                             QString* modelFilename,
                             bool* textModelHasChanged,
                             bool* closingApproved,
                             bool* loaded,
                             Callbacks callbacks);

    // Expose lifecycle actions used by MainWindow compatibility wrappers.
    void onActionModelNewTriggered() const;
    void onActionModelOpenTriggered() const;
    void onActionModelSaveTriggered() const;
    void onActionModelCloseTriggered() const;
    void onActionModelInformationTriggered() const;
    void onActionModelCheckTriggered() const;
    void onActionSimulationConfigureTriggered() const;
    void onActionSimulatorExitTriggered() const;
    bool hasPendingModelChanges() const;
    bool confirmApplicationExit() const;

private:
    // Keep QObject ownership separate while allowing dialog parenting.
    QWidget* _ownerWidget;
    Simulator* _simulator;
    Ui::MainWindow* _ui;
    QString* _modelFilename;
    bool* _textModelHasChanged;
    bool* _closingApproved;
    bool* _loaded;
    Callbacks _callbacks;
};

#endif // MODELLIFECYCLECONTROLLER_H
