/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Store.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:07
 */

#ifndef STORE_H
#define STORE_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../data/MaterialHandling/Storage.h"

/*!
 * \brief Placeholder for adding an entity to a Storage (§5.11).
 *
 * Arena correspondence: the "Store module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 66).
 *
 * \warning Incomplete stub template: this class declares no fields at all,
 * and `_onDispatchEvent()` unconditionally forwards the entity without
 * touching any Storage. `_check()`/`_loadInstance()`/`_saveInstance()` are
 * not implemented either. See
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.17.
 */
class Store : public ModelComponent {
public: // constructors
	Store(Model* model, std::string name = "");
	virtual ~Store() = default;
public: // virtual
	virtual std::string show() override;
public:
    void setStorage(Storage* storage);
    Storage* getStorage() const;
    void setQuantityExpression(std::string quantityExpression);
    std::string getQuantityExpression() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // virtual
	//virtual void _initBetweenReplications();
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	virtual bool _check(std::string& errorMessage) override;

protected:

private: // methods
private: // attributes 1:1
    const struct DEFAULT_VALUES {
        const std::string quantityExpression = "1";
    } DEFAULT;
    Storage* _storage = nullptr;
    std::string _quantityExpression = DEFAULT.quantityExpression;
private: // attributes 1:n
};


#endif /* STORE_H */
