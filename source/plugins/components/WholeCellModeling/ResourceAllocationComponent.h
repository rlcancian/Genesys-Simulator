#ifndef RESOURCEALLOCATIONCOMPONENT_H
#define RESOURCEALLOCATIONCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Fair allocation of molecular machines (RNAP, ribosomes) before each SSA step.
 *
 * Implements the "ResourceAllocation" contract from Karr et al. (2012):
 * available RNAP and ribosome counts are divided proportionally across active
 * molecular species, then written to WholeCellState::setResourceBudget() so
 * StochasticTranscription and StochasticTranslation can honour the budget.
 *
 * Allocation logic:
 *   - Count N_mRNA = number of keys in WholeCellState molecule counts whose
 *     name starts with mRNASpeciesPrefix (default "mRNA_").
 *   - Count N_protein = number of keys starting with proteinSpeciesPrefix
 *     (default "protein_").
 *   - RNAP budget per mRNA gene = floor(RNAP_free / max(N_mRNA, 1))
 *   - Ribosome budget per protein = floor(ribosome_free / max(N_mRNA, 1))
 *     (per active mRNA transcript, not per protein species)
 *   - Write per-species budget to WholeCellState::setResourceBudget().
 *
 * This component must fire BEFORE StochasticTranscription and StochasticTranslation
 * in the simulation pipeline.
 */
class ResourceAllocationComponent : public ModelComponent {
public:
	ResourceAllocationComponent(Model* model, std::string name = "");
	virtual ~ResourceAllocationComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setRnapCountKey(std::string key);
	std::string getRnapCountKey() const;
	void setRibosomeCountKey(std::string key);
	std::string getRibosomeCountKey() const;
	void setMRNASpeciesPrefix(std::string prefix);
	std::string getMRNASpeciesPrefix() const;
	void setProteinSpeciesPrefix(std::string prefix);
	std::string getProteinSpeciesPrefix() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	virtual void _createEditableDataDefinitions() override;

private:
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName   = "";
		std::string rnapCountKey         = "RNAP_free";
		std::string ribosomeCountKey     = "ribosome_free";
		std::string mRNASpeciesPrefix    = "mRNA_";
		std::string proteinSpeciesPrefix = "protein_";
	} DEFAULT;

	WholeCellState* _wholeCellState  = nullptr;
	std::string _rnapCountKey        = DEFAULT.rnapCountKey;
	std::string _ribosomeCountKey    = DEFAULT.ribosomeCountKey;
	std::string _mRNASpeciesPrefix   = DEFAULT.mRNASpeciesPrefix;
	std::string _proteinSpeciesPrefix = DEFAULT.proteinSpeciesPrefix;
};

#endif /* RESOURCEALLOCATIONCOMPONENT_H */
