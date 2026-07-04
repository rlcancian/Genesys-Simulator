/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GenesysShell.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 23 de Maio de 2019, 13:02
 */

#include "GenesysShell.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/util/Util.h"
#include <regex>
#include <assert.h>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>

#include <fstream>
#include <istream>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace std;
//#include "ProbDistribDefaultImpl1.h"

namespace {

/*! \brief Returns true when stdin is attached to an interactive terminal. */
bool IsInteractiveTerminalInput() {
#ifdef _WIN32
	return _isatty(_fileno(stdin)) != 0;
#else
	return isatty(STDIN_FILENO) != 0;
#endif
}

/*! \brief Prints dependency diagnostics and asks whether install commands may run. */
bool ConfirmSystemDependencyInstallationFromShell(const SystemDependencyCheckResult& result) {
	cout << "Plugin system dependencies are not satisfied:" << endl;
	cout << result.diagnosticText(false) << endl;

	if (!result.canAttemptInstallForAllMissing()) {
		cout << "At least one dependency cannot be installed automatically. "
		        "Run the declared install command manually, when available, and try again." << endl;
		return false;
	}
	if (!IsInteractiveTerminalInput()) {
		cout << "Non-interactive input detected. Install commands will not be executed automatically." << endl;
		return false;
	}

	cout << "Run the install command(s) now? [y/N] ";
	string answer;
	getline(cin, answer);
	transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
	return answer == "y" || answer == "yes" || answer == "s" || answer == "sim";
}

/*! \brief Builds insertion options used by interactive shell plugin loading commands. */
PluginInsertionOptions ShellPluginInsertionOptions() {
	PluginInsertionOptions options;
	options.confirmSystemDependencyInstallation = [](const SystemDependencyCheckResult& result) {
		return ConfirmSystemDependencyInstallationFromShell(result);
	};
	return options;
}

}

GenesysShell::GenesysShell()
    : simulator(std::make_unique<Simulator>())
    , facade(simulator.get()) {
	setDefaultTraceHandlers(simulator->getTraceManager());
}

void GenesysShell::Trace(string message) {
	cout<<message<<endl;
}

void GenesysShell::run(List<string>* commandlineArgs) {
	/*
	int i;
	const char progress[] = "|/-\\";

	for (i = 0; i<100; i += 1) {
		printf("$genesys> %3d%%\r", i);
		fflush(stdout);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	printf("\n");
	fflush(stdout);
	printf("$genesys> ");
	for (i = 0; i<100; i += 1) {
		printf("%c\b", progress[i%(sizeof(progress)-1)]); 
		fflush(stdout);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	printf("\n"); 
	fflush(stdout);
	return;
	 */

	facade.setTraceLevel(TraceManager::Level::L1_errorFatal);
	if (!facade.autoInsertPlugins("autoloadplugins.txt", true, ShellPluginInsertionOptions()))
		cout << "Error: Could not auto load plugins from file \"autoloadplugins.txt\"." << endl;
	facade.setTraceLevel(TraceManager::Level::L7_internal);
	_history->resize(100);
	Trace("Genesys Shell is running. Type your command. For help, type the command \"help\".");
	string inputText, historyText; //, shortPrefix, longPrefix, separator; //,longPrefix, separator;
	char c, c1, c2, c3;
	char buf[1];
	unsigned short historyPosition = 0;
	while (!_exitRequested) {
		if (!commandlineArgs->empty()) {
			inputText = commandlineArgs->front();
			commandlineArgs->pop_front();
			_history->push_back(inputText);
			tryExecuteCommand(inputText);
		} else {
			cout.setf(std::ios_base::unitbuf);
			//std::basic_istream::
			cout<<_prompt<<" ";
			inputText = "";
			//std::ios_base::flags();
			//std::cin::setf();
			while (cin.get(c)) {
				if (c=='\n')
					break;
				//cout.put('x');// << c << endl;
				//cout.flush();
				//cout << int(c)<<','<<flush;//<<int(c1)<<','<<int(c2)<<endl;				
				if ((c==65||c==66)&&c1==91&&c2==27) { // Up or down pressed. Show history
					//cout << "ISSO!!!" << endl;
					//printf("isso!!\n");
					//printf("\b\b\b%c%c%c\b\b\b", ' ', ' ', ' ');
					fflush(stdout);
					if (_history->size()>0) {
						historyText = _history->at(historyPosition);
						if (c==65) {
							historyPosition++;
						} else {
							historyPosition--;
						}
						if (historyPosition>_history->size())
							historyPosition = 0;
						inputText = historyText;
						cout<<inputText;
					}
				} else { // normal key. Add to the text being typed
					inputText.push_back(c);
				}
				//c3 = c2;
				c2 = c1;
				c1 = c;
			}
			if (inputText!="") {
				_history->push_back(inputText);
				tryExecuteCommand(inputText);
			}
		}
	}
}

std::vector<std::string> GenesysShell::split(std::string text, std::string separatorRegex) {
	if (text.empty()) {
		return {};
	}
	string expression = "(["+separatorRegex+"]+)";
	regex regex{expression};
	sregex_token_iterator it{text.begin(), text.end(), regex, -1};
	vector<string> fields{it,{}};
	fields.erase(remove_if(fields.begin(), fields.end(), [](const string& field) {
		return field.empty();
	}), fields.end());
	return fields;
}

void GenesysShell::tryExecuteCommand(string inputText) {
	vector<string> fields = split(inputText, "\\s");
	if (fields.empty()) {
		return;
	}
	string typedCommandStr = fields[0];
	bool found = false;
	_typedWords->clear();
	for (string w : fields) {
		_typedWords->insert(_typedWords->end(), w);
	}
	transform(typedCommandStr.begin(), typedCommandStr.end(), typedCommandStr.begin(), ::tolower);
	if (typedCommandStr.empty() || typedCommandStr.substr(0, 1)=="#") {
		return;
	}
	for (ShellCommand *cmd : *_commands->list()) {
		if (cmd->longname==typedCommandStr || cmd->shortname==typedCommandStr) {
			found = true;
			cmd->executer();
		}
	}
	if (!found) {
		Trace("Command \""+typedCommandStr+"\" not found. Type \"help\" for help.");
	}
}

int GenesysShell::main(int argc, char** argv) {
	List<string>* commandlineArgs = new List<string>();
	for (unsigned short i = 1; i<argc; i++) {
		string arg = argv[i];
		commandlineArgs->insert(arg);
	}
	defineCommands();
	this->run(commandlineArgs);
	delete commandlineArgs;
	return 0;
}

void GenesysShell::defineCommands() {
	_commands->insert(new ShellCommand("", "help", "", "Show the list of commands", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdHelp)));
	_commands->insert(new ShellCommand("", "exit", "", "Quit the simulator", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdQuit)));
	_commands->insert(new ShellCommand("", "facade", "[get-version|get-version-number|get-name]", "Query high-level simulator metadata through SimulatorFacade", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdFacade)));
	_commands->insert(new ShellCommand("", "licence", "[show|limits|activation-code|lookfor-activation-code|insert-activation-code|remove-activation-code|model-components-limit|model-datas-limit|entity-limit|hosts-limit|threads-limit]", "Inspect licence information through SimulatorFacade", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdLicence)));
	_commands->insert(new ShellCommand("", "info", "[show|get-name|set-name|get-analyst-name|set-analyst-name|get-description|set-description|get-project-title|set-project-title|get-version|set-version|has-changed|set-has-changed]", "Inspect or update current model metadata", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdInfo)));
	_commands->insert(new ShellCommand("", "sim", "[show|start|pause|step|stop|get-replications|set-replications|get-length|set-length|get-base-length|get-warm-up|get-warm-up-unit|get-length-unit|get-terminating-condition|set-terminating-condition|get-pause-on-event|set-pause-on-event|get-step-by-step|set-step-by-step|get-init-statistics|set-init-statistics|get-init-system|set-init-system|get-pause-on-replication|set-pause-on-replication|get-current-time|get-current-replication|get-running|get-paused|show-reports-after-replication|set-show-reports-after-replication|show-reports-after-simulation|set-show-reports-after-simulation]", "Inspect or control the current model simulation through SimulatorFacade", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSim)));
	_commands->insert(new ShellCommand("", "data", "[count|show|class-names|list|clear|has-changed|set-has-changed]", "Inspect current model data definitions", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdData)));
	_commands->insert(new ShellCommand("", "component", "[count|show|list|clear|find|has-changed|set-has-changed]", "Inspect current model components", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdComponent)));
	_commands->insert(new ShellCommand("", "experiment", "[count|show|current|new|save|load|next|first]", "Inspect simulation experiments", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdExperiment)));
	_commands->insert(new ShellCommand("", "parse", "<expression>", "Evaluate or watch an expression", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdParser)));
	//_commands->insert(new ShellCommand("", "dir", "<path>", "List the files in the <path>", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdListFiles)));
	_commands->insert(new ShellCommand("", "bash", "<bash command>", "Execute a bash command", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdBash)));
	_commands->insert(new ShellCommand("", "script", "[-r|--run|-s|--show]=<script filename>", "Execute or show commands in a script file", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdScript)));
	_commands->insert(new ShellCommand("", "trace", "[-l=<level 1-9>] [-s|--show]", "Set or show current the trace level", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdTraceLevel)));
	_commands->insert(new ShellCommand("", "plugin", "[-l|--list] [-c|--count] [[-i|--info]=<plugin typename>] [[-a|--autoload]=<filename plugin list>] [-t|--template[=<plugintypename>]", "List, count, load or get information about installed plugins and templates of genesys language", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPlugin)));
	//_commands->insert(new ShellCommand("", "plugininfo", "<plugin name>", "Show infos about a plugin", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginInfo)));
	//_commands->insert(new ShellCommand("", "pluginadd", "<plugin filename>", "", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginAdd)));
	//_commands->insert(new ShellCommand("", "pluginremove", "<classname>", "", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginRemove)));
	_commands->insert(new ShellCommand("", "simul", "[-s|--start|-p|--step]", "Control simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulation)));
	//_commands->insert(new ShellCommand("", "step", "", "Step simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationStep)));
	//_commands->insert(new ShellCommand("", "stop", "", "Stop simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationStop)));
	_commands->insert(new ShellCommand("", "config", "[-r|--replications=<number of repliations>] [-l|--length=<replication length>] [-t|--time=<replication time unit>] [-s|--show]", "Configure simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdReplication)));
	//_commands->insert(new ShellCommand("", "showsetup", "", "Show simulation info", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationInfo)));
	//_commands->insert(new ShellCommand("", "showreport", "", "Show simulation report", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdShowReport)));
	//_commands->insert(new ShellCommand("", "show", "", "Show the model structure", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelShow)));
	_commands->insert(new ShellCommand("", "model", "[-n|--new|-r|--remove|-c|--check|-s|--show]", "Create, check, show or close a model", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModel)));
	//_commands->insert(new ShellCommand("", "close", "", "Close the odel", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelClose)));
	//_commands->insert(new ShellCommand("", "check", "", "Check the model", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelCheck)));
	_commands->insert(new ShellCommand("", "load", "<filename>", "Load a model from a file", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelLoad)));
	_commands->insert(new ShellCommand("", "save", "<filename>", "Save a model to a file", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelSave)));
	//_commands->insert(new ShellCommand("", "setInfos", "", "Set information of the model", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelSetInfos)));
}

std::string GenesysShell::joinArguments(std::size_t firstArgumentIndex) const {
	std::string result;
	for (std::size_t i = firstArgumentIndex; i < _typedWords->size(); ++i) {
		if (!result.empty()) {
			result.push_back(' ');
		}
		result += _typedWords->at(i);
	}
	return result;
}

std::string GenesysShell::lowercase(std::string text) const {
	transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return text;
}

bool GenesysShell::requireCurrentModel(const std::string& actionDescription) {
	if (facade.currentModel() != nullptr) {
		return true;
	}
	cout << "Error: There is no loaded model to " << actionDescription << "." << endl;
	return false;
}

bool GenesysShell::parseUnsigned(const std::string& text, unsigned int& value) const {
	try {
		std::size_t pos = 0;
		const unsigned long parsed = std::stoul(text, &pos);
		if (pos != text.size()) {
			return false;
		}
		value = static_cast<unsigned int>(parsed);
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

bool GenesysShell::parseInt(const std::string& text, int& value) const {
	try {
		std::size_t pos = 0;
		const int parsed = std::stoi(text, &pos);
		if (pos != text.size()) {
			return false;
		}
		value = parsed;
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

bool GenesysShell::parseDouble(const std::string& text, double& value) const {
	try {
		std::size_t pos = 0;
		const double parsed = std::stod(text, &pos);
		if (pos != text.size()) {
			return false;
		}
		value = parsed;
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

bool GenesysShell::parseBool(const std::string& text, bool& value) const {
	const std::string lower = lowercase(text);
	if (lower == "1" || lower == "true" || lower == "yes" || lower == "on" || lower == "sim") {
		value = true;
		return true;
	}
	if (lower == "0" || lower == "false" || lower == "no" || lower == "off" || lower == "nao") {
		value = false;
		return true;
	}
	return false;
}

bool GenesysShell::parseTraceLevel(const std::string& text, TraceManager::Level& level) const {
	int levelInt = 0;
	if (!parseInt(text, levelInt)) {
		return false;
	}
	if (levelInt < 0 || levelInt >= static_cast<int>(TraceManager::Level::num_elements)) {
		return false;
	}
	level = static_cast<TraceManager::Level>(levelInt);
	return true;
}

bool GenesysShell::parseTimeUnit(const std::string& text, Util::TimeUnit& timeUnit) const {
	const std::string lower = lowercase(text);
	if (lower == "unknown") { timeUnit = Util::TimeUnit::unknown; return true; }
	if (lower == "picosecond" || lower == "ps") { timeUnit = Util::TimeUnit::picosecond; return true; }
	if (lower == "nanosecond" || lower == "ns") { timeUnit = Util::TimeUnit::nanosecond; return true; }
	if (lower == "microsecond" || lower == "us") { timeUnit = Util::TimeUnit::microsecond; return true; }
	if (lower == "milisecond" || lower == "millisecond" || lower == "ms") { timeUnit = Util::TimeUnit::milisecond; return true; }
	if (lower == "second" || lower == "s") { timeUnit = Util::TimeUnit::second; return true; }
	if (lower == "minute" || lower == "min") { timeUnit = Util::TimeUnit::minute; return true; }
	if (lower == "hour" || lower == "h") { timeUnit = Util::TimeUnit::hour; return true; }
	if (lower == "day" || lower == "d") { timeUnit = Util::TimeUnit::day; return true; }
	if (lower == "week" || lower == "w") { timeUnit = Util::TimeUnit::week; return true; }
	return false;
}

void GenesysShell::cmdHelp() {
	cout<<"List of commands:"<<endl;
	string parameters;
	for (ShellCommand *command : *_commands->list()) {
		parameters = command->parameters;
		if (parameters=="")
			parameters = "\t\t\t";
		cout<<command->longname<<"\t"<<parameters<<"\t\t; "<<command->descrition<<endl;
	}
}

void GenesysShell::cmdQuit() {
	cout<<"Quiting. Bye."<<endl;
	_exitRequested = true;
}

void GenesysShell::cmdParser() {
	if (!requireCurrentModel("evaluate an expression")) {
		return;
	}
	if (_typedWords->size()<2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	const std::string expression = joinArguments(1);
	bool success = false;
	string errorMessage = "";
	cout << "Evaluating expression \"" << expression << "\"" << endl;
	double res = facade.modelParseExpression(expression, success, errorMessage);
	if (success) {
		cout << "Expression evaluates to "<<res<<endl;
	} else {
		cout << "Syntax error: "<<errorMessage<<endl;
	}
}

void GenesysShell::cmdBash() {
	if (_typedWords->size()<2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	std::string parameters = "";
	for (unsigned short i = 1; i<_typedWords->size(); i++) {
		parameters += _typedWords->at(i)+" ";
	}
	const std::string command = parameters;
	system(command.c_str());
}

void GenesysShell::cmdScript() {
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	string parameter = _typedWords->at(1);
	string key = "";
	string value = "";
	Util::SepKeyVal(parameter, key, value);
	bool run = key=="-r"||key=="-run";
	bool show = key=="-s"||key=="--show";
	if (!run && !show) {
		cout<<"Syntax error on "<<parameter<<endl;
		return;
	}
	if (show)
		cout<<"Showing script "<<value<<endl;
	if (run)
		cout<<"Running script "<<value<<endl;
	ifstream file(value);
	if (file.is_open()) {
		string line;
		while (getline(file, line)) {
			if (show) {
				cout<<line<<endl;
			}
			if (run) {
				const std::string trimmed = Util::Trim(line);
				if (!trimmed.empty()) {
					if (trimmed[0]!='#') { // not a comment
						cout<<"$script> "<<line<<endl;
						tryExecuteCommand(line);
					}
				}
			}
		}
		file.close();
	} else {
		cout<<"Error: Could not load the script"<<endl;
	}
}

void GenesysShell::cmdTraceLevel() {
	if (_typedWords->size()<2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	string parameter;
	string key, value;
	for (unsigned short i = 1; i<_typedWords->size(); i++) {
		parameter = _typedWords->at(i);
		key = "";
		value = "";
		Util::SepKeyVal(parameter, key, value);
		if (key=="-l"||key=="--level") {
			TraceManager::Level level;
			if (parseTraceLevel(value, level)) {
				cout<<"Setting tracelevel to "<<value<<endl;
				facade.setTraceLevel(level);
			} else {
				cout<<"Error: "<<value<<" is not a valid level"<<endl;
			}
		} else if (key=="-s"||key=="--show") {
			cout<<"Trace level is "<<static_cast<int> (facade.getTraceLevel())<<endl;
		} else {
			cout<<"Syntax error on "<<parameter<<endl;
		}
	}
}

void GenesysShell::cmdPlugin() {
	if (_typedWords->size()<2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	string parameter;
	string key, value;
	for (unsigned short i = 1; i<_typedWords->size(); i++) {
		parameter = _typedWords->at(i);
		key = "";
		value = "";
		Util::SepKeyVal(parameter, key, value);
		if (key=="-l"||key=="--list") {
			if (facade.pluginCount()==0) {
				cout<<"There is no installed plugins. Install some using the plugin --autoload=<filename>"<<endl;
				return;
			}
			cout<<facade.showPlugins()<<endl;
		} else if (key=="-c"||key=="--count") {
			cout<<"Installed plugins: "<<facade.pluginCount()<<endl;
		} else if (key=="-a"||key=="--autoload") {
			cout<<"Loading list of plugins from file "<<value<<endl;
			facade.autoInsertPlugins(value, true, ShellPluginInsertionOptions());
		} else if (key=="-i"||key=="--info") {
			if (facade.pluginCount()==0) {
				cout<<"There is no installed plugins. Install some using the plugin --autoload=<filename>"<<endl;
				return;
			}
			Plugin* plugin = facade.findPlugin(value);
			if (plugin != nullptr) {
				cout<<"Information for plugin "<<value<<":"<<endl;
				cout<<plugin->show()<<endl;
			} else {
				cout<<"Error: Could not find plugin of type "<<value<<endl;
			}
		} else if (key=="-t"||key=="--template") {
			if (facade.pluginCount()==0) {
				cout<<"There is no installed plugins. Install some using the plugin --autoload=<filename>"<<endl;
				return;
			}
			cout<<"Plugin templates:"<<endl;
			List<Plugin*>* plugins = facade.completePluginsFieldsAndTemplates();
			if (plugins != nullptr) {
				for (Plugin* plugin : *plugins->list()) {
					if (plugin == nullptr) {
						continue;
					}
					if (value.empty() || plugin->getPluginInfo()->getPluginTypename() == value) {
						cout<<"Language syntax for plugin \""<<plugin->getPluginInfo()->getPluginTypename()<<"\":"<<endl;
						cout<<"Template: "<<plugin->getPluginInfo()->getLanguageTemplate()<<endl;
						if (!value.empty()) {
							return;
						}
						cout<<endl;
					}
				}
			}
			if (value!="")
				cout<<"Error: Could not find plugin of type "<<value<<endl;
		} else {
			cout<<"Syntax error on "<<parameter<<endl;
		}
	}
}

void GenesysShell::cmdSimulation() {
	if (!requireCurrentModel("control the simulation")) {
		return;
	}
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	string parameter = _typedWords->at(1);
	if (parameter=="-s"||parameter=="--start")
		facade.simStart();
	else if (parameter=="-p"||parameter=="--step")
		facade.simStep();
	else
		cout<<"Syntax error on "+parameter<<endl;
}

void GenesysShell::cmdReplication() {
	if (!requireCurrentModel("setup the simulation")) {
		return;
	}
	if (_typedWords->size()<2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	string parameter;
	string key, value;
	for (unsigned short i = 1; i<_typedWords->size(); i++) {
		parameter = _typedWords->at(i);
		//vector<string> keyvalue = split(parameter, "=");
		key = "";
		value = "";
		//cout<<parameter<<endl;
		Util::SepKeyVal(parameter, key, value);
		if (key=="-r"||key=="--replications") {
			unsigned int replications = 0;
			if (!parseUnsigned(value, replications)) {
				cout<<"Error: "<<value<<" is not a valid number of replications"<<endl;
				continue;
			}
			cout<<"Setting number of replications to "<<replications<<endl;
			facade.simSetNumberOfReplications(replications);
		} else if (key=="-l"||key=="--length") {
			double length = 0.0;
			if (!parseDouble(value, length)) {
				cout<<"Error: "<<value<<" is not a valid replication length"<<endl;
				continue;
			}
			cout<<"Setting replication length to "<<length<<endl;
			facade.simSetReplicationLength(length);
		} else if (key=="-t"||key=="--time") {
			Util::TimeUnit timeUnit;
			if (!parseTimeUnit(value, timeUnit)) {
				cout<<"Error: "<<value<<" is not a valid time unit"<<endl;
				continue;
			}
			cout<<"Setting replication time unit to "<<value<<endl;
			facade.simSetReplicationLengthTimeUnit(timeUnit);
		} else if (key=="-s"||key=="--show") {
			cout<<facade.simShow()<<endl;
		} else {
			cout<<"Syntax error on "<<parameter<<endl;
		}
	}
}

void GenesysShell::cmdModel() {
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}

	string parameter;
	string key, value;
	for (unsigned short i = 1; i<_typedWords->size(); i++) {
		parameter = _typedWords->at(i);
		if (parameter=="-n"||parameter=="--new") {
			Model* currentModel = facade.currentModel();
			if (currentModel != nullptr) {
				facade.removeModel(currentModel);
				model = nullptr;
			}
			cout<<"Creating a new model"<<endl;
			model = facade.newModel();
		} else if (parameter=="-r"||parameter=="--remove") {
			if (!requireCurrentModel("close the model")) {
				return;
			}
			cout<<"Closing the model"<<endl;
			facade.removeModel(model);
			model = nullptr;
		} else if (parameter=="-c"||parameter=="--check") {
			if (!requireCurrentModel("check the model")) {
				return;
			}
			if (!facade.modelCheck()) {
				cout<<"Model check returned errors."<<endl;
			}
		} else if (parameter=="-s"||parameter=="--show") {
			if (!requireCurrentModel("show the model")) {
				return;
			}
			cout<<facade.modelShowLanguage()<<endl;
		} else {
			cout<<"Syntax error on "+parameter<<endl;
		}
	}

}

void GenesysShell::cmdModelLoad() {
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	std::string parameter = _typedWords->at(1);
	cout<<"Loading model "<<parameter<<endl;
	model = facade.loadModel(parameter);
	if (model==nullptr) {
		cout<<"Error: Could not load the model"<<endl;
	} else {
		cout<<"Model loaded"<<endl;
	}
}

void GenesysShell::cmdModelSave() {
	if (!requireCurrentModel("save the model")) {
		return;
	}
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	std::string parameter = _typedWords->at(1);
	cout<<"Saving model "<<parameter<<endl;
	if (facade.saveModel(parameter)) {
		cout<<"Model saved"<<endl;
	} else {
		cout<<"Error: Could not save the model"<<endl;
	}
}

void GenesysShell::cmdFacade() {
	if (_typedWords->size() == 1) {
		cout << "Facade summary:" << endl;
		cout << "  name: " << facade.getName() << endl;
		cout << "  version: " << facade.getVersion() << endl;
		cout << "  version number: " << facade.getVersionNumber() << endl;
		return;
	}
	const std::string command = lowercase(_typedWords->at(1));
	if (command == "get-version") {
		cout << facade.getVersion() << endl;
	} else if (command == "get-version-number") {
		cout << facade.getVersionNumber() << endl;
	} else if (command == "get-name") {
		cout << facade.getName() << endl;
	} else {
		cout << "Syntax error on " << _typedWords->at(1) << endl;
	}
}

void GenesysShell::cmdLicence() {
	if (_typedWords->size() == 1) {
		cout << facade.licenceShow() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = _typedWords->at(i);
		if (parameter == "show") {
			cout << facade.licenceShow() << endl;
		} else if (parameter == "limits") {
			cout << facade.licenceShowLimits() << endl;
		} else if (parameter == "activation-code") {
			cout << facade.licenceShowActivationCode() << endl;
		} else if (parameter == "lookfor-activation-code") {
			cout << (facade.licenceLookforActivationCode() ? "true" : "false") << endl;
		} else if (parameter == "insert-activation-code") {
			cout << (facade.licenceInsertActivationCode() ? "true" : "false") << endl;
		} else if (parameter == "remove-activation-code") {
			facade.licenceRemoveActivationCode();
			cout << "Activation code removed." << endl;
		} else if (parameter == "model-components-limit") {
			cout << facade.licenceGetModelComponentsLimit() << endl;
		} else if (parameter == "model-datas-limit") {
			cout << facade.licenceGetModelDatasLimit() << endl;
		} else if (parameter == "entity-limit") {
			cout << facade.licenceGetEntityLimit() << endl;
		} else if (parameter == "hosts-limit") {
			cout << facade.licenceGetHostsLimit() << endl;
		} else if (parameter == "threads-limit") {
			cout << facade.licenceGetThreadsLimit() << endl;
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

void GenesysShell::cmdInfo() {
	if (_typedWords->size() == 1) {
		cout << facade.infoShow() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = _typedWords->at(i);
		if (parameter == "show") {
			cout << facade.infoShow() << endl;
		} else if (parameter == "get-name") {
			cout << facade.infoGetName() << endl;
		} else if (parameter == "set-name") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.infoSetName(value);
			return;
		} else if (parameter == "get-analyst-name") {
			cout << facade.infoGetAnalystName() << endl;
		} else if (parameter == "set-analyst-name") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.infoSetAnalystName(value);
			return;
		} else if (parameter == "get-description") {
			cout << facade.infoGetDescription() << endl;
		} else if (parameter == "set-description") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.infoSetDescription(value);
			return;
		} else if (parameter == "get-project-title") {
			cout << facade.infoGetProjectTitle() << endl;
		} else if (parameter == "set-project-title") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.infoSetProjectTitle(value);
			return;
		} else if (parameter == "get-version") {
			cout << facade.infoGetVersion() << endl;
		} else if (parameter == "set-version") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.infoSetVersion(value);
			return;
		} else if (parameter == "has-changed") {
			cout << (facade.infoHasChanged() ? "true" : "false") << endl;
		} else if (parameter == "set-has-changed") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.infoSetHasChanged(value);
			return;
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

void GenesysShell::cmdSim() {
	if (_typedWords->size() == 1) {
		cout << facade.simShow() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = lowercase(_typedWords->at(i));
		if (parameter == "show") {
			cout << facade.simShow() << endl;
		} else if (parameter == "start") {
			facade.simStart();
		} else if (parameter == "pause") {
			facade.simPause();
		} else if (parameter == "step") {
			facade.simStep();
		} else if (parameter == "stop") {
			facade.simStop();
		} else if (parameter == "get-replications") {
			cout << facade.simGetNumberOfReplications() << endl;
		} else if (parameter == "set-replications") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			unsigned int value = 0;
			if (!parseUnsigned(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid replication count" << endl;
				return;
			}
			facade.simSetNumberOfReplications(value);
			return;
		} else if (parameter == "get-length") {
			cout << facade.simGetReplicationLength() << " " << Util::convertEnumToStr(facade.simGetReplicationLengthTimeUnit()) << endl;
		} else if (parameter == "set-length") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			double length = 0.0;
			if (!parseDouble(_typedWords->at(i + 1), length)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid replication length" << endl;
				return;
			}
			Util::TimeUnit unit = Util::TimeUnit::unknown;
			if (i + 2 < _typedWords->size() && parseTimeUnit(_typedWords->at(i + 2), unit)) {
				facade.simSetReplicationLength(length, unit);
				return;
			}
			facade.simSetReplicationLength(length);
			return;
		} else if (parameter == "get-base-length") {
			cout << Util::convertEnumToStr(facade.simGetReplicationBaseTimeUnit()) << endl;
		} else if (parameter == "set-base-length") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			Util::TimeUnit unit;
			if (!parseTimeUnit(_typedWords->at(i + 1), unit)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid time unit" << endl;
				return;
			}
			facade.simSetReplicationReportBaseTimeUnit(unit);
			return;
		} else if (parameter == "get-warm-up") {
			cout << facade.simGetWarmUpPeriod() << " " << Util::convertEnumToStr(facade.simGetWarmUpPeriodTimeUnit()) << endl;
		} else if (parameter == "set-warm-up") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			double value = 0.0;
			if (!parseDouble(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid warm-up value" << endl;
				return;
			}
			Util::TimeUnit unit = Util::TimeUnit::unknown;
			if (i + 2 < _typedWords->size() && parseTimeUnit(_typedWords->at(i + 2), unit)) {
				facade.simSetWarmUpPeriod(value, unit);
				return;
			}
			facade.simSetWarmUpPeriod(value);
			return;
		} else if (parameter == "get-warm-up-unit") {
			cout << Util::convertEnumToStr(facade.simGetWarmUpPeriodTimeUnit()) << endl;
		} else if (parameter == "set-warm-up-unit") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			Util::TimeUnit unit;
			if (!parseTimeUnit(_typedWords->at(i + 1), unit)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid time unit" << endl;
				return;
			}
			facade.simSetWarmUpPeriodTimeUnit(unit);
			return;
		} else if (parameter == "get-length-unit") {
			cout << Util::convertEnumToStr(facade.simGetReplicationLengthTimeUnit()) << endl;
		} else if (parameter == "set-length-unit") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			Util::TimeUnit unit;
			if (!parseTimeUnit(_typedWords->at(i + 1), unit)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid time unit" << endl;
				return;
			}
			facade.simSetReplicationLengthTimeUnit(unit);
			return;
		} else if (parameter == "get-terminating-condition") {
			cout << facade.simGetTerminatingCondition() << endl;
		} else if (parameter == "set-terminating-condition") {
			const std::string value = joinArguments(i + 1);
			if (value.empty()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			facade.simSetTerminatingCondition(value);
			return;
		} else if (parameter == "get-pause-on-event") {
			cout << (facade.simIsPauseOnEvent() ? "true" : "false") << endl;
		} else if (parameter == "set-pause-on-event") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetPauseOnEvent(value);
			return;
		} else if (parameter == "get-step-by-step") {
			cout << (facade.simIsStepByStep() ? "true" : "false") << endl;
		} else if (parameter == "set-step-by-step") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetStepByStep(value);
			return;
		} else if (parameter == "get-init-statistics") {
			cout << (facade.simIsInitializeStatistics() ? "true" : "false") << endl;
		} else if (parameter == "set-init-statistics") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetInitializeStatistics(value);
			return;
		} else if (parameter == "get-init-system") {
			cout << (facade.simIsInitializeSystem() ? "true" : "false") << endl;
		} else if (parameter == "set-init-system") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetInitializeSystem(value);
			return;
		} else if (parameter == "get-pause-on-replication") {
			cout << (facade.simIsPauseOnReplication() ? "true" : "false") << endl;
		} else if (parameter == "set-pause-on-replication") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetPauseOnReplication(value);
			return;
		} else if (parameter == "get-current-time") {
			cout << facade.simGetSimulatedTime() << endl;
		} else if (parameter == "get-current-replication") {
			cout << facade.simGetCurrentReplicationNumber() << endl;
		} else if (parameter == "get-running") {
			cout << (facade.simIsRunning() ? "true" : "false") << endl;
		} else if (parameter == "get-paused") {
			cout << (facade.simIsPaused() ? "true" : "false") << endl;
		} else if (parameter == "show-reports-after-replication") {
			cout << (facade.simIsShowReportsAfterReplication() ? "true" : "false") << endl;
		} else if (parameter == "set-show-reports-after-replication") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetShowReportsAfterReplication(value);
			return;
		} else if (parameter == "show-reports-after-simulation") {
			cout << (facade.simIsShowReportsAfterSimulation() ? "true" : "false") << endl;
		} else if (parameter == "set-show-reports-after-simulation") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetShowReportsAfterSimulation(value);
			return;
		} else if (parameter == "get-simulation-controls-in-report") {
			cout << (facade.simIsShowSimulationControlsInReport() ? "true" : "false") << endl;
		} else if (parameter == "set-simulation-controls-in-report") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetShowSimulationControlsInReport(value);
			return;
		} else if (parameter == "get-simulation-responses-in-report") {
			cout << (facade.simIsShowSimulationResposesInReport() ? "true" : "false") << endl;
		} else if (parameter == "set-simulation-responses-in-report") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.simSetShowSimulationResposesInReport(value);
			return;
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

void GenesysShell::cmdData() {
	if (_typedWords->size() == 1) {
		cout << "Data definitions: " << facade.dataGetNumberOfDataDefinitions() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = lowercase(_typedWords->at(i));
		if (parameter == "count") {
			if (i + 1 < _typedWords->size()) {
				const std::string type = _typedWords->at(i + 1);
				cout << facade.dataGetNumberOfDataDefinitions(type) << endl;
				return;
			}
			cout << facade.dataGetNumberOfDataDefinitions() << endl;
		} else if (parameter == "show") {
			facade.dataShow();
		} else if (parameter == "class-names") {
			const std::list<std::string> names = facade.dataGetDataDefinitionClassnames();
			for (const std::string& name : names) {
				cout << name << endl;
			}
		} else if (parameter == "list") {
			if (i + 1 >= _typedWords->size()) {
				const std::list<std::string> names = facade.dataGetDataDefinitionClassnames();
				for (const std::string& name : names) {
					cout << name << ": " << facade.dataGetNumberOfDataDefinitions(name) << endl;
				}
				return;
			}
			const std::string type = _typedWords->at(i + 1);
			List<ModelDataDefinition*>* items = facade.dataGetDataDefinitionList(type);
			if (items == nullptr || items->size() == 0) {
				cout << "No data definitions of type " << type << endl;
				return;
			}
			for (ModelDataDefinition* item : *items->list()) {
				if (item != nullptr) {
					cout << item->show() << endl;
				}
			}
			return;
		} else if (parameter == "clear") {
			facade.dataClear();
		} else if (parameter == "has-changed") {
			cout << (facade.dataHasChanged() ? "true" : "false") << endl;
		} else if (parameter == "set-has-changed") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.dataSetHasChanged(value);
			return;
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

void GenesysShell::cmdComponent() {
	if (_typedWords->size() == 1) {
		cout << "Components: " << facade.componentGetNumberOfComponents() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = lowercase(_typedWords->at(i));
		if (parameter == "count") {
			cout << facade.componentGetNumberOfComponents() << endl;
		} else if (parameter == "show" || parameter == "list") {
			std::list<ModelComponent*>* components = facade.componentGetAllComponents();
			if (components == nullptr || components->size() == 0) {
				cout << "No components in current model" << endl;
				return;
			}
			for (ModelComponent* component : *components) {
				if (component != nullptr) {
					cout << component->show() << endl;
				}
			}
		} else if (parameter == "find") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			ModelComponent* component = nullptr;
			unsigned int id = 0;
			if (parseUnsigned(_typedWords->at(i + 1), id)) {
				component = facade.componentFind(id);
			} else {
				component = facade.componentFind(_typedWords->at(i + 1));
			}
			if (component != nullptr) {
				cout << component->show() << endl;
			} else {
				cout << "Component not found" << endl;
			}
			return;
		} else if (parameter == "clear") {
			facade.componentClear();
		} else if (parameter == "has-changed") {
			cout << (facade.componentHasChanged() ? "true" : "false") << endl;
		} else if (parameter == "set-has-changed") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			bool value = false;
			if (!parseBool(_typedWords->at(i + 1), value)) {
				cout << "Error: " << _typedWords->at(i + 1) << " is not a valid boolean" << endl;
				return;
			}
			facade.componentSetHasChanged(value);
			return;
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

void GenesysShell::cmdExperiment() {
	if (_typedWords->size() == 1) {
		cout << "Experiments: " << facade.simulationExperimentCount() << endl;
		return;
	}
	for (unsigned short i = 1; i < _typedWords->size(); ++i) {
		const std::string parameter = lowercase(_typedWords->at(i));
		if (parameter == "count") {
			cout << facade.simulationExperimentCount() << endl;
		} else if (parameter == "show" || parameter == "current") {
			SimulationExperiment* experiment = facade.currentSimulationExperiment();
			if (experiment != nullptr) {
				cout << "Current experiment @" << experiment << endl;
			} else {
				cout << "No current experiment" << endl;
			}
		} else if (parameter == "new") {
			SimulationExperiment* experiment = facade.newSimulationExperiment();
			if (experiment != nullptr) {
				cout << "Created experiment @" << experiment << endl;
			} else {
				cout << "Could not create experiment" << endl;
			}
		} else if (parameter == "save") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string filename = _typedWords->at(i + 1);
			cout << (facade.saveSimulationExperiment(filename) ? "true" : "false") << endl;
			return;
		} else if (parameter == "load") {
			if (i + 1 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string filename = _typedWords->at(i + 1);
			cout << (facade.loadSimulationExperiment(filename) ? "true" : "false") << endl;
			return;
		} else if (parameter == "first") {
			SimulationExperiment* experiment = facade.firstSimulationExperiment();
			if (experiment != nullptr) {
				cout << "First experiment @" << experiment << endl;
			} else {
				cout << "No experiments available" << endl;
			}
		} else if (parameter == "next") {
			SimulationExperiment* experiment = facade.nextSimulationExperiment();
			if (experiment != nullptr) {
				cout << "Next experiment @" << experiment << endl;
			} else {
				cout << "No next experiment" << endl;
			}
		} else {
			cout << "Syntax error on " << parameter << endl;
		}
	}
}

/*
 void GenesysShell::cmdShowReport() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to show report."<<endl;
		return;
	}
}

 void GenesysShell::cmdPluginInfo() {
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	std::string parameter = _typedWords->at(1);
	Plugin* p;
	for (unsigned short i = 0; i<simulator->getPlugins()->size(); i++) {
		p = simulator->getPlugins()->getAtRank(i);
		if (p->getPluginInfo()->getPluginTypename()==parameter) {
			cout<<"Information for plugin "<<parameter<<":"<<endl;
			cout<<p->show()<<endl;
			return;
		}
	}
	cout<<"Error: Could not find plugin of type "<<parameter<<endl;
}


void GenesysShell::cmdSimulationStep() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to simulate."<<endl;
		return;
	}
	model->getSimulation()->step();
}

void GenesysShell::cmdSimulationStop() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to simulate."<<endl;
		return;
	}
	cout<<"Stoping simulation"<<endl;
	model->getSimulation()->stop();
}

void GenesysShell::cmdSimulationInfo() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to simulate."<<endl;
		return;
	}
	cout<<model->getSimulation()->show()<<endl;
}

void GenesysShell::cmdModelClose() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to close."<<endl;
		return;
	}
	cout<<"Closing the model"<<endl;
	simulator->getModels()->remove(model);
	model = nullptr;
}

void GenesysShell::cmdModelCheck() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to check."<<endl;
		return;
	}
	model->check();
}



void GenesysShell::cmdModelShow() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to show."<<endl;
		return;
	}
	model->show();
}

void GenesysShell::cmdModelSetInfos() {
	if (model==nullptr) {
		cout<<"Error: There is no loaded model to set information."<<endl;
		return;
	}
}

void GenesysShell::cmdListFiles() {
	if (_typedWords->size()!=2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	std::string parameter = _typedWords->at(1);
	const std::string command = "ls -l "+parameter;
	system(command.c_str());
}
 */
