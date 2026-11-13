#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"
#include "../../../mainwindow.h"
#include "../../../actions/EFSMStatePropertiesUndoCommand.h"
#include "../../../actions/EFSMTransitionPropertiesUndoCommand.h"
#include "../../../actions/EFSMTransitionUndoCommand.h"
#include "../../../graphicals/GraphicalModelDataDefinition.h"
#include "../../../graphicals/EFSMTransitionGraphicsItem.h"
#include "../../../services/EFSMNetworkDiagramProjection.h"
#include "../../../services/EFSMNetworkDiagramCodec.h"
#include "EFSMNetworkEditorController.h"
#include "../../../../../../plugins/data/ModalModel/EFSMNetwork.h"
#include "../../../../../../plugins/components/ModalModel/FSMState.h"
#include "../../../../../../plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "kernel/util/Util.h"

#include <QAction>
#include <QMenu>
#include <QTextStream>

#include <any>
#include <utility>
#include <charconv>

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
        networkEditor.ownsEditableDataDefinition = [](ModelDataDefinition* owner, ModelDataDefinition* candidate) {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* state = dynamic_cast<FSMState*>(candidate);
            return network != nullptr && state != nullptr && network->getStates() != nullptr
                && network->getStates()->find(state) != network->getStates()->list()->end();
        };
        networkEditor.createDataDefinitionPropertyCommand = [](
            ModelDataDefinition* owner, ModelDataDefinition* candidate,
            const std::string& propertyName, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* state = dynamic_cast<FSMState*>(candidate);
            if (network == nullptr || state == nullptr) return nullptr;
            EFSMStatePropertiesUndoCommand::Properties updated;
            updated.name = state->getName();
            updated.label = state->getLabel();
            updated.entryActionExpression = state->getEntryActionExpression();
            updated.exitActionExpression = state->getExitActionExpression();
            updated.traceLevel = static_cast<std::uint32_t>(state->getTraceLevelSpecific());
            updated.reportStatistics = state->isReportStatistics();
            updated.finalNode = state->isFinalNode();
            updated.initialNode = state->isInitialNode();
            const bool boolValue = value == "true" || value == "1";
            if (propertyName == "Name") updated.name = value;
            else if (propertyName == "Label") updated.label = value;
            else if (propertyName == "Entry Action Expression") updated.entryActionExpression = value;
            else if (propertyName == "Exit Action Expression") updated.exitActionExpression = value;
            else if (propertyName == "Report Statistics") updated.reportStatistics = boolValue;
            else if (propertyName == "Final Node") updated.finalNode = boolValue;
            else if (propertyName == "Initial Node") updated.initialNode = boolValue;
            else if (propertyName == "Trace Level") {
                try { updated.traceLevel = static_cast<std::uint32_t>(std::stoul(value)); }
                catch (...) { return nullptr; }
            } else return nullptr;
            return new EFSMStatePropertiesUndoCommand(network, state, std::move(updated));
        };
        networkEditor.canStartRelationCreation = [](ModelDataDefinition* owner, QGraphicsItem* sourceItem) {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* graphicalState = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* state = graphicalState != nullptr
                ? dynamic_cast<FSMState*>(graphicalState->getDataDefinition()) : nullptr;
            return network != nullptr && state != nullptr && network->getStates() != nullptr
                && network->getStates()->find(state) != network->getStates()->list()->end();
        };
        networkEditor.createRelationCommand = [](
            ModelDataDefinition* owner, QGraphicsItem* sourceItem, QGraphicsItem* destinationItem) -> QUndoCommand* {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* sourceGraphic = dynamic_cast<GraphicalModelDataDefinition*>(sourceItem);
            auto* destinationGraphic = dynamic_cast<GraphicalModelDataDefinition*>(destinationItem);
            auto* source = sourceGraphic != nullptr
                ? dynamic_cast<FSMState*>(sourceGraphic->getDataDefinition()) : nullptr;
            auto* destination = destinationGraphic != nullptr
                ? dynamic_cast<FSMState*>(destinationGraphic->getDataDefinition()) : nullptr;
            if (network == nullptr || source == nullptr || destination == nullptr || network->getStates() == nullptr
                || network->getStates()->find(source) == network->getStates()->list()->end()
                || network->getStates()->find(destination) == network->getStates()->list()->end()) return nullptr;
            std::size_t ordinal = network->getTransitions()->size();
            std::string transitionName;
            bool nameInUse = false;
            do {
                transitionName = "T" + std::to_string(ordinal++);
                nameInUse = false;
                for (EFSMTransition* existing : *network->getTransitions()->list()) {
                    if (existing != nullptr && existing->getName() == transitionName) {
                        nameInUse = true;
                        break;
                    }
                }
            } while (nameInUse);
            auto transition = std::make_unique<EFSMTransition>(source, destination, std::move(transitionName));
            return new EFSMTransitionUndoCommand(network, std::move(transition));
        };
        networkEditor.elementProperties = [](ModelDataDefinition* owner, QGraphicsItem* item) {
            std::vector<GuiNetworkEditorContribution::ElementProperty> result;
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* graphic = dynamic_cast<EFSMTransitionGraphicsItem*>(item);
            EFSMTransition* transition = graphic != nullptr ? graphic->transition() : nullptr;
            if (network == nullptr || transition == nullptr || network->getTransitions() == nullptr
                || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return result;
            using Property = GuiNetworkEditorContribution::ElementProperty;
            const auto add = [&result](std::string name, std::string value, Property::Kind kind, bool editable) {
                result.push_back(Property{std::move(name), std::move(value), {}, kind, editable});
            };
            add("Name", transition->getName(), Property::Kind::String, true);
            add("Source State", transition->getSource() != nullptr ? transition->getSource()->getName() : "",
                Property::Kind::String, false);
            add("Destination State", transition->getDestination() != nullptr ? transition->getDestination()->getName() : "",
                Property::Kind::String, false);
            add("Guard Expression", transition->getGuardExpression(), Property::Kind::String, true);
            add("Output Expression", transition->getOutputExpression(), Property::Kind::String, true);
            add("Input Event", transition->getInputEvent(), Property::Kind::String, true);
            add("Priority", std::to_string(transition->getPriority()), Property::Kind::Integer, true);
            add("Trigger Event", transition->getTriggerEvent(), Property::Kind::String, true);
            add("Probability Expression", transition->getProbabilityExpression(), Property::Kind::String, true);
            return result;
        };
        networkEditor.createElementPropertyCommand = [](
            ModelDataDefinition* owner, QGraphicsItem* item,
            const std::string& propertyName, const std::string& value) -> QUndoCommand* {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* graphic = dynamic_cast<EFSMTransitionGraphicsItem*>(item);
            EFSMTransition* transition = graphic != nullptr ? graphic->transition() : nullptr;
            if (network == nullptr || transition == nullptr || network->getTransitions() == nullptr
                || network->getTransitions()->find(transition) == network->getTransitions()->list()->end()) return nullptr;
            EFSMTransitionPropertiesUndoCommand::Properties updated;
            updated.name = transition->getName();
            updated.guardExpression = transition->getGuardExpression();
            updated.outputExpression = transition->getOutputExpression();
            updated.inputEvent = transition->getInputEvent();
            updated.priority = transition->getPriority();
            updated.triggerEvent = transition->getTriggerEvent();
            updated.probabilityExpression = transition->getProbabilityExpression();
            if (propertyName == "Name") updated.name = value;
            else if (propertyName == "Guard Expression") updated.guardExpression = value;
            else if (propertyName == "Output Expression") updated.outputExpression = value;
            else if (propertyName == "Input Event") updated.inputEvent = value;
            else if (propertyName == "Trigger Event") updated.triggerEvent = value;
            else if (propertyName == "Probability Expression") updated.probabilityExpression = value;
            else if (propertyName == "Priority") {
                unsigned int parsed = 0;
                const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), parsed);
                if (error != std::errc{} || end != value.data() + value.size()) return nullptr;
                updated.priority = parsed;
            } else return nullptr;
            return new EFSMTransitionPropertiesUndoCommand(network, transition, std::move(updated));
        };
        networkEditor.acceptsDrop = [](ModelDataDefinition*, const std::string& pluginType, bool isComponent) {
            return !isComponent && pluginType == Util::TypeOf<FSMState>();
        };
        networkEditor.onDataDefinitionDropped = [](ModelDataDefinition* owner, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* state = dynamic_cast<FSMState*>(dropped);
            if (network != nullptr && state != nullptr) network->addState(state);
        };
        networkEditor.onDataDefinitionRemoved = [](ModelDataDefinition* owner, ModelDataDefinition* dropped) {
            auto* network = dynamic_cast<EFSMNetwork*>(owner);
            auto* state = dynamic_cast<FSMState*>(dropped);
            if (network != nullptr && state != nullptr) network->removeState(state);
        };
        registry->addNetworkEditor(std::move(networkEditor));
        addAction(registry, "modal.efsm.create-state", "Create EFSM State",
                  &EFSMNetworkEditorController::canCreateState,
                  &EFSMNetworkEditorController::createState);
        addAction(registry, "modal.efsm.create-transition", "Create EFSM Transition",
                  &EFSMNetworkEditorController::canCreateTransition,
                  &EFSMNetworkEditorController::createTransition);
        addAction(registry, "modal.efsm.set-initial-state", "Set Selected EFSM State as Initial",
                  &EFSMNetworkEditorController::canSetInitialState,
                  &EFSMNetworkEditorController::setInitialState);

        GuiContextMenuContribution transitionMenu;
        transitionMenu.id = "modal.efsm.create-transition.context";
        transitionMenu.isVisible = [](const GuiExtensionRuntimeContext& context, QGraphicsItem* item) {
            auto* graphicalState = dynamic_cast<GraphicalModelDataDefinition*>(item);
            if (graphicalState == nullptr
                || dynamic_cast<FSMState*>(graphicalState->getDataDefinition()) == nullptr) return false;
            MainWindow* mainWindow = host(context);
            if (mainWindow == nullptr) return false;
            EFSMNetworkEditorController controller(mainWindow);
            return controller.canCreateTransition();
        };
        transitionMenu.populate = [](QMenu* menu, const GuiExtensionRuntimeContext& context, QGraphicsItem*) {
            MainWindow* mainWindow = host(context);
            if (menu == nullptr || mainWindow == nullptr) return;
            EFSMNetworkEditorController controller(mainWindow);
            if (controller.canCreateTransition()) {
                QAction* action = menu->addAction(QObject::tr("Create EFSM Transition"));
                QObject::connect(action, &QAction::triggered, mainWindow, [mainWindow]() {
                    EFSMNetworkEditorController currentController(mainWindow);
                    currentController.createTransition();
                });
            }
        };
        registry->addContextMenu(std::move(transitionMenu));
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
