/*
 * File:   DefaultNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef DEFAULTNETWORK_H
#define DEFAULTNETWORK_H

#include <string>
#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/util/List.h"
#include "NetworkActivation.h"

class Counter;

/*!
 * \brief Base class for a formalism-agnostic network model of computation
 * (EFSM, DTMC, Coloured Petri Net, ...) that may be shared by several
 * `ModalModel` process adapters.
 *
 * This is the `DefaultNetwork` described in
 * `docs/ai_assistants/reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md`
 * §4.2, §6-§9: a `ModelDataDefinition` (not a `ModelComponent`) that owns
 * its declared input/output port schema, its own persistent runtime state,
 * and an activation counter, and that never receives a GenESyS `Entity*`.
 * Several attached `ModalModel` components may reference and activate the
 * same `DefaultNetwork` instance; the network is the sole source of truth
 * for its state (architecture reference §5, §17-§18).
 *
 * This base class only implements the infrastructure genuinely common to
 * every formalism: port schema declaration/lookup, the activation counter,
 * and the activation entry point. It intentionally has no notion of nodes,
 * transitions, markings or probabilities — those belong to a
 * specialization (`EFSMNetwork`, `MarkovChainNetwork`,
 * `ColoredPetriNetNetwork`, ...) that overrides `_activate()`.
 *
 * `activate()` always increments the activation counter, even when the
 * specialization's `_activate()` produces no state change and no present
 * outputs (architecture reference §21) — the counter counts activations,
 * not transitions/firings.
 */
class DefaultNetwork : public ModelDataDefinition {
public:
	DefaultNetwork(Model* model, std::string name = "", std::string dataDefinitionTypename = Util::TypeOf<DefaultNetwork>());
	virtual ~DefaultNetwork() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual std::string show() override;
public: // port schema
	/*! \brief Declares a new input port; returns its 0-based index. Rejects a duplicate name (returns the existing index instead of creating a duplicate). */
	unsigned int addInputPort(const std::string& portName);
	/*! \brief Declares a new output port; returns its 0-based index. Rejects a duplicate name (returns the existing index instead of creating a duplicate). */
	unsigned int addOutputPort(const std::string& portName);
	unsigned int getNumInputPorts() const;
	unsigned int getNumOutputPorts() const;
	std::string getInputPortName(unsigned int index) const;
	std::string getOutputPortName(unsigned int index) const;
	/*! \brief Returns the input port index for \p portName, or -1 when not declared. */
	int getInputPortIndex(const std::string& portName) const;
	/*! \brief Returns the output port index for \p portName, or -1 when not declared. */
	int getOutputPortIndex(const std::string& portName) const;
public: // activation
	/*! \brief Number of times activate() has been called since the last replication reset. */
	double getActivationCount() const;
	/*!
	 * \brief Runs exactly one network activation for \p frame.
	 * \details Increments the activation counter unconditionally, then
	 * delegates formalism-specific behavior to the protected virtual
	 * `_activate()`. The base implementation of `_activate()` produces an
	 * all-absent result, so a `DefaultNetwork` used directly (without a
	 * specialization) is a valid, inert network.
	 */
	NetworkActivationResult activate(const NetworkActivationFrame& frame);
protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalStatisticReporters() override;
	/*!
	 * \brief Formalism-specific activation behavior.
	 * \details The base implementation performs no state change and
	 * returns a result sized to `getNumOutputPorts()` with every output
	 * absent. Specializations override this to implement EFSM transition
	 * firing, one DTMC step, one CPN firing step, etc. Must not receive or
	 * depend on `Entity*` (architecture reference §8.4, §19).
	 */
	virtual NetworkActivationResult _activate(const NetworkActivationFrame& frame);
private:
	List<std::string>* _inputPortNames = new List<std::string>();
	List<std::string>* _outputPortNames = new List<std::string>();
	Counter* _activationCounter = nullptr;
};

#endif /* DEFAULTNETWORK_H */
