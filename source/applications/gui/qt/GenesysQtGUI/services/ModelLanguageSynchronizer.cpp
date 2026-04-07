#include "services/ModelLanguageSynchronizer.h"

#include "ui_mainwindow.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Model.h"
#include "../../../../kernel/simulator/ModelPersistence_if.h"

#include <QMessageBox>

#include <fstream>
#include <string>

ModelLanguageSynchronizer::ModelLanguageSynchronizer(Simulator* simulator,
                                                     Ui::MainWindow* ui,
                                                     bool* textModelHasChangedFlag,
                                                     QWidget* ownerWidget,
                                                     std::function<void()> setOnEventHandlers)
    : _simulator(simulator)
    , _ui(ui)
    , _textModelHasChangedFlag(textModelHasChangedFlag)
    , _ownerWidget(ownerWidget)
    , _setOnEventHandlers(std::move(setOnEventHandlers)) {
}

void ModelLanguageSynchronizer::actualizeModelSimLanguage() const {
    if (_simulator == nullptr || _ui == nullptr || _textModelHasChangedFlag == nullptr) {
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
            _ui->TextCodeEditor->clear();
            while (std::getline(file, line)) {
                _ui->TextCodeEditor->appendPlainText(QString::fromStdString(line));
            }
            file.close();
            *_textModelHasChangedFlag = false;
        }
    }
}

bool ModelLanguageSynchronizer::setSimulationModelBasedOnText() const {
    if (_simulator == nullptr || _ui == nullptr) {
        return false;
    }

    Model* model = _simulator->getModelManager()->current();
    if (_textModelHasChangedFlag != nullptr && *_textModelHasChangedFlag) {
        // Keep phase-1 behavior unchanged: text change handling is still deferred by legacy TODO.
        // _simulator->getModels()->remove(model);
        // model = nullptr;
    }

    if (model == nullptr) {
        QString modelLanguage = _ui->TextCodeEditor->toPlainText();
        if (!_simulator->getModelManager()->createFromLanguage(modelLanguage.toStdString())) {
            QMessageBox::critical(_ownerWidget, "Check Model", "Error in the model text. See console for more information.");
        }
        model = _simulator->getModelManager()->current();
        if (model != nullptr && _setOnEventHandlers) {
            _setOnEventHandlers();
        }
    }

    return _simulator->getModelManager()->current() != nullptr;
}
