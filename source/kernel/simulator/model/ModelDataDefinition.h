/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinition.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 21 de Junho de 2018, 19:40
 */

#ifndef MODELELEMENT_H
#define MODELELEMENT_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "../../util/List.h"
#include "../ParserChangesInformation.h"
#include "../Persistence.h"
//#include "PropertyGenesys.h"
#include "ModelDataDefinitionAssociations.h"
#include "../SimulationControlAndResponse.h"
#include "../TraceManager.h"

//namespace GenesysKernel {
class Model;


/*!
 * This class is the basis for any modeldatum of the model (such as Queue, Resource, Variable, etc.) and also for any component of the model.
 * It has the infrastructure to read and write on file and to verify symbols.
 */
class ModelDataDefinition : PersistentObject_base {
public:
	/*!
	 * \brief Creates a model data definition and optionally inserts it into the model.
	 * \param model Parent model that owns this data definition.
	 * \param datadefinitionTypename Runtime type name for the concrete data definition.
	 * \param name Initial object name.
	 * \param insertIntoModel If \c true, the new instance is registered in the model immediately.
	 */
	ModelDataDefinition(Model* model, std::string datadefinitionTypename, std::string name = "",
	                    bool insertIntoModel = true);
	//ModelDataDefinition(Model* model, std::string datadefinitionTypename, std::string name = "", bool insertIntoModel = true);
	//ModelDataDefinition(const ModelDataDefinition &orig);
	/*!
	 * \brief Releases resources owned by this data definition.
	 */
	virtual ~ModelDataDefinition();

public: // get & set
	/*!
	 * \brief Returns the unique identifier of this instance.
	 * \return Instance identifier.
	 */
	Util::identification getId() const;
	/*!
	 * \brief Updates the logical name of this instance.
	 * \param name New name.
	 */
	void setName(const std::string& name);
	/*!
	 * \brief Returns the logical name of this instance.
	 * \return Instance name.
	 */
	const std::string& getName() const;
	/*!
	 * \brief Updates the user-visible label of this instance.
	 * \param label New label.
	 */
	void setLabel(const std::string& label);
	/*!
	 * \brief Returns the user-visible label of this instance.
	 * \return Instance label.
	 */
	const std::string& getLabel() const;
	/*!
	 * \brief Returns the runtime class name used by the plugin system.
	 * \return Concrete class name.
	 */
	const std::string& getClassname() const;
	/*!
	 * \brief Returns the parent model that owns this definition.
	 * \return Parent model pointer, or \c nullptr when detached.
	 */
	Model* getParentModel() const;
	/*!
	 * \brief Indicates whether this data definition contributes report statistics.
	 * \return \c true when this definition emits simulation statistics.
	 */
	bool isReportStatistics() const;
	/*!
	 * \brief Enables or disables report-statistics generation for this definition.
	 * \param reportStatistics New report-statistics flag.
	 */
	void setReportStatistics(bool reportStatistics);

public:
	/*!
	 * \brief Looks up a named internal child data definition.
	 * \param name Child name to search for.
	 * \return Matching internal child, or \c nullptr when not found.
	 */
	ModelDataDefinition* getInternalData(const std::string& name) const;
	/*!
	 * \brief Returns the full map of internal child data definitions.
	 * \return Map of internal children indexed by name.
	 */
	std::map<std::string, ModelDataDefinition*>* getInternalData() const;
	/*!
	 * \brief Returns the full map of attached data definitions.
	 * \return Map of attached data indexed by name.
	 */
	std::map<std::string, ModelDataDefinition*>* getAttachedData() const;
	//ModelDataDefinition* getInternalData(std::string key) const;
	/*!
	 * \brief Indicates whether this instance has been modified since the last reset.
	 * \return \c true when the object has pending changes.
	 */
	bool hasChanged() const;
	/*!
	 * \brief Updates the changed flag for this data definition.
	 * \param hasChanged New changed-state value.
	 */
	void setHasChanged(bool hasChanged);
	/*!
	 * \brief Returns the level in the model hierarchy where this definition lives.
	 * \return Model nesting level.
	 */
	unsigned int getLevel() const;
	/*!
	 * \brief Sets the model nesting level for this definition.
	 * \param _modelLevel New hierarchy level.
	 */
	void setModelLevel(unsigned int _modelLevel);
	/*!
	 * \brief Returns the simulation controls owned by this data definition.
	 * \details Preferred API for writable controls owned by this model data
	 * definition.
	 * \return Writable control list managed by the definition.
	 */
	List<SimulationControl*>* getSimulationControls() const;
	/*!
	 * \brief Returns the trace level that applies specifically to this definition.
	 * \return Explicit trace level for this instance.
	 */
	TraceManager::Level getTraceLevelSpecific() const;
	/*!
	 * \brief Updates the trace level used by this definition.
	 * \param level New trace level.
	 */
	void setTraceLevelSpecific(TraceManager::Level level);
	/*!
	 * \brief Defines the specific trace level and optionally enables it immediately.
	 * \param traceLevelspecific New trace level.
	 * \param traceLevelSpecificEnabled Whether the specific trace level starts enabled.
	 */
	void defineTraceLevelSpecific(TraceManager::Level traceLevelspecific, bool traceLevelSpecificEnabled = true);
	/*!
	 * \brief Indicates whether the specific trace level is enabled.
	 * \return \c true when the instance-specific trace level is active.
	 */
	bool isTraceLevelSpecificEnabled() const;
	/*!
	 * \brief Enables or disables the specific trace level.
	 * \param traceLevelSpecificEnabled New enabled-state value.
	 */
	void setTraceLevelSpecificEnabled(bool traceLevelSpecificEnabled);

public: // public static methods
	/*!
	 * \brief Loads a model data definition instance from serialized fields.
	 * \details This helper receives a map of fields read from a file or other
	 * source, creates the instance, and invokes its protected _loadInstance()
	 * method. The instance can be automatically inserted into the simulation
	 * model when requested.
	 * \param model Parent model that will own the loaded instance.
	 * \param fields Serialized fields to deserialize.
	 * \param insertIntoModel If \c true, registers the loaded instance in the model.
	 * \return Newly created instance when loading succeeds.
	 */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields, bool insertIntoModel);
	// @ToDo: (pequena alteração): return ModelComponent* ?
	/*!
	 * \brief Creates a new instance of a data definition using the plugin registry.
	 * \details This helper invokes the constructor and returns a new instance
	 * that usually requires a cast to the correct subclass. It is used when
	 * plugins are connected through dynamically loaded libraries.
	 * \param model Parent model that will own the instance.
	 * \param name Initial object name.
	 * \return Newly created data definition instance.
	 */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	/*!
	 * \brief Serializes a data definition instance into a persistence record.
	 * \details This helper calls the instance _saveInstance() method and
	 * returns a field map that can be saved to a file or other destination.
	 * \param fields Output record that receives the serialized fields.
	 * \param modeldatum Instance to serialize.
	 */
	static void SaveInstance(PersistenceRecord* fields, ModelDataDefinition* modeldatum);
	/*!
	 * \brief Validates a data definition instance.
	 * \details This helper invokes the instance _check() method to validate the
	 * object and any data it creates or references.
	 * \param modeldatum Instance to validate.
	 * \param errorMessage Output error message when validation fails.
	 * \return \c true when the instance is valid.
	 */
	static bool Check(ModelDataDefinition* modeldatum, std::string& errorMessage);
	/*!
	 * \brief Resets simulation-time state between replications.
	 * \param modeldatum Instance to reset.
	 */
	static void InitBetweenReplications(ModelDataDefinition* modeldatum);
	//static PluginInformation* GetPluginInformation();

public:
	/*!
	 * \brief Creates internal and attached data required by this instance.
	 * \details This helper invokes the protected _check() path of the instance,
	 * which creates internal ModelDataDefinitions and other related data such as
	 * attributes or variables.
	 * \param modeldatum Instance whose associated data should be created.
	 */
	static void CreateInternalData(ModelDataDefinition* modeldatum);
	/*!
	 * \brief Creates related data and validates the instance in a single call.
	 * \details Materializes related data and runs local checks for the instance
	 * in a single step.
	 * \param modeldatum Instance whose related data should be materialized.
	 * \param errorMessage Output error message when validation fails.
	 * \return \c true when creation and validation succeed.
	 */
	static bool CreateRelatedDataElements(ModelDataDefinition* modeldatum, std::string& errorMessage);
	/* This class methood is responsible for invoking the protected method _initBetweenReplication(), which clears all statistics, attributes, counters and other stuff before starting a new repliction */

public: // public virtual methods
	/*!
	 * \brief Returns a textual representation of this data definition.
	 * \details Returns the keys of internal ModelDataDefinitions such as
	 * counters, statistics collectors and other children needed before model
	 * checking.
	 * \return Human-readable summary used by traces and debug output.
	 */
	virtual std::string show();
	/*! Returns a list of keys (names) of internal ModelDatas, cuch as Counters, StatisticsCollectors and others. ChildrenElements are ModelDatas used by this ModelDataDefinition thar are needed before model checking */

protected:
	bool _getSaveDefaultsOption();

protected: //! must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;

protected: //! could be overriden by derived classes
	virtual bool _check(std::string& errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	virtual ParserChangesInformation* _getParserChangesInformation();
	virtual void _initBetweenReplications();
	/*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	virtual void _addSimulationControl(SimulationControl* control);
	//virtual void _addSimulationResponse(SimulationControl* response);

/*@Todo: Fix this mess about diffent kind of internal and attached data related to report statistics,
 * mandatory and optional editable internal or attached dataelements that are properties,
 * and, I don´t know, maybe other kind of internal and attached nedded stuff?
 */
protected:
	/*! Legacy compatibility hook. Prefer the granular create* methods and the composed association manager. */
	virtual void _createInternalAndAttachedData(); // final; //@Todo: will be final
protected:
	virtual void _createInternalStatisticReporters();
	virtual void _createAttachedAttributes();
	virtual void _createNonEditableDataDefinitions();
	virtual void _createEditableDataDefinitions();
protected:
	void _templateCreateInternalStatisticReporters();
	void _templateCreateAttachedAttributes();
	void _templateCreateNonEditableDataDefinitions();
	void _templateCreateEditableDataDefinitions();
protected: //! legacy helper methods used by the granular create* hooks
	void _internaStatisticReportersClear();
	void _internalDataClear();
	void _internalDataInsert(const std::string& key, ModelDataDefinition* child);
	void _internalDataRemove(const std::string& key);
	void _attachedDataClear();
	void _attachedDataInsert(const std::string& key, ModelDataDefinition* data);
	void _attachedDataRemove(const std::string& key);
	void _attachedAttributesInsert(const std::vector<std::string>& neededNames);
	void _statisticReporterInsert(const std::string& key, ModelDataDefinition* data);
	void _statisticReporterRemove(const std::string& key);
	void _statisticReportersClear();
	void _mandatoryAttachedAttributeInsert(const std::string& key, ModelDataDefinition* data);
	void _mandatoryAttachedAttributeRemove(const std::string& key);
	void _mandatoryAttachedAttributesClear();
	void _mandatoryEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data);
	void _mandatoryEditableDataDefinitionRemove(const std::string& key);
	void _mandatoryEditableDataDefinitionsClear();
	void _optionalEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data);
	void _optionalEditableDataDefinitionRemove(const std::string& key);
	void _optionalEditableDataDefinitionsClear();
	void _mandatoryNonEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data);
	void _mandatoryNonEditableDataDefinitionRemove(const std::string& key);
	void _mandatoryNonEditableDataDefinitionsClear();
	void _optionalNonEditableDataDefinitionInsert(const std::string& key, ModelDataDefinition* data);
	void _optionalNonEditableDataDefinitionRemove(const std::string& key);
	void _optionalNonEditableDataDefinitionsClear();
protected:
	//! method to be called to insert attached dataelements that are referenced by string expressions (detected by the parser), to avoid orphaned data definitions
	void _checkCreateAttachedReferencedDataDefinition(const std::string& expression);
	//(std::map<std::string, std::list<std::string>*>* referencedDataDefinitions);


private:
	// name is now private. So changes in name must be throught setName, wich gives oportunity to rename internelElements, SimulationControls and SimulationResponses
	std::string _name;
	std::string _label;

private:
	friend class ModelDataDefinitionAssociations;
	ModelDataDefinitionAssociations _dataAssociations;

protected:
	Util::identification _id;
	std::string _typename;
	bool _reportStatistics;
	bool _hasChanged;
	unsigned int _modelLevel = 0; // the ID of parent component (submodel or process, for now) in the "superlevel"
	Model* _parentModel;

private:
	bool _checkSpecificTraceLevel(TraceManager::Level level);

protected: //! not just an easy access to trace manager, but wrappers to check if specific trace level applies
	void trace(const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed);
	void traceError(const std::string& text, const std::exception& e);
	void traceError(const std::string& text, TraceManager::Level level = TraceManager::Level::L1_errorFatal);
	void traceReport(const std::string& text, TraceManager::Level level = TraceManager::Level::L2_results);
	void traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component, const std::string& text,
	                     TraceManager::Level level = TraceManager::Level::L8_detailed);
	/*!
	 * \brief Traces text to the simulation output.
	 * \details Used only while a simulation is running, for example when
	 * components or data elements need to inform something.
	 * \param thisobject Caller object used for trace attribution.
	 * \param text Trace text to emit.
	 * \param level Trace severity level.
	 */
	void traceSimulation(void* thisobject, const std::string& text,
	                     TraceManager::Level level = TraceManager::Level::L8_detailed);
	void traceSimulation(void* thisobject, TraceManager::Level level, const std::string& text);

protected:
	//List<SimulationControl*>* _simulationResponses = new List<SimulationControl*>();
	List<SimulationControl*>* _simulationControls = new List<SimulationControl*>();
	TraceManager::Level _traceLevelSpecific;
	bool _traceLevelSpecificEnabled = false;
	//PropertyListG* _propertiesG = new PropertyListG();
};

//namespace\\}

#endif /* MODELELEMENT_H */
