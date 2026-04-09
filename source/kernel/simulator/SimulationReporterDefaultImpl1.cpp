/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SimulationReporterDefaultImpl1.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 8 de Agosto de 2018, 10:59
 */

#include "SimulationReporterDefaultImpl1.h"
#include <assert.h>
#include <iomanip>
#include <iostream>
#include "Counter.h"

//using namespace GenesysKernel;

SimulationReporterDefaultImpl1::SimulationReporterDefaultImpl1(ModelSimulation* simulation, Model* model, List<ModelDataDefinition*>* statsCountersSimulation) {
	_simulation = simulation;
	_model = model;
	_statsCountersSimulation = statsCountersSimulation;
}

void SimulationReporterDefaultImpl1::showSimulationControls() {
	_model->getTracer()->traceReport("Simulation Controls:");
	Util::IncIndent();
 	{
 		for (SimulationControl* control : *_model->getControls()->list()) {
 			_model->getTracer()->traceReport(control->getClassname() + "." +control->getName() + ": " + control->getValue());
 			_model->getTracer()->traceReport("("+control->getClassname() + ") "+ control->getElementName()+"." +control->getName() + ": " + control->getValue());
 		}
 	}
	Util::DecIndent();
}

void SimulationReporterDefaultImpl1::showReplicationStatistics() {
	_model->getTracer()->traceReport("");
	_model->getTracer()->traceReport("Begin of Report for replication " + std::to_string(_simulation->getCurrentReplicationNumber()) + " of " + std::to_string(_model->getSimulation()->getNumberOfReplications()));
	/* @TODO: StatisticsCollector and Counter should NOT be special classes. It should iterate classes looking for classes that can generate reports.
	 StatisticsCollector and Counter should ovveride an inherited attribute from ModelDataDefinition to specify they generate report information
	 look for _generateReportInformation = true;  using bool generateReportInformation() const;
	 */
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
	// runs over all elements and list the statistics for each one, and then the statistics with no parent
	Util::IncIndent();

	if (_simulation->isShowSimulationControlsInReport())
		this->showSimulationControls(); // assumed the controls are the same for all replications, show them only for the whole simulation

	// Combine stats and counters in local temporary lists without heap allocation.
	std::list<ModelDataDefinition*> statisticsAndCounters(*(_model->getDataManager()->getDataDefinitionList(UtilTypeOfStatisticsCollector)->list()));
	std::list<ModelDataDefinition*> counters(*(_model->getDataManager()->getDataDefinitionList(UtilTypeOfCounter)->list()));
	statisticsAndCounters.merge(counters);
	//statisticsAndCounters->insert(counters->list()->begin(), counters->list()->end());
	// Group report items by parent type and parent name with automatic storage containers.
	std::map<std::string, std::map<std::string, std::list<ModelDataDefinition*> > > mapMapTypeStat;

	for (ModelDataDefinition* cstatOrCounter : statisticsAndCounters) {
		std::string parentName, parentTypename;
		//std::cout << statOrCnt->getName() << ": " << statOrCnt->getTypename() << std::endl;
		if (cstatOrCounter->getClassname() == UtilTypeOfStatisticsCollector) {
			StatisticsCollector* cstat = dynamic_cast<StatisticsCollector*> (cstatOrCounter);
			parentName = cstat->getParent()->getName();
			parentTypename = cstat->getParent()->getClassname();
		} else {
			if (cstatOrCounter->getClassname() == UtilTypeOfCounter) {
				Counter* counter = dynamic_cast<Counter*> (cstatOrCounter);
				parentName = counter->getParent()->getName();
				parentTypename = counter->getParent()->getClassname();

			}
		}
		// look for key=parentTypename
		// Insert into nested maps/lists via references to avoid heap-allocated pairs and lists.
		std::list<ModelDataDefinition*>& listStatAndCount = mapMapTypeStat[parentTypename][parentName];
		listStatAndCount.insert(listStatAndCount.end(), cstatOrCounter);
		//_model->getTraceManager()->traceReport(parentTypename + " -> " + parentName + " -> " + stat->show());
	}
	//
	//
	// now runs over that map of maps showing the statistics
	Util::IncIndent();
	Util::IncIndent();
	_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
	Util::DecIndent();
	Util::DecIndent();
	const unsigned short _w1 = _w - 1;
	for (const auto& mapmapItem : mapMapTypeStat) {
		_model->getTracer()->traceReport("Statistics for " + mapmapItem.first + ":");
		Util::IncIndent();
		{
			for (const auto& mapItem : mapmapItem.second) {
				_model->getTracer()->traceReport(mapItem.first + ":");
				Util::IncIndent();
				{
					//					_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
					for (ModelDataDefinition* const item : mapItem.second) {
						if (item->getClassname() == UtilTypeOfStatisticsCollector) {
							Statistics_if* stat = dynamic_cast<StatisticsCollector*> (item)->getStatistics();
							_model->getTracer()->traceReport(
									Util::SetW(item->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->numElements()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->min()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->max()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->average()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->variance()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->stddeviation()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->variationCoef()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->halfWidthConfidenceInterval()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->confidenceLevel()), _w)
									, TraceManager::Level::L2_results);
						} else {
							if (item->getClassname() == UtilTypeOfCounter) {
								Counter* count = dynamic_cast<Counter*> (item);
								_model->getTracer()->traceReport(
										Util::SetW(count->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
										Util::SetW(Util::StrTruncIfInt(count->getCountValue()), _w)
										, TraceManager::Level::L2_results);
							}
						}
					}
				}
				Util::DecIndent();
			}
		}
		Util::DecIndent();
	}
	if (_simulation->isShowSimulationResposesInReport())
		this->showSimulationResponses();
	Util::DecIndent();
	_model->getTracer()->traceReport("End of Report for replication " + std::to_string(_simulation->getCurrentReplicationNumber()) + " of " + std::to_string(_model->getSimulation()->getNumberOfReplications()));
}

void SimulationReporterDefaultImpl1::showSimulationResponses() {
	_model->getTracer()->traceReport("Simulation Responses:");
	Util::IncIndent();
	{
		for (SimulationResponse* response : *_model->getResponses()->list()) {
			_model->getTracer()->traceReport("("+response->getClassname() + ") "+ response->getElementName()+"." +response->getName() + ": " + response->getValue());
		}
	}
	Util::DecIndent();
}

void SimulationReporterDefaultImpl1::showSimulationStatistics() {//List<StatisticsCollector*>* cstatsSimulation) {
	_model->getTracer()->traceReport("");
	_model->getTracer()->traceReport("Begin of Report for Simulation (based on " + std::to_string(_model->getSimulation()->getNumberOfReplications()) + " replications)");
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
	Util::IncIndent();
	if (_simulation->isShowSimulationControlsInReport())
		this->showSimulationControls(); // assumed the controls are the same for all replications, show them only for the whole simulation
	// runs over all elements and list the statistics for each one, and then the statistics with no parent
	// COPY the list of statistics and counters into a single new list
	//std::list<ModelDataDefinition*>* statisticsAndCounters = //new std::list<ModelDataDefinition*>(*(this->_statsCountersSimulation->list()));
	// Group simulation-level stats and counters in local containers without heap-allocated nodes.
	std::map<std::string, std::map<std::string, std::list<ModelDataDefinition*> > > mapMapTypeStat;

	for (std::list<ModelDataDefinition*>::iterator it = _statsCountersSimulation->list()->begin(); it != _statsCountersSimulation->list()->end(); it++) {
		std::string parentName, parentTypename;
		ModelDataDefinition* statOrCnt = (*it);
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
		// Insert into grouped lists using map references to avoid manual heap lifecycle.
		std::list<ModelDataDefinition*>& listStat = mapMapTypeStat[parentTypename][parentName];
		listStat.insert(listStat.end(), statOrCnt);
		//_model->getTraceManager()->traceReport(parentTypename + " -> " + parentName + " -> " + stat->show());
	}
	// now runs over that map of maps showing the statistics
	//int w = 12;
	Util::IncIndent();
	Util::IncIndent();
	_model->getTracer()->traceReport(Util::SetW("name", _nameW) + Util::SetW("elems", _w) + Util::SetW("min", _w) + Util::SetW("max", _w) + Util::SetW("average", _w) + Util::SetW("variance", _w) + Util::SetW("stddev", _w) + Util::SetW("varCoef", _w) + Util::SetW("confInterv", _w) + Util::SetW("confLevel", _w));
	Util::DecIndent();
	Util::DecIndent();
	// @TODO: USE REFERENCE TO MAPITEM TO AVOID COPY
	const unsigned short _w1 = _w - 1;
	for (const auto& mapmapItem : mapMapTypeStat) {
		_model->getTracer()->traceReport("Statistics for " + mapmapItem.first + ":");
		Util::IncIndent();
		{
			for (const auto& mapItem : mapmapItem.second) {
				_model->getTracer()->traceReport(mapItem.first + ":");
				Util::IncIndent();
				{
					for (ModelDataDefinition* const item : mapItem.second) {
						if (item->getClassname() == UtilTypeOfStatisticsCollector) {
							Statistics_if* stat = dynamic_cast<StatisticsCollector*> (item)->getStatistics();
							_model->getTracer()->traceReport(
									Util::SetW(item->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->numElements()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->min()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->max()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->average()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->variance()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->stddeviation()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->variationCoef()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->halfWidthConfidenceInterval()), _w1) + " " +
									Util::SetW(Util::StrTruncIfInt(stat->confidenceLevel()), _w)
									, TraceManager::Level::L2_results);
						} else {
							if (item->getClassname() == UtilTypeOfCounter) {
								Counter* cnt = dynamic_cast<Counter*> (item);
								_model->getTracer()->traceReport(
										Util::SetW(cnt->getName() + std::string(_nameW, '.'), _nameW - 1) + " " +
										Util::SetW(Util::StrTruncIfInt(cnt->getCountValue()), _w)
										, TraceManager::Level::L2_results);
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
