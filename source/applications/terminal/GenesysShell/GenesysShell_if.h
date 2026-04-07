/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GenesysShell_if.h
 * Author: rlcancian
 *
 * Created on 24 de Maio de 2019, 11:02
 */

#ifndef GENESYSSHELL_IF_H
#define GENESYSSHELL_IF_H

#include "GenesysApplication_if.h"
#include "Util.h"

class GenesysShell_if : public GenesysApplication_if {
public:
	virtual ~GenesysShell_if() = default;
	virtual void openModel(std::string filename) = 0;
	virtual void saveModelAs(std::string filename) = 0;
	virtual void saveModel() = 0;
	virtual void listElements() = 0;
	virtual void listComponents() = 0;
	virtual void listHosts() = 0;
	virtual void listPlugins() = 0;
	virtual void deleteTraceFiles() = 0;
	virtual void traceLevel(TraceManager::TraceLevel tracelevel) = 0;
	virtual void addPlugin(std::string filename) = 0;
	virtual void addFromFile(std::string filename) = 0;
	virtual void readCommandsFromFile(std::string filename) = 0;
	virtual void redirectTrace(std::string trace, std::string dest, std::string filename) = 0;
	virtual void closeModel() = 0;
	virtual void createModel() = 0;
	virtual void execLinuxCommand(std::string command) = 0;
	virtual void verboseMode(bool on) = 0;
	virtual void check() = 0;
	virtual void getGenesysInfo() = 0;
	virtual void getCommandLine() = 0;
	virtual void sendFile(std::string filename, std::string hostname, std::string portname) = 0;
	virtual void setActivationCode(std::string code) = 0;
	virtual void receiveFile(std::string filename) = 0;
	virtual void startSimulation() = 0;
	virtual void stepSimulation() = 0;
	virtual void stopSimulation() = 0;
	virtual void showInit() = 0;
	virtual void showHelp() = 0;
	virtual void showHostName() = 0;
	//virtual void execute() = 0;
	//property TraceHandler : TProcTrace write aTraceHandler;
};

#endif /* GENESYSSHELL_IF_H */
