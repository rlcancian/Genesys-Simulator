#include "LocalRule.h"
#include "kernel/simulator/Persistence.h"

bool LocalRule::_loadInstance(PersistenceRecord* fields) {
    // Implementação padrão vazia - derivadas podem sobrescrever
    return true;
}

void LocalRule::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    // Implementação padrão vazia - derivadas podem sobrescrever
}
