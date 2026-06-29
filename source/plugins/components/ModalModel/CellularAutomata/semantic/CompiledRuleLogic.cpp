#include "CompiledRuleLogic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"

#include <dlfcn.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

// ============================================================================
// CONSTRUTOR / DESTRUTOR
// ============================================================================

CompiledRuleLogic::CompiledRuleLogic(CellularAutomataBase* parentCellularAutomata, StateSet* stateSet)
    : UserDefinedRuleLogic(parentCellularAutomata, stateSet)
    , libraryHandle(nullptr)
    , functionPtr(nullptr)
    , currentStep(0) {
}

CompiledRuleLogic::~CompiledRuleLogic() {
    unloadLibrary();
}

// ============================================================================
// COMPILAÇÃO
// ============================================================================

bool CompiledRuleLogic::compile(const std::string& sourceCode) {
    if (sourceCode.empty()) {
        compilationError = "Source code is empty";
        return false;
    }

    // Descarrega biblioteca anterior se existir
    unloadLibrary();

    // Salva código-fonte em arquivo temporário
    std::string tempDir = "/tmp/ca_rule_";
    tempDir += std::to_string(getpid());
    std::string sourceFile = tempDir + ".cpp";
    std::string libFile = tempDir + ".so";

    // Cria diretório temporário se não existir
    std::string mkdirCmd = "mkdir -p /tmp";
    system(mkdirCmd.c_str());

    // Escreve código-fonte no arquivo
    std::ofstream outFile(sourceFile);
    if (!outFile.is_open()) {
        compilationError = "Failed to create temporary source file: " + sourceFile;
        return false;
    }
    outFile << sourceCode;
    outFile.close();

    // Compila para biblioteca dinâmica
    std::string compileCmd = "g++ -shared -fPIC -std=c++14 -w -o " + libFile + " " + sourceFile;
    int ret = system(compileCmd.c_str());

    if (ret != 0) {
        compilationError = "Compilation failed with exit code: " + std::to_string(ret);
        return false;
    }

    // Carrega a biblioteca dinâmica
    libraryHandle = dlopen(libFile.c_str(), RTLD_NOW);
    if (libraryHandle == nullptr) {
        compilationError = "Failed to load library: " + std::string(dlerror());
        return false;
    }

    // Obtém ponteiro para a função
    functionPtr = reinterpret_cast<UserRuleFunc>(dlsym(libraryHandle, "userRule"));
    if (functionPtr == nullptr) {
        compilationError = "Failed to find 'userRule' function: " + std::string(dlerror());
        unloadLibrary();
        return false;
    }

    this->sourceCode = sourceCode;
    compiled = true;
    compilationError.clear();
    return true;
}

bool CompiledRuleLogic::compileFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        compilationError = "Failed to open file: " + filename;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return compile(buffer.str());
}

// ============================================================================
// EXECUÇÃO
// ============================================================================

void CompiledRuleLogic::applyRule(Cell* cell) {
    if (cell == nullptr || !isLoaded()) {
        return;
    }

    // Obtém vizinhos da célula
    std::vector<const Cell*> neighbors;
    const std::vector<Cell*>& cellNeighbors = cell->getNeighbors();
    for (Cell* n : cellNeighbors) {
        if (n != nullptr) {
            neighbors.push_back(n);
        }
    }

    // Chama evaluate e define nextState
    State result = evaluate(cell, neighbors, currentStep, parameters);
    cell->setNextState(result);
}

State CompiledRuleLogic::evaluate(const Cell* center,
                                   const std::vector<const Cell*>& neighbors,
                                   unsigned int step,
                                   const std::unordered_map<std::string, double>& params) {
    if (!isLoaded() || center == nullptr) {
        return center ? center->getCurrentState() : State(0);
    }

    currentStep = step;

    // Prepara dados dos vizinhos
    std::vector<long> neighborStates;
    neighborStates.reserve(neighbors.size());
    for (const Cell* neighbor : neighbors) {
        if (neighbor != nullptr) {
            neighborStates.push_back(neighbor->getCurrentState().getValue());
        }
    }

    // Prepara parâmetros
    std::vector<double> paramValues;
    std::vector<std::string> paramNameStrings;
    std::vector<const char*> paramNames;

    for (const auto& pair : params) {
        paramNameStrings.push_back(pair.first);
        paramValues.push_back(pair.second);
    }
    for (const auto& name : paramNameStrings) {
        paramNames.push_back(name.c_str());
    }

    // Chama função compilada
    long centerState = center->getCurrentState().getValue();
    long result = functionPtr(
        centerState,
        neighborStates.data(),
        static_cast<int>(neighborStates.size()),
        step,
        paramValues.data(),
        paramNames.data(),
        static_cast<int>(paramValues.size())
    );

    return State(result);
}

// ============================================================================
// PERSISTÊNCIA
// ============================================================================

bool CompiledRuleLogic::_loadInstance(PersistenceRecord* fields) {
    // Chama implementação da base
    if (!UserDefinedRuleLogic::_loadInstance(fields)) {
        return false;
    }

    // Se há código-fonte, tenta recompilar
    if (!sourceCode.empty()) {
        compiled = compile(sourceCode);
    }

    return true;
}

void CompiledRuleLogic::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    // Chama implementação da base
    UserDefinedRuleLogic::_saveInstance(fields, saveDefaultValues);
}

// ============================================================================
// GERENCIAMENTO DE BIBLIOTECA
// ============================================================================

void CompiledRuleLogic::unloadLibrary() {
    if (libraryHandle != nullptr) {
        dlclose(libraryHandle);
        libraryHandle = nullptr;
        functionPtr = nullptr;
    }
    compiled = false;
}
