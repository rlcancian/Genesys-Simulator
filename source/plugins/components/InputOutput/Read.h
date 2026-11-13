#ifndef READ_H
#define READ_H

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "kernel/simulator/model/ModelComponent.h"

class Model;
class SimulationControl;

/**
 * A typed, ordered destination for one numeric field of a Read record.
 * This is deliberately not an Assignment: no expression is evaluated.
 */
class ReadDestination {
public:
    ReadDestination(Model* model, std::string destination, bool attributeNotVariable = true);
    ReadDestination(std::string destination = "", bool attributeNotVariable = true);
    ~ReadDestination();

    std::string getName() const;
    std::string getDestination() const;
    void setDestination(std::string destination);
    bool isAttributeNotVariable() const;
    void setAttributeNotVariable(bool attributeNotVariable);
    void setChangeCallback(std::function<void()> callback);
    void ensureSimulationControls(Model* model);
    List<SimulationControl*>* getSimulationControls() const;

private:
    std::string _destination;
    bool _attributeNotVariable = true;
    std::function<void()> _onChange;
    List<SimulationControl*>* _controls = new List<SimulationControl*>();
    Model* _ownerModel = nullptr;
};

/**
 * Reads one whitespace-delimited numeric record for each arriving entity.
 *
 * FILE cursor is local to the component and resets at each replication.
 * KEYBOARD uses a host-supplied callback, or interactive terminal stdin
 * when no callback is installed and stdin is a TTY. A failed read stops the
 * simulation without routing the entity or applying any field.
 */
class Read : public ModelComponent {
public:
    enum class ReadFromType : int { FILE = 0, KEYBOARD = 1, num_elements = 2 };
    using KeyboardInputProvider = std::function<bool(const std::string& prompt, std::string& line)>;

    static std::string convertEnumToStr(ReadFromType type);
    Read(Model* model, std::string name = "");
    ~Read() override;

    std::string show() override;
    void setFilename(std::string filename);
    std::string filename() const;
    void setReadFromType(ReadFromType type);
    ReadFromType readFromType() const;

    List<ReadDestination*>* getDestinations() const;
    void addDestination(ReadDestination* destination);
    void removeDestination(ReadDestination* destination);

    /** Host-controlled, non-persistent input callback; false means cancellation. */
    void setKeyboardInputProvider(KeyboardInputProvider provider);

    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
    void _initBetweenReplications() override;
    bool _check(std::string& errorMessage) override;
    bool _loadInstance(PersistenceRecord* fields) override;
    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    void _createEditableDataDefinitions() override;

private:
    static bool _splitDestination(const std::string& text, std::string& base, std::string& index);
    void _prepareDestination(ReadDestination* destination);
    bool _getRecord(std::string& record, std::string& error);
    bool _parseRecord(const std::string& record, std::vector<double>& values, std::string& error) const;
    void _fail(const std::string& reason);

    struct Defaults {
        static constexpr ReadFromType source = ReadFromType::FILE;
    };
    ReadFromType _source = Defaults::source;
    std::string _filename;
    List<ReadDestination*>* _destinations = new List<ReadDestination*>();
    // Retain removed items until destruction to support editor reconciliation.
    std::vector<ReadDestination*> _removedDestinations;
    std::ifstream _stream;
    KeyboardInputProvider _keyboardInput;
    unsigned long long _lineNumber = 0;
};

#endif // READ_H
