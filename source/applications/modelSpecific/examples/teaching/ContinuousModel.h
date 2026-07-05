#pragma once

#include "../../../BaseGenesysTerminalApplication.h"

class ContinuousModel: public BaseGenesysTerminalApplication {
public:
	ContinuousModel();
public: // virtual
	virtual void Simulate(Simulator* genesys, Model* model, PluginManager *plugins) override;
};

