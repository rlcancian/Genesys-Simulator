#include "GenSerializer.h"

#include <cassert>
#include <regex>

#include "Counter.h"
#include "Simulator.h"


GenSerializer::GenSerializer(Model *model):
    _model(model)
{
    assert(model != nullptr);
}


PersistenceRecord* GenSerializer::newPersistenceRecord() {
    return new PersistenceRecord(*_model->getPersistence());
}


bool GenSerializer::dump(std::ostream& output) {
    const bool saveDefaults = _model->getPersistence()->getOption(ModelPersistence_if::Options::SAVEDEFAULTS);

    // grab simulator info
    auto simulator = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
    simulator->saveField("typename", "Simulator");
    simulator->saveField("id", 0);
    simulator->saveField("name", _model->getParentSimulator()->getName());
    simulator->saveField("versionNumber", _model->getParentSimulator()->getVersionNumber());

    // grab model metadata
    auto meta = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
    meta->saveField("id", 0);
    _model->getInfos()->saveInstance(meta.get());

    // grab simulation params
    auto simulation = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
    simulation->saveField("id", 0);
    _model->getSimulation()->saveInstance(simulation.get(), saveDefaults);

    // grab data definitions
    std::vector<std::unique_ptr<PersistenceRecord>> elements;
    const std::string counter = Util::TypeOf<Counter>();
    const std::string statsCollector = Util::TypeOf<StatisticsCollector>();
    for (auto& type : *_model->getDataManager()->getDataDefinitionClassnames()) {
        if (type == statsCollector || type == counter) continue; // these don't need to be saved
        _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing elements of type \"" + type + "\":");
        Util::IncIndent();
        for (ModelDataDefinition *data : *_model->getDataManager()->getDataDefinitionList(type)->list()) {
            _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing " + type + " \"" + data->getName() + "\"");
            auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
            data->SaveInstance(fields.get(), data);
            elements.push_back(std::move(fields));
        }
        Util::DecIndent();
    }

    // grab model components
    std::vector<std::unique_ptr<PersistenceRecord>> components;
    _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, "Writing components\":");
    Util::IncIndent();
    for (ModelComponent* component : *_model->getComponents()) {
        if (component->getLevel() == 0) {
            auto fields = std::unique_ptr<PersistenceRecord>(newPersistenceRecord());
            component->SaveInstance(fields.get(), component);
            components.push_back(std::move(fields));
        }
    }
    Util::DecIndent();

    // now we actually flush the just-gathered contents
    _model->getTracer()->trace(TraceManager::Level::L7_internal, "Saving file");
    Util::IncIndent();
    {
        output << "# Genesys Simulation Model\n";
        output << "# Simulator, Model and Simulation infos\n";
        output << linearize(simulator.get());
        output << linearize(meta.get());
        output << linearize(simulation.get());
        output << "\n# Model Data Definitions\n";
        for (auto& element : elements) output << linearize(element.get());
        output << "\n# Model Components\n";
        for (auto& component : components) output << linearize(component.get());
    }
    Util::DecIndent();

    return true;
}


std::string GenSerializer::linearize(PersistenceRecord *fields) {
    // linearize fields
    std::string id, type, name, attrs;
    for (auto& field : *fields) {
        if (field.first == "id") id = field.second;
        else if (field.first == "typename") type = field.second;
        else if (field.first == "name") name = field.second;
        else attrs += field.first + "=" + field.second + " ";
    }

    // add padding
    while (id.length() < 3) id += " ";
    while (type.length() < 10) type += " ";

    // compose line
    std::string line = id + " " + type + " " + name + " " + attrs + "\n";

    return line;
};


bool GenSerializer::load(std::istream& input) {
    bool res = true;
    std::string line;
    while (std::getline(input, line) && res) {
        line = trim(line);
        if (line.substr(0, 1) == "#" || line.empty()) continue;
        _model->getTracer()->trace(TraceManager::Level::L9_mostDetailed, line);

        // replaces every "quoted" string by {stringX}
        std::regex regexQuoted("\"([^\"]*)\"");
        auto matches_begin = std::sregex_iterator(line.begin(), line.end(), regexQuoted);
        auto matches_end = std::sregex_iterator();
        std::unordered_map<std::string, std::string> strings{};
        int i = 0;
        for (std::sregex_iterator it = matches_begin; it != matches_end; it++, i++) {
            std::string match_str = (*it).str();
            std::string subst = "{string" + std::to_string(i) + "}";
            strings[subst] = match_str;
        }
        for (auto& it : strings) {
            std::string match_str = it.second;
            unsigned int pos = line.find(match_str, 0);
            line.replace(pos, match_str.length(), it.first);
        }

        // split on " "
        std::regex regex{R"([\s]+)"}; 
        std::sregex_token_iterator tit{line.begin(), line.end(), regex, -1};
        std::vector<std::string> lstfields{tit,{}};
        auto fields = std::unique_ptr<PersistenceRecord>(this->newPersistenceRecord());
        // for each field, separate key and value and form a record
        regex = {R"([=]+)"};
        i = 0;
        for (auto it = lstfields.begin(); it != lstfields.end(); it++, i++) {
            std::string key, val;
            // 
            tit = {(*it).begin(), (*it).end(), regex, -1};
            std::vector<std::string> veckeyval = {tit,{}};
            veckeyval[0] = trim((veckeyval[0]));
            if (veckeyval[0] != "") { 
                if (veckeyval.size() > 1) {
                    veckeyval[1] = trim((veckeyval[1]));
                    if (veckeyval[1].substr(0, 1) == "\"" && veckeyval[1].substr(veckeyval[1].length() - 1, 1) == "\"") { // remove ""
                        veckeyval[1] = veckeyval[1].substr(1, veckeyval[1].length() - 2);
                    }
                    veckeyval[1] = std::regex_replace(veckeyval[1], std::regex("\\\\_"), " ");
                    key = veckeyval[0];
                    val = veckeyval[1];
                } else {
                    if (i == 0) {
                        key = "id";
                        val = veckeyval[0];
                    } else if (i == 1) {
                        key = "typename";
                        val = veckeyval[0];
                    } else if (i == 2) {
                        key = "name";
                        val = veckeyval[0];
                    } else {
                        key = veckeyval[0];
                        val = "";
                    }
                }
            }
            // replaces back {stringX} by the strings themselves before saving
            auto sit = strings.find(val);
            if (sit != strings.end()) {
                auto& str = sit->second;
                val = str.substr(1, str.length() - 2);
            }
            fields->insert({key, val}); // notice the raw insert
        }

        // then, save each record
        std::string type = fields->loadField("typename");
        if (type == "") return false;
        Util::identification id = fields->loadField("id", 0);
        std::string name = id == 0 ? type : fields->loadField("name", "$" + std::to_string(id));
        res = put(name, type, id, fields.get());
    }
    return res;
}


bool GenSerializer::get(const std::string& name, PersistenceRecord *entry) {
    assert(entry != nullptr);
    auto it = _components.find(name);
    if (it == _components.end()) return false;
    entry->insert(it->second->begin(), it->second->end());
    return true;
}


bool GenSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
    assert(fields != nullptr);
    auto saved = std::unique_ptr<PersistenceRecord>(this->newPersistenceRecord());
    saved->insert(fields->begin(), fields->end());
    saved->saveField("name", name);
    saved->saveField("typename", type);
    saved->saveField("id", id);
    _components[name] = std::move(saved);
    return true;
}


int GenSerializer::for_each(std::function<int(const std::string&)> delegate) {
    for (auto& entry : _components) {
        int stop = delegate(entry.first);
        if (stop) return stop;
    }
    return 0;
}
