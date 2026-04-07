#include "services/ModelLanguageSynchronizer.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Model.h"
#include "../../../../kernel/simulator/ModelPersistence_if.h"

#include <QMessageBox>
#include <QPlainTextEdit>

#include <fstream>
#include <string>

// Store explicit dependencies once to keep wrapper calls thin.
ModelLanguageSynchronizer::ModelLanguageSynchronizer(Simulator* simulator,
                                                     QPlainTextEdit* modelTextEditor,
                                                     bool* textModelHasChangedFlag,
                                                     QWidget* ownerWidget,
                                                     std::function<void()> onModelCreatedOrLoaded)
    : _simulator(simulator)
    , _modelTextEditor(modelTextEditor)
    , _textModelHasChangedFlag(textModelHasChangedFlag)
    , _ownerWidget(ownerWidget)
    , _onModelCreatedOrLoaded(std::move(onModelCreatedOrLoaded)) {
}

void ModelLanguageSynchronizer::actualizeModelSimLanguage() const {
    if (_simulator == nullptr || _modelTextEditor == nullptr || _textModelHasChangedFlag == nullptr) {
        return;
    }

    Model* model = _simulator->getModelManager()->current();
    if (model != nullptr) {
        model->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
        std::string tempFilename = "./temp.tmp";
        model->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, false);
        model->save(tempFilename);

        std::string line;
        std::ifstream file(tempFilename);
        if (file.is_open()) {
            _modelTextEditor->clear();
            while (std::getline(file, line)) {
                _modelTextEditor->appendPlainText(QString::fromStdString(line));
            }
            file.close();
            *_textModelHasChangedFlag = false;
        }
    }
}

bool ModelLanguageSynchronizer::setSimulationModelBasedOnText() const {
    if (_simulator == nullptr || _modelTextEditor == nullptr) {
        return false;
    }

    Model* model = _simulator->getModelManager()->current();
    if (_textModelHasChangedFlag != nullptr && *_textModelHasChangedFlag) {
        // Keep phase-1 behavior unchanged: text change handling is still deferred by legacy TODO.
        // _simulator->getModels()->remove(model);
        // model = nullptr;
    }

    if (model == nullptr) {
        QString modelLanguage = _modelTextEditor->toPlainText();
        if (!_simulator->getModelManager()->createFromLanguage(modelLanguage.toStdString())) {
            QMessageBox::critical(_ownerWidget, "Check Model", "Error in the model text. See console for more information.");
        }
        model = _simulator->getModelManager()->current();
        if (model != nullptr && _onModelCreatedOrLoaded) {
            _onModelCreatedOrLoaded();
        }
    }

    return _simulator->getModelManager()->current() != nullptr;
}
