#ifndef GENSERIALIZER_H
#define GENSERIALIZER_H

#include <unordered_map>

#include "ModelSerializer.h"
#include "Model.h"


class GenSerializer : public ModelSerializer {
public:
    explicit GenSerializer(Model *model);

public: // ModelSerializer interface
    PersistenceRecord* newPersistenceRecord() override;
    bool dump(std::ostream& output) override;
    bool load(std::istream& input) override;
    bool get(const std::string& name, PersistenceRecord *entry) override;
    bool put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) override;
    int for_each(std::function<int(const std::string&)> delegate) override;

private:
    static std::string linearize(PersistenceRecord *fields);
    friend class ModelPersistenceDefaultImpl2;

private:
    Model *_model{};
    std::unordered_map<std::string, std::unique_ptr<PersistenceRecord>> _components{};
};

#endif // GENSERIALIZER_H
