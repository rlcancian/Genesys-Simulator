#ifndef GENETICCIRCUITPART_H
#define GENETICCIRCUITPART_H

#include <string>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Genetic circuit structural part definition.
 *
 * This data definition represents a reusable genetic part (promoter, RBS, CDS,
 * terminator, etc.) and optionally links it to the BioSpecies produced by the
 * part expression flow.
 */
class GeneticCircuitPart : public ModelDataDefinition {
public:
	GeneticCircuitPart(Model* model, std::string name = "");
	virtual ~GeneticCircuitPart() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setPartType(std::string partType);
	std::string getPartType() const;
	void setSequence(std::string sequence);
	std::string getSequence() const;
	void setProductSpeciesName(std::string productSpeciesName);
	std::string getProductSpeciesName() const;
	void setCopyNumber(double copyNumber);
	double getCopyNumber() const;
	void setBasalExpressionRate(double basalExpressionRate);
	double getBasalExpressionRate() const;
	void setDegradationRate(double degradationRate);
	double getDegradationRate() const;
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
		std::string partType = "Promoter";
		std::string sequence = "";
		std::string productSpeciesName = "";
		double copyNumber = 1.0;
		double basalExpressionRate = 0.0;
		double degradationRate = 0.0;
		bool enabled = true;
	} DEFAULT;

	std::string _partType = DEFAULT.partType;
	std::string _sequence = DEFAULT.sequence;
	std::string _productSpeciesName = DEFAULT.productSpeciesName;
	double _copyNumber = DEFAULT.copyNumber;
	double _basalExpressionRate = DEFAULT.basalExpressionRate;
	double _degradationRate = DEFAULT.degradationRate;
	bool _enabled = DEFAULT.enabled;
};

#endif /* GENETICCIRCUITPART_H */
