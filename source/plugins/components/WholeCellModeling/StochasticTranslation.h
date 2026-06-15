#ifndef STOCHASTICTRANSLATION_H
#define STOCHASTICTRANSLATION_H

#include <random>
#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Stochastic protein synthesis via Poisson tau-leaping for whole-cell simulations.
 *
 * Models the M. genitalium Translation process (Karr et al. 2012):
 *   λ_i = (ribosomeElongationRate / meanProteinLength) × mRNA_count_i
 *           × ribosome_free / max(1, total_mRNA) × Δt
 *   N_protein_i ~ Poisson(λ_i)
 *
 * Each mRNA template (prefix proteinSpeciesPrefix) is matched to its
 * corresponding mRNA species (prefix mRNASpeciesPrefix) in WholeCellState.
 * Free ribosome count is read from WholeCellState with key ribosomeCountKey.
 *
 * Parameters loaded from WholeCellState (set by loadParameters()):
 *   param.ribosomeElongationRate  — 16 AA/s (from parameters.json)
 *
 * mRNA degradation is not modeled here; use a separate StochasticReactionRule
 * for mRNA decay (first-order, rate ~1/mRNA_halflife).
 */
class StochasticTranslation : public ModelComponent {
public:
	StochasticTranslation(Model* model, std::string name = "");
	virtual ~StochasticTranslation() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setElongationRate(double rate);
	double getElongationRate() const;
	void setMeanProteinLength(double length);
	double getMeanProteinLength() const;
	void setTimeWindow(double window);
	double getTimeWindow() const;
	void setMRNASpeciesPrefix(std::string prefix);
	std::string getMRNASpeciesPrefix() const;
	void setProteinSpeciesPrefix(std::string prefix);
	std::string getProteinSpeciesPrefix() const;
	void setRibosomeCountKey(std::string key);
	std::string getRibosomeCountKey() const;
	void setRandomSeed(unsigned int seed);
	unsigned int getRandomSeed() const;
	int getLastSynthesizedCount() const;

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
		std::string wholeCellStateName  = "";
		double elongationRate           = 16.0;    // AA/s — M. genitalium ribosome (parameters.json)
		double meanProteinLength        = 300.0;   // AA   — approximate M. genitalium average
		double timeWindow               = 1.0;     // s
		std::string mRNASpeciesPrefix   = "mRNA_";
		std::string proteinSpeciesPrefix= "prot_";
		std::string ribosomeCountKey    = "ribosome_free";
		unsigned int randomSeed         = 43u;
	} DEFAULT;

	WholeCellState* _wholeCellState     = nullptr;
	double _elongationRate              = DEFAULT.elongationRate;
	double _meanProteinLength           = DEFAULT.meanProteinLength;
	double _timeWindow                  = DEFAULT.timeWindow;
	std::string _mRNASpeciesPrefix      = DEFAULT.mRNASpeciesPrefix;
	std::string _proteinSpeciesPrefix   = DEFAULT.proteinSpeciesPrefix;
	std::string _ribosomeCountKey       = DEFAULT.ribosomeCountKey;
	unsigned int _randomSeed            = DEFAULT.randomSeed;
	int _lastSynthesizedCount           = 0;
	std::mt19937 _rng{DEFAULT.randomSeed};
};

#endif /* STOCHASTICTRANSLATION_H */
