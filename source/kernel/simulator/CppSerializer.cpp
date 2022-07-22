#include "CppSerializer.h"


CppSerializer::CppSerializer(Model *model):
    _model(model)
{}


PersistenceRecord* CppSerializer::newPersistenceRecord() {
    // FIXME: implement
    return nullptr;
}


bool CppSerializer::dump(std::ostream& output) {
    // FIXME: implement
    return false;
}


bool CppSerializer::load(std::istream& input) {
    // FIXME: implement
    return false;
}


bool CppSerializer::get(const std::string& name, PersistenceRecord *entry) {
    // FIXME: implement
    return false;
}


bool CppSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
    // FIXME: implement
    return false;
}


int CppSerializer::for_each(std::function<int(const std::string&)> delegate) {
    // FIXME: implement
    return -1;
}
