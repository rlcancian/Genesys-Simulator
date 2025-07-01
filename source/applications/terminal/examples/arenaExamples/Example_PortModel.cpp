/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Example_PortModel.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Route.h"
#include "../../../../plugins/components/Seize.h"

#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Station.h"
#include "../../../../plugins/data/Variable.h"


Example_PortModel::Example_PortModel() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Example_PortModel::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
        genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L2_results);
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// crete model
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

        Resource* rBuffer = plugins->newInstance<Resource>(model, "rBuffer");
        rBuffer->setCapacity(1);
        Resource* rPier1 = plugins->newInstance<Resource>(model, "rPier1");
        rPier1->setCapacity(1);
        Resource* rPier2 = plugins->newInstance<Resource>(model, "rPier2");
        rPier2->setCapacity(1);
        Queue* seizeBufferResourceQueue = plugins->newInstance<Queue>(model, "Seize_Buffer_Resource.Queue");
        Variable* vPier1Busy = plugins->newInstance<Variable>(model, "vPier1Busy");
        vPier1Busy->setInitialValue(0);
        vPier1Busy->setValue(0);
        Variable* vPier2Busy = plugins->newInstance<Variable>(model, "vPier2Busy");
        vPier2Busy->setInitialValue(0);
        vPier2Busy->setValue(0);
        
        
//	Assign* temp = plugins->newInstance<Assign>(model);
//	temp->getAssignments()->insert(new Assignment("vPier1Busy", "0", false));
//	temp->getAssignments()->insert(new Assignment("vPier2Busy", "0", false));
        
	Create* create1 = plugins->newInstance<Create>(model, "Create_1");
        create1->setEntityTypeName("Ship");
	create1->setTimeBetweenCreationsExpression("expo(40)", Util::TimeUnit::second);
        Station* arrivalStation = plugins->newInstance<Station>(model, "Arrival_Station");
        Enter* enterArrivalStation = plugins->newInstance<Enter>(model, "Enter_Arrival");
        enterArrivalStation->setStation(arrivalStation);
        Route* routeToPierAlign = plugins->newInstance<Route>(model, "Route_to_Pier_Align");
        routeToPierAlign->setRouteTimeTimeUnit(Util::TimeUnit::second);
        routeToPierAlign->setRouteTimeExpression("15");

        Station* stationPierAlign = plugins->newInstance<Station>(model, "Station_Pier_Align");
        Enter* enterStationPierAlign = plugins->newInstance<Enter>(model, "Enter_Pier_Align");
        enterStationPierAlign->setStation(stationPierAlign);
        Seize* seizeBufferResource = plugins->newInstance<Seize>(model, "Seize_Buffer_Resource");
        seizeBufferResource->setQueue(seizeBufferResourceQueue);
	seizeBufferResource->getSeizeRequests()->insert(new SeizableItem(rBuffer, "1"));
        Decide* tryPier1 = plugins->newInstance<Decide>(model, "Try_Pier_1");
        tryPier1->getConditions()->insert("vPier1Busy == 0");

        Release* release3 = plugins->newInstance<Release>(model, "Release_3");
	release3->getReleaseRequests()->insert(new SeizableItem(rBuffer, "1"));
        Station* stationPier1Align = plugins->newInstance<Station>(model, "Station_Pier_1_Align");
        Enter* enterStationPier1Align = plugins->newInstance<Enter>(model, "Enter_Pier_1_Align");
        enterStationPier1Align->setStation(stationPier1Align);
        Assign* setPier1BusyFlag = plugins->newInstance<Assign>(model, "Set_Pier_1_Busy_Flag");
	setPier1BusyFlag->getAssignments()->insert(new Assignment("vPier1Busy", "1", false));
        Route* routePier1 = plugins->newInstance<Route>(model, "Route_Pier_1");
        routePier1->setRouteTimeTimeUnit(Util::TimeUnit::second);
        routePier1->setRouteTimeExpression("5");
        
        Decide* tryPier2 = plugins->newInstance<Decide>(model, "Try_Pier_2");
        tryPier2->getConditions()->insert("vPier2Busy == 0");
        
        Release* release4 = plugins->newInstance<Release>(model, "Release_4");
	release4->getReleaseRequests()->insert(new SeizableItem(rBuffer, "1"));
        Station* stationPier2Align = plugins->newInstance<Station>(model, "Station_Pier_2_Align");
        Enter* enterStationPier2Align = plugins->newInstance<Enter>(model, "Enter_Pier_2_Align");
        enterStationPier2Align->setStation(stationPier2Align);
        Assign* setPier2BusyFlag = plugins->newInstance<Assign>(model, "Set_Pier_2_Busy_Flag");
	setPier2BusyFlag->getAssignments()->insert(new Assignment("vPier2Busy", "1", false));
        Route* routePier2 = plugins->newInstance<Route>(model, "Route_Pier_2");
        routePier2->setRouteTimeTimeUnit(Util::TimeUnit::second);
        routePier2->setRouteTimeExpression("10");
        
	Delay* delay2 = plugins->newInstance<Delay>(model, "Delay_2");
	delay2->setDelayExpression("5", Util::TimeUnit::second);
        
        Station* stationPier1 = plugins->newInstance<Station>(model, "Station_Pier_1");
        Enter* enterStationPier1 = plugins->newInstance<Enter>(model, "Enter_Pier_1");
        enterStationPier1->setStation(stationPier1);
        Process* pier1LoadingOperation = plugins->newInstance<Process>(model, "Pier_1_Loading_Operation");
	pier1LoadingOperation->getSeizeRequests()->insert(new SeizableItem(rPier1, "1"));
	pier1LoadingOperation->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	pier1LoadingOperation->setDelayExpression("tria(35, 40, 60)");
        pier1LoadingOperation->setDelayTimeUnit(Util::TimeUnit::second);
        pier1LoadingOperation->setAllocationType(Util::AllocationType::ValueAdded);
        Assign* resetPier1BusyFlag = plugins->newInstance<Assign>(model, "Reset_Pier_1_Busy_Flag");
	resetPier1BusyFlag->getAssignments()->insert(new Assignment("vPier1Busy", "0", false));
        Route* routeToDeparture = plugins->newInstance<Route>(model, "Route_to_Departure");
        routeToDeparture->setRouteTimeTimeUnit(Util::TimeUnit::second);
        routeToDeparture->setRouteTimeExpression("15");
        
        Station* stationPier2 = plugins->newInstance<Station>(model, "Station_Pier_2");
        Enter* enterStationPier2 = plugins->newInstance<Enter>(model, "Enter_Pier2");
        enterStationPier2->setStation(stationPier2);
        Process* pier2LoadingOperation = plugins->newInstance<Process>(model, "Pier_2_Loading_Operation");
	pier2LoadingOperation->getSeizeRequests()->insert(new SeizableItem(rPier2, "1"));
	pier2LoadingOperation->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	pier2LoadingOperation->setDelayExpression("tria(35, 40, 60)");
        pier2LoadingOperation->setDelayTimeUnit(Util::TimeUnit::second);
        pier2LoadingOperation->setAllocationType(Util::AllocationType::ValueAdded);
        Assign* resetPier2BusyFlag = plugins->newInstance<Assign>(model, "Reset_Pier_2_Busy_Flag");
	resetPier2BusyFlag->getAssignments()->insert(new Assignment("vPier2Busy", "0", false));
        Route* routeToDeparture2 = plugins->newInstance<Route>(model, "Route_to_Departure_2");
        routeToDeparture2->setRouteTimeTimeUnit(Util::TimeUnit::second);
        routeToDeparture2->setRouteTimeExpression("10");
        
        Station* departureStation = plugins->newInstance<Station>(model, "Departure_Station");
        Enter* enterDepartureStation = plugins->newInstance<Enter>(model, "Enter_Departure");
        enterDepartureStation->setStation(departureStation);
	Dispose* dispose2 = plugins->newInstance<Dispose>(model, "Dispose_2");

	// connect model components to create a "workflow"
	create1->getConnections()->insert(enterArrivalStation);
        enterArrivalStation->getConnections()->insert(routeToPierAlign);
        routeToPierAlign->setStation(stationPierAlign);

        enterStationPierAlign->getConnections()->insert(seizeBufferResource);
        seizeBufferResource->getConnections()->insert(tryPier1);
        tryPier1->getConnections()->insert(release3);
        tryPier1->getConnections()->insert(tryPier2);
        
        release3->getConnections()->insert(enterStationPier1Align);
        enterStationPier1Align->getConnections()->insert(setPier1BusyFlag);
        setPier1BusyFlag->getConnections()->insert(routePier1);
        routePier1->setStation(stationPier1);
        
        tryPier2->getConnections()->insert(release4);
        release4->getConnections()->insert(enterStationPier2Align);
        enterStationPier2Align->getConnections()->insert(setPier2BusyFlag);
        setPier2BusyFlag->getConnections()->insert(routePier2);
        routePier2->setStation(stationPier2);
        tryPier2->getConnections()->insert(delay2);
        delay2->getConnections()->insert(tryPier1);
        
        enterStationPier1->getConnections()->insert(pier1LoadingOperation);
        pier1LoadingOperation->getConnections()->insert(resetPier1BusyFlag);
        resetPier1BusyFlag->getConnections()->insert(routeToDeparture);
        
        enterStationPier2->getConnections()->insert(pier2LoadingOperation);
        pier2LoadingOperation->getConnections()->insert(resetPier2BusyFlag);
        resetPier2BusyFlag->getConnections()->insert(routeToDeparture2);
        
        routeToDeparture->setStation(departureStation);
        routeToDeparture2->setStation(departureStation);
        
        enterDepartureStation->getConnections()->insert(dispose2);
        
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(12, Util::TimeUnit::hour);
        model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
        model->getSimulation()->setWarmUpPeriod(0.6);
        model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
        model->getSimulation()->setNumberOfReplications(120);
        
	model->save("./models/Example_PortModel.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

