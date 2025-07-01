#pragma once

/*
 * File:   BufferFIFO.h
 * Author: rlcancian
 *
 * Created on 13 de setembro de 2023, 18:03
 */

#include "../../../BaseGenesysTerminalApplication.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Clone.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Signal.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/data/SignalData.h"
#include "../../../../plugins/data/Variable.h"

class BufferFIFO : public BaseGenesysTerminalApplication {
public:
	BufferFIFO() {}
public:
	virtual int main(int argc, char** argv) {
		Simulator* genesys = new Simulator();
		this->setDefaultTraceHandlers(genesys->getTraceManager());
		genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
		Model* model = genesys->getModelManager()->newModel();
		PluginManager* plugins = genesys->getPluginManager();

		Create* create1 = plugins->newInstance<Create>(model);
		create1->setTimeBetweenCreationsExpression("1", Util::TimeUnit::hour);

		Clone* clone1 = plugins->newInstance<Clone>(model);
		clone1->setNumClonesExpression("1");

		create1->getConnections()->insert(clone1);

		SignalData* sigdata1 = plugins->newInstance<SignalData>(model);
		Queue* queue1 = plugins->newInstance<Queue>(model, "waitingQueue");
		Wait* wait1 = plugins->newInstance<Wait>(model);
		wait1->setWaitType(Wait::WaitType::WaitForSignal);
		wait1->setSignalData(sigdata1);
		wait1->setQueue(queue1);

		clone1->getConnections()->insert(wait1);

		Dispose* dispose1 = plugins->newInstance<Dispose>(model);

		wait1->getConnections()->insert(dispose1);

		Decide* decide1 =  plugins->newInstance<Decide>(model);
		decide1->getConditions()->insert("nq(waitingQueue)<tamBuffer");
		Variable* var1 = plugins->newInstance<Variable>(model, "tamBuffer");
		var1->setInitialValue(5);

		clone1->getConnections()->insert(decide1);

		Dispose* dispose2 = plugins->newInstance<Dispose>(model);
		dispose2->setReportStatistics(false);

		decide1->getConnections()->insert(dispose2);

		Signal* signal1 = plugins->newInstance<Signal>(model);
		signal1->setSignalData(sigdata1);

		decide1->getConnections()->insert(signal1);
		signal1->getConnections()->insert(dispose2);

		ModelSimulation* sim = model->getSimulation();
		sim->setReplicationLength(50, Util::TimeUnit::hour);
		sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::hour);

		model->save("./model/teachingBufferFIFO.gen");
		model->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);

		do {
			sim->step();
			std::cout << "Press ENTER to continue...";
			std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
		} while (sim->isPaused());
		delete genesys;
		return 0;
	}
};


