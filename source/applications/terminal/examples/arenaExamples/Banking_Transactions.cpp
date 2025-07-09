/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Dummy.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Banking_Transactions.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/data/Station.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Set.h"
#include "../../../../plugins/components/Route.h"

Banking_Transactions::Banking_Transactions() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Banking_Transactions::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// create model
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	//ATM customers ------------------------------------------------
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("AtmCustomer");
	create1->setTimeBetweenCreationsExpression("expo(60)", Util::TimeUnit::minute);

	Station* entranceStation = plugins->newInstance<Station>(model, "Estacao_de_Entrada");
	Station* atmArea = plugins->newInstance<Station>(model, "Estacao_de_ATM");
	Route* route1 = plugins->newInstance<Route>(model);
	route1->setStation(atmArea);
	route1->setRouteTimeExpression("UNIF(4,6)", Util::TimeUnit::minute);

	Resource* atm_machine = plugins->newInstance<Resource>(model, "caixa");
	atm_machine->setCapacity(1);
	Queue* queueAtm = plugins->newInstance<Queue>(model, "Fila_ATM");
	Seize* seizeAtm = plugins->newInstance<Seize>(model);
	seizeAtm->setQueue(queueAtm);
	seizeAtm->getSeizeRequests()->insert(new SeizableItem(atm_machine, "1"));
	Delay* delayAtm = plugins->newInstance<Delay>(model);
	delayAtm->setDelayExpression("UNIF(7,12)", Util::TimeUnit::minute);
	Release* releaseAtm = plugins->newInstance<Release>(model);
	releaseAtm->getReleaseRequests()->insert(new SeizableItem(atm_machine, "1"));

	Station* exitArea = plugins->newInstance<Station>(model, "Estacao_de_saida");
	Route* route2 = plugins->newInstance<Route>(model);
	route2->setStation(exitArea);
	route2->setRouteTimeExpression("5", Util::TimeUnit::minute);

	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	Enter* enterEntranceStation = plugins->newInstance<Enter>(model);
	Enter* enterAtmArea = plugins->newInstance<Enter>(model);
	Enter* enterExitArea = plugins->newInstance<Enter>(model);
	enterEntranceStation->setStation(entranceStation);
	enterAtmArea->setStation(atmArea);
	enterExitArea->setStation(exitArea);

	create1->getConnectionManager()->insert(enterEntranceStation);
	enterEntranceStation->getConnectionManager()->insert(route1);
	enterAtmArea->getConnectionManager()->insert(seizeAtm);
	seizeAtm->getConnectionManager()->insert(delayAtm);
	delayAtm->getConnectionManager()->insert(releaseAtm);
	releaseAtm->getConnectionManager()->insert(route2);
	enterExitArea->getConnectionManager()->insert(dispose1);



	//Teller customers --------------------------------------------------
	Create* create2 = plugins->newInstance<Create>(model);
	create2->setEntityTypeName("TellerCustomer");
	create2->setTimeBetweenCreationsExpression("expo(11)", Util::TimeUnit::minute);

	Station* doorStation = plugins->newInstance<Station>(model, "Door_Station");
	Enter* enterDoorStation = plugins->newInstance<Enter>(model);
	enterDoorStation->setStation(doorStation);
	Station* tellerStation = plugins->newInstance<Station>(model, "Teller_Station");
	Enter* enterTellerStation = plugins->newInstance<Enter>(model);
	enterTellerStation->setStation(tellerStation);

	Route* route3 = plugins->newInstance<Route>(model);
	route3->setStation(tellerStation);
	route3->setRouteTimeExpression("UNIF(5,7)", Util::TimeUnit::minute);

	Set* set1 = plugins->newInstance<Set>(model, "Set_Tellers");
	set1->setSetOfType(Util::TypeOf<Resource>());
	set1->getElementSet()->insert(plugins->newInstance<Resource>(model, "Judy"));
	set1->getElementSet()->insert(plugins->newInstance<Resource>(model, "Nancy"));
	Queue* queueTellers = plugins->newInstance<Queue>(model, "Fila_Teller");

	Seize* seizeTeller = plugins->newInstance<Seize>(model);
	seizeTeller->setQueue(queueTellers);
	seizeTeller->setAllocationType(Util::AllocationType::ValueAdded);
	seizeTeller->getSeizeRequests()->insert(new SeizableItem(set1, "1", SeizableItem::SelectionRule::CYCLICAL, "myTeller"));
	Delay* delayTellers = plugins->newInstance<Delay>(model);
	delayTellers->setDelayExpression("EXPO(17)", Util::TimeUnit::minute);
	Release* releaseTellers = plugins->newInstance<Release>(model);
	releaseTellers->getReleaseRequests()->insert(new SeizableItem(set1, "1", SeizableItem::SelectionRule::SPECIFICMEMBER, "myTeller"));

	Route* route4 = plugins->newInstance<Route>(model);
	route4->setStation(exitArea);
	route4->setRouteTimeExpression("5", Util::TimeUnit::minute);

	create2->getConnectionManager()->insert(enterDoorStation);
	enterDoorStation->getConnectionManager()->insert(route3);
	enterTellerStation->getConnectionManager()->insert(seizeTeller);
	seizeTeller->getConnectionManager()->insert(delayTellers);
	delayTellers->getConnectionManager()->insert(releaseTellers);
	releaseTellers->getConnectionManager()->insert(route4);



	//Drive Thru Customers ---------------------------------------------
	Create* create3 = plugins->newInstance<Create>(model);
	create3->setEntityTypeName("DriveCustomer");
	create3->setTimeBetweenCreationsExpression("expo(21)", Util::TimeUnit::minute);

	Station* driveStation = plugins->newInstance<Station>(model, "Drive_Station");
	Enter* enterDriveStation = plugins->newInstance<Enter>(model);
	enterDriveStation->setStation(driveStation);
	Station* windowStation = plugins->newInstance<Station>(model, "Window_Station");
	Enter* enterWindowStation = plugins->newInstance<Enter>(model);
	enterWindowStation->setStation(windowStation);

	Route* route5 = plugins->newInstance<Route>(model);
	route5->setStation(windowStation);
	route5->setRouteTimeExpression("UNIF(5,7)", Util::TimeUnit::minute);

	Resource* windowTeller = plugins->newInstance<Resource>(model, "Window_Teller");
	windowTeller->setCapacity(1);
	Queue* queueDrive = plugins->newInstance<Queue>(model, "Fila_Drive");
	Seize* seizeDrive = plugins->newInstance<Seize>(model);
	seizeDrive->setQueue(queueDrive);
	seizeDrive->getSeizeRequests()->insert(new SeizableItem(windowTeller, "1"));
	Delay* delayDrive = plugins->newInstance<Delay>(model);
	delayDrive->setDelayExpression("UNIF(5,20)", Util::TimeUnit::minute);
	Release* releaseDrive = plugins->newInstance<Release>(model);
	releaseDrive->getReleaseRequests()->insert(new SeizableItem(windowTeller, "1"));

	Station* driveExitStation = plugins->newInstance<Station>(model, "Drive_Exit");
	Enter* enterDriveExitStation = plugins->newInstance<Enter>(model);
	enterDriveExitStation->setStation(driveExitStation);

	Route* route6 = plugins->newInstance<Route>(model);
	route6->setStation(driveExitStation);
	route6->setRouteTimeExpression("5", Util::TimeUnit::minute);

	Dispose* dispose2 = plugins->newInstance<Dispose>(model);

	create3->getConnectionManager()->insert(enterDriveStation);
	enterDriveStation->getConnectionManager()->insert(route5);
	enterWindowStation->getConnectionManager()->insert(seizeDrive);
	seizeDrive->getConnectionManager()->insert(delayDrive);
	delayDrive->getConnectionManager()->insert(releaseDrive);
	releaseDrive->getConnectionManager()->insert(route6);
	enterDriveExitStation->getConnectionManager()->insert(dispose2);

	// set options, save and simulate
	model->getSimulation()->setReplicationLength(8, Util::TimeUnit::hour);
	model->getSimulation()->setWarmUpPeriod(8 * 0.05, Util::TimeUnit::hour);
	model->getSimulation()->setNumberOfReplications(3);
	model->save("./models/Banking_Transactions.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

