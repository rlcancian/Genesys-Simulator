#include "plugins/components/InputOutput/Read.h"

#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelSimulation.h"
#include "plugins/data/Logic/Variable.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <cmath>
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <utility>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &Read::GetPluginInformation;
}
#endif

namespace {
std::string trimCopy(const std::string& value) {
    const auto begin = std::find_if_not(value.begin(), value.end(),
        [](unsigned char ch) { return std::isspace(ch); });
    const auto end = std::find_if_not(value.rbegin(), value.rend(),
        [](unsigned char ch) { return std::isspace(ch); }).base();
    return begin < end ? std::string(begin, end) : "";
}

bool terminalInputAvailable() {
#if defined(__unix__) || defined(__APPLE__)
    return ::isatty(STDIN_FILENO) != 0;
#else
    return false;
#endif
}
// Keep list deletion scoped to Read rather than changing every plugin list.
class ReadDestinationsControl final
    : public SimulationControlGenericListPointer<ReadDestination*, Model*, ReadDestination> {
public:
    using Base = SimulationControlGenericListPointer<ReadDestination*, Model*, ReadDestination>;

    ReadDestinationsControl(Read* owner, Model* model)
        : Base(model,
               [owner]() { return owner->getDestinations(); },
               [owner](ReadDestination* destination) { owner->addDestination(destination); },
               [owner](ReadDestination* destination) { owner->removeDestination(destination); },
               Util::TypeOf<Read>(), owner->getName(), "Destinations",
               "Ordered destinations for numeric input fields.",
               true, true, false,
               [](Model* m, const std::string& name) { return new ReadDestination(m, name); }),
          _owner(owner) {
    }

    bool supportsListElementRemoval() const override { return true; }

    bool removeListElementAt(int index) override {
        _ensureWritable("remove list element from");
        auto* destinations = _owner->getDestinations();
        if (index < 0 || static_cast<std::size_t>(index) >= destinations->size()) return false;
        _owner->removeDestination(destinations->getAtRank(static_cast<unsigned int>(index)));
        return true;
    }

private:
    Read* _owner;
};
} // namespace

ReadDestination::ReadDestination(Model* model, std::string destination, bool attributeNotVariable)
    : ReadDestination(std::move(destination), attributeNotVariable) {
    ensureSimulationControls(model);
}

ReadDestination::ReadDestination(std::string destination, bool attributeNotVariable)
    : _destination(std::move(destination)), _attributeNotVariable(attributeNotVariable) {
}

ReadDestination::~ReadDestination() {
    if (_ownerModel != nullptr) {
        for (SimulationControl* control : *_controls->list()) {
            _ownerModel->getControls()->remove(control);
            delete control;
        }
    }
    delete _controls;
}

std::string ReadDestination::getName() const { return _destination; }
std::string ReadDestination::getDestination() const { return _destination; }
void ReadDestination::setDestination(std::string destination) {
    _destination = std::move(destination);
    if (_onChange) _onChange();
}
bool ReadDestination::isAttributeNotVariable() const { return _attributeNotVariable; }
void ReadDestination::setAttributeNotVariable(bool value) {
    _attributeNotVariable = value;
    if (_onChange) _onChange();
}
void ReadDestination::setChangeCallback(std::function<void()> callback) {
    _onChange = std::move(callback);
}
List<SimulationControl*>* ReadDestination::getSimulationControls() const {
    return _controls;
}
void ReadDestination::ensureSimulationControls(Model* model) {
    if (model == nullptr || _controls->size() != 0) return;
    _ownerModel = model;
    auto* name = new SimulationControlGeneric<std::string>(
        std::bind(&ReadDestination::getDestination, this),
        std::bind(&ReadDestination::setDestination, this, std::placeholders::_1),
        Util::TypeOf<ReadDestination>(), getName(), "Destination", "Attribute or Variable name; optional [index].");
    auto* type = new SimulationControlGeneric<bool>(
        std::bind(&ReadDestination::isAttributeNotVariable, this),
        std::bind(&ReadDestination::setAttributeNotVariable, this, std::placeholders::_1),
        Util::TypeOf<ReadDestination>(), getName(), "AttributeNotVariable", "True: entity attribute; false: model variable.");
    model->getControls()->insert(name);
    model->getControls()->insert(type);
    _controls->insert(name);
    _controls->insert(type);
}

std::string Read::convertEnumToStr(ReadFromType type) {
    switch (type) {
        case ReadFromType::FILE: return "FILE";
        case ReadFromType::KEYBOARD: return "KEYBOARD";
        default: return "Unknown";
    }
}

Read::Read(Model* model, std::string name)
    : ModelComponent(model, Util::TypeOf<Read>(), std::move(name)) {
    auto* filenameControl = new SimulationControlGeneric<std::string>(
        std::bind(&Read::filename, this),
        std::bind(&Read::setFilename, this, std::placeholders::_1),
        Util::TypeOf<Read>(), getName(), "Filename", "Path to a whitespace-separated numeric text file.");
    auto* sourceControl = new SimulationControlGenericEnum<ReadFromType, Read>(
        std::bind(&Read::readFromType, this),
        std::bind(&Read::setReadFromType, this, std::placeholders::_1),
        Util::TypeOf<Read>(), getName(), "ReadFromType", "FILE or KEYBOARD.");
    auto* destinationsControl = new ReadDestinationsControl(this, _parentModel);
    for (SimulationControl* control : {static_cast<SimulationControl*>(filenameControl),
                                       static_cast<SimulationControl*>(sourceControl),
                                       static_cast<SimulationControl*>(destinationsControl)}) {
        _parentModel->getControls()->insert(control);
        _addSimulationControl(control);
    }
}

Read::~Read() {
    for (ReadDestination* destination : *_destinations->list()) {
        destination->setChangeCallback(nullptr);
        delete destination;
    }
    for (ReadDestination* destination : _removedDestinations) {
        destination->setChangeCallback(nullptr);
        delete destination;
    }
    delete _destinations;
}

std::string Read::show() { return ModelComponent::show() + ",source=" + convertEnumToStr(_source); }
void Read::setFilename(std::string filename) {
    _stream.close();
    _stream.clear();
    _lineNumber = 0;
    _filename = std::move(filename);
}
std::string Read::filename() const { return _filename; }
void Read::setReadFromType(ReadFromType type) { _source = type; }
Read::ReadFromType Read::readFromType() const { return _source; }
List<ReadDestination*>* Read::getDestinations() const {
    for (ReadDestination* destination : *_destinations->list()) {
        const_cast<Read*>(this)->_prepareDestination(destination);
    }
    return _destinations;
}
void Read::addDestination(ReadDestination* destination) {
    if (destination == nullptr) return;
    _removedDestinations.erase(
        std::remove(_removedDestinations.begin(), _removedDestinations.end(), destination),
        _removedDestinations.end());
    _prepareDestination(destination);
    _destinations->insert(destination);
    _createEditableDataDefinitions();
}
void Read::removeDestination(ReadDestination* destination) {
    if (destination == nullptr ||
        _destinations->find(destination) == _destinations->list()->end()) return;
    destination->setChangeCallback(nullptr);
    _destinations->remove(destination);
    // Keep the object alive if a GUI editor still refers to it. This component
    // retains ownership and destroys retired items on teardown.
    _removedDestinations.push_back(destination);
    _createEditableDataDefinitions();
}
void Read::setKeyboardInputProvider(KeyboardInputProvider provider) {
    _keyboardInput = std::move(provider);
}

PluginInformation* Read::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<Read>(), &Read::LoadInstance, &Read::NewInstance);
    info->setCategory("InputOutput");
    info->setMinimumInputs(1);
    info->setMaximumInputs(1);
    info->setMinimumOutputs(1);
    info->setMaximumOutputs(1);
    info->insertDynamicLibFileDependence("variable.so");
    info->setDescriptionHelp("Read one whitespace-delimited numeric record from a text file or interactive keyboard "
        "for each arriving entity, into ordered Attribute and Variable destinations.");
    return info;
}
ModelDataDefinition* Read::NewInstance(Model* model, std::string name) {
    return new Read(model, std::move(name));
}
ModelComponent* Read::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* component = new Read(model);
    try {
        // Plugin-template introspection calls loaders with an empty record.
        // A missing ID in that specific case is expected, not a load failure.
        // Malformed nonempty persisted records still produce diagnostics.
        if (fields == nullptr) {
            component->traceError("Failed to load Read instance: null persistence record");
        } else if (!component->_loadInstance(fields) && fields->size() != 0) {
            component->traceError("Failed to load Read instance: invalid persistence record");
        }
    } catch (const std::exception& e) {
        component->traceError("Failed to load Read instance: " + std::string(e.what()));
    }
    return component;
}
bool Read::_loadInstance(PersistenceRecord* fields) {
    if (!ModelComponent::_loadInstance(fields)) return false;
    const int source = fields->loadField("readFromType", static_cast<int>(Defaults::source));
    _source = static_cast<ReadFromType>(source);
    setFilename(fields->loadField("filename", std::string("")));
    const unsigned int count = fields->loadField("readDestinations", 0u);
    for (unsigned int i = 0; i < count; ++i) {
        const std::string suffix = Util::StrIndex(i);
        auto* destination = new ReadDestination(
            fields->loadField("readDest" + suffix, std::string("")),
            fields->loadField("readIsAttrib" + suffix, true));
        addDestination(destination);
    }
    return true;
}
void Read::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    ModelComponent::_saveInstance(fields, saveDefaultValues);
    fields->saveField("readFromType", static_cast<int>(_source), static_cast<int>(Defaults::source), saveDefaultValues);
    fields->saveField("filename", _filename, std::string(""), saveDefaultValues);
    fields->saveField("readDestinations", _destinations->size(), 0u, saveDefaultValues);
    unsigned int i = 0;
    for (const ReadDestination* destination : *_destinations->list()) {
        const std::string suffix = Util::StrIndex(i++);
        fields->saveField("readDest" + suffix, destination->getDestination(), std::string(""), saveDefaultValues);
        fields->saveField("readIsAttrib" + suffix, destination->isAttributeNotVariable(), true, saveDefaultValues);
    }
}
bool Read::_splitDestination(const std::string& input, std::string& base, std::string& index) {
    const std::string text = trimCopy(input);
    const auto open = text.find('[');
    if (open == std::string::npos) {
        base = text;
        index.clear();
        return !base.empty() && text.find(']') == std::string::npos;
    }
    const auto close = text.find(']', open + 1);
    if (close == std::string::npos || close != text.size() - 1 || text.find('[', open + 1) != std::string::npos) {
        return false;
    }
    base = trimCopy(text.substr(0, open));
    index = trimCopy(text.substr(open + 1, close - open - 1));
    return !base.empty() && !index.empty();
}
bool Read::_validIndex(const std::string& index) {
    if (index.empty()) return true;
    // SparseValueStore treats nonnumeric keys as text. Only its numeric-key
    // path uses std::stoul and dimension arithmetic, so preflight that path.
    const bool numeric = std::all_of(index.begin(), index.end(), [](unsigned char ch) {
        return std::isdigit(ch) || ch == ',' || ch == ' ' || ch == '\t';
    });
    if (!numeric) return true;

    // SparseValueStore materializes numeric indices as a dense vector. An
    // otherwise valid index such as [4294967294] would attempt a multi-GB
    // allocation. Keep Read input bounded without changing global stores.
    constexpr std::size_t kMaxNumericIndexCells = 1'000'000;
    std::size_t minimumCells = 1;
    std::istringstream parts(index);
    std::string part;
    while (std::getline(parts, part, ',')) {
        part = trimCopy(part);
        if (part.empty()) return false;
        unsigned int value = 0;
        const char* const begin = part.data();
        const char* const end = begin + part.size();
        const auto parsed = std::from_chars(begin, end, value);
        // SparseValueStore grows a dimension using (value + 1).
        if (parsed.ec != std::errc{} || parsed.ptr != end ||
            value == std::numeric_limits<unsigned int>::max()) {
            return false;
        }
        const std::size_t extent = static_cast<std::size_t>(value) + 1u;
        if (extent > kMaxNumericIndexCells ||
            minimumCells > kMaxNumericIndexCells / extent) {
            return false;
        }
        minimumCells *= extent;
    }
    // A trailing comma would be ignored by std::getline but is not a valid
    // sparse multidimensional key.
    return index.back() != ',';
}
void Read::_prepareDestination(ReadDestination* destination) {
    if (destination == nullptr) return;
    destination->ensureSimulationControls(_parentModel);
    destination->setChangeCallback([this]() { _createEditableDataDefinitions(); });
}
void Read::_createEditableDataDefinitions() {
    _optionalEditableDataDefinitionsClear();
    for (ReadDestination* destination : *_destinations->list()) {
        if (!destination) continue;
        std::string base, index;
        if (!_splitDestination(destination->getDestination(), base, index)) continue;
        const bool attribute = destination->isAttributeNotVariable();
        const std::string type = attribute ? Util::TypeOf<Attribute>() : Util::TypeOf<Variable>();
        auto* definition = _parentModel->getDataManager()->getDataDefinition(type, base);
        if (definition == nullptr) {
            definition = attribute ? static_cast<ModelDataDefinition*>(new Attribute(_parentModel, base))
                                   : static_cast<ModelDataDefinition*>(new Variable(_parentModel, base));
            _parentModel->getDataManager()->insert(type, definition);
        }
        _optionalEditableDataDefinitionInsert(type + "_" + base, definition);
    }
}
bool Read::_check(std::string& errorMessage) {
    bool valid = true;
    if (_source != ReadFromType::FILE && _source != ReadFromType::KEYBOARD) {
        errorMessage += "ReadFromType must be FILE or KEYBOARD. ";
        valid = false;
    }
    if (_source == ReadFromType::FILE) {
        if (_filename.empty()) {
            errorMessage += "Read Filename cannot be empty in FILE mode. ";
            valid = false;
        } else {
            std::ifstream check(_filename);
            if (!check.is_open()) {
                errorMessage += "Read cannot open file: " + _filename + ". ";
                valid = false;
            }
        }
    }
    if (_destinations->empty()) {
        errorMessage += "Read requires at least one destination. ";
        valid = false;
    }
    _createEditableDataDefinitions();
    for (ReadDestination* destination : *_destinations->list()) {
        if (destination == nullptr) {
            errorMessage += "Read contains a null destination. ";
            valid = false;
            continue;
        }
        std::string base, index;
        if (!_splitDestination(destination->getDestination(), base, index) || !_validIndex(index)) {
            errorMessage += "Read destination is empty or has invalid [index] syntax. ";
            valid = false;
            continue;
        }
        const std::string type = destination->isAttributeNotVariable() ? Util::TypeOf<Attribute>() : Util::TypeOf<Variable>();
        if (_parentModel->getDataManager()->getDataDefinition(type, base) == nullptr) {
            errorMessage += "Read destination definition does not exist: " + base + ". ";
            valid = false;
        }
    }
    return valid;
}
void Read::_initBetweenReplications() {
    _stream.close();
    _stream.clear();
    _lineNumber = 0;
}
bool Read::_getRecord(std::string& record, std::string& error) {
    if (_source == ReadFromType::KEYBOARD) {
        std::string prompt = "Read \"" + getName() + "\" - enter " +
            std::to_string(_destinations->size()) + " numeric value(s), separated by spaces: ";
        if (_keyboardInput) {
            try {
                if (_keyboardInput(prompt, record)) return true;
                error = "keyboard input cancelled or unavailable";
            } catch (const std::exception& e) {
                error = "keyboard input provider failed: " + std::string(e.what());
            } catch (...) {
                error = "keyboard input provider failed with an unknown exception";
            }
            return false;
        }
        if (!terminalInputAvailable()) {
            error = "interactive keyboard input is unavailable (no host provider or terminal TTY)";
            return false;
        }
        std::cout << prompt << std::flush;
        if (std::getline(std::cin, record)) return true;
        error = "terminal input ended or failed";
        return false;
    }
    if (_source != ReadFromType::FILE) {
        error = "unsupported ReadFromType";
        return false;
    }
    if (!_stream.is_open()) {
        _stream.clear();
        _stream.open(_filename);
        if (!_stream.is_open()) {
            error = "cannot open input file " + _filename;
            return false;
        }
    }
    while (std::getline(_stream, record)) {
        ++_lineNumber;
        const std::string trimmed = trimCopy(record);
        if (trimmed.empty() || trimmed.front() == '#') continue;
        return true;
    }
    error = _stream.bad() ? "input file read failure" : "end of input file";
    return false;
}
bool Read::_parseRecord(const std::string& record, std::vector<double>& values, std::string& error) const {
    std::istringstream fields(record);
    fields.imbue(std::locale::classic());
    std::string token;
    while (fields >> token) {
        std::istringstream number(token);
        number.imbue(std::locale::classic());
        double value = 0.0;
        if (!(number >> value) || number.peek() != std::char_traits<char>::eof() || !std::isfinite(value)) {
            error = "invalid or non-finite numeric field: " + token;
            return false;
        }
        values.push_back(value);
    }
    if (values.size() != _destinations->size()) {
        error = "record has " + std::to_string(values.size()) + " fields, expected " +
                std::to_string(_destinations->size());
        return false;
    }
    return true;
}
void Read::_fail(const std::string& reason) {
    traceError("Read \"" + getName() + "\" failed: " + reason, TraceManager::Level::L3_errorRecover);
    _parentModel->getSimulation()->stop();
}
void Read::_onDispatchEvent(Entity* entity, unsigned int) {
    if (entity == nullptr) {
        _fail("missing entity");
        return;
    }
    std::string error;
    std::string record;
    if (!_getRecord(record, error)) {
        _fail(error);
        return;
    }
    std::vector<double> values;
    if (!_parseRecord(record, values, error)) {
        _fail(error + (_source == ReadFromType::FILE ? " at line " + std::to_string(_lineNumber) : ""));
        return;
    }
    struct Target {
        bool attribute;
        std::string base;
        std::string index;
        Variable* variable = nullptr;
    };
    std::vector<Target> targets;
    targets.reserve(values.size());
    for (ReadDestination* destination : *_destinations->list()) {
        if (destination == nullptr) { _fail("null destination"); return; }
        Target target;
        target.attribute = destination->isAttributeNotVariable();
        if (!_splitDestination(destination->getDestination(), target.base, target.index) ||
            !_validIndex(target.index)) {
            _fail("invalid destination: " + destination->getDestination());
            return;
        }
        const std::string type = target.attribute ? Util::TypeOf<Attribute>() : Util::TypeOf<Variable>();
        ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(type, target.base);
        if (definition == nullptr) {
            _fail("unresolved destination: " + destination->getDestination());
            return;
        }
        if (!target.attribute) {
            target.variable = dynamic_cast<Variable*>(definition);
            if (target.variable == nullptr) { _fail("destination is not a Variable"); return; }
        }
        targets.push_back(std::move(target));
    }
    // Nothing is applied until the complete record and every destination are valid.
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (targets[i].attribute) {
            entity->setAttributeValue(targets[i].base, values[i], targets[i].index);
        } else {
            targets[i].variable->setValue(values[i], targets[i].index);
        }
    }
    _parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
}
