/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Set.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:11
 */

#ifndef SET_H
#define SET_H

#include <vector>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"
#include "../../../kernel/simulator/essentialPlugins/EntityType.h"

/*!
 * \brief Data definition grouping an ordered list of ModelDataDefinition
 * members of one concrete type, used by selection-rule-based components
 * (e.g. Resource sets consumed by Seize-family components).
 *
 * Arena correspondence: Arena's "Set module"/"Advanced Set module"
 * (Rockwell Automation, *Getting Started with Arena*, "The Basic Process
 * Panel" p. 51 and "The Advanced Process Panel" p. 69), which define
 * closed, named set kinds: Resource, Counter, Tally, Entity (type), Entity
 * Picture, Queue, Storage and Other.
 *
 * Known difference from Arena (broader, not narrower, than the Arena
 * concept): this class is polymorphic rather than a fixed enumeration of
 * kinds. \c setSetOfType()/getSetOfType() record the concrete
 * ModelDataDefinition subclass name of the first inserted member, and
 * \c setAllowedElementTypes()/addAllowedElementType() let a component
 * owner (e.g. SeizableItem, QueueableItem) restrict a Set instance to one
 * or more accepted subclasses. This means a GenESyS Set can, in principle,
 * hold any registered ModelDataDefinition plugin type, not only the five
 * kinds Arena hard-codes. There is no dedicated "picture" data definition,
 * so Arena's Entity Picture sets have no GenESyS equivalent.
 */
class Set : public ModelDataDefinition {
public:
	Set(Model* model, std::string name = "", std::vector<std::string> allowedElementTypes = {});
	virtual ~Set() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
public:
	//void setSetOfType(std::string _setOfType);
	std::string getSetOfType() const;
	/*! \brief Sets the concrete member type for this Set while it is still structurally compatible.
	 *
	 * A Set is polymorphic only before its first member is inserted.  Once members exist, all
	 * subsequent members must match this selected ModelDataDefinition subclass name.
	 */
	bool setSetOfType(const std::string& setOfType);
	/*! \brief Returns true while the selected concrete member type can still be changed. */
	bool canChangeSetOfType() const;
	/*! \brief Restricts which ModelDataDefinition subclasses may be created through this Set.
	 *
	 * Empty means "use every creatable ModelDataDefinition plugin known by the simulator".
	 * Context owners such as SeizableItem and QueueableItem use this to expose Resource-only or
	 * Queue-only Set editing in the GUI without hard-coding those rules in the editor.
	 */
	void setAllowedElementTypes(const std::vector<std::string>& allowedElementTypes);
	/*! \brief Adds one accepted ModelDataDefinition subclass name to this Set creation contract. */
	void addAllowedElementType(const std::string& allowedElementType);
	/*! \brief Returns the currently accepted concrete member types for GUI and tooling contracts. */
	std::vector<std::string> getAllowedElementTypes() const;
	List<ModelDataDefinition*>* getElementSet() const;

	void addElementSet(ModelDataDefinition* newElement);
	void removeElementSet(ModelDataDefinition* element);
	/*! \brief Creates a new model data definition of the selected Set member type.
	 *
	 * This method is the kernel-side factory used by generic property editors.  The editor provides
	 * the intended type; Set validates the current type/allowed-type contract and delegates actual
	 * object construction to PluginManager so plugin registration rules stay centralized.
	 */
	ModelDataDefinition* createElementSetOfType(const std::string& typeName, const std::string& name = "");

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden 
	virtual bool _check(std::string& errorMessage) override;
	virtual ParserChangesInformation* _getParserChangesInformation() override;
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private:
	//ElementManager* _elems;

	const struct DEFAULT_VALUES {
		const unsigned int membersSize = 0;
		const std::string setOfType = "";
	} DEFAULT;
	List<ModelDataDefinition*>* _elementSet = new List<ModelDataDefinition*>();
	std::string _setOfType = DEFAULT.setOfType;
	std::vector<std::string> _allowedElementTypes;
};

#endif /* SET_H */
