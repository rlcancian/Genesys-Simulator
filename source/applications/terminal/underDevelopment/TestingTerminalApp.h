#pragma once

#include "../../BaseGenesysTerminalApplication.h"


class TestingTerminalApp : public BaseGenesysTerminalApplication {
public:
	TestingTerminalApp();
public:
	//virtual int main(int argc, char** argv);
	virtual void Simulate(Simulator* genesys, Model* model, PluginManager *plugins) override;
};



