#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../mainwindow.h"
#include "EFSMNetworkEditorController.h"

class EFSMGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return "modal.efsm.gui"; }
    std::string displayName() const override { return "EFSM Network Editor"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"ModalModelDefault"}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        addAction(registry, "modal.efsm.create-state", "Create EFSM State",
                  &EFSMNetworkEditorController::canCreateState,
                  &EFSMNetworkEditorController::createState);
        addAction(registry, "modal.efsm.create-transition", "Create EFSM Transition",
                  &EFSMNetworkEditorController::canCreateTransition,
                  &EFSMNetworkEditorController::createTransition);
        addAction(registry, "modal.efsm.edit-transition", "Edit EFSM Transition...",
                  &EFSMNetworkEditorController::canEditTransition,
                  &EFSMNetworkEditorController::editTransition);
        addAction(registry, "modal.efsm.set-initial-state", "Set Selected EFSM State as Initial",
                  &EFSMNetworkEditorController::canSetInitialState,
                  &EFSMNetworkEditorController::setInitialState);
        addAction(registry, "modal.efsm.edit-state", "Edit EFSM State...",
                  &EFSMNetworkEditorController::canEditState,
                  &EFSMNetworkEditorController::editState);
    }

private:
    using Predicate = bool (EFSMNetworkEditorController::*)() const;
    using Command = void (EFSMNetworkEditorController::*)();

    static MainWindow* host(const GuiExtensionRuntimeContext& context) {
        return dynamic_cast<MainWindow*>(context.mainWindow);
    }

    static void addAction(GuiExtensionRegistry* registry, const char* id, const char* text,
                          Predicate enabled, Command command) {
        GuiActionContribution action;
        action.id = id;
        action.text = text;
        action.menuPath = "Model";
        action.isVisible = [enabled](const GuiExtensionRuntimeContext& context) {
            MainWindow* mainWindow = host(context);
            if (mainWindow == nullptr) return false;
            EFSMNetworkEditorController controller(mainWindow);
            return controller.hasNetworkContext();
        };
        action.isEnabled = [enabled](const GuiExtensionRuntimeContext& context) {
            MainWindow* mainWindow = host(context);
            if (mainWindow == nullptr) return false;
            EFSMNetworkEditorController controller(mainWindow);
            return (controller.*enabled)();
        };
        action.trigger = [command](const GuiExtensionRuntimeContext& context) {
            MainWindow* mainWindow = host(context);
            if (mainWindow == nullptr) return;
            EFSMNetworkEditorController controller(mainWindow);
            (controller.*command)();
        };
        registry->addAction(std::move(action));
    }
};

REGISTER_GUI_EXTENSION_PLUGIN(EFSMGuiExtensionPlugin);
