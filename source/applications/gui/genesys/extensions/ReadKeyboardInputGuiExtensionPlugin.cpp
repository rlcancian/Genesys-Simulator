#include "GuiExtensionPluginCatalog.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "plugins/components/InputOutput/Read.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QThread>

class ReadKeyboardInputGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return "gui.extensions.read.keyboard"; }
    std::string displayName() const override { return "Read Keyboard Input"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"read"}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        GuiSimulationStartContribution contribution;
        contribution.id = "read.keyboard.prepare-input";
        contribution.onSimulationStart = [](Simulator* simulator, QMainWindow* window) {
            if (simulator == nullptr || window == nullptr || simulator->getModelManager() == nullptr) return;
            Model* model = simulator->getModelManager()->current();
            if (model == nullptr || model->getComponentManager() == nullptr) return;

            QPointer<QMainWindow> guardedWindow(window);
            QThread* const guiThread = window->thread();
            for (auto it = model->getComponentManager()->begin();
                 it != model->getComponentManager()->end(); ++it) {
                auto* reader = dynamic_cast<Read*>(*it);
                if (reader == nullptr) continue;
                reader->setKeyboardInputProvider([guardedWindow, guiThread](
                        const std::string& prompt, std::string& input) -> bool {
                    // Check thread identity before accessing QPointer or widgets.
                    // Off-thread Read events fail without blocking or touching Qt GUI objects.
                    if (QThread::currentThread() != guiThread || !guardedWindow) return false;
                    bool accepted = false;
                    const QString response = QInputDialog::getText(
                        guardedWindow,
                        QObject::tr("Read — Keyboard Input"),
                        QString::fromStdString(prompt),
                        QLineEdit::Normal,
                        {},
                        &accepted);
                    if (!accepted) return false;
                    input = response.toStdString();
                    return true;
                });
            }
        };
        registry->addSimulationStart(std::move(contribution));
    }
};

REGISTER_GUI_EXTENSION_PLUGIN(ReadKeyboardInputGuiExtensionPlugin)
