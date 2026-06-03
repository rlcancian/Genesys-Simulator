#ifndef FTSZPOLYMERIZATIONCOMPONENT_H
#define FTSZPOLYMERIZATIONCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * FtsZ ring polymerization dynamics for whole-cell division timing.
 *
 * Implements a simplified two-state Euler ODE for the FtsZ ring completion
 * fraction f ∈ [0,1], stored as a per-mille integer (0–1000) in WholeCellState
 * under the key ftsZRingKey (default "FtsZ_ring_completion").
 *
 * Monomer availability is read from WholeCellState under ftsZMonomerKey
 * (default "FtsZ_monomer"). The reference count N_ref is captured on the
 * first _onDispatchEvent call (N_m at t=0) and held constant; the fraction
 * N_m/N_ref scales the polymerization rate (lower monomer count = slower growth).
 *
 * ODE (per step):
 *   f_new = f + (k_poly * phi * (1-f) - k_depoly * f) * deltaT
 * where
 *   k_poly   = activationFwd * volumeNorm   [/s]
 *   k_depoly = activationRev * volumeNorm   [/s]
 *   phi      = N_m / N_ref                 [dimensionless monomer availability]
 *
 * Default volumeNorm = 3.94e-5 is calibrated so that k_poly ≈ 4.3e-5/s, which
 * gives ~35% ring completion at t ≈ 12 000 s — consistent with the M. genitalium
 * cell cycle. activationFwd and activationRev are read from WholeCellState
 * "param.ftsZActivationFwd"/"param.ftsZActivationRev" when loadParameters()
 * has been called; otherwise the component uses its own stored defaults.
 */
class FtsZPolymerizationComponent : public ModelComponent {
public:
	FtsZPolymerizationComponent(Model* model, std::string name = "");
	virtual ~FtsZPolymerizationComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setActivationFwd(double rate);
	double getActivationFwd() const;
	void setActivationRev(double rate);
	double getActivationRev() const;
	void setVolumeNorm(double norm);
	double getVolumeNorm() const;
	void setDeltaT(double dt);
	double getDeltaT() const;
	void setFtsZRingKey(std::string key);
	std::string getFtsZRingKey() const;
	void setFtsZMonomerKey(std::string key);
	std::string getFtsZMonomerKey() const;

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
		std::string wholeCellStateName = "";
		double activationFwd  = 1.1;    // /s — processes.FtsZPolymerization.activationFwd
		double activationRev  = 0.01;   // /s — derived from Karr 2012 ratio
		double volumeNorm     = 3.94e-5; // dimensionless — calibrated for 35% at ~12000 s
		double deltaT         = 60.0;   // s — must match clock step
		std::string ftsZRingKey   = "FtsZ_ring_completion";
		std::string ftsZMonomerKey = "FtsZ_monomer";
	} DEFAULT;

	WholeCellState* _wholeCellState  = nullptr;
	double _activationFwd   = DEFAULT.activationFwd;
	double _activationRev   = DEFAULT.activationRev;
	double _volumeNorm      = DEFAULT.volumeNorm;
	double _deltaT          = DEFAULT.deltaT;
	std::string _ftsZRingKey    = DEFAULT.ftsZRingKey;
	std::string _ftsZMonomerKey = DEFAULT.ftsZMonomerKey;
	int _nRef = 0;  // reference monomer count captured on first step
};

#endif /* FTSZPOLYMERIZATIONCOMPONENT_H */
