#ifndef BIOPARAMETER_H
#define BIOPARAMETER_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

class BioParameter : public ModelDataDefinition {
public:
	BioParameter(Model* model, std::string name = "");
	virtual ~BioParameter() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setValue(double value);
	double getValue() const;
	void setUnit(std::string unit);
	std::string getUnit() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	const struct DEFAULT_VALUES {
		double value = 0.0;
		std::string unit = "";
	} DEFAULT;

	double _value = DEFAULT.value;
	std::string _unit = DEFAULT.unit;
};

#endif /* BIOPARAMETER_H */
