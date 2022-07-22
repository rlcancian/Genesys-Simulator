#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include "ModelSerializer.h"
#include "Model.h"


class JsonSerializer : public ModelSerializer {
public:
    explicit JsonSerializer(Model *model);

public: // ModelSerializer interface
    PersistenceRecord* newPersistenceRecord() override;
    bool dump(std::ostream& output) override;
    bool load(std::istream& input) override;
    bool get(const std::string& name, PersistenceRecord *entry) override;
    bool put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) override;
    int for_each(std::function<int(const std::string&)> delegate) override;

private:
    Model *_model{};
};

#endif // JSONSERIALIZER_H
