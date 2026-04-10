#ifndef PROPERTY_H
#define PROPERTY_H

#include "SimulationControlAndResponse.h"
#include "ModelComponent.h"

class PropertyEditorGenesys {
public:
    PropertyEditorGenesys() = default;
    virtual ~PropertyEditorGenesys() = default;

public:
    std::list<ModelComponent*> getElements() {
        return _elements;
    }

public:
    // Current GUI editing path writes values through SimulationControl.
    void changeProperty(SimulationControl* control, const std::string& value, bool remove = false) {
        if (control != nullptr) {
            control->setValue(value, remove);
        }
    }

    SimulationControl* findProperty(const std::string& id, const std::string& attribute) {
        for (auto element : _elements) {
            if (std::to_string(element->getId()) == id) {
                for (auto control : *element->getProperties()->list()) {
                    if (control->getName() == attribute) {
                        return control;
                    }
                }
            }
        }
        return nullptr;
    }

    void addElement(ModelComponent* component) {
        _elements.push_back(component);
    }

private:
    std::list<ModelComponent*> _elements;
};

#endif
