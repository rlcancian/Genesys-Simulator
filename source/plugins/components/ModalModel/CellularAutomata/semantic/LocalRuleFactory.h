#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

class LocalRule;
class CellularAutomataBase;
class PersistenceRecord;

/*!
 * @brief Factory para criação e registro de regras locais de autômatos celulares.
 *
 * Permite registro extensível de novas regras sem modificação do componente principal.
 * Suporta criação por nome e reconstrução a partir de persistência.
 */
class LocalRuleFactory {
public:
    //! Tipo função criadora de regras
    using CreatorFunc = std::function<std::unique_ptr<LocalRule>(CellularAutomataBase*)>;

    /*!
     * @brief Registra uma nova regra no factory.
     * @param name Nome único da regra (ex: "GameOfLife")
     * @param creator Função que cria uma instância da regra
     */
    static void registerRule(const std::string& name, CreatorFunc creator);

    /*!
     * @brief Cria uma regra pelo nome registrado.
     * @param name Nome da regra
     * @param parent Ponteiro para o autômato celular pai
     * @return Unique ptr para a regra criada, ou nullptr se não encontrada
     */
    static std::unique_ptr<LocalRule> create(const std::string& name, CellularAutomataBase* parent);

    /*!
     * @brief Cria uma regra a partir de um PersistenceRecord.
     * @param fields Record de persistência contendo o campo "ruleType"
     * @param parent Ponteiro para o autômato celular pai
     * @return Unique ptr para a regra criada com estado restaurado
     */
    static std::unique_ptr<LocalRule> createFromPersistence(PersistenceRecord* fields, CellularAutomataBase* parent);

    /*!
     * @brief Verifica se uma regra está registrada.
     * @param name Nome da regra
     * @return true se a regra está registrada
     */
    static bool isRegistered(const std::string& name);

    /*!
     * @brief Retorna lista de nomes de regras registradas.
     * @return Vetor com nomes das regras
     */
    static std::vector<std::string> getRegisteredRules();

private:
    static std::unordered_map<std::string, CreatorFunc>& getRegistry();
};
