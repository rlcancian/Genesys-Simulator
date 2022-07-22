/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelDataDefinition.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:40
 */

#ifndef MODELELEMENT_H
#define MODELELEMENT_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include "../util/Util.h"

#include "ParserChangesInformation.h"
#include "Persistence.h"
#include "PluginInformation.h"
#include "Property.h"

//namespace GenesysKernel {
class Model;

/*!
 * This class is the basis for any modeldatum of the model (such as Queue, Resource, Variable, etc.) and also for any component of the model.
 * It has the infrastructure to read and write on file and to verify symbols.
 */
class ModelDataDefinition : PersistentObject_base {
public:
	ModelDataDefinition(Model* model, std::string datadefinitionTypename, std::string name = "", bool insertIntoModel = true);
	//ModelDataDefinition(Model* model, std::string datadefinitionTypename, std::string name = "", bool insertIntoModel = true);
	//ModelDataDefinition(const ModelDataDefinition &orig);
	virtual ~ModelDataDefinition();

public: // get & set
	Util::identification getId() const;
	void setName(std::string name);
	std::string getName() const;
	std::string getClassname() const;
	/*! Return true if this ModelDataDefinition generates statics for simulation reports*/
	bool isReportStatistics() const;
	/*! Defnes if this ModelDataDefinition generates statics for simulation reports*/
	void setReportStatistics(bool reportStatistics);
public: // static
	/*! This class method receives a map of fields readed from a file (or somewhere else) creates an instace of the ModelDatas and inokes the protected method _loadInstance() of that instance, whch fills the field values. The instance can be automatticaly inserted into the simulation model if required*/
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields, bool insertIntoModel); // @TODO: return ModelComponent* ?
	/*! This class method invokes the constructor and returns a new instance (that demands a typecast to the rigth subclass). It is used to construct a new instance when plugins are connected using dynamic loaded libraries*/
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	/*! This class method takes an instance of a ModelDataDefinition, invokes the protected method _saveInstance() of that instance and retorns a map of filds (name=value) that can be saved on a file (or somewhere else)*/
	static void SaveInstance(PersistenceRecord *fields, ModelDataDefinition* modeldatum);
	/*! This class method takes an instance of a ModelDataDefinition and invokes the private method_check() method of that instance, which checks itself */
	static bool Check(ModelDataDefinition* modeldatum, std::string* errorMessage);
	/*! This class method is responsible for invoking the protected method _check() of the instance modeldatum, which creates any internal ModelDataDefinition (such as internelElements) or even other external needed ModelDatas, such as attributes or variables */
	static void CreateInternalData(ModelDataDefinition* modeldatum);
	/* This class methood is responsible for invoking the protected method _initBetweenReplication(), which clears all statistics, attributes, counters and other stuff before starting a new repliction */
	static void InitBetweenReplications(ModelDataDefinition* modeldatum);
	//static PluginInformation* GetPluginInformation();
public:
	virtual std::string show();
	/*! Returns a list of keys (names) of internal ModelDatas, cuch as Counters, StatisticsCollectors and others. ChildrenElements are ModelDatas used by this ModelDataDefinition thar are needed before model checking */
	ModelDataDefinition* getInternalData(std::string name) const;
	std::map<std::string, ModelDataDefinition*>* getInternalData() const;
	std::map<std::string, ModelDataDefinition*>* getAttachedData() const;
	//ModelDataDefinition* getInternalData(std::string key) const;
	bool hasChanged() const;
	unsigned int getLevel() const;
	void setModelLevel(unsigned int _modelLevel);
	std::list<PropertyBase *> *getProperties() const;

protected: // methods to be called inside the _createInternalAndAttachedData() method
	void _internalDataClear();
	void _internalDataInsert(std::string key, ModelDataDefinition* child);
	void _internalDataRemove(std::string key);
	void _attachedDataInsert(std::string key, ModelDataDefinition* data);
	void _attachedDataRemove(std::string key);
	void _attachedAttributesInsert(std::vector<std::string> neededNames);
protected:
	bool _getSaveDefaultsOption();
protected: // must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // could be overriden by derived classes
	virtual bool _check(std::string* errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	virtual ParserChangesInformation* _getParserChangesInformation();
	virtual void _initBetweenReplications();
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	virtual void _addProperty(PropertyBase* property);
private: // name is now private. So changes in name must be throught setName, wich gives oportunity to rename internelElements, SimulationControls and SimulationResponses
	std::string _name;
private:
	std::map<std::string, ModelDataDefinition*>* _internalData = new std::map<std::string, ModelDataDefinition*>();
	std::map<std::string, ModelDataDefinition*>* _attachedData = new std::map<std::string, ModelDataDefinition*>();
protected:
	Util::identification _id;
	std::string _typename;
	bool _reportStatistics;
	bool _hasChanged;
	unsigned int _modelLevel = 0; // the ID of parent component (submodel or process, for now) in the "superlevel"
	Model* _parentModel;
protected:
	std::list<PropertyBase*>* _properties = new std::list<PropertyBase*>();
};
//namespace\\}

#endif /* MODELELEMENT_H */

