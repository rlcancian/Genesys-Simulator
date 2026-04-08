#ifndef MODELLANGUAGESYNCHRONIZER_H
#define MODELLANGUAGESYNCHRONIZER_H

#include <functional>

class QWidget;
class QPlainTextEdit;
class Simulator;

// This Phase-1 service encapsulates synchronization between the textual model editor and kernel model state.
class ModelLanguageSynchronizer {
public:
    // MainWindow provides explicit dependencies once, keeping wrappers thin and stable.
    ModelLanguageSynchronizer(Simulator* simulator,
                              QPlainTextEdit* modelTextEditor,
                              bool* textModelHasChangedFlag,
                              QWidget* ownerWidget,
                              std::function<void()> onModelCreatedOrLoaded);

    void actualizeModelSimLanguage() const;
    bool setSimulationModelBasedOnText() const;

private:
    Simulator* _simulator;
    QPlainTextEdit* _modelTextEditor;
    bool* _textModelHasChangedFlag;
    QWidget* _ownerWidget;
    std::function<void()> _onModelCreatedOrLoaded;
};

#endif // MODELLANGUAGESYNCHRONIZER_H
