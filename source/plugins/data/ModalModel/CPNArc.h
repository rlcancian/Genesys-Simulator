/*
 * File:   CPNArc.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 31 de Agosto de 2026
 */

#ifndef CPNARC_H
#define CPNARC_H

#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

#include <map>
#include <string>

class CPNTransition;
class PetriPlace;

/*!
 * \brief Directed bipartite arc used by ColoredPetriNetNetwork.
 *
 * The initial CPN subset uses fixed inscriptions represented as a multiset
 * from symbolic color name to quantity. Variable bindings and typed token
 * values are deliberately deferred.
 */
class CPNArc : public ModelDataDefinition {
public:
	enum class Direction : unsigned int {
		PlaceToTransition = 0,
		TransitionToPlace = 1
	};

public:
	CPNArc(Model* model, std::string name = "");
	CPNArc(Model* model, PetriPlace* place, CPNTransition* transition, Direction direction, std::string name = "");
	virtual ~CPNArc() override;

public:
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setPlace(PetriPlace* place);
	PetriPlace* getPlace() const;
	void setTransition(CPNTransition* transition);
	CPNTransition* getTransition() const;
	void setDirection(Direction direction);
	Direction getDirection() const;
	bool isInputArc() const;
	bool isOutputArc() const;
	void setInscription(std::string color, unsigned int quantity);
	unsigned int getInscription(std::string color = "default") const;
	const std::map<std::string, unsigned int>& getInscriptions() const;
	void clearInscriptions();
	std::string getPlaceName() const;
	std::string getTransitionName() const;
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	void _syncEndpointNames();

private:
	PetriPlace* _place = nullptr;
	CPNTransition* _transition = nullptr;
	std::string _placeName = "";
	std::string _transitionName = "";
	Direction _direction = Direction::PlaceToTransition;
	std::map<std::string, unsigned int>* _inscriptions = new std::map<std::string, unsigned int>();
};

#endif /* CPNARC_H */
