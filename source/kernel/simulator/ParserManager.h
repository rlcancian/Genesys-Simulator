/*
 * File:   ParserManager.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 20:34
 */

#ifndef PARSERMANAGER_H
#define PARSERMANAGER_H

#include "ParserChangesInformation.h"
#include <string>
#include <list>

class Model;
class ModelDataDefinition;

class ParserManager {
public:
	struct NewParser {
		std::string bisonFilename;
		std::string flexFilename;
		std::string compiledParserFilename;
	};

	struct GenerateNewParserResult {
		bool result = false;
		std::string bisonMessages;
		std::string lexMessages;
		std::string compilationMessages;
		NewParser newParser;
	};
public:
	ParserManager();
	virtual ~ParserManager();
public:
	void setModel(Model* model);
	Model* getModel() const;
	void setSourceDir(const std::string& sourceDir);
	std::string getSourceDir() const;

	std::list<ParserChangesInformation*> aggregateChanges();

	GenerateNewParserResult generateNewParser(ParserChangesInformation* changes);
	bool connectNewParser(NewParser newParser, std::string* errorMessage = nullptr);

	void setWorkDir(const std::string& workDir);
	std::string getWorkDir() const;

private:
	std::string _readFile(const std::string& filename);
	bool _writeFile(const std::string& filename, const std::string& content);
	bool _copyFile(const std::string& src, const std::string& dst);
	std::string _findBaseFile(const std::string& relativePath);

	bool _injectBisonChanges(std::string& content, ParserChangesInformation* changes);
	bool _injectFlexChanges(std::string& content, ParserChangesInformation* changes);
	bool _injectSection(std::string& content, const std::string& beginMarker, const std::string& endMarker, const std::string& block);

	bool _invokeBison(const std::string& workDir, const std::string& bisonFile, std::string& messages);
	bool _invokeFlex(const std::string& workDir, const std::string& flexFile, std::string& messages);
	bool _compileDynamicLibrary(const std::string& workDir, std::string& messages);

	std::string _buildCompilerCommand(const std::string& workDir);

private:
	Model* _model = nullptr;
	std::string _sourceDir = ".";
	std::string _workDir = ".genesys_parser_build/";
	void* _dynamicLibraryHandle = nullptr;
};

#endif /* PARSERMANAGER_H */
