/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EntityType.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 19:24
 */

#ifndef ENTITYTYPE_H
#define ENTITYTYPE_H

#include <string>
#include "ModelElement.h"
#include "StatisticsCollector.h"
#include "ElementManager.h"
#include "Plugin.h"

//#include "Model.h"
//namespace GenesysKernel {

	class EntityType : public ModelElement {
	public:
		//EntityType(Model* model);
		EntityType(Model* model, std::string name = "");
		virtual ~EntityType();
	public:
		virtual std::string show();
	public:
		static PluginInformation* GetPluginInformation();
		static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
	public: //get & set
		void setInitialWaitingCost(double _initialWaitingCost);
		double initialWaitingCost() const;
		void setInitialOtherCost(double _initialOtherCost);
		double initialOtherCost() const;
		void setInitialNVACost(double _initialNVACost);
		double initialNVACost() const;
		void setInitialVACost(double _initialVACost);
		double initialVACost() const;
		void setInitialPicture(std::string _initialPicture);
		std::string initialPicture() const;
	public: //get
		StatisticsCollector* addGetStatisticsCollector(std::string name);

	protected: // must be overriden by derived classes
		virtual bool _loadInstance(std::map<std::string, std::string>* fields);
		virtual std::map<std::string, std::string>* _saveInstance();
		virtual bool _check(std::string* errorMessage);
		virtual void _initBetweenReplications();
		virtual void _createInternalElements();
	private:
		void _initCostsAndStatistics();
	private:
		const struct DEFAULT_VALUES {
			std::string initialPicture = "report";
			double initialCost = 0.0;
		} DEFAULT;
		std::string _initialPicture = DEFAULT.initialPicture;
		double _initialVACost = DEFAULT.initialCost;
		double _initialNVACost = DEFAULT.initialCost;
		double _initialOtherCost = DEFAULT.initialCost;
		double _initialWaitingCost = DEFAULT.initialCost;
	private: //1:n
		List<StatisticsCollector*>* _statisticsCollectors = new List<StatisticsCollector*>();
	};
//namespace\\}
#endif /* ENTITYTYPE_H */

