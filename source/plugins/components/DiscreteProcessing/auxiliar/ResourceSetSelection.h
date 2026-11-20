#ifndef GENESYS_RESOURCE_SET_SELECTION_H
#define GENESYS_RESOURCE_SET_SELECTION_H

#include "SeizableItem.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <vector>

// Shared validation for the Seize/Release request boundary. Never convert an
// invalid floating-point expression to an unsigned index or quantity.
inline double resourceRequestExpression(Model* model, const std::string& expression,
                                        const std::string& property) {
    if (expression.empty() || expression.find('\0') != std::string::npos)
        throw std::runtime_error(property + " is empty or contains NUL");
    bool success = false;
    std::string error;
    const auto value = model->parseExpression(expression, success, error);
    if (!success) throw std::runtime_error(property + " [" + expression + "]: " + error);
    return value;
}

inline unsigned int resourceRequestQuantity(Model* model, const std::string& expression,
                                             const std::string& owner) {
    if (expression.empty() || expression.find('\0') != std::string::npos)
        throw std::runtime_error(owner + ".QuantityExpression is empty or contains NUL");
    const double value = resourceRequestExpression(model, expression, owner + ".QuantityExpression");
    if (!std::isfinite(value) || value < 0 || value != std::floor(value)
        || value > std::numeric_limits<unsigned int>::max())
        throw std::runtime_error(owner + ".QuantityExpression must evaluate to a nonnegative integer: " + expression);
    return static_cast<unsigned int>(value);
}

struct ResourceSetSelectionResult {
    Resource* resource;
    unsigned int index;
};

inline ResourceSetSelectionResult selectRequestResource(Model* model, SeizableItem* request,
                                                        bool releasing, const std::string& owner) {
    const auto fail = [&](const std::string& reason) -> void {
        throw std::runtime_error(owner + ": " + reason);
    };
    if (!request) fail("missing resource request");
    if (request->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
        auto* resource = request->getResource();
        if (!resource) fail("unresolved Resource");
        return {resource, 0};
    }
    auto* set = request->getSet();
    if (!set || request->getSeizableType() != SeizableItem::SeizableType::SET)
        fail("unresolved Resource Set");
    const auto size = set->getElementSet()->size();
    if (size == 0) fail("Resource Set \"" + set->getName() + "\" is empty");
    std::vector<Resource*> members;
    for (auto* datum : *set->getElementSet()->list()) {
        auto* resource = dynamic_cast<Resource*>(datum);
        if (!resource) fail("Set \"" + set->getName() + "\" contains a non-Resource member");
        members.push_back(resource);
    }
    unsigned int index = 0;
    switch (request->getSelectionRule()) {
    case SeizableItem::SelectionRule::CYCLICAL:
        index = (static_cast<unsigned long long>(request->getLastMemberSeized()) + 1) % size;
        break;
    case SeizableItem::SelectionRule::RANDOM:
        // Preserve the existing RNG stream, but map it into the Set, not the
        // request-list size. The upper endpoint is excluded.
        index = static_cast<unsigned int>((std::rand() / (RAND_MAX + 1.0)) * size);
        break;
    case SeizableItem::SelectionRule::SPECIFICMEMBER: {
        // Release's persisted contract consumes the attribute saved by Seize.
        const auto expression = releasing ? request->getSaveAttribute() : request->getIndex();
        if (expression.empty() || expression.find('\0') != std::string::npos)
            fail("SPECIFICMEMBER requires a valid " + std::string(releasing ? "SaveAttribute" : "Index"));
        const double value = resourceRequestExpression(model, expression, owner + "." + (releasing ? "SaveAttribute" : "Index"));
        if (!std::isfinite(value) || value < 0 || value != std::floor(value) || value >= size)
            fail("Set \"" + set->getName() + "\" index out of range: " + expression);
        index = static_cast<unsigned int>(value);
        break;
    }
    case SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY:
    case SeizableItem::SelectionRule::SMALLESTNUMBERBUSY: {
        bool found = false;
        double best = 0;
        for (unsigned int i = 0; i < members.size(); ++i) {
            const double value = request->getSelectionRule() == SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY
                ? static_cast<double>(members[i]->getCapacity()) - members[i]->getNumberBusy()
                : members[i]->getNumberBusy();
            if (!found || (request->getSelectionRule() == SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY
                               ? value > best : value < best)) {
                found = true; best = value; index = i;
            }
        }
        // A valid zero-capacity candidate is distinct from an absent candidate.
        if (!found) fail("no valid Resource candidate");
        break;
    }
    case SeizableItem::SelectionRule::PREFEREDORDER: {
        const auto quantity = resourceRequestQuantity(model, request->getQuantityExpression(), owner);
        std::vector<unsigned int> available;
        for (unsigned int i = 0; i < members.size(); ++i)
            if (static_cast<double>(members[i]->getCapacity()) - members[i]->getNumberBusy() >= quantity)
                available.push_back(i);
        if (available.empty()) {
            index = (static_cast<unsigned long long>(request->getLastPreferedOrder()) + 1) % size;
            request->setLastPreferedOrder(index);
        } else {
            const auto rank = request->getLastPreferedOrder() % available.size();
            index = available[rank];
            request->setLastPreferedOrder(rank + 1);
        }
        break;
    }
    default:
        fail("invalid Resource selection rule");
    }
    if (index >= members.size()) fail("Resource Set index out of range");
    return {members[index], index};
}
#endif
