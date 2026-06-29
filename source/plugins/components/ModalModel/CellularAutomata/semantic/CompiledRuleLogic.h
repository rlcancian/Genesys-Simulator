#pragma once

#include "UserDefinedRuleLogic.h"
#include <string>
#include <vector>
#include <unordered_map>

/*!
 * @brief Implementação de regra local que compila e executa código dinâmico.
 *
 * Usa compilação via system() + dlopen para carregar código do usuário.
 */
class CompiledRuleLogic : public UserDefinedRuleLogic {
public:
    /*!
     * @brief Tipo da função compilada esperada.
     * Assinatura: long userRule(long centerState, const long* neighborStates,
     *                           int numNeighbors, unsigned int step,
     *                           const double* paramValues, const char** paramNames, int numParams)
     */
    using UserRuleFunc = long(*)(long, const long*, int, unsigned int, const double*, const char**, int);

public:
    CompiledRuleLogic(CellularAutomataBase* parentCellularAutomata, StateSet* stateSet = nullptr);
    virtual ~CompiledRuleLogic();

    // Desabilitar cópia
    CompiledRuleLogic(const CompiledRuleLogic&) = delete;
    CompiledRuleLogic& operator=(const CompiledRuleLogic&) = delete;

public:
    virtual std::string getRuleType() const override { return "UserDefined"; }

    /*!
     * @brief Compila o código-fonte.
     * @param sourceCode Código-fonte C++ completo
     * @return true se compilou e carregou com sucesso
     */
    bool compile(const std::string& sourceCode);

    /*!
     * @brief Compila a partir de um arquivo.
     * @param filename Caminho do arquivo .cpp
     * @return true se compilou e carregou com sucesso
     */
    bool compileFromFile(const std::string& filename);

    /*!
     * @brief Implementação de evaluate() que chama a função compilada.
     */
    virtual State evaluate(const Cell* center,
                          const std::vector<const Cell*>& neighbors,
                          unsigned int step,
                          const std::unordered_map<std::string, double>& params) override;

    /*!
     * @brief Implementação de applyRule (requerido por LocalRule).
     * Chama evaluate() internamente.
     */
    virtual void applyRule(Cell* cell) override;

    // Persistência
    virtual bool _loadInstance(PersistenceRecord* fields) override;
    virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;

    // Getters
    bool isLoaded() const { return functionPtr != nullptr; }
    UserRuleFunc getFunctionPointer() const { return functionPtr; }

private:
    void unloadLibrary();

    void* libraryHandle = nullptr;
    UserRuleFunc functionPtr = nullptr;
    unsigned int currentStep = 0;
};
