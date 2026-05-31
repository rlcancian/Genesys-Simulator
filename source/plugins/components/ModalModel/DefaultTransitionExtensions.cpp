#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "kernel/simulator/Model.h"

EFSMTransition::EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	setTransitionKind(TransitionKind::DETERMINISTIC);
}

void EFSMTransition::setTriggerEvent(std::string triggerEvent) {
	_triggerEvent = triggerEvent;
}

std::string EFSMTransition::getTriggerEvent() const {
	return _triggerEvent;
}

void EFSMTransition::setProbabilityExpression(std::string probabilityExpression) {
	_probabilityExpression = probabilityExpression;
}

std::string EFSMTransition::getProbabilityExpression() const {
	return _probabilityExpression;
}

bool EFSMTransition::canFire(Model* model, Entity* entity) const {
	bool parentCanFire = DefaultNodeTransition::canFire(model, entity);
	if (!parentCanFire) {
		return false;
	}
	if (_probabilityExpression != "") {
		double p = model->parseExpression(_probabilityExpression);
		return p > 0.0;
	}
	return true;
}

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}

PetriTransition::PetriTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	//setTransitionKind(TransitionKind::PETRI);
}

void PetriTransition::setInputArcWeight(PetriPlace* place, std::string color, unsigned int weight) {
    if (place != nullptr) {
        _inputPlaces[place][color] = weight;
    }

}
unsigned int PetriTransition::getInputArcWeight(PetriPlace* place, std::string color) {
    auto placeIt = _inputPlaces.find(place);
    if (placeIt != _inputPlaces.end()) {
        auto colorIt = placeIt->second.find(color);
        if (colorIt != placeIt->second.end()) {
            return colorIt->second;
        }
    }
    return 0;
}
void PetriTransition::setOutputArcWeight(PetriPlace* place, std::string color, unsigned int weight) {
    if (place != nullptr) {
        _outputPlaces[place][color] = weight;
    }
}

unsigned int PetriTransition::getOutputArcWeight(PetriPlace* place, std::string color) {
    auto placeIt = _outputPlaces.find(place);
    if (placeIt != _outputPlaces.end()) {
        auto colorIt = placeIt->second.find(color);
        if (colorIt != placeIt->second.end()) {
            return colorIt->second;
        }
    }
    return 0;
}

std::map<PetriPlace*, ColorWeightMap> PetriTransition::getInputPlaces() const {
    return _inputPlaces;
}

std::map<PetriPlace*, ColorWeightMap> PetriTransition::getOutputPlaces() const {
    return _outputPlaces;
}

bool PetriTransition::canFire(Model* model, Entity* entity) const {
    (void) model;
    (void) entity;

    for (const auto& placePair : _inputPlaces) {
        PetriPlace* inputPlace = placePair.first;
        const auto& colorsAndWeights = placePair.second;

        if (inputPlace == nullptr) return false;

        for (const auto& colorWeightPair : colorsAndWeights) {
            std::string color = colorWeightPair.first;
            unsigned int requiredWeight = colorWeightPair.second;

            if (inputPlace->getTokens(color) < requiredWeight) {
                return false; // falta token de alguma cor
            }
        }
    }
    return true; // todos os lugares de entrada estão satisfeitos
}

void PetriTransition::execute(Model* model, Entity* entity) const {
	(void) model;
	(void) entity;

    for (const auto& placePair : _inputPlaces) {
        PetriPlace* inputPlace = placePair.first;
        const auto& colorsAndWeights = placePair.second;

        if (inputPlace == nullptr) continue;

        for (const auto& colorWeightPair : colorsAndWeights) {
            inputPlace->removeTokens(colorWeightPair.second, colorWeightPair.first);
        }
    }

    for (const auto& placePair : _outputPlaces) {
        PetriPlace* outputPlace = placePair.first;
        const auto& colorsAndWeights = placePair.second;

        if (outputPlace == nullptr) continue;

        for (const auto& colorWeightPair : colorsAndWeights) {
            outputPlace->addTokens(colorWeightPair.second, colorWeightPair.first);
        }
    }
}

void PetriTransition::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
    // salvando os Lugares de Entrada
    fields->saveField("inputPlacesCount", _inputPlaces.size(), 0u, saveDefaultValues);
    unsigned int i = 0;
    for (const auto& placePair : _inputPlaces) {
        PetriPlace* place = placePair.first;
        fields->saveField("inputPlaceId_" + Util::StrIndex(i), place->getId());

        const auto& colorsAndWeights = placePair.second;
        fields->saveField("inputPlaceColorsCount_" + Util::StrIndex(i), colorsAndWeights.size(), 0u, saveDefaultValues);

        unsigned int j = 0;
        for (const auto& colorWeightPair : colorsAndWeights) {
            fields->saveField("inputPlace_" + Util::StrIndex(i) + "_color_" + Util::StrIndex(j), colorWeightPair.first, "default", saveDefaultValues);
            fields->saveField("inputPlace_" + Util::StrIndex(i) + "_weight_" + Util::StrIndex(j), colorWeightPair.second, 0u, saveDefaultValues);
            j++;
        }
        i++;
    }

    // salvando os Lugares de Saída
    fields->saveField("outputPlacesCount", _outputPlaces.size(), 0u, saveDefaultValues);
    unsigned int k = 0;
    for (const auto& placePair : _outputPlaces) {
        PetriPlace* place = placePair.first;
        fields->saveField("outputPlaceId_" + Util::StrIndex(k), place->getId());

        const auto& colorsAndWeights = placePair.second;
        fields->saveField("outputPlaceColorsCount_" + Util::StrIndex(k), colorsAndWeights.size(), 0u, saveDefaultValues);

        unsigned int l = 0;
        for (const auto& colorWeightPair : colorsAndWeights) {
            fields->saveField("outputPlace_" + Util::StrIndex(k) + "_color_" + Util::StrIndex(l), colorWeightPair.first, "default", saveDefaultValues);
            fields->saveField("outputPlace_" + Util::StrIndex(k) + "_weight_" + Util::StrIndex(l), colorWeightPair.second, 0u, saveDefaultValues);
            l++;
        }
        k++;
    }
}

bool PetriTransition::_loadInstance(PersistenceRecord *fields, Model* model) {
    bool res = true;
    _inputPlaces.clear();
    _outputPlaces.clear();

    // carregando Lugares de Entrada
    unsigned int inputCount = fields->loadField("inputPlacesCount", 0u);
    for (unsigned int i = 0; i < inputCount; i++) {
        Util::identification id = fields->loadField("inputPlaceId_" + Util::StrIndex(i), 0);

        // busca o PetriPlace pelo ID usando a infraestrutura do Model
        ModelComponent* comp = model->getComponentManager()->find(id);
        PetriPlace* place = dynamic_cast<PetriPlace*>(comp);

        if (place != nullptr) {
            unsigned int colorsCount = fields->loadField("inputPlaceColorsCount_" + Util::StrIndex(i), 0u);
            for (unsigned int j = 0; j < colorsCount; j++) {
                std::string color = fields->loadField("inputPlace_" + Util::StrIndex(i) + "_color_" + Util::StrIndex(j), "default");
                unsigned int weight = fields->loadField("inputPlace_" + Util::StrIndex(i) + "_weight_" + Util::StrIndex(j), 0u);
                _inputPlaces[place][color] = weight;
            }
        }
    }

    // carregando Lugares de Saída (Lógica análoga)
    unsigned int outputCount = fields->loadField("outputPlacesCount", 0u);
    for (unsigned int k = 0; k < outputCount; k++) {
        Util::identification id = fields->loadField("outputPlaceId_" + Util::StrIndex(k), 0);

        ModelComponent* comp = model->getComponentManager()->find(id);
        PetriPlace* place = dynamic_cast<PetriPlace*>(comp);

        if (place != nullptr) {
            unsigned int colorsCount = fields->loadField("outputPlaceColorsCount_" + Util::StrIndex(k), 0u);
            for (unsigned int l = 0; l < colorsCount; l++) {
                std::string color = fields->loadField("outputPlace_" + Util::StrIndex(k) + "_color_" + Util::StrIndex(l), "default");
                unsigned int weight = fields->loadField("outputPlace_" + Util::StrIndex(k) + "_weight_" + Util::StrIndex(l), 0u);
                _outputPlaces[place][color] = weight;
            }
        }
    }

    return res;
}
