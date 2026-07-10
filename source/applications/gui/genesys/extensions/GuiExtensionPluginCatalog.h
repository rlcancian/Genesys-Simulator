#ifndef GUIEXTENSIONPLUGINCATALOG_H
#define GUIEXTENSIONPLUGINCATALOG_H

#include "GuiExtensionContracts.h"

#include <string>
#include <vector>

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
