#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "../../../kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include <vector>
#include <algorithm>
#include <string>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelPetriNet::GetPluginInformation;
}
#endif

ModalModelPetriNet::ModalModelPetriNet(Model* model, std::string name) : ModalModelDefault(model, name) {
    _typename = Util::TypeOf<ModalModelPetriNet>();
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

void ModalModelPetriNet::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
    // saves generic data from the parent class (including nodes and basic transition connections)
    ModalModelDefault::_saveInstance(fields, saveDefaultValues);

    // saves the detailed colors and weights for each petri transition
    unsigned int tIndex = 0;
    for (auto transComponent : *getTransitions()->list()) {
        PetriTransition* pTrans = dynamic_cast<PetriTransition*>(transComponent);
        if (pTrans != nullptr) {

            // saves input places
            auto inputs = pTrans->getInputPlaces();
            fields->saveField("t_" + std::to_string(tIndex) + "_inCount", inputs.size(), 0u, saveDefaultValues);
            unsigned int i = 0;
            for(const auto& pair : inputs) {
                fields->saveField("t_" + std::to_string(tIndex) + "_inPlace_" + std::to_string(i), pair.first->getId());
                fields->saveField("t_" + std::to_string(tIndex) + "_inColorCount_" + std::to_string(i), pair.second.size(), 0u, saveDefaultValues);
                unsigned int j = 0;
                for(const auto& cw : pair.second) {
                    fields->saveField("t_" + std::to_string(tIndex) + "_inCol_" + std::to_string(i) + "_" + std::to_string(j), cw.first, "default", saveDefaultValues);
                    fields->saveField("t_" + std::to_string(tIndex) + "_inWgt_" + std::to_string(i) + "_" + std::to_string(j), cw.second, 0u, saveDefaultValues);
                    j++;
                }
                i++;
            }

            // saves output places
            auto outputs = pTrans->getOutputPlaces();
            fields->saveField("t_" + std::to_string(tIndex) + "_outCount", outputs.size(), 0u, saveDefaultValues);
            unsigned int k = 0;
            for(const auto& pair : outputs) {
                fields->saveField("t_" + std::to_string(tIndex) + "_outPlace_" + std::to_string(k), pair.first->getId());
                fields->saveField("t_" + std::to_string(tIndex) + "_outColorCount_" + std::to_string(k), pair.second.size(), 0u, saveDefaultValues);
                unsigned int l = 0;
                for(const auto& cw : pair.second) {
                    fields->saveField("t_" + std::to_string(tIndex) + "_outCol_" + std::to_string(k) + "_" + std::to_string(l), cw.first, "default", saveDefaultValues);
                    fields->saveField("t_" + std::to_string(tIndex) + "_outWgt_" + std::to_string(k) + "_" + std::to_string(l), cw.second, 0u, saveDefaultValues);
                    l++;
                }
                k++;
            }
        }
        tIndex++;
    }
}

bool ModalModelPetriNet::_loadInstance(PersistenceRecord *fields) {
    // the parent class loads the net but creates generic transitions (defaultnodetransition)
    bool res = ModalModelDefault::_loadInstance(fields);

    if (res) {
        // stores the generic transitions and clears the current model list
        std::vector<DefaultNodeTransition*> oldTransitions;
        for (auto t : *_transitions->list()) {
            oldTransitions.push_back(t);
        }
        _transitions->clear();

        // recreates the transitions as petritransition and repopulates colors and weights
        unsigned int tIndex = 0;
        for (auto oldT : oldTransitions) {

            // promotes to petri transition keeping the same origin, destination, name, and priority
            PetriTransition* pTrans = new PetriTransition(oldT->getSource(), oldT->getDestination(), oldT->getName());
            pTrans->setPriority(oldT->getPriority());

            // restores input places and their colors/weights
            unsigned int inCount = fields->loadField("t_" + std::to_string(tIndex) + "_inCount", 0u);
            for (unsigned int i = 0; i < inCount; i++) {
                Util::identification id = fields->loadField("t_" + std::to_string(tIndex) + "_inPlace_" + std::to_string(i), 0);
                PetriPlace* place = dynamic_cast<PetriPlace*>(_parentModel->getComponentManager()->find(id));

                unsigned int colCount = fields->loadField("t_" + std::to_string(tIndex) + "_inColorCount_" + std::to_string(i), 0u);
                for (unsigned int j = 0; j < colCount; j++) {
                    std::string col = fields->loadField("t_" + std::to_string(tIndex) + "_inCol_" + std::to_string(i) + "_" + std::to_string(j), "default");
                    unsigned int wgt = fields->loadField("t_" + std::to_string(tIndex) + "_inWgt_" + std::to_string(i) + "_" + std::to_string(j), 0u);
                    if (place != nullptr) pTrans->setInputArcWeight(place, col, wgt);
                }
            }

            // restores output places and their colors/weights
            unsigned int outCount = fields->loadField("t_" + std::to_string(tIndex) + "_outCount", 0u);
            for (unsigned int k = 0; k < outCount; k++) {
                Util::identification id = fields->loadField("t_" + std::to_string(tIndex) + "_outPlace_" + std::to_string(k), 0);
                PetriPlace* place = dynamic_cast<PetriPlace*>(_parentModel->getComponentManager()->find(id));

                unsigned int colCount = fields->loadField("t_" + std::to_string(tIndex) + "_outColorCount_" + std::to_string(k), 0u);
                for (unsigned int l = 0; l < colCount; l++) {
                    std::string col = fields->loadField("t_" + std::to_string(tIndex) + "_outCol_" + std::to_string(k) + "_" + std::to_string(l), "default");
                    unsigned int wgt = fields->loadField("t_" + std::to_string(tIndex) + "_outWgt_" + std::to_string(k) + "_" + std::to_string(l), 0u);
                    if (place != nullptr) pTrans->setOutputArcWeight(place, col, wgt);
                }
            }

            // adds the true and finalized transition back to the net
            _transitions->insert(pTrans);
            tIndex++;
        }
    }
    return res;
}
bool ModalModelPetriNet::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= ModalModelDefault::_check(errorMessage);
	resultAll &= getNodes()->size() > 0;
	if (!resultAll) {
		errorMessage += "ModalModelPetriNet requires at least one place/transition node.";
	}
	return resultAll;
}

