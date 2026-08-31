/*
 * File:   MarkovChainNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 31 de Agosto de 2026
 */

#ifndef MARKOVCHAINNETWORK_H
#define MARKOVCHAINNETWORK_H

#include "DefaultNetwork.h"
#include "plugins/data/ModalModel/MarkovState.h"

#include <string>
#include <vector>

class Sampler_if;

/*!
 * \brief Finite time-homogeneous Discrete-Time Markov Chain network.
 *
 * One activation performs exactly one Markov step from the current state by
 * sampling the outgoing transition probabilities of that state using the
 * GenESyS sampler infrastructure. The current state belongs to the network,
 * not to a process Entity attribute.
 */
class MarkovChainNetwork : public DefaultNetwork {
public:
	class MarkovTransition {
	public:
		MarkovTransition(MarkovState* source = nullptr, MarkovState* destination = nullptr, double probability = 1.0, std::string name = "");
		virtual ~MarkovTransition() = default;

	public:
		void setSource(MarkovState* source);
		MarkovState* getSource() const;
		void setDestination(MarkovState* destination);
		MarkovState* getDestination() const;
		void setProbability(double probability);
		double getProbability() const;
		void setName(std::string name);
		std::string getName() const;

	private:
		MarkovState* _source = nullptr;
		MarkovState* _destination = nullptr;
		double _probability = 1.0;
		std::string _name = "";
	};

public:
	MarkovChainNetwork(Model* model, std::string name = "");
	virtual ~MarkovChainNetwork() override;

public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addState(MarkovState* state);
	void removeState(MarkovState* state);
	List<MarkovState*>* getStates() const;
	void addTransition(MarkovTransition* transition);
	void removeTransition(MarkovTransition* transition);
	List<MarkovTransition*>* getTransitions() const;
	std::vector<MarkovTransition*> getOutgoingTransitions(MarkovState* state) const;
	void setInitialState(MarkovState* state);
	MarkovState* getInitialState() const;
	void setCurrentState(MarkovState* state);
	MarkovState* getCurrentState() const;
	unsigned int getStateIndex(MarkovState* state) const;
	MarkovState* getStateAt(unsigned int index) const;
	double getProbabilityTolerance() const;
	void setProbabilityTolerance(double tolerance);
	void resetSampler();

public:
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual NetworkActivationResult _activate(const NetworkActivationFrame& frame) override;

private:
	MarkovState* _resolveInitialState();
	unsigned int _sampleNextStateIndex(const std::vector<MarkovTransition*>& outgoing);
	bool _hasState(MarkovState* state) const;

private:
	List<MarkovState*>* _states = new List<MarkovState*>();
	List<MarkovTransition*>* _transitions = new List<MarkovTransition*>();
	MarkovState* _initialState = nullptr;
	MarkovState* _currentState = nullptr;
	Sampler_if* _sampler = nullptr;
	double _probabilityTolerance = 1e-9;
};

#endif /* MARKOVCHAINNETWORK_H */
