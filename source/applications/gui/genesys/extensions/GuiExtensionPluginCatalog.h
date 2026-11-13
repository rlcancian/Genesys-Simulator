#ifndef GUIEXTENSIONPLUGINCATALOG_H
#define GUIEXTENSIONPLUGINCATALOG_H

#include "GuiExtensionContracts.h"

#include <string>
#include <vector>
#include <functional>
#include <any>

struct GuiExtensionPresentationMetadata {
	std::string category;
	std::string group;
	int priority = 0;
};

class GuiExtensionPluginCatalog {
public:
	static const std::vector<const GuiExtensionPlugin*>& plugins();
	static std::vector<const GuiExtensionPlugin*> resolvedPlugins();
	static GuiExtensionPresentationMetadata presentationMetadata(const std::string& extensionId);
	static void registerPlugin(const GuiExtensionPlugin* plugin);
	static std::vector<GuiDiagramProjectionContribution> diagramProjectionContributions();
	static bool ownsEditableNetworkDataDefinition(ModelDataDefinition* owner, ModelDataDefinition* candidate);
	static bool hasNetworkEditor(ModelDataDefinition* owner);
	static bool hasEditingContext(ModelDataDefinition* owner);
	static bool canDeleteNetworkElement(ModelDataDefinition* owner, QGraphicsItem* element);
	static QUndoCommand* createNetworkDataDefinitionPropertyCommand(
		ModelDataDefinition* owner, ModelDataDefinition* candidate,
		const std::string& propertyName, const std::string& value);
	static bool canStartNetworkRelationCreation(ModelDataDefinition* owner, QGraphicsItem* source);
	static QUndoCommand* createNetworkRelationCommand(
		ModelDataDefinition* owner, QGraphicsItem* source, QGraphicsItem* destination);
	static std::vector<GuiNetworkEditorContribution::ElementProperty> networkElementProperties(
		ModelDataDefinition* owner, QGraphicsItem* element);
	static QUndoCommand* createNetworkElementPropertyCommand(
		ModelDataDefinition* owner, QGraphicsItem* element,
		const std::string& propertyName, const std::string& value);
	static bool acceptsNetworkEditorDrop(ModelDataDefinition* owner, const std::string& pluginType, bool isComponent);
	static void notifyNetworkEditorDataDefinitionDrop(ModelDataDefinition* owner, ModelDataDefinition* dropped);
	static void notifyNetworkEditorDataDefinitionRemoved(ModelDataDefinition* owner, ModelDataDefinition* dropped);
	static void synchronizeDiagramProjections(Simulator* simulator, ModelGraphicsScene* scene);
	static void synchronizeDiagramProjectionsOnSceneThread(Simulator* simulator, ModelGraphicsScene* scene);
	static void restoreDiagramProjections(
		Simulator* simulator, ModelDataDefinition* owner, ModelGraphicsScene* scene,
		const std::function<std::any(const std::string&, unsigned long)>& layoutProvider);
};

class GuiExtensionPluginAutoRegistrar {
public:
	explicit GuiExtensionPluginAutoRegistrar(const GuiExtensionPlugin* plugin);
};

#define REGISTER_GUI_EXTENSION_PLUGIN(PluginType)               \
	namespace {                                                   \
		static const PluginType _instance_##PluginType;          \
		static const GuiExtensionPluginAutoRegistrar             \
		    _registrar_##PluginType(&_instance_##PluginType);    \
	}

#endif /* GUIEXTENSIONPLUGINCATALOG_H */
