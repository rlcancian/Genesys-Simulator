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
    // MainWindow provides explicit dependencies once, keeping wrappers thin and stable.
    ModelLanguageSynchronizer(Simulator* simulator,
                              Ui::MainWindow* ui,
                              bool* textModelHasChangedFlag,
                              QWidget* ownerWidget,
                              std::function<void()> setOnEventHandlers);

    void actualizeModelSimLanguage() const;
    bool setSimulationModelBasedOnText() const;

private:
    Simulator* _simulator;
    Ui::MainWindow* _ui;
    bool* _textModelHasChangedFlag;
    QWidget* _ownerWidget;
    std::function<void()> _setOnEventHandlers;
};

#endif // MODELLANGUAGESYNCHRONIZER_H
