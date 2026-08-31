/*
 * File:   CPNTransition.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 31 de Agosto de 2026
 */

#ifndef CPNTRANSITION_H
#define CPNTRANSITION_H

#include "plugins/components/ModalModel/DefaultNode.h"

/*!
 * \brief Transition node used by ColoredPetriNetNetwork.
 *
 * This is a Petri-net transition node, not a source-to-destination edge.
 * Incidence is represented by explicit CPNArc objects in the owning network.
 */
class CPNTransition : public DefaultNode {
public:
	CPNTransition(Model* model, std::string name = "");
	virtual ~CPNTransition() override = default;

public:
	void setGuardExpression(std::string guardExpression);
	std::string getGuardExpression() const;
	void setPriority(unsigned int priority);
	unsigned int getPriority() const;
	virtual std::string show() override;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	std::string _guardExpression = "";
	unsigned int _priority = 0;
};

#endif /* CPNTRANSITION_H */
