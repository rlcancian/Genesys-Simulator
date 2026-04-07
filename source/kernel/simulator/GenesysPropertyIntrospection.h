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

    std::string currentValue;
    std::vector<std::string> choices;
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
