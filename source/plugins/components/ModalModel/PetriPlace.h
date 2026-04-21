#pragma once

#include "DefaultNode.h"
#include <map>

class PetriPlace : public DefaultNode {
public:
	PetriPlace(Model* model, std::string name = "");
	virtual ~PetriPlace() = default;

public:
	unsigned int getTokens(std::string color = "default") const;
	void addTokens(unsigned int quantity, std::string color = "default");
	bool removeTokens(unsigned int quantity, std::string color = "default");
	std::map<std::string, unsigned int>* getAllTokens();

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	std::map<std::string, unsigned int>* _tokensByColor = new std::map<std::string, unsigned int>();
};
