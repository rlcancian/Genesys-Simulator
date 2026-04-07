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
    void changeProperty(SimulationControl* property, const std::string& value, bool remove = false) {
        if (property != nullptr) {
            property->setValue(value, remove);
        }
    }

    SimulationControl* findProperty(const std::string& id, const std::string& attribute) {
        for (auto element : _elements) {
            if (std::to_string(element->getId()) == id) {
                for (auto prop : *element->getProperties()->list()) {
                    if (prop->getName() == attribute) {
                        return prop;
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
