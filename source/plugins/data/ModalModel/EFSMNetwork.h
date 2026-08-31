/*
 * File:   EFSMNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef EFSMNETWORK_H
#define EFSMNETWORK_H

#include "DefaultNetwork.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"

#include <string>
#include <unordered_map>

/*!
 * \brief Extended finite-state-machine network specialization.
 *
 * `EFSMNetwork` owns the modal state machine semantics that used to live in
 * `ModalModelDefault`: the current state, the initial state, state nodes and
 * EFSM transitions. A `ModalModelDefault` only adapts process entities to this
 * network through explicit input/output bindings.
 *
 * This first implementation supports one deterministic activation step:
 * evaluate outgoing transitions of the current state, select the enabled
 * transition with the lowest priority value, update the current state, and
 * expose the transition output expression on output port 0 when present.
 */
class EFSMNetwork : public DefaultNetwork {
public:
	EFSMNetwork(Model* model, std::string name = "");
	virtual ~EFSMNetwork() override = default;

public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addState(FSMState* state);
	void removeState(FSMState* state);
	List<FSMState*>* getStates() const;
	void addTransition(EFSMTransition* transition);
	void removeTransition(EFSMTransition* transition);
	List<EFSMTransition*>* getTransitions() const;
	void setInitialState(FSMState* state);
	FSMState* getInitialState() const;
	FSMState* getCurrentState() const;
	void setCurrentState(FSMState* state);

public:
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual NetworkActivationResult _activate(const NetworkActivationFrame& frame) override;

private:
	FSMState* _resolveInitialState();

private:
	List<FSMState*>* _states = new List<FSMState*>();
	List<EFSMTransition*>* _transitions = new List<EFSMTransition*>();
	FSMState* _initialState = nullptr;
	FSMState* _currentState = nullptr;
};

#endif /* EFSMNETWORK_H */
