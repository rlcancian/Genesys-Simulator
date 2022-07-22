#include "XmlSerializer.h"


XmlSerializer::XmlSerializer(Model *model):
    _model(model)
{}


PersistenceRecord* XmlSerializer::newPersistenceRecord() {
    // FIXME: implement
    return nullptr;
}


bool XmlSerializer::dump(std::ostream& output) {
    // FIXME: implement
    return false;
}


bool XmlSerializer::load(std::istream& input) {
    // FIXME: implement
    return false;
}


bool XmlSerializer::get(const std::string& name, PersistenceRecord *entry) {
    // FIXME: implement
    return false;
}


bool XmlSerializer::put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) {
    // FIXME: implement
    return false;
}


int XmlSerializer::for_each(std::function<int(const std::string&)> delegate) {
    // FIXME: implement
    return -1;
}
