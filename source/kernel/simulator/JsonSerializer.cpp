#include "JsonSerializer.h"


JsonSerializer::JsonSerializer(Model *model):
    _model(model)
{}


PersistenceRecord* JsonSerializer::newPersistenceRecord() {
    // FIXME: implement
    return nullptr;
}


bool JsonSerializer::dump(std::ostream& output) {
    // FIXME: implement
    return false;
}


bool JsonSerializer::load(std::istream& input) {
    // FIXME: implement
    return false;
}


bool JsonSerializer::get(const std::string& name, PersistenceRecord *entry) {
    // FIXME: implement
    return false;
}


bool JsonSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
    // FIXME: implement
    return false;
}


int JsonSerializer::for_each(std::function<int(const std::string&)> delegate) {
    // FIXME: implement
    return -1;
}
