#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../mainwindow.h"
#include "../../../services/EFSMNetworkDiagramProjection.h"
#include "../../../services/EFSMNetworkDiagramCodec.h"
#include "EFSMNetworkEditorController.h"
#include "../../../../../../plugins/data/ModalModel/EFSMNetwork.h"
#include "kernel/util/Util.h"

#include <QTextStream>

#include <any>
#include <utility>

class EFSMGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
    std::string extensionId() const override { return "modal.efsm.gui"; }
    std::string displayName() const override { return "EFSM Network Editor"; }
    std::string version() const override { return "1.0.0"; }
    std::vector<std::string> requiredModelPlugins() const override { return {"ModalModelDefault"}; }

    void registerContributions(GuiExtensionRegistry* registry) const override {
        if (registry == nullptr) return;
        GuiDiagramProjectionContribution diagram;
        diagram.id = "modal.efsm.gui";
        diagram.sectionName = EFSMNetworkDiagramCodec::kSectionName;
        diagram.supports = [](ModelDataDefinition* network) {
            return dynamic_cast<EFSMNetwork*>(network) != nullptr;
        };
        diagram.synchronize = [](Simulator* simulator, ModelGraphicsScene* scene) {
            EFSMNetworkDiagramProjection::synchronize(simulator, scene);
        };
        diagram.captureLayout = [](Simulator*, ModelDataDefinition* definition, ModelGraphicsScene* scene) {
            auto* network = dynamic_cast<EFSMNetwork*>(definition);
            EFSMNetworkDiagramLayout layout;
            if (network != nullptr && EFSMNetworkDiagramProjection::captureLayout(network, scene, &layout)) {
                return std::any(std::move(layout));
            }
            return std::any{};
        };
        diagram.restoreLayout = [](Simulator* simulator, ModelDataDefinition* definition,
                                  ModelGraphicsScene* scene, const std::any& layout) {
            auto* network = dynamic_cast<EFSMNetwork*>(definition);
            const auto* efsmLayout = std::any_cast<EFSMNetworkDiagramLayout>(&layout);
            if (network != nullptr && efsmLayout != nullptr) {
                EFSMNetworkDiagramProjection::restoreLayout(simulator, network, scene, *efsmLayout);
            }
        };
        diagram.serializeLayouts = [](Simulator* simulator, const GuiDiagramLayouts& layouts) {
            QHash<unsigned long, EFSMNetworkDiagramLayout> typedLayouts;
            for (const auto& [networkId, layout] : layouts) {
                if (const auto* typed = std::any_cast<EFSMNetworkDiagramLayout>(&layout)) {
                    typedLayouts.insert(networkId, *typed);
                }
            }
            QString encoded;
            QTextStream stream(&encoded);
            EFSMNetworkDiagramCodec::writeSection(stream, simulator, typedLayouts);
            return encoded.split('\n', Qt::SkipEmptyParts);
        };
        diagram.parseLayouts = [](Model* model, const QStringList& lines) {
            GuiDiagramLayouts layouts;
            const auto parsed = EFSMNetworkDiagramCodec::parseSection(lines, model);
            for (auto it = parsed.cbegin(); it != parsed.cend(); ++it) {
                layouts.emplace(it.key(), it.value());
            }
            return layouts;
        };
        registry->addDiagramProjection(std::move(diagram));
        GuiNetworkEditorContribution networkEditor;
        networkEditor.id = "modal.efsm.gui";
        networkEditor.supports = [](ModelDataDefinition* definition) {
            return dynamic_cast<EFSMNetwork*>(definition) != nullptr;
        };
        networkEditor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<FSMState>();
        };
        networkEditor.onDataDefinitionDropped = [](ModelDataDefinition* owner, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* state = dynamic_cast<FSMState*>(dropped);
            if (network != nullptr && state != nullptr) network->addState(state);
        };
        registry->addNetworkEditor(std::move(networkEditor));
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
