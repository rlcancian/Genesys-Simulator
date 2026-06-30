#include "ParserManager.h"
#include "model/Model.h"
#include "model/ModelDataManager.h"
#include "PluginManager.h"
#include "Plugin.h"
#include <dlfcn.h>
#include <iostream>
#include <filesystem>
#include <exception>

std::list<ParserChangesInformation*> ParserManager::aggregateChanges() {
	std::list<ParserChangesInformation*> result;
	if (_model == nullptr || _model->getDataManager() == nullptr) {
		return result;
	}
	std::list<std::string> classnames = _model->getDataManager()->getDataDefinitionClassnames();
	for (const std::string& classname : classnames) {
		List<ModelDataDefinition*>* list = _model->getDataManager()->getDataDefinitionList(classname);
		if (list == nullptr) {
			continue;
		}
		for (ModelDataDefinition* mdd : *list->list()) {
			if (mdd == nullptr) {
				continue;
			}
			ParserChangesInformation* changes = mdd->_getParserChangesInformation();
			if (changes != nullptr && changes->hasChanges()) {
				result.push_back(changes);
			} else {
				delete changes;
			}
		}
	}
	return result;
}

bool ParserManager::connectNewParser(ParserManager::NewParser newParser, std::string* errorMessage) {
	auto log = [errorMessage](const std::string& msg) {
		if (errorMessage != nullptr) {
			*errorMessage += msg + "\n";
		}
	};
	if (_model == nullptr) {
		log("Error: model is nullptr");
		return false;
	}
	if (newParser.compiledParserFilename.empty()) {
		log("Error: compiledParserFilename is empty");
		return false;
	}
	if (!std::filesystem::exists(newParser.compiledParserFilename)) {
		log("Error: compiled parser file not found: " + newParser.compiledParserFilename);
		return false;
	}

	log("Loading: " + newParser.compiledParserFilename);
	void* oldLibraryHandle = _dynamicLibraryHandle;
	void* newLibraryHandle = dlopen(newParser.compiledParserFilename.c_str(), RTLD_NOW | RTLD_DEEPBIND);
	if (newLibraryHandle == nullptr) {
		log("dlopen failed: " + std::string(dlerror()));
		return false;
	}
	log("dlopen OK (with DEEPBIND for self-contained parser symbols)");

	auto* createFn = reinterpret_cast<Parser_if* (*)(Model*, Sampler_if*)>(dlsym(newLibraryHandle, "genesys_createParser"));
	if (createFn == nullptr) {
		log("dlsym genesys_createParser failed: " + std::string(dlerror()));
		dlclose(newLibraryHandle);
		return false;
	}
	log("dlsym OK");

	Parser_if* oldParser = _model->getParser();
	Sampler_if* sampler = oldParser != nullptr ? oldParser->releaseSampler() : nullptr;
	Parser_if* newParserInstance = nullptr;
	try {
		newParserInstance = createFn(_model, sampler);
	} catch (const std::exception& e) {
		log("genesys_createParser threw: " + std::string(e.what()));
		if (oldParser != nullptr && sampler != nullptr) {
			oldParser->setSamplerOwned(sampler);
		}
		dlclose(newLibraryHandle);
		return false;
	} catch (...) {
		log("genesys_createParser threw an unknown exception");
		if (oldParser != nullptr && sampler != nullptr) {
			oldParser->setSamplerOwned(sampler);
		}
		dlclose(newLibraryHandle);
		return false;
	}
	if (newParserInstance == nullptr) {
		log("createFn returned nullptr");
		if (oldParser != nullptr && sampler != nullptr) {
			oldParser->setSamplerOwned(sampler);
		}
		dlclose(newLibraryHandle);
		return false;
	}
	log("Parser created OK");

	_dynamicLibraryHandle = newLibraryHandle;
	_model->setParser(newParserInstance);
	if (oldLibraryHandle != nullptr) {
		dlclose(oldLibraryHandle);
	}
	log("New parser connected successfully.");
	return true;
}
