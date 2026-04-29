#ifndef BIOSPECIES_H
#define BIOSPECIES_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Biochemical state variable used by BioReaction and BioNetwork.
 *
 * A BioSpecies stores an initial amount for replication reset and a current
 * amount for simulation. Constant and boundary-condition flags follow SBML-like
 * semantics: BioNetwork still uses the current amount as kinetic input, but it
 * must not overwrite the species amount while advancing the ODE state.
 */
class BioSpecies : public ModelDataDefinition {
public:
	BioSpecies(Model* model, std::string name = "");
	virtual ~BioSpecies() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setInitialAmount(double initialAmount);
	double getInitialAmount() const;
	void setAmount(double amount);
	double getAmount() const;
	void setConstant(bool constant);
	bool isConstant() const;
	void setBoundaryCondition(bool boundaryCondition);
	bool isBoundaryCondition() const;
	void setUnit(std::string unit);
	std::string getUnit() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		double initialAmount = 0.0;
		double amount = 0.0;
		bool constant = false;
		bool boundaryCondition = false;
		std::string unit = "";
	} DEFAULT;

	double _initialAmount = DEFAULT.initialAmount;
	double _amount = DEFAULT.amount;
	bool _constant = DEFAULT.constant;
	bool _boundaryCondition = DEFAULT.boundaryCondition;
	std::string _unit = DEFAULT.unit;
};

#endif /* BIOSPECIES_H */
