/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   DynamicLinkedCode.cpp
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:24
 */

#include "plugins/data/ExternalIntegration/CppCompiler.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <grp.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <bits/stdc++.h>
#include <thread>
// dynamic load
#include <dlfcn.h>

#include <iostream>
#include <string>
// execv
#include <unistd.h>
//
#include <thread>


#include "../../../kernel/simulator/model/Model.h"

#include <spawn.h>
#include <signal.h>
#include <fcntl.h>
#include <filesystem>
#include <sstream>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cctype>
#include <algorithm>

extern char **environ;

namespace {
namespace fs = std::filesystem;

// Tokenize user-configurable compiler options without invoking a shell.
// Quotes group whitespace; no shell expansion, pipes or redirections are interpreted.
std::vector<std::string> splitCompilerArguments(const std::string& text) {
    std::vector<std::string> result;
    std::string current;
    char quote = 0;
    bool escaped = false;
    bool inToken = false;
    for (char ch : text) {
        if (escaped) {
            current += ch;
            escaped = false;
            inToken = true;
        } else if (ch == '\\' && quote != '\'') {
            escaped = true;
            inToken = true;
        } else if (quote != 0) {
            if (ch == quote) quote = 0;
            else current += ch;
        } else if (ch == '\'' || ch == '"') {
            quote = ch;
            inToken = true;
        } else if (std::isspace(static_cast<unsigned char>(ch))) {
            if (inToken) {
                result.push_back(current);
                current.clear();
                inToken = false;
            }
        } else {
            current += ch;
            inToken = true;
        }
    }
    if (quote || escaped) throw std::invalid_argument("Unclosed quote or escape in compiler options");
    if (inToken) result.push_back(current);
    return result;
}

struct TemporaryLog {
    std::string path;
    int fd = -1;
    ~TemporaryLog() {
        if (fd >= 0) ::close(fd);
        if (!path.empty()) std::remove(path.c_str());
    }
    bool open(const fs::path& dir, const std::string& label) {
        path = (dir / ("genesys-" + label + "-XXXXXX")).string();
        fd = ::mkstemp(path.data());
        return fd >= 0;
    }
    void close() {
        if (fd >= 0) { ::close(fd); fd = -1; }
    }
};

std::string makeOutputPath(const std::string& dir, const std::string& filename) {
    if (filename.empty()) throw std::invalid_argument("OutputFilename must not be empty");
    const fs::path name(filename);
    return fs::absolute(name.is_absolute() ? name : fs::path(dir) / name).lexically_normal().string();
}

std::vector<std::string> buildCompilerArguments(const std::string& compiler,
    const std::string& general, const std::string& specific,
    const std::string& objectFiles, const std::string& source, const std::string& output) {
    auto args = splitCompilerArguments(compiler);
    if (args.empty() || args[0].empty()) throw std::invalid_argument("CompilerCommand must not be empty");
    for (const auto& part : {general, specific, objectFiles}) {
        auto more = splitCompilerArguments(part);
        args.insert(args.end(), more.begin(), more.end());
    }
    if (source.empty()) throw std::invalid_argument("SourceFilename must not be empty");
    args.push_back(fs::absolute(source).lexically_normal().string());
    args.push_back("-o");
    args.push_back(output);
    return args;
}
} // namespace



#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CppCompiler::GetPluginInformation;
}
#endif

ModelDataDefinition* CppCompiler::NewInstance(Model* model, std::string name) {
	return new CppCompiler(model, name);
}

CppCompiler::CppCompiler(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<CppCompiler>(), name) {
	SimulationControlGeneric<std::string>* propSourceFilename = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getSourceFilename, this), std::bind(&CppCompiler::setSourceFilename, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "SourceFilename", "");
	SimulationControlGeneric<bool>* propLibraryLoaded = new SimulationControlGeneric<bool>(
									std::bind(&CppCompiler::IsLibraryLoaded, this), std::bind(&CppCompiler::setLibraryLoaded, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "LibraryLoaded", "");
	SimulationControlGeneric<std::string>* propFlagsExecutable = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getFlagsExecutable, this), std::bind(&CppCompiler::setFlagsExecutable, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "FlagsExecutable", "");
	SimulationControlGeneric<std::string>* propFlagsStaticLibrary = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getFlagsStaticLibrary, this), std::bind(&CppCompiler::setFlagsStaticLibrary, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "FlagsStaticLibrary", "");
	SimulationControlGeneric<std::string>* propFlagsDynamicLibrary = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getFlagsDynamicLibrary, this), std::bind(&CppCompiler::setFlagsDynamicLibrary, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "FlagsDynamicLibrary", "");
	SimulationControlGeneric<std::string>* propFlagsGeneral = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getFlagsGeneral, this), std::bind(&CppCompiler::setFlagsGeneral, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "FlagsGeneral", "");
	SimulationControlGeneric<std::string>* propCompilerCommand = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getCompilerCommand, this), std::bind(&CppCompiler::setCompilerCommand, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "CompilerCommand", "");
	SimulationControlGeneric<std::string>* propOutputDir = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getOutputDir, this), std::bind(&CppCompiler::setOutputDir, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "OutputDir", "");
	SimulationControlGeneric<std::string>* propTempDir = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getTempDir, this), std::bind(&CppCompiler::setTempDir, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "TempDir", "");
	SimulationControlGeneric<std::string>* propOutputFilename = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getOutputFilename, this), std::bind(&CppCompiler::setOutputFilename, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "OutputFilename", "");
	SimulationControlGeneric<std::string>* propObjectFiles = new SimulationControlGeneric<std::string>(
									std::bind(&CppCompiler::getObjectFiles, this), std::bind(&CppCompiler::setObjectFiles, this, std::placeholders::_1),
									Util::TypeOf<CppCompiler>(), getName(), "ObjectFiles", "");																																													

	_parentModel->getControls()->insert(propSourceFilename);
	_parentModel->getControls()->insert(propLibraryLoaded);
	_parentModel->getControls()->insert(propFlagsExecutable);
	_parentModel->getControls()->insert(propFlagsStaticLibrary);
	_parentModel->getControls()->insert(propFlagsDynamicLibrary);
	_parentModel->getControls()->insert(propFlagsGeneral);
	_parentModel->getControls()->insert(propCompilerCommand);
	_parentModel->getControls()->insert(propOutputDir);
	_parentModel->getControls()->insert(propTempDir);
	_parentModel->getControls()->insert(propOutputFilename);
	_parentModel->getControls()->insert(propObjectFiles);

	// setting properties
	_addSimulationControl(propSourceFilename);
	_addSimulationControl(propLibraryLoaded);
	_addSimulationControl(propFlagsExecutable);
	_addSimulationControl(propFlagsStaticLibrary);
	_addSimulationControl(propFlagsDynamicLibrary);
	_addSimulationControl(propFlagsGeneral);
	_addSimulationControl(propCompilerCommand);
	_addSimulationControl(propOutputDir);
	_addSimulationControl(propTempDir);
	_addSimulationControl(propOutputFilename);
	_addSimulationControl(propObjectFiles);
}

CppCompiler::~CppCompiler() {
    unloadLibrary();
}

// static

ModelDataDefinition* CppCompiler::LoadInstance(Model* model, PersistenceRecord *fields) {
	CppCompiler* newElement = new CppCompiler(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load CppCompiler instance: " + std::string(e.what()));
	}
	return newElement;
}

PluginInformation* CppCompiler::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CppCompiler>(), &CppCompiler::LoadInstance, &CppCompiler::NewInstance);
	info->setDescriptionHelp("Compiles C/C++ source files into executables or libraries and optionally loads the generated dynamic library at runtime.");
	//info->setObservation("");
	//info->setMinimumOutputs();
	//info->setDynamicLibFilenameDependencies();
	//info->setFields();
	// ...
	info->setCategory("ExternalIntegration");
	return info;
}

//

std::string CppCompiler::show() {
	return ModelDataDefinition::show() +
			",sourceFilename=\"" + _sourceFilename + "\"" +
			",outputFilename=\"" + _outputFilename + "\"" +
			",compilerCommand=\"" + _compilerCommand + "\"" +
			",outputDir=\"" + _outputDir + "\"" +
			",tempDir=\"" + _tempDir + "\"" +
			",libraryLoaded=" + (_libraryLoaded ? "true" : "false");
}

void CppCompiler::setSourceFilename(std::string _code) {
	this->_sourceFilename = _code;
}

std::string CppCompiler::getSourceFilename() const {
	return _sourceFilename;
}
// must be overriden

bool CppCompiler::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		/*!
		 * \brief Load persisted compiler configuration.
		 *
		 * The structure intentionally follows the same pattern used by other
		 * ModelDataDefinition implementations to simplify future maintenance.
		 */
		_sourceFilename = fields->loadField("sourceFilename", DEFAULT.sourceFilename);
		_tempDir = fields->loadField("tempDir", DEFAULT.tempDir);
		_outputDir = fields->loadField("outputDir", DEFAULT.outputDir);
		_outputFilename = fields->loadField("outputFilename", DEFAULT.outputFilename);
		_compilerCommand = fields->loadField("compilerCommand", DEFAULT.compiler);
		_flagsGeneral = fields->loadField("flagsGeneral", DEFAULT.flagsGeneral);
		_flagsDynamicLibrary = fields->loadField("flagsDynamicLibrary", DEFAULT.flagsDynamicLibrary);
		_flagsStaticLibrary = fields->loadField("flagsStaticLibrary", DEFAULT.flagsStaticLibrary);
		_flagsExecutable = fields->loadField("flagsExecutable", DEFAULT.flagsExecutable);
		_objectFiles = fields->loadField("objectFiles", DEFAULT.objectFiles);
		_libraryLoaded = fields->loadField("libraryLoaded", false);
		// Example extension point:
		// _compiledToDynamicLibrary = fields->loadField("compiledToDynamicLibrary", false);
	}
	return res;
}

void CppCompiler::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Persist compiler configuration.
	 *
	 * Keep field names symmetric to \ref _loadInstance for predictable
	 * serialization behavior and easier backward-compatible migrations.
	 */
	fields->saveField("sourceFilename", _sourceFilename, DEFAULT.sourceFilename, saveDefaultValues);
	fields->saveField("tempDir", _tempDir, DEFAULT.tempDir, saveDefaultValues);
	fields->saveField("outputDir", _outputDir, DEFAULT.outputDir, saveDefaultValues);
	fields->saveField("outputFilename", _outputFilename, DEFAULT.outputFilename, saveDefaultValues);
	fields->saveField("compilerCommand", _compilerCommand, DEFAULT.compiler, saveDefaultValues);
	fields->saveField("flagsGeneral", _flagsGeneral, DEFAULT.flagsGeneral, saveDefaultValues);
	fields->saveField("flagsDynamicLibrary", _flagsDynamicLibrary, DEFAULT.flagsDynamicLibrary, saveDefaultValues);
	fields->saveField("flagsStaticLibrary", _flagsStaticLibrary, DEFAULT.flagsStaticLibrary, saveDefaultValues);
	fields->saveField("flagsExecutable", _flagsExecutable, DEFAULT.flagsExecutable, saveDefaultValues);
	fields->saveField("objectFiles", _objectFiles, DEFAULT.objectFiles, saveDefaultValues);
	fields->saveField("libraryLoaded", _libraryLoaded, false, saveDefaultValues);
	// Example extension point:
	// fields->saveField("compiledToDynamicLibrary", _compiledToDynamicLibrary, false, saveDefaultValues);
}

// could be overriden

bool CppCompiler::_check(std::string& errorMessage) {
	/*!
	 * \brief Validate minimal compiler configuration before use.
	 */
	bool resultAll = true;
	if (_compilerCommand == "") {
		errorMessage += "CompilerCommand must not be empty. ";
		resultAll = false;
	}
	if (_sourceFilename == "") {
		errorMessage += "SourceFilename must not be empty. ";
		resultAll = false;
	}
	if (_outputFilename == "") {
		errorMessage += "OutputFilename must not be empty. ";
		resultAll = false;
	}
	// Optional strict checks that can be enabled later:
	// resultAll &= Util::FileExists(_compilerCommand);
	// resultAll &= (_sourceFilename != "");
	return resultAll;
}

// void CppCompiler::_createAttachedAttributes() {
// }

void CppCompiler::_initBetweenReplications() {

}


CppCompiler::CompilationResult CppCompiler::compileToExecutable() {
    CompilationResult result;
    try {
        if (_dynamicLibraryHandle != nullptr) throw std::runtime_error("Unload the library before compiling");
        const auto output = makeOutputPath(_outputDir, _outputFilename);
        const auto args = buildCompilerArguments(_compilerCommand, _flagsGeneral, _flagsExecutable,
                                                 _objectFiles, _sourceFilename, output);
        result = _invokeCompiler(args, output);
        if (result.success) _outputFilename = output;
        _compiledToDynamicLibrary = false;
    } catch (const std::exception& e) {
        result.compilationErrOutput = e.what();
    }
    return result;
}

CppCompiler::CompilationResult CppCompiler::compileToDynamicLibrary() {
    CompilationResult result;
    try {
        if (_dynamicLibraryHandle != nullptr) throw std::runtime_error("Unload the library before compiling");
        const auto output = makeOutputPath(_outputDir, _outputFilename);
        const auto args = buildCompilerArguments(_compilerCommand, _flagsGeneral, _flagsDynamicLibrary,
                                                 _objectFiles, _sourceFilename, output);
        result = _invokeCompiler(args, output);
        _compiledToDynamicLibrary = result.success;
        if (result.success) _outputFilename = output;
    } catch (const std::exception& e) {
        _compiledToDynamicLibrary = false;
        result.compilationErrOutput = e.what();
    }
    return result;
}

CppCompiler::CompilationResult CppCompiler::compileToStaticLibrary() {
    CompilationResult result;
    try {
        if (_dynamicLibraryHandle != nullptr) throw std::runtime_error("Unload the library before compiling");
        const auto output = makeOutputPath(_outputDir, _outputFilename);
        const auto args = buildCompilerArguments(_compilerCommand, _flagsGeneral, _flagsStaticLibrary,
                                                 _objectFiles, _sourceFilename, output);
        result = _invokeCompiler(args, output);
        if (result.success) _outputFilename = output;
        _compiledToDynamicLibrary = false;
    } catch (const std::exception& e) {
        result.compilationErrOutput = e.what();
    }
    return result;
}

bool CppCompiler::loadLibrary(std::string& errorMessage) {
    if (_dynamicLibraryHandle != nullptr) {
        errorMessage += "A library is already loaded; unload it before loading again.";
        return false;
    }
    if (_outputFilename.empty()) {
        errorMessage += "OutputFilename must not be empty.";
        return false;
    }
    ::dlerror();
    void* handle = ::dlopen(_outputFilename.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        const char* error = ::dlerror();
        errorMessage += error ? error : "Unknown dlopen error";
        _libraryLoaded = false;
        return false;
    }
    _dynamicLibraryHandle = handle;
    _libraryLoaded = true;
    return true;
}

bool CppCompiler::unloadLibrary() {
    if (_dynamicLibraryHandle == nullptr) {
        _libraryLoaded = false;
        return true;
    }
    if (::dlclose(_dynamicLibraryHandle) != 0) return false;
    _dynamicLibraryHandle = nullptr;
    _libraryLoaded = false;
    return true;
}

void* CppCompiler::getDynamicLibraryHandler() const {
    return _dynamicLibraryHandle;
}

void CppCompiler::setObjectFiles(std::string _objectFiles) {
	this->_objectFiles = _objectFiles;
}

std::string CppCompiler::getObjectFiles() const {
	return _objectFiles;
}

void CppCompiler::setFlagsExecutable(std::string _flagsExecutable) {
	this->_flagsExecutable = _flagsExecutable;
}

std::string CppCompiler::getFlagsExecutable() const {
	return _flagsExecutable;
}

void CppCompiler::setLibraryLoaded(bool libraryLoaded) {
	if (!libraryLoaded) {
        unloadLibrary();
    } else if (_dynamicLibraryHandle == nullptr) {
        std::string error;
        loadLibrary(error);
    }
}

void CppCompiler::setFlagsStaticLibrary(std::string _flagsStaticLibrary) {
	this->_flagsStaticLibrary = _flagsStaticLibrary;
}

std::string CppCompiler::getFlagsStaticLibrary() const {
	return _flagsStaticLibrary;
}

void CppCompiler::setFlagsDynamicLibrary(std::string _flagsDynamicLibrary) {
	this->_flagsDynamicLibrary = _flagsDynamicLibrary;
}

std::string CppCompiler::getFlagsDynamicLibrary() const {
	return _flagsDynamicLibrary;
}

void CppCompiler::setFlagsGeneral(std::string _flagsGeneral) {
	this->_flagsGeneral = _flagsGeneral;
}

std::string CppCompiler::getFlagsGeneral() const {
	return _flagsGeneral;
}

void CppCompiler::setCompilerCommand(std::string _compilerCommand) {
	this->_compilerCommand = _compilerCommand;
}

std::string CppCompiler::getCompilerCommand() const {
	return _compilerCommand;
}

void CppCompiler::setOutputDir(std::string _outputDir) {
	this->_outputDir = _outputDir;
}

std::string CppCompiler::getOutputDir() const {
	return _outputDir;
}

void CppCompiler::setTempDir(std::string _tempDir) {
	this->_tempDir = _tempDir;
}

std::string CppCompiler::getTempDir() const {
	return _tempDir;
}

void CppCompiler::setOutputFilename(std::string _outputFilename) {
	this->_outputFilename = _outputFilename;
}

std::string CppCompiler::getOutputFilename() const {
	return _outputFilename;
}

bool CppCompiler::IsLibraryLoaded() const {
	return _libraryLoaded;
}

std::string CppCompiler::_read(std::string filename) {
	std::string res = "";
	std::ifstream file(filename);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			res += line+"\n";
		}
		file.close();
	}
	return res;
}

CppCompiler::CompilationResult CppCompiler::_invokeCompiler(
        const std::vector<std::string>& arguments, const std::string& outputPath) {
    CompilationResult result;
    result.destinationPath = outputPath;
    try {
        const fs::path output(outputPath);
        fs::create_directories(output.parent_path());
        const fs::path tempDir = fs::absolute(_tempDir.empty() ? output.parent_path() : fs::path(_tempDir));
        fs::create_directories(tempDir);
        // Never accept an artifact left by an earlier compilation.
        std::error_code removeError;
        fs::remove(output, removeError);
        if (removeError) throw fs::filesystem_error("Cannot remove previous compiler output", output, removeError);

        TemporaryLog outLog, errLog;
        if (!outLog.open(tempDir, "compiler-out") || !errLog.open(tempDir, "compiler-err"))
            throw std::runtime_error("Unable to create temporary compiler logs");

        posix_spawn_file_actions_t actions;
        const int actionError = ::posix_spawn_file_actions_init(&actions);
        if (actionError != 0) throw std::runtime_error(std::string("posix_spawn actions: ") + std::strerror(actionError));
        int setup = ::posix_spawn_file_actions_adddup2(&actions, outLog.fd, STDOUT_FILENO);
        if (setup == 0) setup = ::posix_spawn_file_actions_adddup2(&actions, errLog.fd, STDERR_FILENO);
        if (setup == 0) setup = ::posix_spawn_file_actions_addclose(&actions, outLog.fd);
        if (setup == 0) setup = ::posix_spawn_file_actions_addclose(&actions, errLog.fd);

        std::vector<char*> argv;
        argv.reserve(arguments.size() + 1);
        for (const std::string& item : arguments) argv.push_back(const_cast<char*>(item.c_str()));
        argv.push_back(nullptr);
        pid_t pid = -1;
        int spawnError = setup == 0 ? ::posix_spawnp(&pid, argv[0], &actions, nullptr, argv.data(), environ) : setup;
        ::posix_spawn_file_actions_destroy(&actions);
        outLog.close();
        errLog.close();

        if (spawnError != 0)
            throw std::runtime_error(std::string("Cannot launch compiler: ") + std::strerror(spawnError));

        int status = 0;
        bool timedOut = false;
        bool waited = false;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(120);
        while (std::chrono::steady_clock::now() < deadline) {
            const pid_t finished = ::waitpid(pid, &status, WNOHANG);
            if (finished == pid) { waited = true; break; }
            if (finished < 0 && errno != EINTR) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (!waited) {
            ::kill(pid, SIGKILL);
            while (::waitpid(pid, &status, 0) < 0 && errno == EINTR) {}
            timedOut = true;
        }
        result.compilationStdOutput = _read(outLog.path);
        result.compilationErrOutput = _read(errLog.path);
        result.success = !timedOut && WIFEXITED(status) && WEXITSTATUS(status) == 0 &&
                         fs::is_regular_file(output);
        if (timedOut) result.compilationErrOutput += "\nCompiler timed out or could not be waited for.";
        else if (!result.success) result.compilationErrOutput +=
            "\nCompiler failed or did not generate the expected output.";
        if (!result.success) {
            std::error_code ec;
            fs::remove(output, ec);
        }
    } catch (const std::exception& e) {
        result.compilationErrOutput += std::string("\n") + e.what();
        std::error_code ec;
        fs::remove(outputPath, ec);
    }
    return result;
}

// void CppCompiler::_createInternalStatisticReporters() { }

// void CppCompiler::_createEditableDataDefinitions() { }

// void CppCompiler::_createAttachedAttributes() { }
