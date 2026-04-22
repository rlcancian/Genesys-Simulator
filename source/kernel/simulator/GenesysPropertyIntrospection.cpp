#include "GenesysPropertyIntrospection.h"

#include <exception>

namespace {

static std::vector<std::string> _copyStringList(List<std::string>* list) {
    std::vector<std::string> result;
    if (list == nullptr) {
        return result;
    }

    for (const std::string& value : *list->list()) {
        result.push_back(value);
    }

    delete list;
    return result;
}

static std::vector<SimulationControl*> _copyControlList(List<SimulationControl*>* list) {
    std::vector<SimulationControl*> result;
    if (list == nullptr) {
        return result;
    }

    for (SimulationControl* control : *list->list()) {
        result.push_back(control);
    }

    return result;
}

} // unnamed namespace

GenesysPropertyKind GenesysPropertyIntrospection::deduceKind(const SimulationControl* control) {
    if (control == nullptr) {
        return GenesysPropertyKind::Unknown;
    }

    if (control->getIsList()) {
        return GenesysPropertyKind::List;
    }
    if (control->getIsClass()) {
        return GenesysPropertyKind::Object;
    }

    const std::string typeName = control->propertyType();

    if (typeName == Util::TypeOf<Util::TimeUnit>()) {
        return GenesysPropertyKind::TimeUnit;
    }
    if (control->getIsEnum()) {
        return GenesysPropertyKind::Enum;
    }
    if (typeName == Util::TypeOf<bool>()) {
        return GenesysPropertyKind::Boolean;
    }
    if (typeName == Util::TypeOf<int>()) {
        return GenesysPropertyKind::Integer;
    }
    if (typeName == Util::TypeOf<unsigned int>()) {
        return GenesysPropertyKind::UnsignedInteger;
    }
    if (typeName == Util::TypeOf<unsigned short>()) {
        return GenesysPropertyKind::UnsignedShort;
    }
    if (typeName == Util::TypeOf<double>()) {
        return GenesysPropertyKind::Double;
    }
    if (typeName == Util::TypeOf<std::string>()) {
        return GenesysPropertyKind::String;
    }

    return GenesysPropertyKind::Unknown;
}

GenesysPropertyDescriptor GenesysPropertyIntrospection::describe(SimulationControl* control) {
    GenesysPropertyDescriptor desc;
    if (control == nullptr) {
        return desc;
    }

    desc.control = control;
    desc.ownerClassName = control->getClassname();
    desc.ownerElementName = control->getElementName();
    desc.displayName = control->getName();
    desc.technicalTypeName = control->propertyType();
    desc.kind = deduceKind(control);
    desc.readOnly = control->isReadOnly();
    desc.isList = control->getIsList();
    desc.isClass = control->getIsClass();
    desc.isEnum = control->getIsEnum();
    // This block maps the explicit kernel contract flags into GUI-facing descriptor metadata.
    desc.supportsInlineExpansion = control->supportsInlineExpansion();
    desc.supportsListEditor = control->supportsListEditor();
    desc.supportsExistingObjectSelection = control->supportsExistingObjectSelection();
    desc.supportsObjectCreation = control->supportsObjectCreation();
    desc.supportsNewListElementCreation = control->supportsNewListElementCreation();
    desc.isInlineObject = control->isInlineObjectProperty();
    desc.isModelDataDefinitionReference = control->isModelDataDefinitionReference();
    desc.currentValue = control->getValue();

    desc.choices = _copyStringList(control->getStrValues());
    desc.currentListElementType = control->getCurrentListElementType();
    desc.creatableListElementTypes = _copyStringList(control->getCreatableListElementTypes());

    if (desc.kind == GenesysPropertyKind::TimeUnit && desc.choices.empty()) {
        for (int i = 0; i < static_cast<int>(Util::TimeUnit::num_elements); ++i) {
            desc.choices.push_back(
                Util::convertEnumToStr(static_cast<Util::TimeUnit>(i))
                );
        }
    }

    return desc;
}

std::vector<GenesysPropertyDescriptor> GenesysPropertyIntrospection::describe(List<SimulationControl*>* controls) {
    std::vector<GenesysPropertyDescriptor> result;
    if (controls == nullptr) {
        return result;
    }

    for (SimulationControl* control : *controls->list()) {
        result.push_back(describe(control));
    }
    return result;
}

bool GenesysPropertyIntrospection::setValue(
    SimulationControl* control,
    const std::string& value,
    bool remove,
    std::string* errorMessage
    ) {
    if (control == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = "SimulationControl nulo";
        }
        return false;
    }

    try {
        control->setValue(value, remove);
        return true;
    } catch (const std::exception& e) {
        if (errorMessage != nullptr) {
            *errorMessage = e.what();
        }
        return false;
    } catch (...) {
        if (errorMessage != nullptr) {
            *errorMessage = "Erro desconhecido ao alterar propriedade";
        }
        return false;
    }
}

std::vector<SimulationControl*> GenesysPropertyIntrospection::children(
    SimulationControl* control,
    int index
    ) {
    if (control == nullptr) {
        return {};
    }
    return _copyControlList(control->getChildSimulationControls(index));
}
