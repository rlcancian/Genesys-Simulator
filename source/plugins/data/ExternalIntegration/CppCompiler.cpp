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


#include "kernel/simulator/Model.h"


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

// static

ModelDataDefinition* CppCompiler::LoadInstance(Model* model, PersistenceRecord *fields) {
	CppCompiler* newElement = new CppCompiler(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

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

void CppCompiler::_createInternalAndAttachedData() {
}

void CppCompiler::_initBetweenReplications() {

}


CppCompiler::CompilationResult CppCompiler::compileToExecutable() {
	CppCompiler::CompilationResult result;
	std::string outputDir = _outputDir;
	if (!outputDir.empty() && outputDir.back() != Util::DirSeparator()) {
		outputDir += Util::DirSeparator();
	}
	const std::string outputPath = outputDir + _outputFilename;
	Util::FileDelete(outputPath);
	std::string command(_compilerCommand + " " + _flagsGeneral + " " + _flagsExecutable + " " + _objectFiles + " " + _sourceFilename + " -o " + outputPath);
	result = _invokeCompiler(command);
	if (result.success) {
		_outputFilename = outputPath;
	}
	_compiledToDynamicLibrary = false;
	return result;
}

CppCompiler::CompilationResult CppCompiler::compileToDynamicLibrary() {
	CppCompiler::CompilationResult result;
	std::string outputDir = _outputDir;
	if (!outputDir.empty() && outputDir.back() != Util::DirSeparator()) {
		outputDir += Util::DirSeparator();
	}
	const std::string outputPath = outputDir + _outputFilename;
	std::string command(_compilerCommand + " " + _flagsGeneral + " " + _flagsDynamicLibrary + " " + _objectFiles + " " + _sourceFilename + " -o " + outputPath);
	result = _invokeCompiler(command);
	_compiledToDynamicLibrary = result.success;
	if (result.success) {
		_outputFilename = outputPath;
	}
	return result;
}

CppCompiler::CompilationResult CppCompiler::compileToStaticLibrary() {
	CppCompiler::CompilationResult result;
	std::string outputDir = _outputDir;
	if (!outputDir.empty() && outputDir.back() != Util::DirSeparator()) {
		outputDir += Util::DirSeparator();
	}
	const std::string outputPath = outputDir + _outputFilename;
	Util::FileDelete(outputPath);
	std::string command(_compilerCommand + " " + _flagsGeneral + " " + _flagsStaticLibrary + " " + _objectFiles + " " + _sourceFilename + " -o " + outputPath);
	result = _invokeCompiler(command);
	if (result.success) {
		_outputFilename = outputPath;
	}
	_compiledToDynamicLibrary = false;
	return result;
}

bool CppCompiler::loadLibrary(std::string& errorMessage) {
	try {
		_dynamicLibraryHandle = dlopen(_outputFilename.c_str(), RTLD_LAZY);
	} catch (const std::exception& e) {
		if (dlerror() != NULL)
			errorMessage += dlerror();
		errorMessage += e.what();
		return false;
	}
	if (_dynamicLibraryHandle == nullptr) {
		errorMessage += dlerror();
	}
	_libraryLoaded = _dynamicLibraryHandle != nullptr;
	return _libraryLoaded;
}

bool CppCompiler::unloadLibrary() {
	if (!_libraryLoaded || _dynamicLibraryHandle == nullptr) {
		_dynamicLibraryHandle = nullptr;
		_libraryLoaded = false;
		return true;
	}

	const int closeResult = dlclose(_dynamicLibraryHandle);
	if (closeResult != 0) {
		return false;
	}

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
	this->_libraryLoaded = libraryLoaded;
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

CppCompiler::CompilationResult CppCompiler::_invokeCompiler(std::string command) {
	std::string destPath = _tempDir.empty() ? _outputDir : _tempDir;
	if (!destPath.empty() && destPath.back() != Util::DirSeparator()) {
		destPath += Util::DirSeparator();
	}
	const std::string stdoutFile = destPath + "stdout.log";
	const std::string stderrFile = destPath + "stderr.log";
	const std::string redirect = " >" + stdoutFile + " 2>" + stderrFile;

	Util::IncIndent();

	Util::FileDelete(_outputFilename);
	Util::FileDelete(stdoutFile);
	Util::FileDelete(stderrFile);

	const std::string execCommand = command + redirect;
	//trace(execCommand);
	system(execCommand.c_str());
	for (short i = 0; i < 32; i++)
        std::this_thread::yield(); // give the system some time // TODO: Does it work? Is this enough?
	const std::string resultStdout = _read(stdoutFile);
	const std::string resultStderr = _read(stderrFile);
	//trace(resultStdout);
	//trace(resultStderr);

	CppCompiler::CompilationResult result;
	std::ifstream f(_outputFilename.c_str());
	result.success =  f.good();
	result.compilationStdOutput = resultStdout;
	result.compilationErrOutput = resultStderr;
	result.destinationPath = destPath;

	Util::FileDelete(stdoutFile);
	Util::FileDelete(stderrFile);

	Util::DecIndent();

	return result;
}

void CppCompiler::_createReportStatisticsDataDefinitions() {
}

void CppCompiler::_createEditableDataDefinitions() {
}

void CppCompiler::_createOthersDataDefinitions() {
}
