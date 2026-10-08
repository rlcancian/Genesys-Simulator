#include "services/ModelLanguageSynchronizer.h"

#include "kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/model/Model.h"
#include "../../../../kernel/simulator/persistence/Persistence_if.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSignalBlocker>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>

#include <string>

/**
 * @brief Stores explicit dependencies once to keep wrapper calls thin.
 */
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

/**
 * @brief Regenerates the text editor content from the current kernel model.
 */
void ModelLanguageSynchronizer::actualizeModelSimLanguage() const {
    if (_simulator == nullptr || _modelTextEditor == nullptr || _textModelHasChangedFlag == nullptr) {
        return;
    }

    Model* model = _simulator->getModelManager()->current();
    if (model == nullptr || model->getPersistence() == nullptr) {
        return;
    }

    // Keep the .gen suffix so Model::save selects GenSerializer, but avoid a
    // process-global working-directory filename and let Qt own cleanup.
    QTemporaryFile temporaryFile(QDir::tempPath() + QStringLiteral("/genesys-model-XXXXXX.gen"));
    temporaryFile.setAutoRemove(true);
    if (!temporaryFile.open()) {
        return;
    }
    const QString temporaryFilename = temporaryFile.fileName();
    temporaryFile.close();

    auto* persistence = model->getPersistence();
    const bool previousSaveDefaults =
        persistence->getOption(Persistence_if::Options::SAVEDEFAULTS);
    persistence->setOption(Persistence_if::Options::SAVEDEFAULTS, true);
    const bool saved = model->save(temporaryFilename.toStdString());
    persistence->setOption(Persistence_if::Options::SAVEDEFAULTS, previousSaveDefaults);
    if (!saved) {
        return;
    }

    QFile file(temporaryFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QStringList visibleLines;
    QTextStream input(&file);
    while (!input.atEnd()) {
        const QString line = input.readLine();
        // GenSerializer structural comments are physical # lines. Complex text
        // literals encode embedded newlines, so a payload line beginning with #
        // remains inside e"..." and is not mistaken for serializer metadata.
        if (line.trimmed().startsWith(QLatin1Char('#'))) {
            continue;
        }
        visibleLines.push_back(line);
    }

    const QString refreshedText = visibleLines.join(QLatin1Char('\n'));
    const QSignalBlocker blocker(_modelTextEditor);
    _modelTextEditor->setPlainText(refreshedText);
    *_textModelHasChangedFlag = false;
}

/**
 * @brief Applies the text editor content to the current kernel model.
 */
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
