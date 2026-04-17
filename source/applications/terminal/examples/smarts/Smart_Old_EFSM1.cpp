#include "Smart_Old_EFSM1.h"
#include <map>

// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/EntityType.h"
#include "kernel/simulator/ModelSimulation.h"
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/components/DiscreteProcessing/Assign.h"
#include "plugins/data/DiscreteProcessing/Variable.h"
#include "../../../../plugins/data/EFSM.h"
#include "../../../../plugins/components/FSM_State.h"
#include "../../../../plugins/components/FSM_Transition.h"
#include "../../../../plugins/components/FSM_State.h"
#include "../../../../plugins/components/FSM_ModalModel.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

#include "../../../TraitsApp.h"


Smart_Old_EFSM1::Smart_Old_EFSM1() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Old_EFSM1::main(int argc, char** argv) {
	// instantiate simulator
	Simulator* genesys = new Simulator();
	genesys->getTracer()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTracer());
    PluginManager* plugins = genesys->getPlugins();
    plugins->autoInsertPlugins("autoloadplugins.txt");

	// create model
	Model* model = genesys->getModels()->newModel();

    EntityType* entityType = plugins->newInstance<EntityType>(model, "Person");
    Create* create1 = plugins->newInstance<Create>(model);
    create1->setDescription("Enter Garage");
    create1->setEntityType(entityType);
    create1->setTimeBetweenCreationsExpression("1");
    create1->setTimeUnit(Util::TimeUnit::minute);

    Assign* assign1 = plugins->newInstance<Assign>(model);
    assign1->setDescription("Verify if has car");
    Assignment* assigment1 = new Assignment("hasCar", "1");
    assign1->getAssignments()->insert(assigment1);
    create1->getConnections()->insert(assign1);

	Delay* delay1 = plugins->newInstance<Delay>(model); // the default delay time is 1.0 s
    delay1->setDescription("Browse");
    delay1->setDelayExpression("10");
    delay1->setDelayTimeUnit(Util::TimeUnit::second);
    assign1->getConnections()->insert(delay1);

    ExtendedFSM* efsm1 = plugins->newInstance<ExtendedFSM>(model, "efsm_1");
    
	Variable* var1 = plugins->newInstance<Variable>(model, "carsParked");
	var1->setInitialValue(0.0, "carsParked"); 
    efsm1->insertVariable(var1);

    efsm1->CreateInternalData(efsm1);   
    FSM_State* state1 = plugins->newInstance<FSM_State>(model, "state_1");
    state1->setEFSM(efsm1);
    state1->setAsInitialState();

    FSM_Transition* transition1 = plugins->newInstance<FSM_Transition>(model, "transition_1");
    transition1->setGuardExpression("(hasCar == 1) and (carsParked < 5)");
    transition1->setOutputActions("hasCar = 0");
    transition1->setSetActions("carsParked = carsParked + 1");
    transition1->setNondeterministic(true);
    state1->getConnections()->insert(transition1);
    transition1->getConnections()->insert(state1);

    FSM_Transition* transition2 = plugins->newInstance<FSM_Transition>(model,"transition_2");
    transition2->setGuardExpression("hasCar == 0 and carsParked > 0");
    transition2->setOutputActions("hasCar = 1");
    transition2->setSetActions("carsParked = carsParked - 1");
    transition2->setNondeterministic(true);
    state1->getConnections()->insert(transition2);
    transition2->getConnections()->insert(state1);

    FSM_Transition* transition3 = plugins->newInstance<FSM_Transition>(model,"transition_3");
    transition3->setGuardExpression("");
    transition3->setOutputActions("");
    transition3->setSetActions("");
    transition3->setDefault(true);
    transition3->setNondeterministic(true);
    state1->getConnections()->insert(transition3);
    transition3->getConnections()->insert(state1);

    FSM_ModalModel* modalmodel1 = plugins->newInstance<FSM_ModalModel>(model, "modalmodel_1");
    modalmodel1->setEFSMData(efsm1);
    delay1->getConnections()->insert(modalmodel1);

    Dispose* dispose1 = plugins->newInstance<Dispose>(model);
    dispose1->setReportStatistics(true);
    modalmodel1->getConnections()->insert(dispose1);
	
    // set options, save and simulate
	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::minute);
	model->getSimulation()->setTerminatingCondition("");
	model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::hour);
    model->save("./models/Smart_EFSM1.gen");
	model->getSimulation()->start();	
	// free memory
	delete genesys;
	
	return 0;
};
