/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   SPICERunner.h
 * Author: bnmfw & joaomai
 *
 * Created on 10 de outubro de 2023, 23:00
 */

#ifndef SPICERUNNER_H
#define SPICERUNNER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/PluginInformation.h"

const std::vector<std::pair<std::string, float>> spice_unit_map{{"T",1e12},{"G",1e9},{"Meg",1e6},{"K",1e3},{"",1e0},{"m",1e-3},{"u",1e-6},{"n",1e-9},{"p",1e-12},{"f",1e-15},{"a",1e-18}};
inline std::string uc(double value){
	float precision = 0.9999;
	for (auto element: spice_unit_map){
		if (value/element.second < precision) continue;
		return std::to_string(value/element.second) + element.first + " ";
	}
	return std::to_string(value/1e-18) + "a ";
};

class SPICERunner : public ModelDataDefinition {
public: /// constructors
	SPICERunner(Model* model, std::string name = "");
	virtual ~SPICERunner();

public: /// new public user methods for this component
	std::string CompileSpiceFile();
	void SendComponent(std::string *instance, std::string subcircuit = "", std::string model = "");
	void SendCallback(std::function<void()> callback);
	void ConfigSim(double duration, double step);
	void Run();
	void PlotV(std::string net);
	template<typename... Args> void PlotV(std::string net, Args... args);
	void PlotVRelative(std::string comparison_base, std::string net);
	template<typename... Args> void PlotVRelative(std::string comparison_base, std::string net, Args... args);
	void PlotI(std::string net);
	template<typename... Args> void PlotI(std::string net, Args... args);
	void PlotIRelative(std::string comparison_base, std::string net);
	template<typename... Args> void PlotIRelative(std::string comparison_base, std::string net, Args... args);
	double* MeasurePeak(std::string label, std::string peak, std::string quantity, std::string node, float start, float finish);
	double* MeasureTrigTarg(std::string label, std::string trig, float trig_value, std::string trig_inclin, std::string targ, float targ_value, std::string targ_inclin);
	void ParseOutput();
	void setRunnerCommand(std::string runnerCommand);
	std::string getRunnerCommand() const;
	void setModelsPath(std::string modelsPath);
	std::string getModelsPath() const;
	void setWorkingInputFilename(std::string workingInputFilename);
	std::string getWorkingInputFilename() const;
	void setWorkingOutputFilename(std::string workingOutputFilename);
	std::string getWorkingOutputFilename() const;
	void setWorkingDirectory(std::string workingDirectory);
	std::string getWorkingDirectory() const;
	std::string getLastRunCommand() const;

public: /// virtual public methods
	virtual std::string show() override;

public: /// static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	// Added
	static std::vector<std::string> split(std::string word, char split_char);

protected: /// virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

protected: /// virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	// virtual void _initBetweenReplications();
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addProperty(PropertyBase* property);

private: /// new private user methods
	// ...

private: /// Attributes that should be loaded or saved with this component (Persistent Fields)
	const struct DEFAULT_VALUES {
		const std::string runnerCommand = "ngspice";
		const std::string modelsPath = "./";
		const std::string workingInputFilename = "input.cir";
		const std::string workingOutputFilename = "output";
		const std::string workingDirectory = "";
	} DEFAULT;
	std::string _runnerCommand = DEFAULT.runnerCommand;
	std::string _modelsPath = DEFAULT.modelsPath;
	std::string _workingInputFilename = DEFAULT.workingInputFilename;
	std::string _workingOutputFilename = DEFAULT.workingOutputFilename;
	std::string _workingDirectory = DEFAULT.workingDirectory;
	std::string _lastRunCommand = "";

	std::set<std::string> subcircuits;
	std::set<std::string> models;

	std::vector<std::function<void()>> subscribers;
	std::vector<std::string*> instances;

	// Plot
	std::vector<std::string> vplots;
	std::vector<std::string> iplots;

	// Measure
	std::vector<std::string> measures;
	std::map<std::string, double*> promises;

    std::string config;

private: /// internal DataElements (Composition)
	// DummyElement* _internalDataDefinition = nullptr;

private: /// attached DataElements (Agrregation)
	// ...
};

#endif /* SPICERUNNER_H */
