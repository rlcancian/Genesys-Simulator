#include "ModelPersistencePartialLoadImpl1.h"

#include <exception>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ModelSerializer.h"
#include "Simulator.h"
#include "../util/Util.h"

#include "GenSerializer.h"
#include "JsonSerializer.h"
#include "Plugin.h"
#include "XmlSerializer.h"

ModelPersistencePartialLoadImpl1::ModelPersistencePartialLoadImpl1(Model* model) :
ModelPersistenceDefaultImpl2(model),
_model(model) {
}

bool ModelPersistencePartialLoadImpl1::load(std::string filename) {
	_model->getTracer()->trace(TraceManager::Level::L7_internal, "Loading file \"" + filename + "\"");
	Util::IncIndent();

	std::unique_ptr<ModelSerializer> parser;
	{
		auto extension = filename.substr(filename.find_last_of('.') + 1);
		if (extension == "xml") {
			_model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as XML");
			parser = std::make_unique<XmlSerializer>(_model);
		} else if (extension == "json") {
			_model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as JSON");
			parser = std::make_unique<JsonSerializer>(_model);
		} else if (extension == "cpp") {
			_model->getTracer()->trace(TraceManager::Level::L4_warning, "Cannot parse C++");
			Util::DecIndent();
			return false;
		} else { // default
			_model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as GenESyS simulation language");
			parser = std::make_unique<GenSerializer>(_model);
		}
	}

	try {
		std::ifstream file(filename);
		if (!parser->load(file)) {
			throw std::runtime_error("serializer parsing failed");
		}
	} catch (const std::exception& e) {
		_model->getTracer()->traceError("Error loading file \"" + filename + "\": " + std::string(e.what()));
		Util::DecIndent();
		return false;
	}

	struct DeferredLoadItem {
		std::string name;
		std::string type;
		Util::identification id{0};
		Plugin* plugin{nullptr};
		std::unique_ptr<PersistenceRecord> fields;
	};

	const auto itemLabel = [](const DeferredLoadItem& item) {
		std::ostringstream out;
		out << "name=\"" << item.name << "\", type=\"" << item.type << "\", id=" << item.id;
		return out.str();
	};

	unsigned int recoverableErrors = 0;
	unsigned int loadedDataDefinitions = 0;
	unsigned int loadedComponents = 0;
	unsigned int loadedCoreEntries = 0;
	unsigned int expectedPayloadEntries = 0;
	std::vector<std::unique_ptr<PersistenceRecord>> componentFields;
	std::vector<DeferredLoadItem> deferredDataDefinitions;
	std::vector<DeferredLoadItem> deferredComponents;

	Util::IncIndent();
	parser->for_each([&](const std::string& name) {
		auto fields = std::unique_ptr<PersistenceRecord>(parser->newPersistenceRecord());
		if (!parser->get(name, fields.get())) {
			_model->getTracer()->traceError("Error loading item \"" + name + "\": could not read serialized fields");
			++recoverableErrors;
			return 0;
		}

		DeferredLoadItem item;
		item.name = name;
		item.type = fields->loadField("typename", "__UNTYPED__");
		item.id = fields->loadField("id", static_cast<unsigned int>(0));
		item.fields = std::move(fields);

		_model->getTracer()->trace(TraceManager::Level::L7_internal, "Read item (" + itemLabel(item) + ")");

		try {
			if (item.type == "SimulatorInfo" || item.type == "Simulator") {
				unsigned int savedVersion = item.fields->loadField("versionNumber", 0);
				unsigned int currentVersion = _model->getParentSimulator()->getVersionNumber();
				if (savedVersion != currentVersion) {
					_model->getTracer()->trace(
							"WARNING: The version of the saved model differs from the simulator. Loading may not be possible",
							TraceManager::Level::L3_errorRecover);
				}
				++loadedCoreEntries;
				return 0;
			}

			if (item.type == "ModelInfo") {
				_model->getInfos()->loadInstance(item.fields.get());
				setHasChanged(true);
				++loadedCoreEntries;
				return 0;
			}

			if (item.type == "ModelSimulation") {
				_model->getSimulation()->loadInstance(item.fields.get());
				setHasChanged(true);
				++loadedCoreEntries;
				return 0;
			}

			++expectedPayloadEntries;
			item.plugin = _model->getParentSimulator()->getPluginManager()->find(item.type);
			if (item.plugin == nullptr) {
				_model->getTracer()->traceError(
						"Error loading item (" + itemLabel(item) + "): unknown typename \"" + item.type + "\"");
				++recoverableErrors;
				return 0;
			}

			if (item.plugin->getPluginInfo()->isComponent()) {
				deferredComponents.push_back(std::move(item));
			} else {
				deferredDataDefinitions.push_back(std::move(item));
			}
		} catch (const std::exception& e) {
			_model->getTracer()->traceError(
					"Error loading item (" + itemLabel(item) + "): " + std::string(e.what()));
			++recoverableErrors;
		} catch (...) {
			_model->getTracer()->traceError(
					"Error loading item (" + itemLabel(item) + "): unknown exception");
			++recoverableErrors;
		}
		return 0;
	});
	Util::DecIndent();

	const auto loadDeferredItemsWithRetry = [&](std::vector<DeferredLoadItem>& pending,
												std::vector<std::unique_ptr<PersistenceRecord>>* loadedComponentFields,
												unsigned int* loadedCount,
												const std::string& phaseName) {
		if (pending.empty()) {
			return;
		}

		_model->getTracer()->trace("Loading " + phaseName + " with retry");
		Util::IncIndent();
		const std::size_t maxPasses = pending.size();
		for (std::size_t pass = 0; pass < maxPasses && !pending.empty(); ++pass) {
			std::vector<DeferredLoadItem> nextPending;
			bool progressed = false;
			for (auto& item : pending) {
				try {
					const bool ok = item.plugin->loadAndInsertNew(_model, item.fields.get());
					if (ok) {
						setHasChanged(true);
						++(*loadedCount);
						progressed = true;
						if (loadedComponentFields != nullptr && item.plugin->getPluginInfo()->isComponent()) {
							loadedComponentFields->push_back(std::move(item.fields));
						}
					} else {
						_model->getTracer()->trace(
								"Deferred item still unresolved (" + itemLabel(item) + ")",
								TraceManager::Level::L7_internal);
						nextPending.push_back(std::move(item));
					}
				} catch (const std::exception& e) {
					_model->getTracer()->trace(
							"Deferred item exception (" + itemLabel(item) + "): " + std::string(e.what()),
							TraceManager::Level::L7_internal);
					nextPending.push_back(std::move(item));
				} catch (...) {
					_model->getTracer()->trace(
							"Deferred item exception (" + itemLabel(item) + "): unknown",
							TraceManager::Level::L7_internal);
					nextPending.push_back(std::move(item));
				}
			}
			pending = std::move(nextPending);
			if (!progressed) {
				break;
			}
		}
		for (const auto& item : pending) {
			_model->getTracer()->traceError(
					"Error loading item (" + itemLabel(item)
					+ "): unresolved after retry (likely missing dependency or invalid reference)");
			++recoverableErrors;
		}
		Util::DecIndent();
	};

	loadDeferredItemsWithRetry(deferredDataDefinitions, nullptr, &loadedDataDefinitions, "data definitions");
	loadDeferredItemsWithRetry(deferredComponents, &componentFields, &loadedComponents, "components");

	ComponentManager* cm = _model->getComponentManager();
	_model->getTracer()->trace("Connecting loaded components");
	Util::IncIndent();
	for (auto& fields : componentFields) {
		try {
			Util::identification id = fields->loadField("id", -1);
			ModelComponent* component = cm->find(id);
			if (component == nullptr) {
				_model->getTracer()->traceError(
						"Error connecting component id=" + std::to_string(id) + ": component not registered");
				++recoverableErrors;
				continue;
			}

			unsigned short nextSize = fields->loadField("nexts", 1);
			for (unsigned short i = 0; i < nextSize; i++) {
				Util::identification nextId = fields->loadField("nextId" + Util::StrIndex(i), 0);
				if (nextSize == 1) {
					nextId = fields->loadField("nextId", static_cast<unsigned int>(nextId));
				}

				ModelComponent* nextComponent = cm->find(nextId);
				if (nextComponent == nullptr) {
					_model->getTracer()->traceError(
							"Error connecting component id=" + std::to_string(id)
							+ ": target id=" + std::to_string(nextId) + " not registered");
					++recoverableErrors;
					continue;
				}

				unsigned short nextPort = fields->loadField("nextinputPortNumber" + Util::StrIndex(i), 0);
				component->getConnectionManager()->insert(nextComponent, nextPort);
				_model->getTracer()->trace(component->getName() + "<" + std::to_string(i) + ">"
										   + " --> " + nextComponent->getName() + "<" + std::to_string(nextPort) + ">");
			}
		} catch (const std::exception& e) {
			_model->getTracer()->traceError(
					"Error connecting loaded component: " + std::string(e.what()));
			++recoverableErrors;
		} catch (...) {
			_model->getTracer()->traceError("Error connecting loaded component: unknown exception");
			++recoverableErrors;
		}
	}
	Util::DecIndent();

	if (recoverableErrors > 0) {
		_model->getTracer()->trace(
				"Model loaded with recoverable errors (" + std::to_string(recoverableErrors)
				+ " item(s) were skipped).", TraceManager::Level::L3_errorRecover);
	}

	const bool loadedAnyPayload = (loadedDataDefinitions + loadedComponents) > 0;
	if (expectedPayloadEntries > 0 && !loadedAnyPayload) {
		_model->getTracer()->traceError(
				"Model load failed: no payload item could be loaded ("
				+ std::to_string(expectedPayloadEntries) + " item(s) found in file).");
		Util::DecIndent();
		return false;
	}

	_model->getTracer()->trace(
			"Loaded summary: core=" + std::to_string(loadedCoreEntries)
			+ ", dataDefinitions=" + std::to_string(loadedDataDefinitions)
			+ ", components=" + std::to_string(loadedComponents)
			+ ", skipped=" + std::to_string(recoverableErrors),
			TraceManager::Level::L3_errorRecover);

	Util::DecIndent();
	return true;
}
