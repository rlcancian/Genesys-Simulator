#include "ParserManager.h"
#include "model/Model.h"
#include "model/ModelDataManager.h"
#include "PluginManager.h"
#include "Plugin.h"
#include <dlfcn.h>
#include <iostream>
#include <filesystem>

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
			if (changes != nullptr) {
				result.push_back(changes);
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

	if (_dynamicLibraryHandle != nullptr) {
		dlclose(_dynamicLibraryHandle);
		_dynamicLibraryHandle = nullptr;
	}

	log("Loading: " + newParser.compiledParserFilename);
	_dynamicLibraryHandle = dlopen(newParser.compiledParserFilename.c_str(), RTLD_NOW | RTLD_DEEPBIND);
	if (_dynamicLibraryHandle == nullptr) {
		log("dlopen failed: " + std::string(dlerror()));
		return false;
	}
	log("dlopen OK (with DEEPBIND for self-contained parser symbols)");

	auto* createFn = reinterpret_cast<Parser_if* (*)(Model*, Sampler_if*)>(dlsym(_dynamicLibraryHandle, "genesys_createParser"));
	if (createFn == nullptr) {
		log("dlsym genesys_createParser failed: " + std::string(dlerror()));
		dlclose(_dynamicLibraryHandle);
		_dynamicLibraryHandle = nullptr;
		return false;
	}
	log("dlsym OK");

	Sampler_if* sampler = (_model->getParser() != nullptr) ? _model->getParser()->releaseSampler() : nullptr;
	Parser_if* newParserInstance = createFn(_model, nullptr);
	if (newParserInstance == nullptr) {
		log("createFn returned nullptr");
		dlclose(_dynamicLibraryHandle);
		_dynamicLibraryHandle = nullptr;
		return false;
	}
	log("Parser created OK");
	if (sampler != nullptr) {
		newParserInstance->setSampler(sampler);
	}

	_model->setParser(newParserInstance);
	log("New parser connected successfully.");
	return true;
}
