#include "Persistence.h"


PersistenceRecord::PersistenceRecord(ModelPersistence_if& config):
    _fields{}, _config(config)
{}

std::size_t PersistenceRecord::size() const {
    return _fields.size();
}

void PersistenceRecord::insert(PersistenceRecord::Entry entry) {
    _fields[entry.first] = entry;
}

void PersistenceRecord::insert(Iterator it, Iterator end) {
    for (; it != end; ++it) insert(it->second);
}

void PersistenceRecord::erase(const std::string& key) {
    _fields.erase(key);
}

void PersistenceRecord::clear() {
    _fields.clear();
}

PersistenceRecord::Iterator PersistenceRecord::find(const std::string& key) {
    return _fields.find(key);
}

PersistenceRecord::Iterator PersistenceRecord::begin() {
    return _fields.begin();
}

PersistenceRecord::Iterator PersistenceRecord::end() {
    return _fields.end();
}


PersistenceRecord::~PersistenceRecord() {}

PersistenceRecord* PersistenceRecord::newInstance() {
    return new PersistenceRecord(_config);
}

std::string PersistenceRecord::loadField(std::string key, std::string defaultValue) {
    auto it = _fields.find(key);
    return it != _fields.end() ? it->second.second : defaultValue;
}

double PersistenceRecord::loadField(std::string key, double defaultValue) {
    auto it = _fields.find(key);
    return it != _fields.end() ? std::stod(it->second.second) : defaultValue;
}

unsigned int PersistenceRecord::loadField(std::string key, unsigned int defaultValue) {
    auto it = _fields.find(key);
    return it != _fields.end() ? std::stoi(it->second.second) : defaultValue;
}

int PersistenceRecord::loadField(std::string key, int defaultValue) {
    auto it = _fields.find(key);
    return it != _fields.end() ? std::stoi(it->second.second) : defaultValue;
}

Util::TimeUnit PersistenceRecord::loadField(std::string key, Util::TimeUnit defaultValue) {
    auto it = _fields.find(key);
    return it != _fields.end() ? static_cast<Util::TimeUnit>(std::stoi(it->second.second)) : defaultValue;
}

void PersistenceRecord::saveField(std::string key, std::string value, const std::string defaultValue, const bool saveIfDefault) {
	if (saveIfDefault || (value != defaultValue)) saveField(key, value);
}

void PersistenceRecord::saveField(std::string key, std::string value) {
    _fields[key] = { key, value, Entry::text };
}

void PersistenceRecord::saveField(std::string key, double value, const double defaultValue, const bool saveIfDefault) {
    if (saveIfDefault || (value != defaultValue)) _fields[key] = { key, std::to_string(value), Entry::Kind::numeric };
}

void PersistenceRecord::saveField(std::string key, unsigned int value, const unsigned int defaultValue, const bool saveIfDefault) {
    if (saveIfDefault || (value != defaultValue)) _fields[key] = { key, std::to_string(value), Entry::Kind::numeric };
}

void PersistenceRecord::saveField(std::string key, unsigned int value) {
    _fields[key] = { key, std::to_string(value), Entry::Kind::numeric };
}

void PersistenceRecord::saveField(std::string key, int value, const int defaultValue, const bool saveIfDefault) {
    if (saveIfDefault || (value != defaultValue)) _fields[key] =  { key, std::to_string(value), Entry::Kind::numeric };
}

void PersistenceRecord::saveField(std::string key, Util::TimeUnit value, const Util::TimeUnit defaultValue, const bool saveIfDefault) {
    if (saveIfDefault || (value != defaultValue)) _fields[key] =  { key, std::to_string(static_cast<int>(value)), Entry::Kind::numeric };
}
