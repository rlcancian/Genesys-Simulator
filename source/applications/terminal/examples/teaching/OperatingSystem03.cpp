/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SistemaOperacional03.cpp
 * Author: rlcancian
 * 
 * Created on 23 de abril de 2021, 14:03
 */

#include "OperatingSystem03.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Model.h"
#include "../../../../kernel/simulator/Attribute.h"

#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Station.h"

#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Route.h"

#include <iostream>

OperatingSystem03::OperatingSystem03() {
}

int OperatingSystem03::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();
	//
	// CREATE Processo é criado no computador
	//EntityType* et = plugins->newInstance<EntityType>(model, "processo");
	Create* createProc = plugins->newInstance<Create>(model);
	createProc->setDescription("Processo é criado no computador");
	createProc->setEntityTypeName("processo"); //(et);
	createProc->setTimeBetweenCreationsExpression("expo(4)", Util::TimeUnit::milisecond);
	//
	// ASSIGN Define informacoes do processo
	Assign* assignProc = plugins->newInstance<Assign>(model);
	assignProc->setDescription("Define informacoes do processo");
	assignProc->getAssignments()->insert(new Assignment("memoriaOcupada", "TRUNC(UNIF(2,32))"));
	assignProc->getAssignments()->insert(new Assignment("tempoExecucao", "NORM(6,2) * memoriaOcupada/5"));
	assignProc->getAssignments()->insert(new Assignment("processoSO", "UNIF(0,1) < 0.1"));
	// atributos (não precisa guardar objetos se não for usar)
	plugins->newInstance<Attribute>(model, "memoriaOcupada");
	plugins->newInstance<Attribute>(model, "tempoExecucao");
	plugins->newInstance<Attribute>(model, "processoSO");

	createProc->getConnections()->insert(assignProc);
	//
	// SEIZE memoria
	Resource* memoria = plugins->newInstance<Resource>(model, "memoria");
	memoria->setCapacity(256);
	Queue* queueMem = plugins->newInstance<Queue>(model, "Fila_Alocacao_Memoria");
	Seize* seizeMem = plugins->newInstance<Seize>(model);
	seizeMem->setDescription("Processo aloca memória");
	seizeMem->setQueue(queueMem);
	seizeMem->getSeizeRequests()->insert(new SeizableItem(memoria, "memoriaOcupada"));

	assignProc->getConnections()->insert(seizeMem);
	//
	// DECIDE Define nucleo execucao do processo
	Decide* decideNucleo = plugins->newInstance<Decide>(model);
	decideNucleo->setDescription("Define nucleo execucao do processo");
	decideNucleo->getConditions()->insert("processoSO == 1");
	decideNucleo->getConditions()->insert("NQ(filaNucleo1) <= NQ(filaNucleo2) and NQ(filaNucleo1) <= NQ(filaNucleo3)");
	decideNucleo->getConditions()->insert("NQ(filaNucleo2) <= NQ(filaNucleo1) and NQ(filaNucleo2) <= NQ(filaNucleo3)");

	seizeMem->getConnections()->insert(decideNucleo);
	//
	// ASSIGN Nucleo 0
	Assign* assignDefNucleo0 = plugins->newInstance<Assign>(model);
	assignDefNucleo0->getAssignments()->insert(new Assignment("nucleoExecucao", "0"));
	plugins->newInstance<Attribute>(model, "nucleoExecucao");
	//
	// ASSIGN Nucleo 1
	Assign* assignDefNucleo1 = plugins->newInstance<Assign>(model);
	assignDefNucleo1->getAssignments()->insert(new Assignment("nucleoExecucao", "1"));
	//
	// ASSIGN Nucleo 2
	Assign* assignDefNucleo2 = plugins->newInstance<Assign>(model);
	assignDefNucleo2->getAssignments()->insert(new Assignment("nucleoExecucao", "2"));
	//
	// ASSIGN Nucleo 3
	Assign* assignDefNucleo3 = plugins->newInstance<Assign>(model);
	assignDefNucleo3->getAssignments()->insert(new Assignment("nucleoExecucao", "3"));

	decideNucleo->getConnections()->insert(assignDefNucleo0);
	decideNucleo->getConnections()->insert(assignDefNucleo1);
	decideNucleo->getConnections()->insert(assignDefNucleo2);
	decideNucleo->getConnections()->insert(assignDefNucleo3);
	//
	// ROUTE Processo é enviado para execução na CPU
	Station* stationExecucao = plugins->newInstance<Station>(model, "Estacao_de_Execucao");
	Route* routeExecucao1 = plugins->newInstance<Route>(model);
	routeExecucao1->setDescription("Processo é enviado para execução na CPU");
	routeExecucao1->setStation(stationExecucao);

	assignDefNucleo0->getConnections()->insert(routeExecucao1);
	assignDefNucleo1->getConnections()->insert(routeExecucao1);
	assignDefNucleo2->getConnections()->insert(routeExecucao1);
	assignDefNucleo3->getConnections()->insert(routeExecucao1);

	//
	// ENTER Processo chega para ser executado
	Enter* enterStationExecucao = plugins->newInstance<Enter>(model);
	enterStationExecucao->setDescription("Processo chega para ser executado");
	enterStationExecucao->setStation(stationExecucao);
	//
	// SEIZE Processo aloca Núcleo de Execução
	//     SET RESOURCES
	Set* setNucleos = plugins->newInstance<Set>(model, "Set_Nucleos");
	setNucleos->setSetOfType(Util::TypeOf<Resource>());
	setNucleos->getElementSet()->insert(plugins->newInstance<Resource>(model, "nucleo0"));
	setNucleos->getElementSet()->insert(plugins->newInstance<Resource>(model, "nucleo1"));
	setNucleos->getElementSet()->insert(plugins->newInstance<Resource>(model, "nucleo2"));
	setNucleos->getElementSet()->insert(plugins->newInstance<Resource>(model, "nucleo3"));
	//std::cout << setNucleos->getElementSet()->show() << std::endl;
	//     SET QUEUES
	Set* setFilas = plugins->newInstance<Set>(model, "Set_Filas");
	setFilas->setSetOfType(Util::TypeOf<Queue>());
	setFilas->getElementSet()->insert(plugins->newInstance<Queue>(model, "filaNucleo0"));
	setFilas->getElementSet()->insert(plugins->newInstance<Queue>(model, "filaNucleo1"));
	setFilas->getElementSet()->insert(plugins->newInstance<Queue>(model, "filaNucleo2"));
	setFilas->getElementSet()->insert(plugins->newInstance<Queue>(model, "filaNucleo3"));
	std::cout << setFilas->getElementSet()->show() << std::endl;
	// SEIZE
	Seize* seizeNucleo = plugins->newInstance<Seize>(model);
	seizeNucleo->setDescription("Processo aloca Núcleo de Execução");
	seizeNucleo->setQueueableItem(new QueueableItem(setFilas, QueueableItem::QueueableType::SET, "nucleoExecucao"));
	seizeNucleo->getSeizeRequests()->insert(new SeizableItem(setNucleos, "1", SeizableItem::SelectionRule::SPECIFICMEMBER, "", "nucleoExecucao"));

	enterStationExecucao->getConnections()->insert(seizeNucleo);
	//
	// DECIDE Define tempo de execução da fatia de tempo atual
	Decide* decideFatiaTempo = plugins->newInstance<Decide>(model);
	decideFatiaTempo->setDescription("Define tempo de execução da fatia de tempo atual");
	decideFatiaTempo->getConditions()->insert("tempoExecucao >= 2");

	seizeNucleo->getConnections()->insert(decideFatiaTempo);
	//
	// ASSIGN Define execucao por um quantum de tempo
	Assign* assignExecFatia = plugins->newInstance<Assign>(model);
	assignExecFatia->setDescription("Define execucao por um quantum de tempo");
	assignExecFatia->getAssignments()->insert(new Assignment("fatiaTempo", "2"));
	assignExecFatia->getAssignments()->insert(new Assignment("tempoExecucao", "tempoExecucao-fatiaTempo"));
	plugins->newInstance<Attribute>(model, "fatiaTempo");

	decideFatiaTempo->getConnections()->insert(assignExecFatia);
	//
	// ASSIGN Executa até o final
	Assign* assignExecAteFinal = plugins->newInstance<Assign>(model);
	assignExecAteFinal->setDescription("Executa até o final");
	assignExecAteFinal->getAssignments()->insert(new Assignment("fatiaTempo", "tempoExecucao"));
	assignExecAteFinal->getAssignments()->insert(new Assignment("tempoExecucao", "tempoExecucao - fatiaTempo"));

	decideFatiaTempo->getConnections()->insert(assignExecAteFinal);
	//
	// DELAY Processo executa no nucleo de execucao
	Delay* delayProcExecNucleo = plugins->newInstance<Delay>(model);
	delayProcExecNucleo->setDescription("Processo executa no nucleo de execucao");
	delayProcExecNucleo->setDelayExpression("fatiaTempo", Util::TimeUnit::milisecond);

	assignExecFatia->getConnections()->insert(delayProcExecNucleo);
	assignExecAteFinal->getConnections()->insert(delayProcExecNucleo);
	//
	// RELEASE Processo libera nucleo execucao
	Release* releaseNucleo = plugins->newInstance<Release>(model);
	releaseNucleo->setDescription("Processo libera nucleo execucao");
	releaseNucleo->getReleaseRequests()->insert(new SeizableItem(setNucleos, "1", SeizableItem::SelectionRule::SPECIFICMEMBER, "", "nucleoExecucao"));

	delayProcExecNucleo->getConnections()->insert(releaseNucleo);
	//
	// DECIDE Se processo ainda precisa executar então vai para estação de execução
	Decide* decideAindaExec = plugins->newInstance<Decide>(model);
	decideAindaExec->setDescription("Se processo ainda precisa executar então vai para estação de execução");
	decideAindaExec->getConditions()->insert("tempoExecucao > 0");

	releaseNucleo->getConnections()->insert(decideAindaExec);
	//
	// ROUTE Processo é enviado de volta para execução
	Route* routeExecucao2 = plugins->newInstance<Route>(model);
	routeExecucao2->setDescription("Processo é enviado de volta para execução");
	routeExecucao2->setStation(stationExecucao);

	decideAindaExec->getConnections()->insert(routeExecucao2);
	// ROUTE Processo é enviado para liberar memória
	Station* stationLiberaMem = plugins->newInstance<Station>(model, "Estacao_de_liberacao_de_memoria");
	Route* routeLiberaMem = plugins->newInstance<Route>(model);
	routeLiberaMem->setDescription("Processo é enviado para liberar memória");
	routeLiberaMem->setStation(stationLiberaMem);

	decideAindaExec->getConnections()->insert(routeLiberaMem);
	//
	// ENTER Processo chega para liberar memória
	Enter* enterStationLibera = plugins->newInstance<Enter>(model);
	enterStationLibera->setDescription("Processo chega para liberar memória");
	enterStationLibera->setStation(stationLiberaMem);
	//
	// RELEASE 	Processo libera memória
	Release* releaseMem = plugins->newInstance<Release>(model);
	releaseMem->setDescription("Processo libera memória");
	releaseMem->getReleaseRequests()->insert(new SeizableItem(memoria, "memoriaOcupada"));

	enterStationLibera->getConnections()->insert(releaseMem);
	//
	// DISPOSE Processo é encerrado
	Dispose* disp1 = plugins->newInstance<Dispose>(model);
	disp1->setDescription("Processo é encerrado");

	releaseMem->getConnections()->insert(disp1);

	//
	// AJUSTA EXPERIMENTO (MODEL SIMULATION)
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(2, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::milisecond);
	sim->setNumberOfReplications(200);
	sim->setWarmUpPeriod(0.02);
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::second);
	sim->setShowReportsAfterReplication(true);
	sim->setShowReportsAfterSimulation(true);
	//sim->getBreakpointsOnComponent()->insert(routeExecucao1);
	//sim->getBreakpointsOnComponent()->insert(assignDefNucleo0);
	//sim->getBreakpointsOnComponent()->insert(assignDefNucleo1);
	//sim->getBreakpointsOnComponent()->insert(assignDefNucleo2);
	//sim->getBreakpointsOnComponent()->insert(assignDefNucleo3);
	model->getTracer()->setTraceLevel(TraceManager::Level::L7_internal);
	model->save("./models/SistemaOperacional03.cpp");
	//genesys->getTracer()->setTraceLevel(TraceManager::Level::L2_results); // :L9_mostDetailed);
	do {
		sim->start(); //step();
		if (sim->isPaused()) {
			std::cout << "Press ENTER to continue...";
			std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
		}
	} while (sim->isPaused());
	delete genesys;
	return 0;
}


