#pragma once

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "kernel/simulator/Persistence.h"

#include <string>
#include <vector>
#include <unordered_map>

/*!
 * @brief Classe abstrata para regras locais definidas pelo usuário.
 *
 * Suporta código-fonte que pode ser compilado dinamicamente via CppCompiler.
 * O usuário implementa evaluate() para definir a lógica da regra.
 */
class UserDefinedRuleLogic : public LocalRule {
public:
    UserDefinedRuleLogic(CellularAutomataBase* parentCellularAutomata, StateSet* stateSet = nullptr)
        : LocalRule(parentCellularAutomata, stateSet) {}

    virtual ~UserDefinedRuleLogic() = default;

public:
    virtual std::string getRuleType() const override { return "UserDefined"; }

    /*!
     * @brief Método virtual puro para avaliação da regra.
     * @param center A célula central sendo avaliada
     * @param neighbors Vetor de vizinhos da célula central
     * @param step Passo de simulação atual
     * @param params Parâmetros nomeados da regra
     * @return Novo estado calculado para a célula
     */
    virtual State evaluate(const Cell* center,
                          const std::vector<const Cell*>& neighbors,
                          unsigned int step,
                          const std::unordered_map<std::string, double>& params) = 0;

    // Getters/Setters para código-fonte e parâmetros
    const std::string& getSourceCode() const { return sourceCode; }
    void setSourceCode(const std::string& code) { sourceCode = code; }

    const std::unordered_map<std::string, double>& getParameters() const { return parameters; }
    void setParameters(const std::unordered_map<std::string, double>& params) { parameters = params; }
    void setParameter(const std::string& name, double value) { parameters[name] = value; }
    double getParameter(const std::string& name, double defaultValue = 0.0) const {
        auto it = parameters.find(name);
        return (it != parameters.end()) ? it->second : defaultValue;
    }

    const std::string& getCompilationError() const { return compilationError; }
    bool isCompiled() const { return compiled; }

    // Persistência
    virtual bool _loadInstance(PersistenceRecord* fields) override;
    virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;

protected:
    std::string sourceCode;
    std::unordered_map<std::string, double> parameters;
    std::string compilationError;
    bool compiled = false;
};
