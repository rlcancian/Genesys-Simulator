/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelElement.h
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
#include "Util.h"

#include "ParserChangesInformation.h"
#include "PersistentObject_base.h"

//namespace GenesysKernel {
	class Model;

	/*!
	 * This class is the basis for any element of the model (such as Queue, Resource, Variable, etc.) and also for any component of the model. 
	 * It has the infrastructure to read and write on file and to verify symbols.
	 */
	class ModelElement: PersistentObject_base {
	public:
		ModelElement(Model* model, std::string elementTypename, std::string name = "", bool insertIntoModel = true);
		//ModelElement(Model* model, std::string elementTypename, std::string name = "", bool insertIntoModel = true);
		//ModelElement(const ModelElement &orig);
		virtual ~ModelElement();

	public: // get & set
		Util::identification getId() const;
		void setName(std::string name);
		std::string getName() const;
		std::string getClassname() const;
        bool isReportStatistics() const;
        void setReportStatistics(bool reportStatistics);
	public: // static
		static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields, bool insertIntoModel); // \todo: return ModelComponent* ?
		static std::map<std::string, std::string>* SaveInstance(ModelElement* element);
		static bool Check(ModelElement* element, std::string* errorMessage);
		static void CreateInternalElements(ModelElement* element);
		static void InitBetweenReplications(ModelElement* element);
	public:
		virtual std::string show();
		std::list<std::string>* getChildrenElementKeys() const;
		ModelElement* getChildElement(std::string key) const;
        bool hasChanged() const;
	protected:
		void _setChildElement(std::string key, ModelElement* child);
		void _removeChildrenElements();
		void _removeChildElement(std::string key);
	protected: // must be overriden by derived classes
		virtual bool _loadInstance(std::map<std::string, std::string>* fields);
		virtual std::map<std::string, std::string>* _saveInstance();
	protected: // could be overriden by derived classes
		virtual bool _check(std::string* errorMessage);
		virtual ParserChangesInformation* _getParserChangesInformation();
		virtual void _initBetweenReplications();
		virtual void _createInternalElements(); ///< This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the element must exist when checking such expression    
	private:
		void _build(Model* model, std::string thistypename, bool insertIntoModel);
	private: // name is now private. So changes in name must be throught setName, wich gives oportunity to rename childrenElements, SimulationControls and SimulationResponses
		std::string _name;
	protected:
		Util::identification _id;
		std::string _typename;
		bool _reportStatistics;
		bool _hasChanged;
		Model* _parentModel;
	protected:
		std::map<std::string, ModelElement*>* _childrenElements = new std::map<std::string, ModelElement*>();
	};
//namespace\\}

#endif /* MODELELEMENT_H */

