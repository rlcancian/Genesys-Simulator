#ifndef MODELLANGUAGESYNCHRONIZER_H
#define MODELLANGUAGESYNCHRONIZER_H

#include <functional>

class QWidget;
class Simulator;

namespace Ui {
class MainWindow;
}

// This service encapsulates synchronization between the textual model editor and kernel model state.
class ModelLanguageSynchronizer {
public:
    // This method refreshes the textual model editor from the current kernel model serialization.
    void actualizeModelSimLanguage(Simulator* simulator, Ui::MainWindow* ui, bool* textModelHasChangedFlag) const;

    // This method creates or refreshes the simulation model from text while preserving the current behavior.
    bool setSimulationModelBasedOnText(QWidget* ownerWidget,
                                       Simulator* simulator,
                                       Ui::MainWindow* ui,
                                       bool textModelHasChanged,
                                       const std::function<void()>& setOnEventHandlers) const;
};

#endif // MODELLANGUAGESYNCHRONIZER_H
