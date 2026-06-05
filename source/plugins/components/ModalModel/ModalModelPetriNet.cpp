#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "kernel/simulator/Model.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"

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

    // valida a existência de nós
    if (getNodes()->size() == 0) {
        errorMessage += "ModalModelPetriNet requires at least one place/transition node. ";
        resultAll = false;
    }

    for (auto transComponent : *getTransitions()->list()) {
        PetriTransition* pTrans = dynamic_cast<PetriTransition*>(transComponent);

        if (pTrans == nullptr) continue;

        auto inputPlaces = pTrans->getInputPlaces();
        auto outputPlaces = pTrans->getOutputPlaces();

        // bloqueio de transições isoladas (sem entradas e saídas)
        if (inputPlaces.empty() && outputPlaces.empty()) {
            errorMessage += "Transition '" + pTrans->getName() + "' is isolated (no inputs or outputs). ";
            resultAll = false;
        }

        // validação de arcos de entrada
        for (const auto& inPair : inputPlaces) {
            PetriPlace* place = inPair.first;
            const auto& colorsAndWeights = inPair.second;

            // barra se o lugar de origem for nulo
            if (place == nullptr) {
                errorMessage += "Transition '" + pTrans->getName() + "' has a null/invalid input place. ";
                resultAll = false;
                continue;
            }

            for (const auto& cwPair : colorsAndWeights) {
                // barra se houver cores vazias
                if (cwPair.first == "") {
                    errorMessage += "Transition '" + pTrans->getName() + "' has an empty color string for place '" + place->getName() + "'. ";
                    resultAll = false;
                }
                // barra se houver pesos vazios
                if (cwPair.second == 0) {
                    errorMessage += "Transition '" + pTrans->getName() + "' requires 0 tokens of color '" + cwPair.first + "' from place '" + place->getName() + "' (Weight cannot be 0). ";
                    resultAll = false;
                }
            }
        }

        // validação de arcos de saída
        for (const auto& outPair : outputPlaces) {
            PetriPlace* place = outPair.first;
            const auto& colorsAndWeights = outPair.second;

            // barra se o lugar de destino for nulo
            if (place == nullptr) {
                errorMessage += "Transition '" + pTrans->getName() + "' has a null/invalid output place. ";
                resultAll = false;
                continue;
            }

            for (const auto& cwPair : colorsAndWeights) {
                // barra se produzir cores vazias
                if (cwPair.first == "") {
                    errorMessage += "Transition '" + pTrans->getName() + "' produces an empty color string for place '" + place->getName() + "'. ";
                    resultAll = false;
                }
                // barra se produzir pesos vazios
                if (cwPair.second == 0) {
                    errorMessage += "Transition '" + pTrans->getName() + "' produces 0 tokens of color '" + cwPair.first + "' for place '" + place->getName() + "' (Weight cannot be 0). ";
                    resultAll = false;
                }
            }
        }
    }

    return resultAll;
}
