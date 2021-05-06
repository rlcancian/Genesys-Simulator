/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationReporterDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 8 de Agosto de 2018, 10:59
 */

#include "SimulationReporterDefaultImpl1.h"
#include <assert.h>
#include <iomanip>
#include <iostream>
#include "Counter.h"

//using namespace GenesysKernel;

SimulationReporterDefaultImpl1::SimulationReporterDefaultImpl1(ModelSimulation* simulation, Model* model, List<ModelElement*>* statsCountersSimulation) {
	_simulation = simulation;
	_model = model;
	_statsCountersSimulation = statsCountersSimulation;
}

void SimulationReporterDefaultImpl1::showSimulationControls() {
	_model->getTracer()->traceReport("Simulation Controls:");
	Util::IncIndent();
	{
		for (std::list<SimulationControl*>::iterator it = _model->getControls()->list()->begin(); it != _model->getControls()->list()->end(); it++) {
			SimulationControl* control = (*it);
			_model->getTracer()->traceReport(control->getName() + ": " + std::to_string(control->getValue()));
		}
	}
	Util::DecIndent();
}

void SimulationReporterDefaultImpl1::showReplicationStatistics() {
	_model->getTracer()->traceReport("");
	_model->getTracer()->traceReport("Begin of Report for replication " + std::to_string(_simulation->getCurrentReplicationNumber()) + " of " + std::to_string(_model->getSimulation()->getNumberOfReplications()));
	/* \todo: StatisticsCollector and Counter should NOT be special classes. It should iterate classes looking for classes that can generate reports.
	 StatisticsCollector and Counter should ovveride an inherited attribute from ModelElement to specify they generate report information
	 look for _generateReportInformation = true;  using bool generateReportInformation() const;
	 */
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
	// runs over all elements and list the statistics for each one, and then the statistics with no parent
	Util::IncIndent();
	//this->showSimulationControls();
	// copy the ist of statistics and counters into a single new list
	std::list<ModelElement*>* statisticsAndCounters = new std::list<ModelElement*>(*(_model->getElements()->getElementList(UtilTypeOfStatisticsCollector)->list()));
	std::list<ModelElement*>* counters = new std::list<ModelElement*>(*(_model->getElements()->getElementList(UtilTypeOfCounter)->list()));
	statisticsAndCounters->merge(*counters);
	// organizes statistics into a map of maps
	std::map< std::string, std::map<std::string, std::list<ModelElement*>*>* >* mapMapTypeStat = new std::map<std::string, std::map<std::string, std::list<ModelElement*>*>*>();

	for (std::list<ModelElement*>::iterator it = statisticsAndCounters->begin(); it != statisticsAndCounters->end(); it++) {
		std::string parentName, parentTypename;
		ModelElement* statOrCnt = (*it);
		//std::cout << statOrCnt->getName() << ": " << statOrCnt->getTypename() << std::endl;
		if ((*it)->getClassname() == UtilTypeOfStatisticsCollector) {
			StatisticsCollector* stat = dynamic_cast<StatisticsCollector*> (statOrCnt);
			parentName = stat->getParent()->getName();
			parentTypename = stat->getParent()->getClassname();
		} else {
			if ((*it)->getClassname() == UtilTypeOfCounter) {
				Counter* cnt = dynamic_cast<Counter*> (statOrCnt);
				parentName = cnt->getParent()->getName();
				parentTypename = cnt->getParent()->getClassname();

			}
		}
		// look for key=parentTypename
		std::map<std::string, std::map<std::string, std::list<ModelElement*>*>*>::iterator mapMapIt = mapMapTypeStat->find(parentTypename);
		if (mapMapIt == mapMapTypeStat->end()) { // parentTypename does not exists in map. Include it.
			std::pair< std::string, std::map<std::string, std::list<ModelElement*>*>* >* newPair = new std::pair<std::string, std::map<std::string, std::list<ModelElement*>*>*>(parentTypename, new std::map<std::string, std::list<ModelElement*>*>());
			mapMapTypeStat->insert(*newPair);
			mapMapIt = mapMapTypeStat->find(parentTypename); // find again. Now it will.
		}
		//assert(mapMapIt != mapMapTypeStat->end());
		std::map<std::string, std::list<ModelElement*>*>* mapTypeStat = (*mapMapIt).second;
		assert(mapTypeStat != nullptr);
		// look for key=parentName
		std::map<std::string, std::list<ModelElement*>*>::iterator mapIt = mapTypeStat->find(parentName);
		if (mapIt == mapTypeStat->end()) { // parentTypename does not exists in map. Include it.
			std::pair< std::string, std::list<ModelElement*>* >* newPair = new std::pair<std::string, std::list<ModelElement*>*>(parentName, new std::list<ModelElement*>());
			mapTypeStat->insert(*newPair);
			mapIt = mapTypeStat->find(parentName); // find again. Now it will.
		}
		// get the list and insert the stat in that list
		std::list<ModelElement*>* listStatAndCount = (*mapIt).second;
		assert(listStatAndCount != nullptr);
		listStatAndCount->insert(listStatAndCount->end(), statOrCnt);
		//_model->getTraceManager()->traceReport(parentTypename + " -> " + parentName + " -> " + stat->show());
	}
	// now runs over that map of maps showing the statistics
	Util::IncIndent();
	Util::IncIndent();
	_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
	Util::DecIndent();
	Util::DecIndent();
	for (auto const mapmapItem : *mapMapTypeStat) {
		_model->getTracer()->traceReport("Statistis for " + mapmapItem.first + ":");
		Util::IncIndent();
		{
			for (auto const mapItem : *(mapmapItem.second)) {
				_model->getTracer()->traceReport(mapItem.first + ":");
				Util::IncIndent();
				{
					//					_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
					for (ModelElement * const item : *(mapItem.second)) {
						if (item->getClassname() == UtilTypeOfStatisticsCollector) {
							Statistics_if* stat = dynamic_cast<StatisticsCollector*> (item)->getStatistics();
							_model->getTracer()->traceReport(Util::TraceLevel::L3_results,
									Util::SetW(item->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
									Util::SetW(std::to_string(stat->numElements()), _w) +
									Util::SetW(std::to_string(stat->min()), _w) +
									Util::SetW(std::to_string(stat->max()), _w) +
									Util::SetW(std::to_string(stat->average()), _w) +
									Util::SetW(std::to_string(stat->variance()), _w) +
									Util::SetW(std::to_string(stat->stddeviation()), _w) +
									Util::SetW(std::to_string(stat->variationCoef()), _w) +
									Util::SetW(std::to_string(stat->halfWidthConfidenceInterval()), _w) +
									Util::SetW(std::to_string(stat->getConfidenceLevel()), _w)
									);
						} else {
							if (item->getClassname() == UtilTypeOfCounter) {
								Counter* count = dynamic_cast<Counter*> (item);
								_model->getTracer()->traceReport(Util::TraceLevel::L3_results,
										Util::SetW(count->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
										Util::SetW(std::to_string(count->getCountValue()), _w)
										);
							}
						}
					}
				}
				Util::DecIndent();
			}
		}
		Util::DecIndent();
	}
	//this->showSimulationResponses();
	Util::DecIndent();
	_model->getTracer()->traceReport("End of Report for replication " + std::to_string(_simulation->getCurrentReplicationNumber()) + " of " + std::to_string(_model->getSimulation()->getNumberOfReplications()));
	_model->getTracer()->traceReport("-------------------------------------");
}

void SimulationReporterDefaultImpl1::showSimulationResponses() {
	_model->getTracer()->traceReport("Simulation Responses:");
	Util::IncIndent();
	{
		for (std::list<SimulationResponse*>::iterator it = _model->getResponses()->list()->begin(); it != _model->getResponses()->list()->end(); it++) {
			SimulationResponse* response = (*it);
			_model->getTracer()->traceReport(response->getName() + ": " + std::to_string(response->getValue()));
		}
	}
	Util::DecIndent();
}

void SimulationReporterDefaultImpl1::showSimulationStatistics() {//List<StatisticsCollector*>* cstatsSimulation) {
	_model->getTracer()->traceReport("");
	_model->getTracer()->traceReport("Begin of Report for Simulation (based on " + std::to_string(_model->getSimulation()->getNumberOfReplications()) + " replications)");
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
	// runs over all elements and list the statistics for each one, and then the statistics with no parent
	Util::IncIndent();
	// COPY the list of statistics and counters into a single new list
	//std::list<ModelElement*>* statisticsAndCounters = //new std::list<ModelElement*>(*(this->_statsCountersSimulation->list()));
	// organizes statistics into a map of maps
	std::map< std::string, std::map<std::string, std::list<ModelElement*>*>* >* mapMapTypeStat = new std::map<std::string, std::map<std::string, std::list<ModelElement*>*>*>();

	for (std::list<ModelElement*>::iterator it = _statsCountersSimulation->list()->begin(); it != _statsCountersSimulation->list()->end(); it++) {
		std::string parentName, parentTypename;
		ModelElement* statOrCnt = (*it);
		//std::cout << statOrCnt->getName() << ": " << statOrCnt->getTypename() << std::endl;
		if ((*it)->getClassname() == UtilTypeOfStatisticsCollector) {
			StatisticsCollector* stat = dynamic_cast<StatisticsCollector*> (statOrCnt);
			parentName = stat->getParent()->getName();
			parentTypename = stat->getParent()->getClassname();
		} else {
			if ((*it)->getClassname() == UtilTypeOfCounter) {
				Counter* cnt = dynamic_cast<Counter*> (statOrCnt);
				parentName = cnt->getParent()->getName();
				parentTypename = cnt->getParent()->getClassname();

			}
		}
		// look for key=parentTypename
		std::map<std::string, std::map<std::string, std::list<ModelElement*>*>*>::iterator mapMapIt = mapMapTypeStat->find(parentTypename);
		if (mapMapIt == mapMapTypeStat->end()) { // parentTypename does not exists in map. Include it.
			std::pair< std::string, std::map<std::string, std::list<ModelElement*>*>* >* newPair = new std::pair<std::string, std::map<std::string, std::list<ModelElement*>*>*>(parentTypename, new std::map<std::string, std::list<ModelElement*>*>());
			mapMapTypeStat->insert(*newPair);
			mapMapIt = mapMapTypeStat->find(parentTypename); // find again. Now it will.
		}
		assert(mapMapIt != mapMapTypeStat->end());
		std::map<std::string, std::list<ModelElement*>*>* mapTypeStat = (*mapMapIt).second;
		assert(mapTypeStat != nullptr);
		// look for key=parentName
		std::map<std::string, std::list<ModelElement*>*>::iterator mapIt = mapTypeStat->find(parentName);
		if (mapIt == mapTypeStat->end()) { // parentTypename does not exists in map. Include it.
			std::pair< std::string, std::list<ModelElement*>* >* newPair = new std::pair<std::string, std::list<ModelElement*>*>(parentName, new std::list<ModelElement*>());
			mapTypeStat->insert(*newPair);
			mapIt = mapTypeStat->find(parentName); // find again. Now it will.
		}
		// get the list and insert the stat in that list
		std::list<ModelElement*>* listStat = (*mapIt).second;
		listStat->insert(listStat->end(), statOrCnt);
		//_model->getTraceManager()->traceReport(parentTypename + " -> " + parentName + " -> " + stat->show());
	}
	// now runs over that map of maps showing the statistics
	//int w = 12;
	Util::IncIndent();
	Util::IncIndent();
	_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
	Util::DecIndent();
	Util::DecIndent();
	for (auto const mapmapItem : *mapMapTypeStat) {
		_model->getTracer()->traceReport("Statistis for " + mapmapItem.first + ":");
		Util::IncIndent();
		{
			for (auto const mapItem : *(mapmapItem.second)) {
				_model->getTracer()->traceReport(mapItem.first + ":");
				Util::IncIndent();
				{
					for (ModelElement * const item : *(mapItem.second)) {
						if (item->getClassname() == UtilTypeOfStatisticsCollector) {
							Statistics_if* stat = dynamic_cast<StatisticsCollector*> (item)->getStatistics();
							_model->getTracer()->traceReport(Util::TraceLevel::L3_results,
									Util::SetW(item->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
									Util::SetW(std::to_string(stat->numElements()), _w) +
									Util::SetW(std::to_string(stat->min()), _w) +
									Util::SetW(std::to_string(stat->max()), _w) +
									Util::SetW(std::to_string(stat->average()), _w) +
									Util::SetW(std::to_string(stat->variance()), _w) +
									Util::SetW(std::to_string(stat->stddeviation()), _w) +
									Util::SetW(std::to_string(stat->variationCoef()), _w) +
									Util::SetW(std::to_string(stat->halfWidthConfidenceInterval()), _w) +
									Util::SetW(std::to_string(stat->getConfidenceLevel()), _w)
									);
						} else {
							if (item->getClassname() == UtilTypeOfCounter) {
								Counter* cnt = dynamic_cast<Counter*> (item);
								_model->getTracer()->traceReport(Util::TraceLevel::L3_results,
										Util::SetW(cnt->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
										Util::SetW(std::to_string(cnt->getCountValue()), _w)
										);

							}
						}
					}
				}
				Util::DecIndent();
			}
		}
		Util::DecIndent();
	}

	Util::DecIndent();
	_model->getTracer()->traceReport("End of Report for Simulation");
}
