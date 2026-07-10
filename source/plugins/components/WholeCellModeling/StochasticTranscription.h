#ifndef STOCHASTICTRANSCRIPTION_H
#define STOCHASTICTRANSCRIPTION_H

#include <random>
#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Stochastic mRNA synthesis via Poisson tau-leaping for whole-cell simulations.
 *
 * Models the M. genitalium Transcription process (Karr et al. 2012):
 *   λ_i = (elongationRate / meanGeneLength) × bindingProbability × RNAP_free × Δt
 *   N_synth_i ~ Poisson(λ_i)
 *
 * This is a tau-leaping approximation (not full Gillespie) appropriate for
 * transcription because RNAP count (~200) acting over many genes (~473) with
 * a 1–10 s window produces moderate propensities where the Poisson approximation
 * is valid. Full Gillespie is implemented in StochasticReactionComponent for
 * individual reactions with low copy numbers.
 *
 * Parameters loaded from WholeCellState (set by loadParameters()):
 *   param.rnaPolymeraseElongationRate  — 50 nt/s (from parameters.json)
 *
 * mRNA species in WholeCellState are identified by the prefix set in
 * mRNASpeciesPrefix (default "mRNA_"). RNAP free count is read from the
 * WholeCellState molecule count with key rnapCountKey (default "RNAP_free").
 */
class StochasticTranscription : public ModelComponent {
public:
	StochasticTranscription(Model* model, std::string name = "");
	virtual ~StochasticTranscription() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setElongationRate(double rate);
	double getElongationRate() const;
	void setMeanGeneLength(double length);
	double getMeanGeneLength() const;
	void setBindingProbability(double prob);
	double getBindingProbability() const;
	void setTimeWindow(double window);
	double getTimeWindow() const;
	void setMRNASpeciesPrefix(std::string prefix);
	std::string getMRNASpeciesPrefix() const;
	void setRnapCountKey(std::string key);
	std::string getRnapCountKey() const;
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
		double elongationRate           = 50.0;   // nt/s — M. genitalium RNA pol (parameters.json)
		double meanGeneLength           = 900.0;  // nt   — approximate M. genitalium average
		double bindingProbability       = 1.0;    // dimensionless; scale to calibrate output
		double timeWindow               = 1.0;    // s
		std::string mRNASpeciesPrefix   = "mRNA_";
		std::string rnapCountKey        = "RNAP_free";
		unsigned int randomSeed         = 42u;
	} DEFAULT;

	WholeCellState* _wholeCellState   = nullptr;
	double _elongationRate            = DEFAULT.elongationRate;
	double _meanGeneLength            = DEFAULT.meanGeneLength;
	double _bindingProbability        = DEFAULT.bindingProbability;
	double _timeWindow                = DEFAULT.timeWindow;
	std::string _mRNASpeciesPrefix    = DEFAULT.mRNASpeciesPrefix;
	std::string _rnapCountKey         = DEFAULT.rnapCountKey;
	unsigned int _randomSeed          = DEFAULT.randomSeed;
	int _lastSynthesizedCount         = 0;
	std::mt19937 _rng{DEFAULT.randomSeed};
};

#endif /* STOCHASTICTRANSCRIPTION_H */
