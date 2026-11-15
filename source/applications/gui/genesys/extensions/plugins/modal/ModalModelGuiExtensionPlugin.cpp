#include "../../GuiExtensionContracts.h"
#include "../../GuiExtensionPluginCatalog.h"

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/Submodel.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"

#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/ModelGraphicsScene.h"
#include "propertyeditor/ObjectPropertyBrowser.h"

#include <QUndoCommand>
#include <QMessageBox>
#include <QPointer>

#include <cctype>
#include <string>
#include <utility>

namespace {
class ModalModelBindingUndoCommand final : public QUndoCommand {
public:
	ModalModelBindingUndoCommand(ModalModelDefault* adapter, DefaultNetwork* network, bool output,
		unsigned int port, std::string before, std::string after, ObjectPropertyBrowser* browser)
		: _adapter(adapter), _network(network), _output(output), _port(port),
		  _before(std::move(before)), _after(std::move(after)), _browser(browser) {
		setText(QObject::tr("Edit Network binding"));
	}
	void undo() override { apply(_before); }
	void redo() override { apply(_after); }
private:
	void apply(const std::string& value) {
		if (_adapter == nullptr || _network == nullptr || _adapter->getNetwork() != _network) {
			setObsolete(true);
			return;
		}
		if (_output) _adapter->setOutputBinding(_port, value);
		else _adapter->setInputBinding(_port, value);
		if (_browser != nullptr) _browser->refreshAfterUndo();
	}
	ModalModelDefault* _adapter;
	DefaultNetwork* _network;
	bool _output;
	unsigned int _port;
	std::string _before;
	std::string _after;
	QPointer<ObjectPropertyBrowser> _browser;
};

class ModalModelNetworkReferenceUndoCommand final : public QUndoCommand {
public:
	ModalModelNetworkReferenceUndoCommand(ModalModelDefault* adapter, DefaultNetwork* before,
		DefaultNetwork* after, ModelGraphicsScene* scene, ObjectPropertyBrowser* browser)
		: _adapter(adapter), _before(before), _after(after), _scene(scene), _browser(browser) {
		setText(QObject::tr("Change ModalModel Network"));
	}
	void undo() override { apply(_before); }
	void redo() override { apply(_after); }
private:
	void apply(DefaultNetwork* network) {
		if (_adapter == nullptr) { setObsolete(true); return; }
		try { _adapter->setNetwork(network); }
		catch (const std::exception&) { setObsolete(true); return; }
		if (_scene != nullptr) {
			auto* graphical = _scene->findGraphicalModelComponent(_adapter->getId());
			if (graphical == nullptr || graphical->getComponent() != _adapter) {
				graphical = nullptr;
				for (QGraphicsItem* item : _scene->items()) {
					auto* candidate = dynamic_cast<GraphicalModelComponent*>(item);
					if (candidate != nullptr && candidate->getComponent() == _adapter) { graphical = candidate; break; }
				}
			}
			if (graphical != nullptr) graphical->synchronizeDynamicPorts();
			_scene->update();
		}
		if (_browser != nullptr) _browser->refreshAfterUndo();
	}
	ModalModelDefault* _adapter;
	DefaultNetwork* _before;
	DefaultNetwork* _after;
	QPointer<ModelGraphicsScene> _scene;
	QPointer<ObjectPropertyBrowser> _browser;
};

DefaultNetwork* resolveNetworkChoice(ModalModelDefault* adapter, const std::string& choice) {
	if (adapter == nullptr || adapter->getParentModel() == nullptr || adapter->getParentModel()->getDataManager() == nullptr)
		return nullptr;
	const std::size_t delimiter = choice.find("::");
	const std::size_t identityStart = choice.rfind(" [#");
	if (delimiter == std::string::npos || identityStart == std::string::npos || identityStart <= delimiter + 2
		|| choice.empty() || choice.back() != ']') return nullptr;
	try {
		const auto id = static_cast<Util::identification>(std::stoul(choice.substr(identityStart + 3,
			choice.size() - identityStart - 4)));
		auto* network = dynamic_cast<DefaultNetwork*>(adapter->getParentModel()->getDataManager()->getDataDefinition(
			choice.substr(0, delimiter), id));
		const std::string expectedName = choice.substr(delimiter + 2, identityStart - delimiter - 2);
		return network != nullptr && network->getName() == expectedName ? network : nullptr;
	} catch (...) { return nullptr; }
}

bool hasDifferentPortSchema(const DefaultNetwork* first, const DefaultNetwork* second) {
	if (first == nullptr || second == nullptr) return first != second;
	if (first->getNumInputPorts() != second->getNumInputPorts()
		|| first->getNumOutputPorts() != second->getNumOutputPorts()) return true;
	for (unsigned int i = 0; i < first->getNumInputPorts(); ++i)
		if (first->getInputPortName(i) != second->getInputPortName(i)) return true;
	for (unsigned int i = 0; i < first->getNumOutputPorts(); ++i)
		if (first->getOutputPortName(i) != second->getOutputPortName(i)) return true;
	return false;
}

class ModalModelGuiExtensionPlugin final : public GuiExtensionPlugin {
public:
	std::string extensionId() const override { return "gui.extensions.modal.context"; }
	std::string displayName() const override { return "ModalModel editing contexts"; }
	std::string version() const override { return "1.0.0"; }
	std::vector<std::string> requiredModelPlugins() const override { return {}; }

	void registerContributions(GuiExtensionRegistry* registry) const override {
		if (registry == nullptr) return;
		GuiEditingContextContribution context;
		context.id = "gui.modal.network-editing-context";
		context.supports = [](ModelDataDefinition* owner) {
			return dynamic_cast<ModalModelDefault*>(owner) != nullptr
				|| dynamic_cast<DefaultNetwork*>(owner) != nullptr
				|| dynamic_cast<Submodel*>(owner) != nullptr;
		};
		context.resolveOwner = [](ModelDataDefinition* owner) -> ModelDataDefinition* {
			if (auto* adapter = dynamic_cast<ModalModelDefault*>(owner)) return adapter->getNetwork();
			return owner;
		};
		context.isNetwork = [](ModelDataDefinition* owner) {
			if (dynamic_cast<ModalModelDefault*>(owner) != nullptr) return true;
			return dynamic_cast<DefaultNetwork*>(owner) != nullptr;
		};
		context.canOpen = [](Model* model, ModelDataDefinition* owner) {
			if (model == nullptr) return false;
			if (dynamic_cast<Submodel*>(owner) != nullptr) {
				if (model->getComponentManager() == nullptr) return false;
				for (ModelComponent* component : *model->getComponentManager()->getAllComponents())
					if (component == owner) return true;
				return false;
			}
			if (model->getDataManager() == nullptr) return false;
			if (auto* adapter = dynamic_cast<ModalModelDefault*>(owner)) owner = adapter->getNetwork();
			auto* network = dynamic_cast<DefaultNetwork*>(owner);
			return network != nullptr
				&& model->getDataManager()->getDataDefinition(network->getClassname(), network->getId()) == network;
		};
		registry->addEditingContext(std::move(context));

		GuiModelReferenceContribution reference;
		reference.id = "gui.modal.network-reference-editing";
		reference.supports = [](ModelDataDefinition* owner) { return dynamic_cast<ModalModelDefault*>(owner) != nullptr; };
		reference.currentReference = [](ModelDataDefinition* owner) -> ModelDataDefinition* {
			auto* adapter = dynamic_cast<ModalModelDefault*>(owner);
			return adapter != nullptr ? adapter->getNetwork() : nullptr;
		};
		reference.resolveChoice = [](ModelDataDefinition* owner, const std::string& choice) -> ModelDataDefinition* {
			return resolveNetworkChoice(dynamic_cast<ModalModelDefault*>(owner), choice);
		};
		reference.formatChoice = [](ModelDataDefinition* owner, const std::string& property,
			ModelDataDefinition* target) {
			if (dynamic_cast<ModalModelDefault*>(owner) == nullptr || property != "Network" || target == nullptr) return std::string{};
			return target->getClassname() + "::" + target->getName() + " [#" + std::to_string(target->getId()) + "]";
		};
		reference.confirmChange = [](ModelDataDefinition* owner, ModelDataDefinition* before,
			ModelDataDefinition* after, QWidget* parent) {
			auto* adapter = dynamic_cast<ModalModelDefault*>(owner);
			auto* previous = dynamic_cast<DefaultNetwork*>(before);
			auto* selected = dynamic_cast<DefaultNetwork*>(after);
			if (adapter == nullptr || previous == selected || selected == nullptr || !hasDifferentPortSchema(previous, selected))
				return true;
			return QMessageBox::question(parent, QObject::tr("Replace Network with a different port schema?"),
				QObject::tr("The selected Network has different input or output ports. Bindings will be reconciled by port name; unmatched default bindings may be initialized, and changes that would lose custom bindings or alter connected-port meaning will be rejected. Continue?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
		};
		reference.referenceChanged = [](ModelDataDefinition* owner, ModelDataDefinition* before,
			ModelDataDefinition* after, ModelGraphicsScene* scene, ObjectPropertyBrowser* browser) {
			auto* adapter = dynamic_cast<ModalModelDefault*>(owner);
			if (adapter == nullptr || before == after || scene == nullptr || scene->getUndoStack() == nullptr) return;
			scene->getUndoStack()->push(new ModalModelNetworkReferenceUndoCommand(adapter,
				dynamic_cast<DefaultNetwork*>(before), dynamic_cast<DefaultNetwork*>(after), scene, browser));
		};
		reference.propertyChanged = [](ModelDataDefinition* owner, const std::string& property,
			const std::string& before, const std::string& after, ModelGraphicsScene* scene,
			ObjectPropertyBrowser* browser) {
			auto* adapter = dynamic_cast<ModalModelDefault*>(owner);
			auto* network = adapter != nullptr ? adapter->getNetwork() : nullptr;
			if (network == nullptr || scene == nullptr || scene->getUndoStack() == nullptr) return;
			const bool output = property.rfind("Output Binding ", 0) == 0;
			const std::string prefix = output ? "Output Binding " : "Input Binding ";
			if (!output && property.rfind(prefix, 0) != 0) return;
			try {
				const std::size_t end = property.find_first_not_of("0123456789", prefix.size());
				const auto port = static_cast<unsigned int>(std::stoul(property.substr(prefix.size(), end - prefix.size())));
				scene->getUndoStack()->push(new ModalModelBindingUndoCommand(adapter, network, output, port, before, after, browser));
			} catch (const std::exception&) { }
		};
		registry->addModelReference(std::move(reference));
	}
};

REGISTER_GUI_EXTENSION_PLUGIN(ModalModelGuiExtensionPlugin);
} // namespace
