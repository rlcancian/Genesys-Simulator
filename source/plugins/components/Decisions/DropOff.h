/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DropOff.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#ifndef DROPOFF_H
#define DROPOFF_H

#include "../../../kernel/simulator/model/ModelComponent.h"

/*!
 * \brief Placeholder for releasing a subset of a batched group's members.
 *
 * Arena correspondence: the "Dropoff module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 54).
 *
 * \warning Incomplete stub template: this class declares no fields at all,
 * and `_onDispatchEvent()` unconditionally forwards the entity without
 * interacting with any EntityGroup. See
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.17.
 */
class DropOff : public ModelComponent {
public: // constructors
	DropOff(Model* model, std::string name = "");
	virtual ~DropOff() = default;
public: // virtual
	virtual std::string show() override;
public:
    void setQuantityExpression(std::string quantityExpression);
    std::string getQuantityExpression() const;
    void setStartingRankExpression(std::string startingRankExpression);
    std::string getStartingRankExpression() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
protected: // virtual
	//virtual void _initBetweenReplications();
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

protected:

private: // methods
private: // attributes 1:1
    const struct DEFAULT_VALUES {
        const std::string quantityExpression = "1";
        const std::string startingRankExpression = "1";
    } DEFAULT;
    std::string _quantityExpression = DEFAULT.quantityExpression;
    std::string _startingRankExpression = DEFAULT.startingRankExpression;
private: // attributes 1:n
};


#endif /* DROPOFF_H */
