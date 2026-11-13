#include "GuiExtensionPluginCatalog.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../systempreferences.h"
#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/model/ModelComponent.h"
#include "../graphicals/GraphicalModelComponent.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QProcessEnvironment>
#include <QMetaObject>
#include <QPointer>
#include <QThread>

namespace {
std::vector<const GuiExtensionPlugin*>& registryStorage() {
	static std::vector<const GuiExtensionPlugin*> plugins;
	return plugins;
}

using PresentationMap = std::unordered_map<std::string, GuiExtensionPresentationMetadata>;

std::string normalizeId(const std::string& input) {
	std::string normalized;
	normalized.reserve(input.size());
	for (unsigned char c : input) {
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
			continue;
		}
		normalized.push_back(static_cast<char>(std::tolower(c)));
	}
	return normalized;
}

std::string normalizeId(const QString& input) {
	return normalizeId(input.trimmed().toStdString());
}

QString defaultGuiExtensionsConfigPath() {
	QFileInfo preferencesPath(SystemPreferences::configFilePath());
	const QDir baseDir = preferencesPath.dir();
	return baseDir.absoluteFilePath("gui_extensions.json");
}

QJsonObject loadConfigObject() {
	const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	const QString envPath = env.value("GENESYS_GUI_EXTENSIONS_CONFIG").trimmed();
	const QString configPath = envPath.isEmpty() ? defaultGuiExtensionsConfigPath() : envPath;
	if (configPath.isEmpty()) {
		return {};
	}

	QFile file(configPath);
	if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return {};
	}

	QJsonParseError parseError;
	const QByteArray jsonBytes = file.readAll();
	const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
	if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
		return {};
	}
	return doc.object();
}

std::unordered_set<std::string> parseIdList(const QJsonValue& value) {
	std::unordered_set<std::string> ids;
	if (!value.isArray()) {
		return ids;
	}
	const QJsonArray array = value.toArray();
	for (const QJsonValue& item : array) {
		if (!item.isString()) {
			continue;
		}
		const std::string id = normalizeId(item.toString());
		if (!id.empty()) {
			ids.insert(id);
		}
	}
	return ids;
}

std::vector<std::string> parseOrderedIds(const QJsonValue& value) {
	std::vector<std::string> orderedIds;
	if (!value.isArray()) {
		return orderedIds;
	}
	const QJsonArray array = value.toArray();
	orderedIds.reserve(static_cast<size_t>(array.size()));
	for (const QJsonValue& item : array) {
		if (!item.isString()) {
			continue;
		}
		const std::string id = normalizeId(item.toString());
		if (!id.empty()) {
			orderedIds.push_back(id);
		}
	}
	return orderedIds;
}

PresentationMap parsePresentationMap(const QJsonValue& value) {
	PresentationMap metadataById;
	if (!value.isObject()) {
		return metadataById;
	}

	const QJsonObject presentation = value.toObject();
	metadataById.reserve(static_cast<size_t>(presentation.size()));
	for (auto it = presentation.constBegin(); it != presentation.constEnd(); ++it) {
		const std::string id = normalizeId(it.key());
		if (id.empty() || !it.value().isObject()) {
			continue;
		}
		const QJsonObject obj = it.value().toObject();
		GuiExtensionPresentationMetadata metadata;
		metadata.category = obj.value("category").toString().trimmed().toStdString();
		metadata.group = obj.value("group").toString().trimmed().toStdString();
		metadata.priority = obj.value("priority").toInt(0);
		metadataById[id] = std::move(metadata);
	}
	return metadataById;
}

GuiExtensionPresentationMetadata metadataFor(
	const PresentationMap& map,
	const GuiExtensionPlugin* plugin) {
	if (plugin == nullptr) {
		return {};
	}
	const std::string id = normalizeId(plugin->extensionId());
	auto found = map.find(id);
	return found != map.end() ? found->second : GuiExtensionPresentationMetadata{};
}

std::vector<const GuiExtensionPlugin*> applyOrder(
	const std::vector<const GuiExtensionPlugin*>& plugins,
	const std::vector<std::string>& orderedIds) {
	if (orderedIds.empty()) {
		return plugins;
	}

	std::unordered_map<std::string, const GuiExtensionPlugin*> byId;
	byId.reserve(plugins.size());
	for (const GuiExtensionPlugin* plugin : plugins) {
		if (plugin == nullptr) {
			continue;
		}
		byId.emplace(normalizeId(plugin->extensionId()), plugin);
	}

	std::unordered_set<std::string> consumed;
	consumed.reserve(orderedIds.size());
	std::vector<const GuiExtensionPlugin*> ordered;
	ordered.reserve(plugins.size());

	for (const std::string& id : orderedIds) {
		auto found = byId.find(id);
		if (found == byId.end()) {
			continue;
		}
		if (consumed.insert(id).second) {
			ordered.push_back(found->second);
		}
	}

	for (const GuiExtensionPlugin* plugin : plugins) {
		if (plugin == nullptr) {
			continue;
		}
		const std::string id = normalizeId(plugin->extensionId());
		if (consumed.find(id) == consumed.end()) {
			ordered.push_back(plugin);
		}
	}
	return ordered;
}

void sortUnpinnedByPresentation(
	std::vector<const GuiExtensionPlugin*>* plugins,
	const std::unordered_set<std::string>& pinnedIds,
	const PresentationMap& metadataById) {
	if (plugins == nullptr || plugins->empty()) {
		return;
	}

	auto firstSortable = std::find_if(plugins->begin(), plugins->end(), [&pinnedIds](const GuiExtensionPlugin* plugin) {
		return plugin != nullptr && pinnedIds.find(normalizeId(plugin->extensionId())) == pinnedIds.end();
	});
	if (firstSortable == plugins->end()) {
		return;
	}

	std::stable_sort(firstSortable, plugins->end(), [&metadataById](const GuiExtensionPlugin* left, const GuiExtensionPlugin* right) {
		if (left == nullptr || right == nullptr) {
			return left != nullptr;
		}

		const GuiExtensionPresentationMetadata leftMeta = metadataFor(metadataById, left);
		const GuiExtensionPresentationMetadata rightMeta = metadataFor(metadataById, right);
		if (leftMeta.priority != rightMeta.priority) {
			return leftMeta.priority > rightMeta.priority;
		}
		if (leftMeta.category != rightMeta.category) {
			return leftMeta.category < rightMeta.category;
		}
		if (leftMeta.group != rightMeta.group) {
			return leftMeta.group < rightMeta.group;
		}
		if (left->displayName() != right->displayName()) {
			return left->displayName() < right->displayName();
		}
		return left->extensionId() < right->extensionId();
	});
}
} // namespace

const std::vector<const GuiExtensionPlugin*>& GuiExtensionPluginCatalog::plugins() {
	return registryStorage();
}

std::vector<const GuiExtensionPlugin*> GuiExtensionPluginCatalog::resolvedPlugins() {
	const std::vector<const GuiExtensionPlugin*>& registered = registryStorage();
	QJsonObject config = loadConfigObject();
	if (config.isEmpty()) {
		return std::vector<const GuiExtensionPlugin*>(registered.begin(), registered.end());
	}

	const std::unordered_set<std::string> enabledIds = parseIdList(config.value("enabled"));
	const std::unordered_set<std::string> disabledIds = parseIdList(config.value("disabled"));
	std::vector<const GuiExtensionPlugin*> filtered;
	filtered.reserve(registered.size());
	for (const GuiExtensionPlugin* plugin : registered) {
		if (plugin == nullptr) {
			continue;
		}
		const std::string id = normalizeId(plugin->extensionId());
		if (!enabledIds.empty() && enabledIds.find(id) == enabledIds.end()) {
			continue;
		}
		if (disabledIds.find(id) != disabledIds.end()) {
			continue;
		}
		filtered.push_back(plugin);
	}

	const std::vector<std::string> orderedIds = parseOrderedIds(config.value("order"));
	std::vector<const GuiExtensionPlugin*> ordered = applyOrder(filtered, orderedIds);
	std::unordered_set<std::string> pinnedIds;
	pinnedIds.reserve(orderedIds.size());
	for (const std::string& id : orderedIds) {
		pinnedIds.insert(id);
	}
	const PresentationMap presentation = parsePresentationMap(config.value("presentation"));
	sortUnpinnedByPresentation(&ordered, pinnedIds, presentation);
	return ordered;
}

GuiExtensionPresentationMetadata GuiExtensionPluginCatalog::presentationMetadata(const std::string& extensionId) {
	QJsonObject config = loadConfigObject();
	if (config.isEmpty()) {
		return {};
	}
	const PresentationMap presentation = parsePresentationMap(config.value("presentation"));
	const std::string normalizedId = normalizeId(extensionId);
	auto found = presentation.find(normalizedId);
	return found != presentation.end() ? found->second : GuiExtensionPresentationMetadata{};
}

void GuiExtensionPluginCatalog::registerPlugin(const GuiExtensionPlugin* plugin) {
	if (plugin == nullptr) {
		return;
	}
	std::vector<const GuiExtensionPlugin*>& registry = registryStorage();
	if (std::find(registry.begin(), registry.end(), plugin) == registry.end()) {
		registry.push_back(plugin);
	}
}

void GuiExtensionPluginCatalog::synchronizeDiagramProjections(Simulator* simulator, ModelGraphicsScene* scene) {
	if (scene == nullptr) return;
	for (const GuiDiagramProjectionContribution& projection : diagramProjectionContributions()) {
		if (projection.synchronize) projection.synchronize(simulator, scene);
	}
}

void GuiExtensionPluginCatalog::synchronizeDiagramProjectionsOnSceneThread(Simulator* simulator,
                                                                            ModelGraphicsScene* scene) {
	if (scene == nullptr) return;
	if (QThread::currentThread() == scene->thread()) {
		synchronizeDiagramProjections(simulator, scene);
		return;
	}
	QPointer<ModelGraphicsScene> guardedScene(scene);
	QMetaObject::invokeMethod(scene, [guardedScene]() {
		if (guardedScene != nullptr) {
			synchronizeDiagramProjections(guardedScene->getSimulator(), guardedScene);
		}
	}, Qt::QueuedConnection);
}

std::vector<GuiDiagramProjectionContribution> GuiExtensionPluginCatalog::diagramProjectionContributions() {
	std::vector<GuiDiagramProjectionContribution> projections;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		const auto& registered = registry.diagramProjections();
		projections.insert(projections.end(), registered.begin(), registered.end());
	}
	return projections;
}

bool GuiExtensionPluginCatalog::hasNetworkEditor(ModelDataDefinition* owner) {
	if (owner == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors())
			if (editor.supports && editor.supports(owner)) return true;
	}
	return false;
}

bool GuiExtensionPluginCatalog::hasEditingContext(ModelDataDefinition* owner) {
	if (owner == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiEditingContextContribution& context : registry.editingContexts())
			if (context.supports && context.supports(owner)) return true;
	}
	return false;
}

bool GuiExtensionPluginCatalog::canDeleteNetworkElement(ModelDataDefinition* owner, QGraphicsItem* element) {
	if (owner == nullptr || element == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors())
			if (editor.supports && editor.supports(owner) && editor.canDeleteElement
				&& editor.canDeleteElement(owner, element)) return true;
	}
	return false;
}

bool GuiExtensionPluginCatalog::ownsEditableNetworkDataDefinition(
	ModelDataDefinition* owner, ModelDataDefinition* candidate) {
	if (owner == nullptr || candidate == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (editor.supports && editor.supports(owner) && editor.ownsEditableDataDefinition
				&& editor.ownsEditableDataDefinition(owner, candidate)) return true;
		}
	}
	return false;
}

QUndoCommand* GuiExtensionPluginCatalog::createNetworkDataDefinitionPropertyCommand(
	ModelDataDefinition* owner, ModelDataDefinition* candidate,
	const std::string& propertyName, const std::string& value) {
	if (owner == nullptr || candidate == nullptr) return nullptr;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (!editor.supports || !editor.supports(owner) || !editor.ownsEditableDataDefinition
				|| !editor.ownsEditableDataDefinition(owner, candidate)
				|| !editor.createDataDefinitionPropertyCommand) continue;
			if (QUndoCommand* command = editor.createDataDefinitionPropertyCommand(
					owner, candidate, propertyName, value)) return command;
		}
	}
	return nullptr;
}

bool GuiExtensionPluginCatalog::canStartNetworkRelationCreation(
	ModelDataDefinition* owner, QGraphicsItem* source) {
	if (owner == nullptr || source == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (editor.supports && editor.supports(owner) && editor.canStartRelationCreation
				&& editor.canStartRelationCreation(owner, source)) return true;
		}
	}
	return false;
}

QUndoCommand* GuiExtensionPluginCatalog::createNetworkRelationCommand(
	ModelDataDefinition* owner, QGraphicsItem* source, QGraphicsItem* destination) {
	if (owner == nullptr || source == nullptr || destination == nullptr) return nullptr;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (!editor.supports || !editor.supports(owner) || !editor.createRelationCommand) continue;
			if (QUndoCommand* command = editor.createRelationCommand(owner, source, destination)) return command;
		}
	}
	return nullptr;
}

std::vector<GuiNetworkEditorContribution::ElementProperty>
GuiExtensionPluginCatalog::networkElementProperties(ModelDataDefinition* owner, QGraphicsItem* element) {
	if (owner == nullptr || element == nullptr) return {};
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (!editor.supports || !editor.supports(owner) || !editor.elementProperties) continue;
			auto properties = editor.elementProperties(owner, element);
			if (!properties.empty()) return properties;
		}
	}
	return {};
}

QUndoCommand* GuiExtensionPluginCatalog::createNetworkElementPropertyCommand(
	ModelDataDefinition* owner, QGraphicsItem* element,
	const std::string& propertyName, const std::string& value) {
	if (owner == nullptr || element == nullptr) return nullptr;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (!editor.supports || !editor.supports(owner) || !editor.elementProperties
				|| !editor.createElementPropertyCommand) continue;
			if (editor.elementProperties(owner, element).empty()) continue;
			if (QUndoCommand* command = editor.createElementPropertyCommand(
					owner, element, propertyName, value)) return command;
		}
	}
	return nullptr;
}

bool GuiExtensionPluginCatalog::acceptsNetworkEditorDrop(
	ModelDataDefinition* owner, const std::string& pluginType, bool isComponent) {
	if (owner == nullptr || pluginType.empty() || isComponent) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (editor.supports && editor.supports(owner) && editor.acceptsDrop
				&& editor.acceptsDrop(owner, pluginType, isComponent)) return true;
		}
	}
	return false;
}

void GuiExtensionPluginCatalog::notifyNetworkEditorDataDefinitionDrop(
	ModelDataDefinition* owner, ModelDataDefinition* dropped) {
	if (owner == nullptr || dropped == nullptr) return;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (editor.supports && editor.supports(owner) && editor.acceptsDrop
				&& editor.acceptsDrop(owner, dropped->getClassname(), false)
				&& editor.onDataDefinitionDropped) {
				editor.onDataDefinitionDropped(owner, dropped);
				return;
			}
		}
	}
}

void GuiExtensionPluginCatalog::notifyNetworkEditorDataDefinitionRemoved(
		ModelDataDefinition* owner, ModelDataDefinition* dropped) {
	if (owner == nullptr || dropped == nullptr) return;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiNetworkEditorContribution& editor : registry.networkEditors()) {
			if (editor.supports && editor.supports(owner) && editor.onDataDefinitionRemoved) {
				editor.onDataDefinitionRemoved(owner, dropped);
				return;
			}
		}
	}
}

void GuiExtensionPluginCatalog::restoreDiagramProjections(
	Simulator* simulator, ModelDataDefinition* owner, ModelGraphicsScene* scene,
	const std::function<std::any(const std::string&, unsigned long)>& layoutProvider) {
	if (owner == nullptr || scene == nullptr || !layoutProvider) return;
	for (const GuiDiagramProjectionContribution& diagram : diagramProjectionContributions()) {
		if (!diagram.supports || !diagram.supports(owner) || !diagram.restoreLayout) continue;
		diagram.restoreLayout(simulator, owner, scene,
		                     layoutProvider(diagram.id, static_cast<unsigned long>(owner->getId())));
	}
}

void GuiExtensionPluginCatalog::configureComponentVisualization(GraphicalModelComponent* component) {
	if (component == nullptr || component->getComponent() == nullptr) return;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiComponentVisualizationContribution& contribution : registry.componentVisualizations()) {
			if (contribution.supports && contribution.supports(component->getComponent()) && contribution.configure) {
				contribution.configure(component);
				return;
			}
		}
	}
}

bool GuiExtensionPluginCatalog::animateComponentArrival(ModelGraphicsScene* scene, GraphicalModelComponent* component, bool visible) {
	if (scene == nullptr || component == nullptr || component->getComponent() == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiComponentVisualizationContribution& contribution : registry.componentVisualizations()) {
			if (contribution.supports && contribution.supports(component->getComponent()) && contribution.animateArrival) {
				contribution.animateArrival(scene, component, visible);
				return true;
			}
		}
	}
	return false;
}

bool GuiExtensionPluginCatalog::animateComponentDeparture(ModelGraphicsScene* scene, GraphicalModelComponent* component) {
	if (scene == nullptr || component == nullptr || component->getComponent() == nullptr) return false;
	for (const GuiExtensionPlugin* plugin : resolvedPlugins()) {
		if (plugin == nullptr) continue;
		GuiExtensionRegistry registry;
		plugin->registerContributions(&registry);
		for (const GuiComponentVisualizationContribution& contribution : registry.componentVisualizations()) {
			if (contribution.supports && contribution.supports(component->getComponent()) && contribution.animateDeparture) {
				contribution.animateDeparture(scene, component);
				return true;
			}
		}
	}
	return false;
}

GuiExtensionPluginAutoRegistrar::GuiExtensionPluginAutoRegistrar(const GuiExtensionPlugin* plugin) {
	GuiExtensionPluginCatalog::registerPlugin(plugin);
}
