/*
 * File:   ParserManager.cpp
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 20:34
 */

#include "ParserManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <dlfcn.h>

ParserManager::ParserManager() {
}

ParserManager::~ParserManager() {
	if (_dynamicLibraryHandle != nullptr) {
		dlclose(_dynamicLibraryHandle);
		_dynamicLibraryHandle = nullptr;
	}
}

void ParserManager::setModel(Model* model) {
	_model = model;
}

Model* ParserManager::getModel() const {
	return _model;
}

void ParserManager::setSourceDir(const std::string& sourceDir) {
	_sourceDir = sourceDir;
}

std::string ParserManager::getSourceDir() const {
	return _sourceDir;
}

void ParserManager::setWorkDir(const std::string& workDir) {
	_workDir = workDir;
	if (!_workDir.empty() && _workDir.back() != '/') {
		_workDir += '/';
	}
}

std::string ParserManager::getWorkDir() const {
	return _workDir;
}

std::string ParserManager::_readFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

bool ParserManager::_writeFile(const std::string& filename, const std::string& content) {
	std::ofstream file(filename);
	if (!file.is_open()) {
		return false;
	}
	file << content;
	return true;
}

bool ParserManager::_copyFile(const std::string& src, const std::string& dst) {
	std::string content = _readFile(src);
	if (content.empty() && !std::filesystem::exists(src)) {
		return false;
	}
	return _writeFile(dst, content);
}

std::string ParserManager::_findBaseFile(const std::string& relativePath) {
	std::string candidate = _sourceDir + "/" + relativePath;
	if (std::filesystem::exists(candidate)) {
		return candidate;
	}
	candidate = _sourceDir + "/../" + relativePath;
	if (std::filesystem::exists(candidate)) {
		return candidate;
	}
	candidate = "./" + relativePath;
	if (std::filesystem::exists(candidate)) {
		return candidate;
	}
	return "";
}

bool ParserManager::_injectSection(std::string& content, const std::string& beginMarker, const std::string& endMarker, const std::string& block) {
	if (block.empty()) {
		return true;
	}
	size_t beginPos = content.find(beginMarker);
	if (beginPos == std::string::npos) {
		return false;
	}
	size_t endPos = content.find(endMarker, beginPos);
	if (endPos == std::string::npos) {
		return false;
	}
	std::string insertion = block;
	if (!insertion.empty() && insertion.back() != '\n') {
		insertion += '\n';
	}
	content.insert(endPos, insertion);
	return true;
}

bool ParserManager::_injectBisonChanges(std::string& content, ParserChangesInformation* changes) {
	bool ok = true;
	ok &= _injectSection(content, "/****begin_Includes_plugins****/", "/****end_Includes_plugins****/", changes->getincludes());
	ok &= _injectSection(content, "/****begin_Tokens_plugins****/", "/****end_Tokens_plugins****/", changes->gettokens());
	ok &= _injectSection(content, "/****begin_TypeObj_plugins****/", "/****end_TypeObj_plugins****/", changes->gettypeObjs());
	ok &= _injectSection(content, "/****begin_Expression_plugins****/", "/****end_Expression_plugins****/", changes->getexpressions());
	ok &= _injectSection(content, "/****begin_ExpressionProdution_plugins****/", "/****end_ExpressionProdution_plugins****/", changes->getexpressionProductions());
	ok &= _injectSection(content, "/****begin_Assignment_plugins****/", "/****end_Assignment_plugins****/", changes->getassignments());
	ok &= _injectSection(content, "/**begin_FunctionProdution_plugins**", "/****end_FunctionProdution_plugins****/", changes->getfunctionProdutions());
	return ok;
}

bool ParserManager::_injectFlexChanges(std::string& content, ParserChangesInformation* changes) {
	bool ok = true;
	ok &= _injectSection(content, "/**begin_Includes_plugins**/", "/**end_Includes_plugins**/", changes->getincludes());
	ok &= _injectSection(content, "%{/****begin_Lexical_plugins****/%}", "%{/****end_Lexical_plugins****/%}", changes->getlexicalRules());
	ok &= _injectSection(content, "/****begin_LexicalLiterals_plugins****/", "/****end_LexicalLiterals_plugins****/", changes->getlexicalLiterals());
	return ok;
}

bool ParserManager::_invokeBison(const std::string& workDir, const std::string& bisonFile, std::string& messages) {
	std::string cmd = "bison --defines=" + workDir + "GenesysParser.h --locations --debug -Wall -v " + bisonFile + " -o " + workDir + "GenesysParser.cpp 2>&1";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		messages = "Failed to run bison";
		return false;
	}
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		messages += buffer;
	}
	int status = pclose(pipe);
	return status == 0;
}

bool ParserManager::_invokeFlex(const std::string& workDir, const std::string& flexFile, std::string& messages) {
	std::string cmd = "flex -o " + workDir + "Genesys++-scanner.cpp " + flexFile + " 2>&1";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		messages = "Failed to run flex";
		return false;
	}
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		messages += buffer;
	}
	int status = pclose(pipe);
	return status == 0;
}

std::string ParserManager::_buildCompilerCommand(const std::string& workDir) {
	std::string cmd = "g++ -std=c++23 -fPIC -shared ";
	cmd += "-I" + _sourceDir + " ";
	cmd += "-I" + _sourceDir + "/source ";
	cmd += "-I" + _sourceDir + "/source/parser ";
	cmd += "-I" + workDir + " ";
	cmd += workDir + "GenesysParser.cpp ";
	cmd += workDir + "Genesys++-scanner.cpp ";
	cmd += workDir + "Genesys++-driver.cpp ";
	cmd += workDir + "obj_t.cpp ";
	cmd += workDir + "ParserFactory.cpp ";
	cmd += _sourceDir + "/source/kernel/simulator/ParserDefaultImpl2.cpp ";
	cmd += "-o " + workDir + "genesys_parser_dynamic.so ";
	return cmd;
}

bool ParserManager::_compileDynamicLibrary(const std::string& workDir, std::string& messages) {
	std::string cmd = _buildCompilerCommand(workDir) + " 2>&1";
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		messages = "Failed to run compiler";
		return false;
	}
	char buffer[4096];
	while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
		messages += buffer;
	}
	int status = pclose(pipe);
	return status == 0;
}

ParserManager::GenerateNewParserResult ParserManager::generateNewParser(ParserChangesInformation* changes) {
	GenerateNewParserResult result;
	if (changes == nullptr) {
		result.compilationMessages = "No changes provided";
		return result;
	}

	std::string workDir = _workDir;
	std::filesystem::create_directories(workDir);

	std::string bisonBase = _findBaseFile("source/parser/parserBisonFlex/bisonparser.yy");
	std::string flexBase = _findBaseFile("source/parser/parserBisonFlex/lexerparser.ll");
	if (bisonBase.empty() || flexBase.empty()) {
		result.compilationMessages = "Could not find base parser files";
		return result;
	}

	std::string bisonContent = _readFile(bisonBase);
	std::string flexContent = _readFile(flexBase);
	if (bisonContent.empty() || flexContent.empty()) {
		result.compilationMessages = "Could not read base parser files";
		return result;
	}

	if (!_injectBisonChanges(bisonContent, changes)) {
		result.compilationMessages = "Failed to inject Bison changes";
		return result;
	}
	if (!_injectFlexChanges(flexContent, changes)) {
		result.compilationMessages = "Failed to inject Flex changes";
		return result;
	}

	std::string bisonOut = workDir + "bisonparser.yy";
	std::string flexOut = workDir + "lexerparser.ll";
	if (!_writeFile(bisonOut, bisonContent) || !_writeFile(flexOut, flexContent)) {
		result.compilationMessages = "Failed to write modified parser files";
		return result;
	}

	std::string driverCpp = _findBaseFile("source/parser/Genesys++-driver.cpp");
	std::string driverH = _findBaseFile("source/parser/Genesys++-driver.h");
	std::string objCpp = _findBaseFile("source/parser/obj_t.cpp");
	std::string objH = _findBaseFile("source/parser/obj_t.h");
	std::string factoryCpp = _findBaseFile("source/parser/dynamic/ParserFactory.cpp");
	std::string factoryH = _findBaseFile("source/parser/dynamic/ParserFactory.h");
	if (driverCpp.empty() || driverH.empty() || objCpp.empty() || objH.empty() || factoryCpp.empty() || factoryH.empty()) {
		result.compilationMessages = "Could not find parser source files";
		return result;
	}
	_copyFile(driverCpp, workDir + "Genesys++-driver.cpp");
	_copyFile(driverH, workDir + "Genesys++-driver.h");
	_copyFile(objCpp, workDir + "obj_t.cpp");
	_copyFile(objH, workDir + "obj_t.h");
	_copyFile(factoryCpp, workDir + "ParserFactory.cpp");
	_copyFile(factoryH, workDir + "ParserFactory.h");

	if (!_invokeBison(workDir, bisonOut, result.bisonMessages)) {
		result.compilationMessages = "Bison generation failed";
		return result;
	}
	if (!_invokeFlex(workDir, flexOut, result.lexMessages)) {
		result.compilationMessages = "Flex generation failed";
		return result;
	}
	if (!_compileDynamicLibrary(workDir, result.compilationMessages)) {
		return result;
	}

	result.result = true;
	result.newParser.bisonFilename = bisonOut;
	result.newParser.flexFilename = flexOut;
	result.newParser.compiledParserFilename = workDir + "genesys_parser_dynamic.so";
	return result;
}
