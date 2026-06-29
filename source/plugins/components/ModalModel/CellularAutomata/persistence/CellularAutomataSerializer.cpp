#include "CellularAutomataSerializer.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

#include <sstream>
#include <algorithm>

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void CellularAutomataSerializer::save(const Lattice* lattice,
                                      const Neighborhood* neighborhood,
                                      const LocalRule* localRule,
                                      const CellularAutomataBase* automaton,
                                      PersistenceRecord* fields) {
    if (fields == nullptr) {
        return;
    }

    fields->saveField("ca.formatVersion", CellularAutomataMemento::FORMAT_VERSION);

    saveSpatial(lattice, fields);
    saveRelational(neighborhood, fields);
    saveSemantic(localRule, fields);
    saveTemporal(automaton, fields);
}

CellularAutomataMemento CellularAutomataSerializer::load(PersistenceRecord* fields) {
    CellularAutomataMemento memento;

    if (fields == nullptr) {
        return memento;
    }

    int version = static_cast<int>(fields->loadField("ca.formatVersion", 1));
    if (version > CellularAutomataMemento::FORMAT_VERSION) {
        // Versão futura - não compatível
        return memento;
    }

    loadSpatial(fields, memento);
    loadRelational(fields, memento);
    loadSemantic(fields, memento);
    loadTemporal(fields, memento);

    return memento;
}

// ============================================================================
// SPATIAL LAYER SERIALIZATION
// ============================================================================

void CellularAutomataSerializer::saveSpatial(const Lattice* lattice, PersistenceRecord* fields) {
    if (lattice == nullptr) {
        return;
    }

    std::vector<unsigned short> dimensions = lattice->getDimensions();
    fields->saveField(std::string(SPATIAL_PREFIX) + "dimensions",
                      serializeDimensions(dimensions));

    fields->saveField(std::string(SPATIAL_PREFIX) + "latticeType",
                      static_cast<int>(lattice->getLatticeType()),
                      static_cast<int>(LatticeType::RETICULAR));

    fields->saveField(std::string(SPATIAL_PREFIX) + "totalCells",
                      static_cast<unsigned int>(lattice->getTotalCells()));

    // Serializa estados das células
    std::vector<long> cellStates;
    unsigned long cellsSize = lattice->getCellsSize();
    cellStates.reserve(cellsSize);
    for (unsigned long i = 0; i < cellsSize; ++i) {
        const Cell* cell = lattice->getCell(static_cast<long>(i));
        if (cell != nullptr) {
            cellStates.push_back(cell->getCurrentState().getValue());
        } else {
            cellStates.push_back(0);
        }
    }
    fields->saveField(std::string(SPATIAL_PREFIX) + "cellStates",
                      serializeCellStates(cellStates));

    // Serializa estados disponíveis (StateSet)
    const CellularAutomataBase* parent = lattice->getParentCellularAutomata();
    if (parent != nullptr) {
        const StateSet* stateSet = parent->getStateSet();
        if (stateSet != nullptr) {
            // Tenta obter estados enumerados se disponível
            const StateSet_Enumerable* enumerable = dynamic_cast<const StateSet_Enumerable*>(stateSet);
            if (enumerable != nullptr) {
                std::vector<long> availableStates;
                for (unsigned int i = 0; i < enumerable->getStatesSize(); ++i) {
                    const State* s = enumerable->getState(i);
                    if (s != nullptr) {
                        availableStates.push_back(s->getValue());
                    }
                }
                fields->saveField(std::string(SPATIAL_PREFIX) + "availableStates",
                                  serializeLongVector(availableStates));
            }
            fields->saveField(std::string(SPATIAL_PREFIX) + "stateSetType", 1); // ENUMERATED
        }
    }
}

void CellularAutomataSerializer::loadSpatial(PersistenceRecord* fields, CellularAutomataMemento& memento) {
    std::string key = std::string(SPATIAL_PREFIX) + "dimensions";
    if (fields->find(key) != fields->end()) {
        memento.spatial.dimensions = deserializeDimensions(fields->loadField(key));
    }

    memento.spatial.stateSetType = static_cast<int>(
        fields->loadField(std::string(SPATIAL_PREFIX) + "stateSetType", 1));

    key = std::string(SPATIAL_PREFIX) + "cellStates";
    if (fields->find(key) != fields->end()) {
        memento.spatial.cellStates = deserializeCellStates(fields->loadField(key));
    }

    key = std::string(SPATIAL_PREFIX) + "availableStates";
    if (fields->find(key) != fields->end()) {
        memento.spatial.availableStates = deserializeLongVector(fields->loadField(key));
    }
}

// ============================================================================
// RELATIONAL LAYER SERIALIZATION
// ============================================================================

void CellularAutomataSerializer::saveRelational(const Neighborhood* neighborhood, PersistenceRecord* fields) {
    if (neighborhood == nullptr) {
        return;
    }

    fields->saveField(std::string(RELATIONAL_PREFIX) + "radius",
                      static_cast<unsigned int>(neighborhood->getRadius()), 1u);
    fields->saveField(std::string(RELATIONAL_PREFIX) + "includeCellItself",
                      static_cast<int>(neighborhood->getIncludeCellItself()), 0);

    // Nome da vizinhança (para reconstrução)
    fields->saveField(std::string(RELATIONAL_PREFIX) + "name", neighborhood->getName());

    // Boundary condition
    const BoundaryCondition* boundary = neighborhood->getBoundary();
    if (boundary != nullptr) {
        fields->saveField(std::string(RELATIONAL_PREFIX) + "boundaryName",
                          boundary->getName());
    }
}

void CellularAutomataSerializer::loadRelational(PersistenceRecord* fields, CellularAutomataMemento& memento) {
    memento.relational.radius = static_cast<unsigned short>(
        fields->loadField(std::string(RELATIONAL_PREFIX) + "radius", 1u));
    memento.relational.includeCellItself = static_cast<bool>(
        fields->loadField(std::string(RELATIONAL_PREFIX) + "includeCellItself", 0));
}

// ============================================================================
// SEMANTIC LAYER SERIALIZATION
// ============================================================================

void CellularAutomataSerializer::saveSemantic(const LocalRule* localRule, PersistenceRecord* fields) {
    if (localRule == nullptr) {
        return;
    }

    // O tipo da regra é salvo pelo componente (via enum LocalRuleType)
    // Aqui salvamos o nome da regra e parâmetros específicos
    fields->saveField(std::string(SEMANTIC_PREFIX) + "ruleName", localRule->getName());
}

void CellularAutomataSerializer::loadSemantic(PersistenceRecord* fields, CellularAutomataMemento& memento) {
    // Parâmetros específicos são carregados por cada regra concreta
    // O tipo base é restaurado pelo componente
}

// ============================================================================
// TEMPORAL LAYER SERIALIZATION
// ============================================================================

void CellularAutomataSerializer::saveTemporal(const CellularAutomataBase* automaton, PersistenceRecord* fields) {
    if (automaton == nullptr) {
        return;
    }

    // CellularAutomataBase não expõe simulatedTime diretamente
    // A subclasse CellularAutomata tem, mas usamos o base aqui
    // O tempo é gerenciado pelo componente
    fields->saveField(std::string(TEMPORAL_PREFIX) + "cellularAutomataType",
                      static_cast<int>(1)); // CLASSIC
    fields->saveField(std::string(TEMPORAL_PREFIX) + "syncPolicy",
                      static_cast<int>(0)); // Síncrono
}

void CellularAutomataSerializer::loadTemporal(PersistenceRecord* fields, CellularAutomataMemento& memento) {
    memento.temporal.cellularAutomataType = static_cast<int>(
        fields->loadField(std::string(TEMPORAL_PREFIX) + "cellularAutomataType", 1));
    memento.temporal.syncPolicy = static_cast<int>(
        fields->loadField(std::string(TEMPORAL_PREFIX) + "syncPolicy", 0));
}

// ============================================================================
// SERIALIZATION HELPERS
// ============================================================================

std::string CellularAutomataSerializer::serializeDimensions(const std::vector<unsigned short>& dims) {
    std::ostringstream oss;
    for (size_t i = 0; i < dims.size(); ++i) {
        if (i > 0) oss << ",";
        oss << dims[i];
    }
    return oss.str();
}

std::vector<unsigned short> CellularAutomataSerializer::deserializeDimensions(const std::string& str) {
    std::vector<unsigned short> result;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (!token.empty()) {
            result.push_back(static_cast<unsigned short>(std::stoi(token)));
        }
    }
    return result;
}

std::string CellularAutomataSerializer::serializeCellStates(const std::vector<long>& states) {
    std::ostringstream oss;
    for (size_t i = 0; i < states.size(); ++i) {
        if (i > 0) oss << ",";
        oss << states[i];
    }
    return oss.str();
}

std::vector<long> CellularAutomataSerializer::deserializeCellStates(const std::string& str) {
    return deserializeLongVector(str);
}

std::string CellularAutomataSerializer::serializeLongVector(const std::vector<long>& values) {
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << ",";
        oss << values[i];
    }
    return oss.str();
}

std::vector<long> CellularAutomataSerializer::deserializeLongVector(const std::string& str) {
    std::vector<long> result;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (!token.empty()) {
            result.push_back(std::stol(token));
        }
    }
    return result;
}
