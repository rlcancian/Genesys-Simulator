/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Modelo_SistemasOperacionais02.cpp
 * Author: rlcancian
 *
 * Created on 12 de abril de 2021, 21:54
 */

#include "Modelo_SistemaOperacional02.h"

#include "Simulator.h"
#include "Model.h"
#include "Create.h"
#include "Assign.h"
#include "Seize.h"
#include "Route.h"
#include "Enter.h"
#include "Decide.h"
#include "Delay.h"
#include "Release.h"
#include "Dispose.h"

//#include "EntityType.h"
//#include "Queue.h"
//#include "SeizableItemRequest.h"
//#include "Station.h"
#include "Attribute.h"

Modelo_SistemaOperacional02::Modelo_SistemaOperacional02() {
}

int Modelo_SistemaOperacional02::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->insertFakePluginsByHand(genesys);
	this->setDefaultTraceHandlers(genesys->getTracer());
	Model* model = genesys->getModels()->newModel();
	//
	// CREATE Processo é criado no computador
	EntityType* et = new EntityType(model, "processo");
	Create* createProc = new Create(model);
	createProc->setDescription("Processo é criado no computador");
	createProc->setEntityType(et);
	createProc->setTimeBetweenCreationsExpression("expo(10)");
	createProc->setTimeUnit(Util::TimeUnit::milisecond);
	//
	// ASSIGN Define tempo de execução e memória ocupada
	Assign* assignProc = new Assign(model);
	assignProc->setDescription("Define tempo de execução e memória ocupada");
	assignProc->getAssignments()->insert(new Assign::Assignment("memoriaOcupada", "TRUNC(UNIF(2,32))"));
	assignProc->getAssignments()->insert(new Assign::Assignment("tempoExecucao", "NORM(5,1) * memoriaOcupada/10"));
	// atributos (não precisa guardar objetos se não for usar)
	new Attribute(model, "memoriaOcupada");
	new Attribute(model, "tempoExecucao");

	createProc->getNextComponents()->insert(assignProc);
	//
	// SEIZE memoria
	Resource* memoria = new Resource(model, "memoria");
	memoria->setCapacity(64);
	Queue* queueMem = new Queue(model, "Fila_Alocacao_Memoria");
	Seize* seizeMem = new Seize(model);
	seizeMem->setDescription("Processo aloca memória");
	seizeMem->setQueue(queueMem);
	seizeMem->getSeizeRequests()->insert(new SeizableItemRequest(memoria, "memoriaOcupada"));

	assignProc->getNextComponents()->insert(seizeMem);
	//
	// ROUTE Processo é enviado para execução na CPU
	Station* stationExecucao = new Station(model, "Estacao_de_Execucao");
	Route* routeExecucao1 = new Route(model);
	routeExecucao1->setDescription("Processo é enviado para execução na CPU");
	routeExecucao1->setStation(stationExecucao);

	seizeMem->getNextComponents()->insert(routeExecucao1);
	//
	// ENTER Processo chega para ser executado
	Enter* enterStationExecucao = new Enter(model);
	enterStationExecucao->setDescription("Processo chega para ser executado");
	enterStationExecucao->setStation(stationExecucao);
	//
	// SEIZE Processo aloca CPU
	Resource* cpu = new Resource(model, "CPU");
	Queue* queueCPU = new Queue(model, "Fila_CPU");
	Seize* seizeCPU = new Seize(model);
	seizeCPU->setDescription("Processo aloca CPU");
	seizeCPU->setQueue(queueCPU);
	seizeCPU->getSeizeRequests()->insert(new SeizableItemRequest(cpu, "1"));

	enterStationExecucao->getNextComponents()->insert(seizeCPU);
	//
	// DECIDE Define tempo de execução da fatia de tempo atual
	Decide* decideFatiaTempo = new Decide(model);
	decideFatiaTempo->setDescription("Define tempo de execução da fatia de tempo atual");
	decideFatiaTempo->getConditions()->insert("tempoExecucao >= 1");

	seizeCPU->getNextComponents()->insert(decideFatiaTempo);
	//
	// ASSIGN Define execucao por um quantum de tempo
	Assign* assignExecFatia = new Assign(model);
	assignExecFatia->setDescription("Define execucao por um quantum de tempo");
	assignExecFatia->getAssignments()->insert(new Assign::Assignment("fatiaTempo", "1"));
	assignExecFatia->getAssignments()->insert(new Assign::Assignment("tempoExecucao", "tempoExecucao-fatiaTempo"));
	new Attribute(model, "fatiaTempo");

	decideFatiaTempo->getNextComponents()->insert(assignExecFatia);
	//
	// ASSIGN Executa até o final
	Assign* assignExecAteFinal = new Assign(model);
	assignExecAteFinal->setDescription("Executa até o final");
	assignExecAteFinal->getAssignments()->insert(new Assign::Assignment("fatiaTempo", "tempoExecucao"));
	assignExecAteFinal->getAssignments()->insert(new Assign::Assignment("tempoExecucao", "tempoExecucao - fatiaTempo"));

	decideFatiaTempo->getNextComponents()->insert(assignExecAteFinal);
	//
	// DELAY Processo executa na CPU
	Delay* delayProcExecCPU = new Delay(model);
	delayProcExecCPU->setDescription("Processo executa na CPU");
	delayProcExecCPU->setDelayExpression("fatiaTempo");
	delayProcExecCPU->setDelayTimeUnit(Util::TimeUnit::milisecond);

	assignExecFatia->getNextComponents()->insert(delayProcExecCPU);
	assignExecAteFinal->getNextComponents()->insert(delayProcExecCPU);
	//
	// RELEASE Processo libera CPU
	Release* releaseCPU = new Release(model);
	releaseCPU->setDescription("Processo libera CPU");
	releaseCPU->getReleaseRequests()->insert(new SeizableItemRequest(cpu, "1"));

	delayProcExecCPU->getNextComponents()->insert(releaseCPU);
	//
	// DECIDE Se processo ainda precisa executar então vai para estação de execução
	Decide* decideAindaExec = new Decide(model);
	decideAindaExec->setDescription("Se processo ainda precisa executar então vai para estação de execução");
	decideAindaExec->getConditions()->insert("tempoExecucao > 0");

	releaseCPU->getNextComponents()->insert(decideAindaExec);
	//
	// ROUTE Processo é enviado de volta para execução
	Route* routeExecucao2 = new Route(model);
	routeExecucao2->setDescription("Processo é enviado de volta para execução");
	routeExecucao2->setStation(stationExecucao);

	decideAindaExec->getNextComponents()->insert(routeExecucao2);
	// ROUTE Processo é enviado para liberar memória
	Station* stationLiberaMem = new Station(model, "Estacao_de_liberacao_de_memoria");
	Route* routeLiberaMem = new Route(model);
	routeLiberaMem->setDescription("Processo é enviado para liberar memória");
	routeLiberaMem->setStation(stationLiberaMem);

	decideAindaExec->getNextComponents()->insert(routeLiberaMem);
	//
	// ENTER Processo chega para liberar memória
	Enter* enterStationLibera = new Enter(model);
	enterStationLibera->setDescription("Processo chega para liberar memória");
	enterStationLibera->setStation(stationLiberaMem);
	//
	// RELEASE 	Processo libera memória
	Release* releaseMem = new Release(model);
	releaseMem->setDescription("Processo libera memória");
	releaseMem->getReleaseRequests()->insert(new SeizableItemRequest(memoria, "memoriaOcupada"));

	enterStationLibera->getNextComponents()->insert(releaseMem);
	//
	// DISPOSE Processo é encerrado
	Dispose* disp1 = new Dispose(model);
	disp1->setDescription("Processo é encerrado");

	releaseMem->getNextComponents()->insert(disp1);

	//
	// AJUSTA EXPERIMENTO (MODEL SIMULATION)
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(1e3);
	sim->setReplicationLengthTimeUnit(Util::TimeUnit::milisecond);
	sim->setNumberOfReplications(30);
	sim->setPauseOnReplication(true);
	// AJUSTA NÍVEL DE TRACE
	genesys->getTracer()->setTraceLevel(Util::TraceLevel::L3_results);
	// SALVA MODELO
	model->save("./models/SistemaOperacional02.txt");
	// EXECUTA CADA REPLICAÇÃO, RECOMEÇANDO APÓS CADA PAUSA
	do {
		sim->start();
	} while (sim->isPaused());
	return 0;
}
