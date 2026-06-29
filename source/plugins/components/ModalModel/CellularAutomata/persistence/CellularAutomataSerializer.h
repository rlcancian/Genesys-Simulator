#pragma once

#include "CellularAutomataMemento.h"
#include "kernel/simulator/Persistence.h"

class Lattice;
class Neighborhood;
class LocalRule;
class CellularAutomataBase;

/*!
 * @brief Serializa e deserializa o estado de um autômato celular.
 *
 * Responsável por converter entre o estado interno do autômato e o
 * PersistenceRecord do kernel GenESyS, permitindo salvar e restaurar
 * simulações completas.
 */
class CellularAutomataSerializer {
public:
    /*!
     * @brief Salva o estado completo do autômato em um PersistenceRecord.
     * @param lattice O lattice (camada espacial)
     * @param neighborhood A vizinhança (camada relacional)
     * @param localRule A regra local (camada semântica)
     * @param automaton O autômato (camada temporal)
     * @param fields O record de persistência a ser preenchido
     */
    static void save(const Lattice* lattice,
                     const Neighborhood* neighborhood,
                     const LocalRule* localRule,
                     const CellularAutomataBase* automaton,
                     PersistenceRecord* fields);

    /*!
     * @brief Carrega o estado de um PersistenceRecord para um Memento.
     * @param fields O record de persistência a ser lido
     * @return Memento preenchido com o estado carregado
     */
    static CellularAutomataMemento load(PersistenceRecord* fields);

private:
    // Prefixos para namespaces nos campos persistidos
    static constexpr const char* SPATIAL_PREFIX = "ca.spatial.";
    static constexpr const char* RELATIONAL_PREFIX = "ca.relational.";
    static constexpr const char* SEMANTIC_PREFIX = "ca.semantic.";
    static constexpr const char* TEMPORAL_PREFIX = "ca.temporal.";

    static void saveSpatial(const Lattice* lattice, PersistenceRecord* fields);
    static void saveRelational(const Neighborhood* neighborhood, PersistenceRecord* fields);
    static void saveSemantic(const LocalRule* localRule, PersistenceRecord* fields);
    static void saveTemporal(const CellularAutomataBase* automaton, PersistenceRecord* fields);

    static void loadSpatial(PersistenceRecord* fields, CellularAutomataMemento& memento);
    static void loadRelational(PersistenceRecord* fields, CellularAutomataMemento& memento);
    static void loadSemantic(PersistenceRecord* fields, CellularAutomataMemento& memento);
    static void loadTemporal(PersistenceRecord* fields, CellularAutomataMemento& memento);

    // Helpers para serialização de vetores
    static std::string serializeDimensions(const std::vector<unsigned short>& dims);
    static std::vector<unsigned short> deserializeDimensions(const std::string& str);
    static std::string serializeCellStates(const std::vector<long>& states);
    static std::vector<long> deserializeCellStates(const std::string& str);
    static std::string serializeLongVector(const std::vector<long>& values);
    static std::vector<long> deserializeLongVector(const std::string& str);
};
