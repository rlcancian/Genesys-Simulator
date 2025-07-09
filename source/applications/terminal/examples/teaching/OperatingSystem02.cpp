/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_SistemasOperacionais02.cpp
 * Author: rlcancian
 *
 * Created on 12 de abril de 2021, 21:54
 */

#include "OperatingSystem02.h"

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

OperatingSystem02::OperatingSystem02() {
}

int OperatingSystem02::main(int argc, char** argv) {
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
	createProc->setTimeBetweenCreationsExpression("expo(10)");
	createProc->setTimeUnit(Util::TimeUnit::milisecond);
	//
	// ASSIGN Define tempo de execução e memória ocupada
	Assign* assignProc = plugins->newInstance<Assign>(model);
	assignProc->setDescription("Define tempo de execução e memória ocupada");
	assignProc->getAssignments()->insert(new Assignment("memoriaOcupada", "TRUNC(UNIF(2,32))"));
	assignProc->getAssignments()->insert(new Assignment("tempoExecucao", "NORM(5,1) * memoriaOcupada/10"));
	// atributos (não precisa guardar objetos se não for usar)
	plugins->newInstance<Attribute>(model, "memoriaOcupada");
	plugins->newInstance<Attribute>(model, "tempoExecucao");

	createProc->getConnectionManager()->insert(assignProc);
	//
	// SEIZE memoria
	Resource* memoria = plugins->newInstance<Resource>(model, "memoria");
	memoria->setCapacity(64);
	Queue* queueMem = plugins->newInstance<Queue>(model, "Fila_Alocacao_Memoria");
	Seize* seizeMem = plugins->newInstance<Seize>(model);
	seizeMem->setDescription("Processo aloca memória");
	seizeMem->setQueue(queueMem);
	seizeMem->getSeizeRequests()->insert(new SeizableItem(memoria, "memoriaOcupada"));

	assignProc->getConnectionManager()->insert(seizeMem);
	//
	// ROUTE Processo é enviado para execução na CPU
	Station* stationExecucao = plugins->newInstance<Station>(model, "Estacao_de_Execucao");
	Route* routeExecucao1 = plugins->newInstance<Route>(model);
	routeExecucao1->setDescription("Processo é enviado para execução na CPU");
	routeExecucao1->setStation(stationExecucao);

	seizeMem->getConnectionManager()->insert(routeExecucao1);
	//
	// ENTER Processo chega para ser executado
	Enter* enterStationExecucao = plugins->newInstance<Enter>(model);
	enterStationExecucao->setDescription("Processo chega para ser executado");
	enterStationExecucao->setStation(stationExecucao);
	//
	// SEIZE Processo aloca CPU
	Resource* cpu = plugins->newInstance<Resource>(model, "CPU");
	Queue* queueCPU = plugins->newInstance<Queue>(model, "Fila_CPU");
	Seize* seizeCPU = plugins->newInstance<Seize>(model);
	seizeCPU->setDescription("Processo aloca CPU");
	seizeCPU->setQueue(queueCPU);
	seizeCPU->getSeizeRequests()->insert(new SeizableItem(cpu, "1"));

	enterStationExecucao->getConnectionManager()->insert(seizeCPU);
	//
	// DECIDE Define tempo de execução da fatia de tempo atual
	Decide* decideFatiaTempo = plugins->newInstance<Decide>(model);
	decideFatiaTempo->setDescription("Define tempo de execução da fatia de tempo atual");
	decideFatiaTempo->getConditions()->insert("tempoExecucao >= 1");

	seizeCPU->getConnectionManager()->insert(decideFatiaTempo);
	//
	// ASSIGN Define execucao por um quantum de tempo
	Assign* assignExecFatia = plugins->newInstance<Assign>(model);
	assignExecFatia->setDescription("Define execucao por um quantum de tempo");
	assignExecFatia->getAssignments()->insert(new Assignment("fatiaTempo", "1"));
	assignExecFatia->getAssignments()->insert(new Assignment("tempoExecucao", "tempoExecucao-fatiaTempo"));
	plugins->newInstance<Attribute>(model, "fatiaTempo");

	decideFatiaTempo->getConnectionManager()->insert(assignExecFatia);
	//
	// ASSIGN Executa até o final
	Assign* assignExecAteFinal = plugins->newInstance<Assign>(model);
	assignExecAteFinal->setDescription("Executa até o final");
	assignExecAteFinal->getAssignments()->insert(new Assignment("fatiaTempo", "tempoExecucao"));
	assignExecAteFinal->getAssignments()->insert(new Assignment("tempoExecucao", "tempoExecucao - fatiaTempo"));

	decideFatiaTempo->getConnectionManager()->insert(assignExecAteFinal);
	//
	// DELAY Processo executa na CPU
	Delay* delayProcExecCPU = plugins->newInstance<Delay>(model);
	delayProcExecCPU->setDescription("Processo executa na CPU");
	delayProcExecCPU->setDelayExpression("fatiaTempo");
	delayProcExecCPU->setDelayTimeUnit(Util::TimeUnit::milisecond);

	assignExecFatia->getConnectionManager()->insert(delayProcExecCPU);
	assignExecAteFinal->getConnectionManager()->insert(delayProcExecCPU);
	//
	// RELEASE Processo libera CPU
	Release* releaseCPU = plugins->newInstance<Release>(model);
	releaseCPU->setDescription("Processo libera CPU");
	releaseCPU->getReleaseRequests()->insert(new SeizableItem(cpu, "1"));

	delayProcExecCPU->getConnectionManager()->insert(releaseCPU);
	//
	// DECIDE Se processo ainda precisa executar então vai para estação de execução
	Decide* decideAindaExec = plugins->newInstance<Decide>(model);
	decideAindaExec->setDescription("Se processo ainda precisa executar então vai para estação de execução");
	decideAindaExec->getConditions()->insert("tempoExecucao > 0");

	releaseCPU->getConnectionManager()->insert(decideAindaExec);
	//
	// ROUTE Processo é enviado de volta para execução
	Route* routeExecucao2 = plugins->newInstance<Route>(model);
	routeExecucao2->setDescription("Processo é enviado de volta para execução");
	routeExecucao2->setStation(stationExecucao);

	decideAindaExec->getConnectionManager()->insert(routeExecucao2);
	// ROUTE Processo é enviado para liberar memória
	Station* stationLiberaMem = plugins->newInstance<Station>(model, "Estacao_de_liberacao_de_memoria");
	Route* routeLiberaMem = plugins->newInstance<Route>(model);
	routeLiberaMem->setDescription("Processo é enviado para liberar memória");
	routeLiberaMem->setStation(stationLiberaMem);

	decideAindaExec->getConnectionManager()->insert(routeLiberaMem);
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

	enterStationLibera->getConnectionManager()->insert(releaseMem);
	//
	// DISPOSE Processo é encerrado
	Dispose* disp1 = plugins->newInstance<Dispose>(model);
	disp1->setDescription("Processo é encerrado");

	releaseMem->getConnectionManager()->insert(disp1);

	//
	// AJUSTA EXPERIMENTO (MODEL SIMULATION)
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(1e3, Util::TimeUnit::milisecond);
	sim->setNumberOfReplications(30);
	sim->setPauseOnReplication(true);
	// AJUSTA NÍVEL DE TRACE
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L2_results);
	// SALVA MODELO
	model->save("./models/SistemaOperacional02.gen");
	// EXECUTA CADA REPLICAÇÃO, RECOMEÇANDO APÓS CADA PAUSA
	do {
		sim->start();
	} while (sim->isPaused());
	delete genesys;
	return 0;
}
