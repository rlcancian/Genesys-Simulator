/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   DynamicLinkedCode.h
 * Author: rlcancian
 *
 * Created on 11 de janeiro de 2022, 22:24
 */

#ifndef CPPCOMPILER_H
#define CPPCOMPILER_H

#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif

#include <string>

#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
//#include <stdio.h>
#include <unistd.h>
//#include <errno.h>
#include <paths.h>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"


class CompilerThread {
public:
	void operator()();
};

class CppCompiler : public ModelDataDefinition {
public:

	class CompilationResult {
	public:
		CompilationResult() { }
		bool success = false;
		std::string compilationStdOutput = "";
		std::string compilationErrOutput = "";
		std::string destinationPath = "";
		unsigned int lineNumber = 0;
	};

public:
	CppCompiler(Model* model, std::string name = "");
	virtual ~CppCompiler() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
public: // gets & sets
	void setSourceFilename(std::string sourceFilename);
	std::string getSourceFilename() const;
	bool IsLibraryLoaded() const;
	void setFlagsExecutable(std::string _flagsExecutable);
	std::string getFlagsExecutable() const;
	void setLibraryLoaded(bool libraryLoaded);
	void setFlagsStaticLibrary(std::string _flagsStaticLibrary);
	std::string getFlagsStaticLibrary() const;
	void setFlagsDynamicLibrary(std::string _flagsDynamicLibrary);
	std::string getFlagsDynamicLibrary() const;
	void setFlagsGeneral(std::string _flagsGeneral);
	std::string getFlagsGeneral() const;
	void setCompilerCommand(std::string _compilerCommand);
	std::string getCompilerCommand() const;
	void setOutputDir(std::string _outputDir);
	std::string getOutputDir() const;
	void setTempDir(std::string _tempDir);
	std::string getTempDir() const;
	void setOutputFilename(std::string _outputFilename);
	std::string getOutputFilename() const;
	void setObjectFiles(std::string _objectFiles);
	std::string getObjectFiles() const;
public: // new methods
	CompilationResult compileToExecutable();
	CompilationResult compileToDynamicLibrary();
	CompilationResult compileToStaticLibrary();
	bool loadLibrary(std::string& errorMessage);
	bool unloadLibrary();
	void* getDynamicLibraryHandler() const;

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden
	virtual bool _check(std::string& errorMessage) override;
	// virtual ParserChangesInformation* _getParserChangesInformation();
	virtual void _initBetweenReplications() override;
	virtual void _createInternalAndAttachedData() override;
protected:
	CompilationResult _invokeCompiler(std::string command);  //system(command.c_str());
	std::string _read(std::string filename);

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:

	const struct DEFAULT_VALUES {
		const std::string sourceFilename = "";
		const std::string tempDir = ".temp/";
		const std::string outputDir = ".temp/";
		const std::string outputFilename = "";
		const std::string compiler = "g++";
		const std::string flagsGeneral = "-v -w -std=c++14";
		const std::string flagsDynamicLibrary = "-shared -fPIC";
		const std::string flagsStaticLibrary = "-fPIC";
		const std::string flagsExecutable = "";
		const std::string objectFiles = "";
	} DEFAULT;
	std::string _sourceFilename = DEFAULT.sourceFilename;
	std::string _outputFilename = DEFAULT.outputFilename;
	std::string _tempDir = DEFAULT.tempDir;
	std::string _outputDir = DEFAULT.outputDir;
	std::string _compilerCommand = DEFAULT.compiler;
	std::string _flagsGeneral = DEFAULT.flagsGeneral;
	std::string _flagsDynamicLibrary = DEFAULT.flagsDynamicLibrary;
	std::string _flagsStaticLibrary = DEFAULT.flagsStaticLibrary;
	std::string _flagsExecutable = DEFAULT.flagsExecutable;
	std::string _objectFiles = DEFAULT.objectFiles;
	//
	bool _libraryLoaded = false;
	bool _compiledToDynamicLibrary = false;
	void* _dynamicLibraryHandle = nullptr;
};

#endif /* CPPCOMPILER_H */

