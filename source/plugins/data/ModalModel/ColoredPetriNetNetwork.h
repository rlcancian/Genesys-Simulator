/*
 * File:   ColoredPetriNetNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 31 de Agosto de 2026
 */

#ifndef COLOREDPETRINETNETWORK_H
#define COLOREDPETRINETNETWORK_H

#include "DefaultNetwork.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "plugins/data/ModalModel/CPNTransition.h"

#include <map>
#include <string>
#include <vector>

/*!
 * \brief Pragmatic fixed-inscription Colored Petri Net subset.
 *
 * The topology is formally bipartite: places and transitions are nodes, and
 * CPNArc objects connect exactly one place with one transition. The initial
 * subset supports symbolic colors with unsigned multiplicities, fixed arc
 * inscriptions, optional transition guards through the GenESyS parser, and a
 * deterministic single firing per activation.
 */
class ColoredPetriNetNetwork : public DefaultNetwork {
public:
	enum class FiringMode : unsigned int {
		SingleDeterministic = 0
	};

public:
	ColoredPetriNetNetwork(Model* model, std::string name = "");
	virtual ~ColoredPetriNetNetwork() override;

public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addPlace(PetriPlace* place);
	void removePlace(PetriPlace* place);
	List<PetriPlace*>* getPlaces() const;
	void addTransition(CPNTransition* transition);
	void removeTransition(CPNTransition* transition);
	List<CPNTransition*>* getTransitions() const;
	bool addArc(CPNArc* arc);
	void removeArc(CPNArc* arc);
	List<CPNArc*>* getArcs() const;
	std::vector<CPNArc*> getInputArcs(CPNTransition* transition) const;
	std::vector<CPNArc*> getOutputArcs(CPNTransition* transition) const;
	bool isEnabled(CPNTransition* transition) const;
	CPNTransition* firstEnabledTransition() const;
	bool fire(CPNTransition* transition);
	void setInitialTokens(PetriPlace* place, std::string color, unsigned int quantity);
	unsigned int getInitialTokens(PetriPlace* place, std::string color = "default") const;
	void captureCurrentMarkingAsInitial();
	void restoreInitialMarking();
	FiringMode getFiringMode() const;
	void setFiringMode(FiringMode firingMode);
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual NetworkActivationResult _activate(const NetworkActivationFrame& frame) override;

private:
	bool _hasPlace(PetriPlace* place) const;
	bool _hasTransition(CPNTransition* transition) const;
	bool _hasArc(CPNArc* arc) const;
	bool _guardAllows(CPNTransition* transition) const;
	void _removeIncidentArcs(PetriPlace* place);
	void _removeIncidentArcs(CPNTransition* transition);

private:
	List<PetriPlace*>* _places = new List<PetriPlace*>();
	List<CPNTransition*>* _transitions = new List<CPNTransition*>();
	List<CPNArc*>* _arcs = new List<CPNArc*>();
	std::map<PetriPlace*, std::map<std::string, unsigned int>> _initialMarking;
	FiringMode _firingMode = FiringMode::SingleDeterministic;
};

#endif /* COLOREDPETRINETNETWORK_H */
