#include "services/ModelLanguageSynchronizer.h"

// This include gives access to generated Qt widgets used by the service implementation.
#include "ui_mainwindow.h"

// These includes provide kernel simulator/model APIs used to keep UI and model synchronized.
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Model.h"
#include "../../../../kernel/simulator/ModelPersistence_if.h"

// This include provides QMessageBox used by the existing error-reporting behavior.
#include <QMessageBox>

// These includes provide stream facilities used by the existing text serialization flow.
#include <fstream>
#include <string>

void ModelLanguageSynchronizer::actualizeModelSimLanguage(Simulator* simulator, Ui::MainWindow* ui, bool* textModelHasChangedFlag) const {
    // This guard preserves safety when dependencies are not available.
    if (simulator == nullptr || ui == nullptr || textModelHasChangedFlag == nullptr) {
        return;
    }

    // This block keeps the same persistence roundtrip currently used to regenerate model language text.
    Model* model = simulator->getModelManager()->current();
    if (model != nullptr) {
        model->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
        std::string tempFilename = "./temp.tmp";
        model->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, false);
        model->save(tempFilename);

        std::string line;
        std::ifstream file(tempFilename);
        if (file.is_open()) {
            ui->TextCodeEditor->clear();
            while (std::getline(file, line)) {
                ui->TextCodeEditor->appendPlainText(QString::fromStdString(line));
            }
            file.close();
            *textModelHasChangedFlag = false;
        }
    }
}

bool ModelLanguageSynchronizer::setSimulationModelBasedOnText(QWidget* ownerWidget,
                                                              Simulator* simulator,
                                                              Ui::MainWindow* ui,
                                                              bool textModelHasChanged,
                                                              const std::function<void()>& setOnEventHandlers) const {
    // This guard preserves safety when dependencies are not available.
    if (simulator == nullptr || ui == nullptr) {
        return false;
    }

    // This block intentionally keeps the legacy TODO semantics for text-change handling.
    Model* model = simulator->getModelManager()->current();
    if (textModelHasChanged) {
        // @TODO: Keep behavior unchanged in phase 1 while delegating logic to a service.
        // simulator->getModels()->remove(model);
        // model = nullptr;
    }

    // This block preserves the existing "create from text only when model is null" behavior.
    if (model == nullptr) {
        QString modelLanguage = ui->TextCodeEditor->toPlainText();
        if (!simulator->getModelManager()->createFromLanguage(modelLanguage.toStdString())) {
            QMessageBox::critical(ownerWidget, "Check Model", "Error in the model text. See console for more information.");
        }
        model = simulator->getModelManager()->current();
        if (model != nullptr && setOnEventHandlers) {
            setOnEventHandlers();
        }
    }

    // This return keeps the same success condition currently used by MainWindow.
    return simulator->getModelManager()->current() != nullptr;
}
