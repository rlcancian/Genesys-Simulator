/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   CppForG.cpp
 * Author: rlcancian
 * 
 * Created on 11 de janeiro de 2022, 22:37
 */


#include "plugins/components/ExternalIntegration/CppForG.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#include <fstream>
#include <dlfcn.h>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <unistd.h>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CppForG::GetPluginInformation;
}
#endif

ModelDataDefinition* CppForG::NewInstance(Model* model, std::string name) {
	return new CppForG(model, name);
}

CppForG::CppForG(Model* model, std::string name)
    : CppForG(model, name, true) { }

CppForG::~CppForG() {
    dispatchEvent_SharedLibHandler = nullptr;
    initBetweenReplications_SharedLibHandler = nullptr;
    if (_cppCompiler != nullptr) _cppCompiler->unloadLibrary();
    if (!_workDirectory.empty()) {
        std::error_code error;
        std::filesystem::remove_all(_workDirectory, error);
    }
}

CppForG::CppForG(Model* model, std::string name, bool createOwnedCompiler)
	: ModelComponent(model, Util::TypeOf<CppForG>(), name) {
	if (createOwnedCompiler) {
		_createNonEditableDataDefinitions();
	}

	SimulationControlSourceCodeString* propDispatchEvent = new SimulationControlSourceCodeString(
									[this]() { return SourceCodeString(getOnDispatchEventCode()); }, [this](SourceCodeString value) { setOnDispatchEventCode(value.str()); },
									Util::TypeOf<CppForG>(), getName(), "OnDispatchEventCode", "");
	SimulationControlSourceCodeString* propBetweenRep = new SimulationControlSourceCodeString(
									[this]() { return SourceCodeString(getInitBetweenReplicationCode()); }, [this](SourceCodeString value) { setInitBetweenReplicationCode(value.str()); },
									Util::TypeOf<CppForG>(), getName(), "InitBetweenReplicationCode", "");
	SimulationControlSourceCodeString* propIncludes = new SimulationControlSourceCodeString(
									[this]() { return SourceCodeString(getIncludesCode()); }, [this](SourceCodeString value) { setIncludesCode(value.str()); },
									Util::TypeOf<CppForG>(), getName(), "IncludesCode", "");
	SimulationControlGenericClass<CppCompiler*, Model*, CppCompiler>* propCppCompiler = new SimulationControlGenericClass<CppCompiler*, Model*, CppCompiler>(
									_parentModel,
									std::bind(&CppForG::getCppCompiler, this), nullptr,
									Util::TypeOf<CppForG>(), getName(), "CppCompiler", "", false, true, false,
									nullptr, false, true);

	_parentModel->getControls()->insert(propDispatchEvent);
	_parentModel->getControls()->insert(propBetweenRep);
	_parentModel->getControls()->insert(propIncludes);
	_parentModel->getControls()->insert(propCppCompiler);

	// setting properties
	_addSimulationControl(propDispatchEvent);
	_addSimulationControl(propBetweenRep);
	_addSimulationControl(propIncludes);
	_addSimulationControl(propCppCompiler);
}

std::string CppForG::show() {
	return ModelComponent::show() + "";
}

ModelComponent* CppForG::LoadInstance(Model* model, PersistenceRecord *fields) {
	// DataDefinitions are loaded before components. Reuse the persisted internal
	// compiler after the component's persisted name is known, instead of creating
	// a second compiler using a generated temporary component name.
	CppForG* newComponent = new CppForG(model, fields->loadField("name", ""), false);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CppForG instance: " + std::string(e.what()));
	}
	return newComponent;
}

void CppForG::setCppCompiler(CppCompiler* _cppCompiler) {
	if (_cppCompiler != this->_cppCompiler) {
		throw std::invalid_argument("CppForG owns its CppCompiler; edit its settings through the inline property.");
	}
}

CppCompiler* CppForG::getCppCompiler() const {
	return _cppCompiler;
}

void CppForG::setInitBetweenReplicationCode(std::string _initBetweenReplicationCode) {
	this->_initBetweenReplicationCode = _initBetweenReplicationCode;
}

std::string CppForG::getInitBetweenReplicationCode() const {
	return _initBetweenReplicationCode;
}

void CppForG::setOnDispatchEventCode(std::string _onDispatchEventCode) {
	this->_onDispatchEventCode = _onDispatchEventCode;
}

std::string CppForG::getOnDispatchEventCode() const {
	return _onDispatchEventCode;
}

void CppForG::setIncludesCode(std::string _includesCode) {
	this->_includesCode = _includesCode;
}

std::string CppForG::getIncludesCode() const {
	return _includesCode;
}

void CppForG::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    traceSimulation(this, "Invoking cpp user code for dispatchEvent");
    try {
        if (dispatchEvent_SharedLibHandler != nullptr) {
            dispatchEvent_SharedLibHandler(_parentModel->getParentSimulator(), _parentModel, entity);
        } else {
            traceError("CppForG dispatch library entry point is not loaded: " + getName());
        }
    } catch (const std::exception& e) {
        traceError("CppForG user code exception in " + getName() + ": " + e.what());
    } catch (...) {
        traceError("CppForG user code threw an unknown exception in " + getName());
    }
    // Preserve the original flow-through behavior even after a user-code error.
    _parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool CppForG::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		if (_cppCompiler == nullptr) {
			_cppCompiler = dynamic_cast<CppCompiler*>(_parentModel->getDataManager()->getDataDefinition(
				Util::TypeOf<CppCompiler>(), getName() + ".CppCompiler"));
			if (_cppCompiler == nullptr) {
				_createNonEditableDataDefinitions();
			} else {
				_mandatoryNonEditableDataDefinitionInsert("CppCompiler", _cppCompiler);
			}
		}
		_onDispatchEventCode = fields->loadField("onDispatchEventCode", "");
		_initBetweenReplicationCode = fields->loadField("initBetweenReplicationCode", "");
		_includesCode = fields->loadField("includesCode", "");
	}
	return res;
}

//void CppForG::_initBetweenReplications() {}

void CppForG::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("onDispatchEventCode", _onDispatchEventCode, "", saveDefaultValues);
	fields->saveField("initBetweenReplicationCode", _initBetweenReplicationCode, "", saveDefaultValues);
	fields->saveField("includesCode", _includesCode, "", saveDefaultValues);
}

bool CppForG::_check(std::string& errorMessage) {
    namespace fs = std::filesystem;
    if (_cppCompiler == nullptr) {
        errorMessage += "CppForG has no CppCompiler instance.";
        return false;
    }

    // The generated code is distinct from a dynamically loaded GenESyS plugin.
    // It shares the host C++ toolchain and uses the existing entry-point signatures.
    const std::string sourceCode = R"GENESYS(
#include <iostream>
#include <string>
#include <exception>
#if __has_include("kernel/simulator/model/Model.h")
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"
#else
class Simulator;
class Model;
class Entity;
#endif
)GENESYS" + _includesCode + R"GENESYS(
static void genesys_cppforg_dispatch(Simulator* simulator, Model* model, Entity* entity) {
)GENESYS" + _onDispatchEventCode + R"GENESYS(
}
static void genesys_cppforg_initialize(Model* model) {
)GENESYS" + _initBetweenReplicationCode + R"GENESYS(
}
extern "C" void onDispatchEvent(Simulator* simulator, Model* model, Entity* entity) {
    genesys_cppforg_dispatch(simulator, model, entity);
}
extern "C" void initBetweenReplications(Model* model) {
    genesys_cppforg_initialize(model);
}
)GENESYS";

    const std::string originalFlags = _cppCompiler->getFlagsGeneral();
    const std::string originalTempDir = _cppCompiler->getTempDir();
    std::string includeRoot;
    // CMake currently compiles the source with an absolute path; avoid requiring
    // the runtime working directory to equal the source-tree directory.
    const fs::path sourceRoot = fs::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
    if (fs::exists(sourceRoot / "kernel/simulator/model/Model.h"))
        includeRoot = fs::absolute(sourceRoot).string();

    const std::string signature = sourceCode + "\n" + originalFlags +
        "\n" + _cppCompiler->getCompilerCommand() + "\n" +
        _cppCompiler->getFlagsDynamicLibrary() + "\n" +
        _cppCompiler->getObjectFiles() + "\n" + includeRoot;
    if (!_lastCompiledSignature.empty() && _lastCompiledSignature == signature &&
        dispatchEvent_SharedLibHandler != nullptr &&
        initBetweenReplications_SharedLibHandler != nullptr &&
        _cppCompiler->getDynamicLibraryHandler() != nullptr) {
        return true;
    }

    // Never leave function pointers referencing a library being replaced.
    dispatchEvent_SharedLibHandler = nullptr;
    initBetweenReplications_SharedLibHandler = nullptr;
    _lastCompiledSignature.clear();
    if (!_cppCompiler->unloadLibrary()) {
        errorMessage += "Unable to unload the previous CppForG library.";
        return false;
    }

    try {
        if (_workDirectory.empty()) {
            fs::path baseDir = fs::temp_directory_path();
            if (!originalTempDir.empty() && originalTempDir != ".temp/" &&
                originalTempDir != ".temp") baseDir = originalTempDir;
            fs::create_directories(baseDir);
            std::string pattern = (fs::absolute(baseDir) / "genesys-cppforg-XXXXXX").string();
            std::vector<char> data(pattern.begin(), pattern.end());
            data.push_back('\0');
            char* created = ::mkdtemp(data.data());
            if (created == nullptr) throw std::runtime_error("Unable to create a private CppForG directory");
            _workDirectory = created;
        }

        _sourceFilename = (fs::path(_workDirectory) / "generated.cpp").string();
        _outputFilename = (fs::path(_workDirectory) / "generated.so").string();
        {
            std::ofstream outfile(_sourceFilename, std::ios::trunc);
            if (!outfile) throw std::runtime_error("Unable to open generated source file");
            outfile << sourceCode;
            outfile.flush();
            if (!outfile.good()) throw std::runtime_error("Unable to write generated source file");
        }

        _cppCompiler->setSourceFilename(_sourceFilename);
        _cppCompiler->setOutputFilename(_outputFilename);
        _cppCompiler->setTempDir(_workDirectory);

        // The source-tree headers are available in development installations.
        // Source-less installations can still compile code using opaque pointers.
        if (!includeRoot.empty()) {
            if (includeRoot.find('"') != std::string::npos)
                throw std::runtime_error("Unsupported quote in source include directory");
            _cppCompiler->setFlagsGeneral(originalFlags + " -I\"" + includeRoot + "\"");
        }
        const auto result = _cppCompiler->compileToDynamicLibrary();
        _cppCompiler->setFlagsGeneral(originalFlags);
        _cppCompiler->setTempDir(originalTempDir);
        if (!result.success) {
            errorMessage += result.compilationErrOutput.empty() ?
                result.compilationStdOutput : result.compilationErrOutput;
            return false;
        }

        if (!_cppCompiler->loadLibrary(errorMessage)) return false;
        void* handle = _cppCompiler->getDynamicLibraryHandler();
        ::dlerror();
        auto dispatch = reinterpret_cast<onDispatchEvent_t>(::dlsym(handle, "onDispatchEvent"));
        const char* dispatchError = ::dlerror();
        if (dispatchError != nullptr || dispatch == nullptr) {
            errorMessage += dispatchError ? dispatchError : "Missing onDispatchEvent symbol";
            _cppCompiler->unloadLibrary();
            return false;
        }
        ::dlerror();
        auto initializer = reinterpret_cast<initBetweenReplications_t>(::dlsym(handle, "initBetweenReplications"));
        const char* initError = ::dlerror();
        if (initError != nullptr || initializer == nullptr) {
            errorMessage += initError ? initError : "Missing initBetweenReplications symbol";
            _cppCompiler->unloadLibrary();
            return false;
        }
        dispatchEvent_SharedLibHandler = dispatch;
        initBetweenReplications_SharedLibHandler = initializer;
        _lastCompiledSignature = signature;
        return true;
    } catch (const std::exception& e) {
        _cppCompiler->setFlagsGeneral(originalFlags);
        _cppCompiler->setTempDir(originalTempDir);
        errorMessage += std::string("CppForG compilation/loading error: ") + e.what();
        _cppCompiler->unloadLibrary();
        return false;
    }
}

void CppForG::_initBetweenReplications() {
	//@ TODO
	try {
		if (initBetweenReplications_SharedLibHandler != nullptr) {
			initBetweenReplications_SharedLibHandler(_parentModel);
		} else {
			traceError("Could not call function in shared library for "+getName());
		}
	} catch (...) {
		traceError("Could not call function in shared library for "+getName());
	}
}

//void CppForG::_createAttachedAttributes() {}

PluginInformation* CppForG::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CppForG>(), &CppForG::LoadInstance, &CppForG::NewInstance);
	info->setCategory("ExternalIntegration");
	info->insertDynamicLibFileDependence("cppcompiler.so");
	info->setDescriptionHelp("//@TODO");
	return info;
}

// void CppForG::_createInternalStatisticReporters() { }

void CppForG::_createNonEditableDataDefinitions() {
	if (_cppCompiler == nullptr) {
		_cppCompiler = new CppCompiler(_parentModel, getName() + ".CppCompiler");
		_cppCompiler->setSourceFilename(getName() + ".cpp");
		_cppCompiler->setOutputFilename(getName() + ".so");
	}
	if (_cppCompiler != nullptr) {
		_mandatoryNonEditableDataDefinitionInsert("CppCompiler", _cppCompiler);
	} else {
		_mandatoryNonEditableDataDefinitionRemove("CppCompiler");
	}

}
