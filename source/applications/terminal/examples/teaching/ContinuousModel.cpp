#include "ContinuousModel.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DiffEquations.h"
#include "../../../../plugins/components/Dispose.h"


ContinuousModel::ContinuousModel() {

}

void ContinuousModel::Simulate(Simulator* genesys, Model* model, PluginManager *plugins) {
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setMaxCreations(1);
	DiffEquations *ode1 = plugins->newInstance<DiffEquations>(model);
	ode1->getEquations()->insert("x[1]^2 - 2*x[2]");
	ode1->getEquations()->insert("x[2]^2 - 2*x[3]");
	ode1->getEquations()->insert("x[3]^2 - 2*x[1]");
	ode1->setFinalTime(10.0);
	Dispose *dispose1 = plugins->newInstance<Dispose>(model);
	create1->getConnectionManager()->insert(ode1);
	ode1->getConnectionManager()->insert(dispose1);

	model->getSimulation()->start();
}
