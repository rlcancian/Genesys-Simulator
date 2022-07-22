#include "ModelPersistenceDefaultImpl2.h"

#include <cassert>
#include <memory>
#include <fstream>
#include <vector>
#include <sstream>

#include "ModelSerializer.h"
#include "Simulator.h"
#include "../util/Util.h"
#include "Counter.h"

#include "GenSerializer.h"
#include "XmlSerializer.h"
#include "JsonSerializer.h"
#include "CppSerializer.h"


ModelPersistenceDefaultImpl2::ModelPersistenceDefaultImpl2(Model* model):
    _model(model)
{
    assert(model != nullptr);
}


bool ModelPersistenceDefaultImpl2::save(std::string filename) {
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Saving file \"" + filename + "\"");
    Util::IncIndent();

    // choose format
    std::unique_ptr<ModelSerializer> serializer;
    {
        auto extension = filename.substr(filename.find_last_of('.') + 1);
        if (extension == "xml") {
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Serializing as XML");
            serializer = std::make_unique<XmlSerializer>(_model);
        } else if (extension == "json") {
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Serializing as JSON");
            serializer = std::make_unique<JsonSerializer>(_model);
        } else if (extension == "cpp") {
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Serializing as C++");
            serializer = std::make_unique<CppSerializer>(_model);
        } else { // default
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Serializing in GenESyS lang");
            serializer = std::make_unique<GenSerializer>(_model);
        }
    }

    const bool saveDefaults = _model->getPersistence()->getOption(ModelPersistence_if::Options::SAVEDEFAULTS);
    auto fields = std::unique_ptr<PersistenceRecord>(serializer->newPersistenceRecord());

    // grab simulator info
    fields->clear();
    fields->saveField("typename", "Simulator");
    fields->saveField("name", _model->getParentSimulator()->getName());
    fields->saveField("versionNumber", _model->getParentSimulator()->getVersionNumber());
    serializer->put("SimulatorInfo", "Simulator", 0, fields.get());

    // grab model metadata
    fields->clear();
    _model->getInfos()->saveInstance(fields.get());
    serializer->put("ModelInfo", "ModelInfo", 0, fields.get());

    // grab simulation params
    fields->clear();
    _model->getSimulation()->saveInstance(fields.get(), saveDefaults);
    serializer->put("ModelSimulation", "ModelSimulation", 0, fields.get());

    // gather data definitions
    const std::string counter = Util::TypeOf<Counter>();
    const std::string statsCollector = Util::TypeOf<StatisticsCollector>();
    for (auto& type : *_model->getDataManager()->getDataDefinitionClassnames()) {
        if (type == statsCollector || type == counter) continue; // these don't need to be saved
        _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing elements of type \"" + type + "\":");
        Util::IncIndent();
        for (ModelDataDefinition *data : *_model->getDataManager()->getDataDefinitionList(type)->list()) {
            _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing " + type + " \"" + data->getName() + "\"");
            fields->clear();
            data->SaveInstance(fields.get(), data);
            auto name = data->getName();
            auto id = data->getId();
            auto type = data->getClassname();
            serializer->put(name, type, id, fields.get());
        }
        Util::DecIndent();
    }

    // gather model components
    _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing components:");
    Util::IncIndent();
    for (ModelComponent* component : *_model->getComponents()) {
        if (component->getLevel() == 0) {
            fields->clear();
            component->SaveInstance(fields.get(), component);
            auto name = component->getName();
            auto id = component->getId();
            auto type = component->getClassname();
            serializer->put(name, type, id, fields.get());
        }
    }
    Util::DecIndent();

    // write contents to file
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Saving file");
    Util::IncIndent();
    std::ofstream file{filename};
    bool ok = serializer->dump(file);
    Util::DecIndent();

    // finish save
    Util::DecIndent();
    if (ok) _dirty = false;
    return ok;
}


bool ModelPersistenceDefaultImpl2::load(std::string filename) {
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Loading file \"" + filename + "\"");
    Util::IncIndent();

    // choose format
    std::unique_ptr<ModelSerializer> parser;
    {
        auto extension = filename.substr(filename.find_last_of('.') + 1);
        if (extension == "xml") {
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as XML");
            parser = std::make_unique<XmlSerializer>(_model);
        } else if (extension == "json") {
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as JSON");
            parser = std::make_unique<JsonSerializer>(_model);
        } else if (extension == "cpp") {
            _model->getTracer()->trace(TraceManager::Level::L4_warning, "Cannot parse C++");
            Util::DecIndent();
            return false;
        } else { // default
            _model->getTracer()->trace(TraceManager::Level::L7_internal, "Parsing as GenESyS lang");
            parser = std::make_unique<GenSerializer>(_model);
        }
    }

    // load file
    bool ok = true;
    try {
        std::ifstream file(filename);
        ok &= parser->load(file);
        if (!ok) throw std::exception();
    } catch(const std::exception& e) {
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error loading file \"" + filename + "\"");
        Util::DecIndent();
        return false;
    }

    // instantiate and load components/datadefs
    std::vector<std::unique_ptr<PersistenceRecord>> componentFields;
    Util::IncIndent();
    parser->for_each([&](auto name){
        // for each component, we reach for its fields
        auto fields = std::unique_ptr<PersistenceRecord>(parser->newPersistenceRecord());
        parser->get(name, fields.get());
        // then, dispatch on type to instantiate and set up the rest
        auto type = fields->loadField("typename", "__UNTYPED__");
        _model->getTracer()->trace(TraceManager::Level::L7_internal, "loading " + type);
        if (type == "SimulatorInfo" || type == "Simulator") {
            unsigned int savedVersion = fields->loadField("versionNumber", 0);
            unsigned int currentVersion = _model->getParentSimulator()->getVersionNumber();
            if (savedVersion != currentVersion) {
                _model->getTracer()->trace("WARNING: The version of the saved model differs from the simulator. Loading may not be possible", TraceManager::Level::L3_errorRecover);
            }
        } else if (type == "ModelInfo") {
            _model->getInfos()->loadInstance(fields.get());
            _dirty = true;
        } else if (type == "ModelSimulation") {
            _model->getSimulation()->loadInstance(fields.get());
            _dirty = true;
        } else {
            Plugin* plugin = this->_model->getParentSimulator()->getPlugins()->find(type);
            if (plugin != nullptr) {
                ok = plugin->loadAndInsertNew(_model, fields.get());
                if (ok && plugin->getPluginInfo()->isComponent()) {
                    _dirty = true;
                    componentFields.push_back(std::move(fields));
                }
            } else {
                _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Error loading file: Could not identity typename \"" + type + "\"");
                ok = false;
            }
        }
        return !ok;
    });
    Util::IncIndent();

    // after all components have been loaded, connect them at the toplevel
    if (ok) {
        ComponentManager* cm = _model->getComponents();
        _model->getTracer()->trace(TraceManager::Level::L8_detailed, "Connecting loaded components");
        Util::IncIndent();
        for (auto& fields : componentFields) {
            // get a handle to the component
            Util::identification id = fields->loadField("id", -1);
            ModelComponent *component = cm->find(id);
            if (component == nullptr) {
                _model->getTracer()->traceError("found unregistered component ID " + std::to_string(id));
                continue;
            }
            // then set up its connections
            unsigned short nextSize = fields->loadField("nexts", 1);
            for (unsigned short i = 0; i < nextSize; i++) {
                // target id
                Util::identification nextId = fields->loadField("nextId" + strIndex(i), 0);
                if (nextSize == 1) nextId = fields->loadField("nextId", static_cast<unsigned int>(nextId));
                // id -> component
                ModelComponent* nextComponent = cm->find(nextId);
                if (nextComponent == nullptr) {
                    _model->getTracer()->traceError("found unregistered target ID " + std::to_string(nextId));
                    continue;
                }
                // target port
                unsigned short nextPort = fields->loadField("nextinputPortNumber" + strIndex(i), 0);
                // connect
                component->getConnections()->insert(nextComponent, nextPort);
                _model->getTracer()->trace(TraceManager::Level::L8_detailed, component->getName() + "<" + std::to_string(i) + ">" + " --> " + nextComponent->getName() + "<" + std::to_string(nextPort) + ">");
            }
        }
        Util::DecIndent();
    }

    Util::DecIndent();
    return ok;
}


bool ModelPersistenceDefaultImpl2::hasChanged() {
    return _dirty;
}


bool ModelPersistenceDefaultImpl2::getOption(ModelPersistence_if::Options option) {
    return (_options & static_cast<unsigned>(option)) != 0u;
}


void ModelPersistenceDefaultImpl2::setOption(ModelPersistence_if::Options option, bool value) {
    unsigned bit = static_cast<unsigned>(option);
    if (value) _options |= bit;
    else       _options &= ~bit;
}


std::string ModelPersistenceDefaultImpl2::getFormatedField(PersistenceRecord *fields) {
    return GenSerializer::linearize(fields);
}
