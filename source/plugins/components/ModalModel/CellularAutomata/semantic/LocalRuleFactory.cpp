#include "LocalRuleFactory.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "kernel/simulator/Persistence.h"

std::unordered_map<std::string, LocalRuleFactory::CreatorFunc>& LocalRuleFactory::getRegistry() {
    static std::unordered_map<std::string, CreatorFunc> registry;
    return registry;
}

void LocalRuleFactory::registerRule(const std::string& name, CreatorFunc creator) {
    getRegistry()[name] = std::move(creator);
}

std::unique_ptr<LocalRule> LocalRuleFactory::create(const std::string& name, CellularAutomataBase* parent) {
    auto& registry = getRegistry();
    auto it = registry.find(name);
    if (it != registry.end()) {
        return it->second(parent);
    }
    return nullptr;
}

std::unique_ptr<LocalRule> LocalRuleFactory::createFromPersistence(PersistenceRecord* fields, CellularAutomataBase* parent) {
    if (fields == nullptr) {
        return nullptr;
    }

    std::string ruleType = fields->loadField("ca.semantic.ruleType", std::string(""));
    if (ruleType.empty()) {
        return nullptr;
    }

    auto rule = create(ruleType, parent);
    if (rule != nullptr) {
        // Tenta carregar estado específico da regra
        // Cada regra concreta pode sobrescrever este comportamento
        // via _loadInstance se necessário
    }

    return rule;
}

bool LocalRuleFactory::isRegistered(const std::string& name) {
    return getRegistry().find(name) != getRegistry().end();
}

std::vector<std::string> LocalRuleFactory::getRegisteredRules() {
    std::vector<std::string> names;
    for (const auto& pair : getRegistry()) {
        names.push_back(pair.first);
    }
    return names;
}
