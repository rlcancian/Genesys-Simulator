/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SynchronizingParallelEntities.cpp
 * Author: rlcancian
 * Author: douglaspereira04
 * 
 */

#include "Smart_SynchronizingParallelEntities.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Clone.h"
#include "../../../../plugins/components/Batch.h"

#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Variable.h"
#include "../../../TraitsApp.h"

Smart_SynchronizingParallelEntities::Smart_SynchronizingParallelEntities() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SynchronizingParallelEntities::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        //Orders Arrive
	Create* orders_arrive = plugins->newInstance<Create>(model, "OrdersArrive");
        orders_arrive->setEntityTypeName("Entity1");
	orders_arrive->setTimeBetweenCreationsExpression("expo(5.0)");
	orders_arrive->setTimeUnit(Util::TimeUnit::minute);
	orders_arrive->setEntitiesPerCreation(1);
	orders_arrive->setFirstCreation(0.0);
        orders_arrive->setReportStatistics(true);
        
        Variable* serial = plugins->newInstance<Variable>(model, "Serial");
        serial->setInitialValue(0);
        Attribute* serial_number = plugins->newInstance<Attribute>(model, "SerialNumber");
        Assign* assign_serial_number = plugins->newInstance<Assign>(model, "Assign serial number");
	assign_serial_number->getAssignments()->insert(new Assignment(model, "Serial", "Serial + 1", false));
	assign_serial_number->getAssignments()->insert(new Assignment(model, "SerialNumber", "Serial", true));
        
        //Duplicate the order entity
	Clone* duplicate_the_order_entity = plugins->newInstance<Clone>(model, "DuplicateTheOrderEntity");
	duplicate_the_order_entity->setNumClonesExpression("1");
        duplicate_the_order_entity->setReportStatistics(false);
        
        //Fill Order
        Resource* clerk_1 = plugins->newInstance<Resource>(model, "Clerk1");
        clerk_1->setCapacity(2);
        clerk_1->setReportStatistics(true);
        
        Queue* fill_order_queue = plugins->newInstance<Queue>(model, "FillOrder.Queue");
        fill_order_queue->setOrderRule(Queue::OrderRule::FIFO);
        fill_order_queue->setReportStatistics(true);
        
        Process* fill_order = plugins->newInstance<Process>(model, "FillOrder");
	fill_order->getSeizeRequests()->insert(new SeizableItem(clerk_1, "1"));
        fill_order->setQueueableItem(new QueueableItem(fill_order_queue));
        fill_order->setDelayTimeUnit(Util::TimeUnit::minute);
        fill_order->setDelayExpression("unif(5.0,15.0)");
	fill_order->setReportStatistics(true);
        
        //Process Billing Information
        Resource* clerk_2 = plugins->newInstance<Resource>(model, "Clerk2");
        clerk_2->setCapacity(2);
        clerk_2->setReportStatistics(true);
        
        Queue* process_billing_information_queue = plugins->newInstance<Queue>(model, "ProcessBillingInformation.Queue");
        process_billing_information_queue->setOrderRule(Queue::OrderRule::FIFO);
        process_billing_information_queue->setReportStatistics(true);
        
        Process* process_billing_information = plugins->newInstance<Process>(model, "ProcessBillingInformation");
	process_billing_information->getSeizeRequests()->insert(new SeizableItem(clerk_2, "1"));
        process_billing_information->setQueueableItem(new QueueableItem(process_billing_information_queue));
        process_billing_information->setDelayTimeUnit(Util::TimeUnit::minute);
        process_billing_information->setDelayExpression("unif(8.0,15.0)");
	process_billing_information->setReportStatistics(true);
        
        //Attach Bill to Order
        Batch* attach_bill_to_order = plugins->newInstance<Batch>(model, "AttachBillToOrder");
        attach_bill_to_order->setBatchSize("2");
	attach_bill_to_order->setRule(Batch::Rule::ByAttribute);
	attach_bill_to_order->setAttributeName("SerialNumber");
	attach_bill_to_order->setGroupedAttributes(Batch::GroupedAttribs::LastEntity);
	attach_bill_to_order->setGroupedEntityTypeName("Entity1");
        attach_bill_to_order->setReportStatistics(true);
        
        //Deliver Order
        Dispose* deliver_order = plugins->newInstance<Dispose>(model, "DeliverOrder");
        deliver_order->setReportStatistics(true);
	// connect model components to create a "workflow"
	
	orders_arrive->getConnections()->insert(assign_serial_number);
	assign_serial_number->getConnections()->insert(duplicate_the_order_entity);
        
        duplicate_the_order_entity->getConnections()->insert(fill_order);
	duplicate_the_order_entity->getConnections()->insert(process_billing_information);
        
	fill_order->getConnections()->insert(attach_bill_to_order);
	process_billing_information->getConnections()->insert(attach_bill_to_order);
        
        attach_bill_to_order->getConnections()->insert(deliver_order);
        
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
        sim->setWarmUpPeriod(4312.813209*0.05);
        sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setNumberOfReplications(300);
	sim->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week);
	sim->setTerminatingCondition("count(DeliverOrder.CountNumberIn)>1000 or (((count(OrdersArrive.CountNumberOut)-count(DeliverOrder.CountNumberIn))*2)>=150)");
        sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
        model->save("./models/Smart_SynchronizingParallelEntities.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};

