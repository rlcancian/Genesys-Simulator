#pragma once

#include <string>

class Lattice;
class LocalRule;
class Neighborhood;

/*!
 * @brief Classe abstrata para políticas de atualização de autômatos celulares.
 *
 * Define como as células são atualizadas a cada passo de simulação.
 * Implementações concretas incluem síncrona, assíncrona aleatória e sequencial.
 */
class UpdatePolicy {
public:
    virtual ~UpdatePolicy() = default;

    /*!
     * @brief Executa um passo de atualização no autômato.
     * @param lattice O lattice contendo as células
     * @param rule A regra local a ser aplicada
     * @param neighborhood A vizinhança usada para consulta
     */
    virtual void execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) = 0;

    /*!
     * @brief Retorna o nome da política.
     */
    virtual std::string getName() const = 0;
};

// ============================================================================
// Implementações concretas
// ============================================================================

/*!
 * @brief Atualização síncrona: todas as células são calculadas com base no estado
 * atual, e só então todas são atualizadas simultaneamente.
 */
class SynchronousUpdate : public UpdatePolicy {
public:
    virtual ~SynchronousUpdate() = default;

    virtual void execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) override;
    virtual std::string getName() const override { return "Synchronous"; }
};

/*!
 * @brief Atualização assíncrona aleatória: células são atualizadas em ordem
 * aleatória, com efeito imediato (células posteriores veem estados já atualizados).
 */
class RandomAsynchronousUpdate : public UpdatePolicy {
public:
    virtual ~RandomAsynchronousUpdate() = default;

    virtual void execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) override;
    virtual std::string getName() const override { return "RandomAsynchronous"; }
};

/*!
 * @brief Atualização sequencial: células são atualizadas em ordem determinística
 * (linha a linha, coluna a coluna), com efeito imediato.
 */
class SequentialUpdate : public UpdatePolicy {
public:
    virtual ~SequentialUpdate() = default;

    virtual void execute(Lattice* lattice, LocalRule* rule, Neighborhood* neighborhood) override;
    virtual std::string getName() const override { return "Sequential"; }
};
