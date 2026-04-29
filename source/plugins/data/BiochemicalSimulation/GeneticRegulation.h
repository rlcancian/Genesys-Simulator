#ifndef GENETICREGULATION_H
#define GENETICREGULATION_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Transcriptional regulation relationship for genetic circuits.
 */
class GeneticRegulation : public ModelDataDefinition {
public:
	GeneticRegulation(Model* model, std::string name = "");
	virtual ~GeneticRegulation() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setRegulatorSpeciesName(std::string regulatorSpeciesName);
	std::string getRegulatorSpeciesName() const;
	void setTargetPartName(std::string targetPartName);
	std::string getTargetPartName() const;
	void setRegulationType(std::string regulationType);
	std::string getRegulationType() const;
	void setHillCoefficient(double hillCoefficient);
	double getHillCoefficient() const;
	void setDissociationConstant(double dissociationConstant);
	double getDissociationConstant() const;
	void setMaxFoldChange(double maxFoldChange);
	double getMaxFoldChange() const;
	void setLeakiness(double leakiness);
	double getLeakiness() const;
	void setEnabled(bool enabled);
	bool isEnabled() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		std::string regulatorSpeciesName = "";
		std::string targetPartName = "";
		std::string regulationType = "Activation";
		double hillCoefficient = 1.0;
		double dissociationConstant = 1.0;
		double maxFoldChange = 1.0;
		double leakiness = 0.0;
		bool enabled = true;
	} DEFAULT;

	std::string _regulatorSpeciesName = DEFAULT.regulatorSpeciesName;
	std::string _targetPartName = DEFAULT.targetPartName;
	std::string _regulationType = DEFAULT.regulationType;
	double _hillCoefficient = DEFAULT.hillCoefficient;
	double _dissociationConstant = DEFAULT.dissociationConstant;
	double _maxFoldChange = DEFAULT.maxFoldChange;
	double _leakiness = DEFAULT.leakiness;
	bool _enabled = DEFAULT.enabled;
};

#endif /* GENETICREGULATION_H */
