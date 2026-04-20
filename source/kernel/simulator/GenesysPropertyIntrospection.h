#pragma once

#include <string>
#include <vector>

#include "SimulationControlAndResponse.h"
#include "../util/Util.h"

enum class GenesysPropertyKind {
    Unknown,
    Boolean,
    Integer,
    UnsignedInteger,
    UnsignedShort,
    Double,
    String,
    TimeUnit,
    Enum,
    Object,
    List
};

struct GenesysPropertyDescriptor {
    SimulationControl* control = nullptr;

    std::string ownerClassName;
    std::string ownerElementName;
    std::string displayName;
    std::string technicalTypeName;

    GenesysPropertyKind kind = GenesysPropertyKind::Unknown;

    bool readOnly = true;
    bool isList = false;
    bool isClass = false;
    bool isEnum = false;
    bool isInlineObject = false;
    bool isModelDataDefinitionReference = false;
    bool supportsInlineExpansion = false;
    bool supportsListEditor = false;
    bool supportsExistingObjectSelection = false;
    bool supportsObjectCreation = false;
    bool supportsNewListElementCreation = false;

    std::string currentValue;
    std::vector<std::string> choices;
    // For polymorphic list controls, this is the currently selected concrete element type.
    //
    // Example: a Set physically stores ModelDataDefinition* but may currently be a Set of Resource.
    // Empty means the list has not been typed yet.
    std::string currentListElementType;
    // For polymorphic list controls, these are the concrete element types that the kernel allows
    // the GUI to select/create. Empty means the list should use its legacy single-type behavior.
    std::vector<std::string> creatableListElementTypes;
};

class GenesysPropertyIntrospection {
public:
    static GenesysPropertyDescriptor describe(SimulationControl* control);
    static std::vector<GenesysPropertyDescriptor> describe(List<SimulationControl*>* controls);

    static GenesysPropertyKind deduceKind(const SimulationControl* control);

    static bool setValue(
        SimulationControl* control,
        const std::string& value,
        bool remove = false,
        std::string* errorMessage = nullptr
        );

    static std::vector<SimulationControl*> children(
        SimulationControl* control,
        int index = 0
        );
};
