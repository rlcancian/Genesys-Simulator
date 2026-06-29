#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/*!
 * @brief Captura o estado completo de um autômato celular para serialização.
 *
 * Estrutura que armazena o estado de cada camada (espacial, relacional,
 * semântica e temporal) permitindo persistência e reconstrução do autômato.
 */
class CellularAutomataMemento {
public:
    //! Estado da camada espacial (lattice, células, estados)
    struct SpatialState {
        std::vector<unsigned short> dimensions;
        std::vector<long> cellStates;
        int stateSetType = 1;  // StateSetType::ENUMERATED
        std::vector<long> availableStates;
    };

    //! Estado da camada relacional (vizinhança, contorno)
    struct RelationalState {
        int neighborhoodType = 4;  // NeighboorhoodType::VONNEUMANN
        int boundaryType = 1;      // BoundaryType::FIXED
        unsigned short radius = 1;
        bool includeCellItself = false;
        long fixedBoundaryState = 0;
    };

    //! Estado da camada semântica (regra local e parâmetros)
    struct SemanticState {
        int ruleType = 2;  // LocalRuleType::GAME_OF_LIFE
        std::unordered_map<std::string, double> ruleParameters;
        std::unordered_map<std::string, std::string> ruleStringParameters;
    };

    //! Estado da camada temporal (tempo, política de sincronismo)
    struct TemporalState {
        unsigned int simulatedTime = 0;
        int cellularAutomataType = 1;  // CellularAutomataType::CLASSIC
        int syncPolicy = 0;            // 0 = síncrono, 1 = assíncrono
    };

    SpatialState spatial;
    RelationalState relational;
    SemanticState semantic;
    TemporalState temporal;

    //! Versão do formato para compatibilidade futura
    static constexpr int FORMAT_VERSION = 1;
};
