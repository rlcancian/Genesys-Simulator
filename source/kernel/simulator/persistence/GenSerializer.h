#ifndef GENSERIALIZER_H
#define GENSERIALIZER_H

#include <unordered_map>
#include<memory>

#include "Serializer.h"
#include "../model/Model.h"

class GenSerializer : public Serializer {
public:
	explicit GenSerializer(Model *model);

public: // Serializer interface
	PersistenceRecord* newPersistenceRecord() override;
	bool dump(std::ostream& output) override;
	bool load(std::istream& input) override;
	bool get(const std::string& name, PersistenceRecord *entry) override;
	bool put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) override;
	int for_each(std::function<int(const std::string&) > delegate) override;

private:
	static std::string linearize(PersistenceRecord *fields);
	friend class PersistenceDefaultImpl2;

private:
	Model *_model{};
	std::unordered_map<std::string, std::unique_ptr<PersistenceRecord>> _components
	{
	};
};

#endif // GENSERIALIZER_H
