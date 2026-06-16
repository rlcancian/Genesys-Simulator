#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "kernel/simulator/Model.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include <vector>
#include <algorithm>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &ModalModelPetriNet::GetPluginInformation;
}
#endif

ModalModelPetriNet::ModalModelPetriNet(Model* model, std::string name) : ModalModelDefault(model, name) {
}

PluginInformation* ModalModelPetriNet::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelPetriNet>(), &ModalModelPetriNet::LoadInstance, &ModalModelPetriNet::NewInstance);
    info->setCategory("ModalModel");
    info->setDescriptionHelp("Specialization of ModalModelDefault for colored Petri net style models.");
    return info;
}

ModelComponent* ModalModelPetriNet::LoadInstance(Model* model, PersistenceRecord *fields) {
    ModalModelPetriNet* component = new ModalModelPetriNet(model);
    component->_loadInstance(fields);
    return component;
}

ModelDataDefinition* ModalModelPetriNet::NewInstance(Model* model, std::string name) {
    return new ModalModelPetriNet(model, name);
}

bool ModalModelPetriNet::_check(std::string& errorMessage) {
    bool resultAll = true;
    resultAll &= ModalModelDefault::_check(errorMessage);

    // validate the existence of nodes
    if (getNodes()->size() == 0) {
        errorMessage += "ModalModelPetriNet requires at least one place/transition node. ";
        resultAll = false;
    }

    // validate the existence of transitions
    if (_transitions->size() == 0) {
        errorMessage += "The Petri Net does not have any transitions (dead net). ";
        resultAll = false;
    }

    for (auto transComponent : *getTransitions()->list()) {
        PetriTransition* pTrans = dynamic_cast<PetriTransition*>(transComponent);
        if (pTrans == nullptr) {
            errorMessage += "A non-Petri element was inserted into the transitions list. ";
            resultAll = false;
            continue;
        }
        auto inputPlaces = pTrans->getInputPlaces();
        auto outputPlaces = pTrans->getOutputPlaces();

        // check for isolated transitions (no inputs OR no outputs)
        if (inputPlaces.empty()) {
            errorMessage += "Transition '" + pTrans->getName() + "' is isolated (no input places). ";
            resultAll = false;
        }
        if (outputPlaces.empty()) {
            errorMessage += "Transition '" + pTrans->getName() + "' is isolated (no output places). ";
            resultAll = false;
        }

        // validation of input arcs
        for (const auto& inPair : inputPlaces) {
            PetriPlace* place = inPair.first;
            const auto& colorsAndWeights = inPair.second;
            if (place == nullptr) { // block if the origin place is null
                errorMessage += "Transition '" + pTrans->getName() + "' has a null/invalid input place. ";
                resultAll = false;
                continue;
            }
            for (const auto& cwPair : colorsAndWeights) {
                if (cwPair.first == "") { // block if there are empty colors
                    errorMessage += "Transition '" + pTrans->getName() + "' has an empty color string for place '" + place->getName() + "'. ";
                    resultAll = false;
                }
                if (cwPair.second == 0) { // block if there are zero weights
                    errorMessage += "Transition '" + pTrans->getName() + "' requires 0 tokens of color '" + cwPair.first + "' from place '" + place->getName() + "' (Weight cannot be 0). ";
                    resultAll = false;
                }
            }
        }

        // validation of output arcs
        for (const auto& outPair : outputPlaces) {
            PetriPlace* place = outPair.first;
            const auto& colorsAndWeights = outPair.second;
            if (place == nullptr) { // block if the destination place is null
                errorMessage += "Transition '" + pTrans->getName() + "' has a null/invalid output place. ";
                resultAll = false;
                continue;
            }
            for (const auto& cwPair : colorsAndWeights) {
                if (cwPair.first == "") { // block if producing empty colors
                    errorMessage += "Transition '" + pTrans->getName() + "' produces an empty color string for place '" + place->getName() + "'. ";
                    resultAll = false;
                }
                if (cwPair.second == 0) { // block if producing zero weights
                    errorMessage += "Transition '" + pTrans->getName() + "' produces 0 tokens of color '" + cwPair.first + "' for place '" + place->getName() + "' (Weight cannot be 0). ";
                    resultAll = false;
                }
            }
        }
    }

    return resultAll;
}

void ModalModelPetriNet::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    (void)inputPortNumber;
    unsigned int transitionsFired = 0;

    // lambda function to print the current network marking
    auto printTrace = [&]() {
        traceSimulation(this, ">>> [CPN Trace] Current Petri Net marking <<<", TraceManager::Level::L7_internal);
        for (DefaultNode* node : *getNodes()->list()) {
            PetriPlace* pPlace = dynamic_cast<PetriPlace*>(node);
            if (pPlace != nullptr) {
                std::string marking = "  Place '" + pPlace->getName() + "': ";
                auto tokens = pPlace->getAllTokens();
                bool hasTokens = false;
                for (const auto& kv : *tokens) {
                    if (kv.second > 0) hasTokens = true;
                }
                if (!hasTokens) {
                    marking += "[Empty]";
                } else {
                    bool first = true;
                    for (const auto& kv : *tokens) {
                        if (kv.second > 0) {
                            if (!first) marking += ", ";
                            marking += std::to_string(kv.second) + " token(s) of color '" + kv.first + "'";
                            first = false;
                        }
                    }
                }
                traceSimulation(this, marking, TraceManager::Level::L7_internal);
            }
        }
        traceSimulation(this, ">>> ------------------------------------------- <<<", TraceManager::Level::L7_internal);
    };

    // loop that fires transitions respecting the configured maximum limit
    while (transitionsFired < _maxTransitionsPerDispatch) {
        std::vector<DefaultNodeTransition*> enabled;
        for (DefaultNodeTransition* transition : *_transitions->list()) {
            if (transition->canFire(_parentModel, entity)) {
                enabled.push_back(transition);
            }
        }
        if (enabled.size() == 0) {
            traceSimulation(this, "No transition is enabled to fire (lack of tokens or end of flow).", TraceManager::Level::L7_internal);
            // printTrace(); // commented out to avoid log pollution
            break;
        }

        // if priorities are equal, the first transition inserted in 'enabled' will be fired
        std::sort(enabled.begin(), enabled.end(), [](DefaultNodeTransition* a, DefaultNodeTransition* b) {
            return a->getPriority() < b->getPriority();
        });

        DefaultNodeTransition* chosen = enabled.front();
        PetriTransition* pChosen = dynamic_cast<PetriTransition*>(chosen); // ensure it is a Petri transition
        if (pChosen == nullptr) break;

        // network inspection before firing
        for (auto entry : pChosen->getInputPlaces()) {
            PetriPlace* place = entry.first;
            for (auto pairColorWeight : entry.second) {
                if (place->getTokens(pairColorWeight.first) < pairColorWeight.second) {
                    traceError("Transition attempted to fire without sufficient balance in " + place->getName());
                }
            }
        }

        chosen->execute(_parentModel, entity);
        traceSimulation(this, "Transition '" + chosen->getName() + "' successfully fired.", TraceManager::Level::L7_internal);
        transitionsFired++;
        printTrace();

        // network inspection after firing
        for (auto saida : pChosen->getOutputPlaces()) {
            PetriPlace* place = saida.first;
            for (auto parCorPeso : saida.second) {
                if (place->getTokens(parCorPeso.first) == 0 && parCorPeso.second > 0) {
                    traceError("Token not deposited in " + place->getName());
                }
            }
        }

    }
    traceSimulation(this, "Petri Net simulation completed. " + std::to_string(transitionsFired) + " transition(s) fired. Moving on...", TraceManager::Level::L7_internal);

    double waitTime = _parentModel->parseExpression(_timeDelayExpressionPerDispatch);
    Util::TimeUnit stu = _parentModel->getSimulation()->getReplicationBaseTimeUnit();
    waitTime *= Util::TimeUnitConvert(_timeDelayPerDispatchTimeUnit, stu);

    _parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection(), waitTime);
}

void ModalModelPetriNet::_initBetweenReplications() {
    traceSimulation(this, ">>> [Tracing] Initial Petri Net marking (M0) <<<", TraceManager::Level::L7_internal);
    for (DefaultNode* node : *getNodes()->list()) {
        PetriPlace* pPlace = dynamic_cast<PetriPlace*>(node);
        if (pPlace != nullptr) {
            std::string marking = "  Place '" + pPlace->getName() + "': ";
            auto tokens = pPlace->getAllTokens();
            bool hasTokens = false;
            for (const auto& token : *tokens) {
                if (token.second > 0) hasTokens = true;
            }
            if (!hasTokens) {
                marking += "[Empty]";
            } else {
                bool firstColor = true;
                for (const auto& token : *tokens) {
                    if (token.second > 0) {
                        if (!firstColor) marking += ", ";
                        marking += std::to_string(token.second) + " token(s) of color '" + token.first + "'";
                        firstColor = false;
                    }
                }
            }
            traceSimulation(this, marking, TraceManager::Level::L7_internal);
        }
    }
    traceSimulation(this, ">>> ------------------------------------------- <<<", TraceManager::Level::L7_internal);
}
