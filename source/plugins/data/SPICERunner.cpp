/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   SPICERunner.cpp
 * Author: bnmfw & joaomai
 *
 * Created on 10 de outubro de 2023, 23:00
 */

#include "SPICERunner.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &SPICERunner::GetPluginInformation;
}
#endif

std::vector<std::string> SPICERunner::split(std::string word, char split_char = ' ') {
    std::vector<std::string> tokenized_string;
    std::string token = "";
    for (char character: word){
        if (character == split_char){
            if (token.size()) tokenized_string.push_back(token);
            token = "";
            continue;
        }
        token += character;
    }
    if (token.size()) tokenized_string.push_back(token);
    return tokenized_string;
}

std::string strip(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos || end == std::string::npos) return "";

    return str.substr(start, end - start + 1);
}

std::pair<std::string, double> ParseFromLine(std::string line){
    std::vector<std::string> tokens = SPICERunner::split(line, '=');
    std::string label = strip(tokens[0]), value = strip(tokens[1].substr(0, tokens[1].size()-2));
    return {label, std::stof(value)};
}

std::pair<std::string, double> ParseTrigLine(std::string line){
    std::vector<std::string> tokens = SPICERunner::split(line, '=');
    std::string label = strip(tokens[0]), value = strip(tokens[1].substr(0, tokens[1].size()-2));
    return {label, std::stof(value)};
}

//
// public: /// constructors
//

SPICERunner::SPICERunner(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<SPICERunner>(), name) {
    SimulationControlGeneric<std::string>* propRunnerCommand = new SimulationControlGeneric<std::string>(
            std::bind(&SPICERunner::getRunnerCommand, this), std::bind(&SPICERunner::setRunnerCommand, this, std::placeholders::_1),
            Util::TypeOf<SPICERunner>(), getName(), "RunnerCommand", "");
    SimulationControlGeneric<std::string>* propModelsPath = new SimulationControlGeneric<std::string>(
            std::bind(&SPICERunner::getModelsPath, this), std::bind(&SPICERunner::setModelsPath, this, std::placeholders::_1),
            Util::TypeOf<SPICERunner>(), getName(), "ModelsPath", "");
    SimulationControlGeneric<std::string>* propWorkingInputFilename = new SimulationControlGeneric<std::string>(
            std::bind(&SPICERunner::getWorkingInputFilename, this), std::bind(&SPICERunner::setWorkingInputFilename, this, std::placeholders::_1),
            Util::TypeOf<SPICERunner>(), getName(), "WorkingInputFilename", "");
    SimulationControlGeneric<std::string>* propWorkingOutputFilename = new SimulationControlGeneric<std::string>(
            std::bind(&SPICERunner::getWorkingOutputFilename, this), std::bind(&SPICERunner::setWorkingOutputFilename, this, std::placeholders::_1),
            Util::TypeOf<SPICERunner>(), getName(), "WorkingOutputFilename", "");
    SimulationControlGeneric<std::string>* propWorkingDirectory = new SimulationControlGeneric<std::string>(
            std::bind(&SPICERunner::getWorkingDirectory, this), std::bind(&SPICERunner::setWorkingDirectory, this, std::placeholders::_1),
            Util::TypeOf<SPICERunner>(), getName(), "WorkingDirectory", "");

    _parentModel->getControls()->insert(propRunnerCommand);
    _parentModel->getControls()->insert(propModelsPath);
    _parentModel->getControls()->insert(propWorkingInputFilename);
    _parentModel->getControls()->insert(propWorkingOutputFilename);
    _parentModel->getControls()->insert(propWorkingDirectory);

    _addProperty(propRunnerCommand);
    _addProperty(propModelsPath);
    _addProperty(propWorkingInputFilename);
    _addProperty(propWorkingOutputFilename);
    _addProperty(propWorkingDirectory);
}

SPICERunner::~SPICERunner() {
    // Deletes all promises
    for (auto[promise, value]: promises) delete value;
}


//
// public: /// new public user methods for this component
//

std::string SPICERunner::CompileSpiceFile() {
    for (auto callback : subscribers) callback();

    std::string spicefile = "GenESyS generated circuit\n\n";//set spicebehavior=all\n\n";
    // Resolves all model dependencies
	for (std::string model: models) spicefile += ".include " + _modelsPath + model + ".cir\n";
    spicefile += "\n";
    // Compiles all subcircuit templates
    for (std::string subcircuit: subcircuits) spicefile += subcircuit + "\n";
    // Compiles all circuit instances
    for (std::string* instance: instances) spicefile += *instance + "\n";
    spicefile += config;
    spicefile += ".end";
    return spicefile;
}

void SPICERunner::SendCallback(std::function<void ()> callback) {
	subscribers.push_back(callback);
}

void SPICERunner::SendComponent(std::string *instance, std::string subcircuit, std::string model) {
    // Recieves component data
    instances.push_back(instance);
    if (subcircuit.size()) subcircuits.insert(subcircuit);
    if (model.size()) models.insert(model);
}

void SPICERunner::PlotV(std::string net) {
	vplots.push_back("v("+net+") ");
}

template<typename... Args>
void SPICERunner::PlotV(std::string net, Args... args) {
	vplots.push_back("v("+net+") ");
    PlotV(args...);
}

void SPICERunner::PlotVRelative(std::string comparison_base, std::string net) {
	vplots.push_back("v("+ net + ")-v(" + comparison_base + ") ");
}

template<typename... Args>
void SPICERunner::PlotVRelative(std::string comparison_base, std::string net, Args... args) {
	vplots.push_back("v("+ net + ")-v(" + comparison_base + ") ");
    PlotVRelative(comparison_base, args...);
}

void SPICERunner::PlotI(std::string net) {
	iplots.push_back("i("+net+") ");
}

template<typename... Args>
void SPICERunner::PlotI(std::string net, Args... args) {
	iplots.push_back("i("+net+") ");
    PlotI(args...);
}

void SPICERunner::PlotIRelative(std::string comparison_base, std::string net) {
	iplots.push_back("i("+ net + ")-i(" + comparison_base + ") ");
}

template<typename... Args>
void SPICERunner::PlotIRelative(std::string comparison_base, std::string net, Args... args) {
	iplots.push_back("i("+ net + ")-i(" + comparison_base + ") ");
    PlotIRelative(comparison_base, args...);
}

double* SPICERunner::MeasurePeak(std::string label, std::string peak, std::string quantity, std::string node, float start, float finish) {
    double* promise = new double;
    promises[label] = promise;
    measures.push_back("meas tran "+label+" "+peak+" "+quantity+"("+node+") from="+uc(start)+" to="+uc(finish)+"\n");
    return promise;
}

double* SPICERunner::MeasureTrigTarg(std::string label, std::string trig, float trig_value, std::string trig_inclin, std::string targ, float targ_value, std::string targ_inclin) {
    double* promise = new double;
    promises[label] = promise;
    measures.push_back("meas tran "+label+" TRIG v("+trig+") val='"+uc(trig_value)+"' "+trig_inclin+"=1 TARG v("+targ+") val='"+uc(targ_value)+"' "+targ_inclin+"=1\n");
    return promise;
}

void SPICERunner::ParseOutput(){
    std::string outputFilepath = _workingDirectory.empty() ? _workingOutputFilename : (_workingDirectory + Util::DirSeparator() + _workingOutputFilename);
    std::ifstream data(outputFilepath);
    if (!data.is_open()) return;
    std::string line;
    while (getline(data, line)){
        // Determines if the any promise is in the line
        bool one_key = false;
        for (auto [label, promise]: promises){
            size_t found = line.find(label);
            if (found != std::string::npos) {
                one_key = true;
                break;
            }
        }
        size_t found = line.find("trig=");
        if (found == std::string::npos) {
            auto[label, value] = ParseTrigLine(line);
            *promises[label] = value;    
        } else {
            auto[label, value] = ParseFromLine(line);
            *promises[label] = value;
        }
    }
}

void SPICERunner::ConfigSim(double duration, double step) {
	config = "\n.control\n";
    // Simulation Duration
    config += "tran " + uc(step) + " " + uc(duration) + "\n";

    // Measures
    for (std::string measure: measures) config += measure;
    
    // Plots
	std::vector<std::vector<std::string>> plots = {iplots, vplots};
	for (int i = 0; i < plots.size(); ++i){
		std::vector<std::string> plot_type = plots[i];
        std::string plot_string = "hardcopy plot"+std::to_string(i)+".ps ";
		for (std::string plot: plot_type) plot_string += plot;
        plot_string += "\n";
        if (plot_type.size()) config += plot_string;
    }
    config+= "\n.endc\n";
}

void SPICERunner::Run() {
    const std::string compiledInput = CompileSpiceFile();
    std::string inputFilepath = _workingDirectory.empty() ? _workingInputFilename : (_workingDirectory + Util::DirSeparator() + _workingInputFilename);
    std::string outputFilepath = _workingDirectory.empty() ? _workingOutputFilename : (_workingDirectory + Util::DirSeparator() + _workingOutputFilename);
    std::string s;
    std::ofstream out(inputFilepath);
    out << compiledInput;
    out.close();

	_lastRunCommand = _runnerCommand + " -b -o \"" + outputFilepath + "\" \"" + inputFilepath + "\"";
	int code = std::system(_lastRunCommand.c_str());

    if (code == 0) {
        std::ifstream in(outputFilepath);
        while (std::getline(in, s)) {
            std::cout << s << '\n';
        }
    }

    ParseOutput();
}


// ...


//
// public: /// virtual methods
//

std::string SPICERunner::show() {
	return ModelDataDefinition::show() +
            ",runnerCommand=\"" + _runnerCommand + "\"" +
            ",modelsPath=\"" + _modelsPath + "\"" +
            ",workingInputFilename=\"" + _workingInputFilename + "\"" +
            ",workingOutputFilename=\"" + _workingOutputFilename + "\"" +
            ",workingDirectory=\"" + _workingDirectory + "\"" +
            ",subscribers=" + std::to_string(subscribers.size()) +
            ",instances=" + std::to_string(instances.size()) +
            ",plots=" + std::to_string(vplots.size() + iplots.size()) +
            ",measures=" + std::to_string(measures.size());
}



//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//


PluginInformation* SPICERunner::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<SPICERunner>(), &SPICERunner::LoadInstance, &SPICERunner::NewInstance);
	info->setCategory("Electronics simulation");
	info->setDescriptionHelp("Executes SPICE circuit simulations through ngspice. The runner generates an input circuit file, runs ngspice in batch mode and parses the output file for measured values.");
	info->insertSystemDependency(SystemDependency(
			SystemDependency::OS::Linux,
			"ngspice",
			"sudo apt install -y ngspice",
			"ngspice -v"));
	return info;
}

ModelDataDefinition* SPICERunner::LoadInstance(Model* model, PersistenceRecord *fields) {
	SPICERunner* newElement = new SPICERunner(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

ModelDataDefinition* SPICERunner::NewInstance(Model* model, std::string name) {
	return new SPICERunner(model, name);
}


//
// protected: /// virtual method that must be overriden
//

bool SPICERunner::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_runnerCommand = fields->loadField("runnerCommand", DEFAULT.runnerCommand);
		_modelsPath = fields->loadField("modelsPath", DEFAULT.modelsPath);
		_workingInputFilename = fields->loadField("workingInputFilename", DEFAULT.workingInputFilename);
		_workingOutputFilename = fields->loadField("workingOutputFilename", DEFAULT.workingOutputFilename);
		_workingDirectory = fields->loadField("workingDirectory", DEFAULT.workingDirectory);
	}
	return res;
}

void SPICERunner::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("runnerCommand", _runnerCommand, DEFAULT.runnerCommand, saveDefaultValues);
	fields->saveField("modelsPath", _modelsPath, DEFAULT.modelsPath, saveDefaultValues);
	fields->saveField("workingInputFilename", _workingInputFilename, DEFAULT.workingInputFilename, saveDefaultValues);
	fields->saveField("workingOutputFilename", _workingOutputFilename, DEFAULT.workingOutputFilename, saveDefaultValues);
	fields->saveField("workingDirectory", _workingDirectory, DEFAULT.workingDirectory, saveDefaultValues);
}

bool SPICERunner::_check(std::string& errorMessage) {
    bool resultAll = true;
    if (_runnerCommand.empty()) {
        errorMessage += "RunnerCommand must not be empty. ";
        resultAll = false;
    }
    if (_workingInputFilename.empty()) {
        errorMessage += "WorkingInputFilename must not be empty. ";
        resultAll = false;
    }
    if (_workingOutputFilename.empty()) {
        errorMessage += "WorkingOutputFilename must not be empty. ";
        resultAll = false;
    }
    if (!models.empty() && _modelsPath.empty()) {
        errorMessage += "ModelsPath must not be empty when model includes are used. ";
        resultAll = false;
    }
    return resultAll;
}

void SPICERunner::setRunnerCommand(std::string runnerCommand) {
    _runnerCommand = runnerCommand;
}

std::string SPICERunner::getRunnerCommand() const {
    return _runnerCommand;
}

void SPICERunner::setModelsPath(std::string modelsPath) {
    _modelsPath = modelsPath;
}

std::string SPICERunner::getModelsPath() const {
    return _modelsPath;
}

void SPICERunner::setWorkingInputFilename(std::string workingInputFilename) {
    _workingInputFilename = workingInputFilename;
}

std::string SPICERunner::getWorkingInputFilename() const {
    return _workingInputFilename;
}

void SPICERunner::setWorkingOutputFilename(std::string workingOutputFilename) {
    _workingOutputFilename = workingOutputFilename;
}

std::string SPICERunner::getWorkingOutputFilename() const {
    return _workingOutputFilename;
}

void SPICERunner::setWorkingDirectory(std::string workingDirectory) {
    _workingDirectory = workingDirectory;
}

std::string SPICERunner::getWorkingDirectory() const {
    return _workingDirectory;
}

std::string SPICERunner::getLastRunCommand() const {
    return _lastRunCommand;
}
