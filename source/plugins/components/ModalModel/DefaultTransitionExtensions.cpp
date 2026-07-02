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
