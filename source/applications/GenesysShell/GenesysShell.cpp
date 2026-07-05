/*
 *
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GenesysShell.cpp
 * Author: rafael.luiz.cancian
 * , caso simm, como
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
	_commands->insert(new ShellCommand("", "execute-script", "<filename>", "Execute commands from a GenesysShell script file", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdExecuteScript)));
	_commands->insert(new ShellCommand("", "trace", "[show|level <level 1-9>]", "Set or show current the trace level", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdTraceLevel)));
	_commands->insert(new ShellCommand("", "plugin", "[list|count|info <plugin typename>|autoload <filename plugin list>|template [plugintypename]]", "List, count, load or get information about installed plugins and templates of genesys language", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPlugin)));
	//_commands->insert(new ShellCommand("", "plugininfo", "<plugin name>", "Show infos about a plugin", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginInfo)));
	//_commands->insert(new ShellCommand("", "pluginadd", "<plugin filename>", "", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginAdd)));
	//_commands->insert(new ShellCommand("", "pluginremove", "<classname>", "", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdPluginRemove)));
	_commands->insert(new ShellCommand("", "simul", "[start|step]", "Control simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulation)));
	//_commands->insert(new ShellCommand("", "step", "", "Step simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationStep)));
	//_commands->insert(new ShellCommand("", "stop", "", "Stop simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationStop)));
	_commands->insert(new ShellCommand("", "config", "[show|replications <number of replications>|length <replication length> [time unit]|base-length <time unit>|warm-up <value> [time unit]|warm-up-unit <time unit>|terminating-condition <expression>|pause-on-event <true|false>|step-by-step <true|false>|init-statistics <true|false>|init-system <true|false>|pause-on-replication <true|false>|show-reports-after-replication <true|false>|show-reports-after-simulation <true|false>|simulation-controls-in-report <true|false>|simulation-responses-in-report <true|false>]", "Configure simulation", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdReplication)));
	//_commands->insert(new ShellCommand("", "showsetup", "", "Show simulation info", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdSimulationInfo)));
	//_commands->insert(new ShellCommand("", "showreport", "", "Show simulation report", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdShowReport)));
	//_commands->insert(new ShellCommand("", "show", "", "Show the model structure", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModelShow)));
	_commands->insert(new ShellCommand("", "model", "[new|remove|check|show|load <filename>|save <filename>]", "Create, check, show or close a model", DefineExecuterMember<GenesysShell>(this, &GenesysShell::cmdModel)));
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
	const auto printTopic = [](const std::string& name, const std::string& summary, const std::vector<std::string>& lines) {
		cout << name << " - " << summary << endl;
		cout << "  help " << name << endl;
		for (const std::string& line : lines) {
			cout << "    " << line << endl;
		}
	};

	if (_typedWords->size() == 1) {
		cout << "List of commands:" << endl;
		cout << "Main commands:" << endl;
		for (ShellCommand *command : *_commands->list()) {
			const bool special = command->longname == "plugin" || command->longname == "model" || command->longname == "sim" || command->longname == "config" || command->longname == "trace";
			if (!special) {
				cout << command->longname << " - " << command->descrition;
				if (!command->parameters.empty()) {
					cout << " (" << command->parameters << ")";
				}
				cout << endl;
			}
		}
		cout << "Special commands:" << endl;
		for (ShellCommand *command : *_commands->list()) {
			const bool special = command->longname == "plugin" || command->longname == "model" || command->longname == "sim" || command->longname == "config" || command->longname == "trace";
			if (special) {
				cout << command->longname << " - " << command->descrition;
				if (!command->parameters.empty()) {
					cout << " (" << command->parameters << ")";
				}
				cout << endl;
			}
		}
		return;
	}

	const std::string topic = lowercase(_typedWords->at(1));
	if (topic == "sim") {
		printTopic("sim", "inspect or control the current model simulation through SimulatorFacade", {
			"show - display the current simulation state",
			"start - start the simulation",
			"pause - pause a running simulation",
			"step - process one event and pause again",
			"stop - stop the current simulation",
			"get-replications - show the number of replications",
			"set-replications <number> - change the number of replications",
			"get-length - show the replication length and time unit",
			"set-length <value> [time unit] - change the replication length; the unit is optional",
			"get-base-length - show the base time unit used for reports",
			"set-base-length <time unit> - change the report base time unit",
			"get-warm-up - show the warm-up period and time unit",
			"set-warm-up <value> [time unit] - change the warm-up period; the unit is optional",
			"get-warm-up-unit - show the warm-up time unit",
			"set-warm-up-unit <time unit> - change the warm-up time unit",
			"get-terminating-condition - show the terminating condition expression",
			"set-terminating-condition <expression> - change the terminating condition",
			"get-pause-on-event - show whether the simulation pauses on events",
			"set-pause-on-event <true|false> - enable or disable pause on event",
			"get-step-by-step - show whether step-by-step mode is enabled",
			"set-step-by-step <true|false> - enable or disable step-by-step mode",
			"get-init-statistics - show whether statistics reset between replications",
			"set-init-statistics <true|false> - enable or disable statistics reset",
			"get-init-system - show whether the system is reinitialized between replications",
			"set-init-system <true|false> - enable or disable system reinitialization",
			"get-pause-on-replication - show whether the simulation pauses between replications",
			"set-pause-on-replication <true|false> - enable or disable pause between replications",
			"get-current-time - show the current simulated time",
			"get-current-replication - show the current replication number",
			"get-running - show whether the simulation is running",
			"get-paused - show whether the simulation is paused",
			"current-event - show the current event, if any",
			"reporter - show whether a simulation reporter is configured",
			"breakpoints list time|entity|component - list breakpoints of the selected type",
			"breakpoints add time <value> - add a time breakpoint",
			"breakpoints add entity <id|name> - add an entity breakpoint",
			"breakpoints add component <id|name> - add a component breakpoint",
			"breakpoints remove time <value> - remove a time breakpoint",
			"breakpoints remove entity <id|name> - remove an entity breakpoint",
			"breakpoints remove component <id|name> - remove a component breakpoint",
			"breakpoints clear time|entity|component - clear the selected breakpoint list",
			"error-messages - show trace error messages",
			"show-reports-after-replication - show whether replication reports are enabled",
			"set-show-reports-after-replication <true|false> - enable or disable replication reports",
			"show-reports-after-simulation - show whether simulation reports are enabled",
			"set-show-reports-after-simulation <true|false> - enable or disable simulation reports",
			"get-simulation-controls-in-report - show whether simulation controls are included in reports",
			"set-simulation-controls-in-report <true|false> - enable or disable simulation controls in reports",
			"get-simulation-responses-in-report - show whether simulation responses are included in reports",
			"set-simulation-responses-in-report <true|false> - enable or disable simulation responses in reports"
		});
		return;
	}
	if (topic == "config") {
		printTopic("config", "configure simulation defaults and reporting", {
			"show - display the current simulation configuration",
			"replications <number> - change the number of replications",
			"length <value> [time unit] - change the replication length; the unit is optional",
			"base-length <time unit> - change the report base time unit",
			"warm-up <value> [time unit] - change the warm-up period; the unit is optional",
			"warm-up-unit <time unit> - change the warm-up time unit",
			"terminating-condition <expression> - change the terminating condition",
			"pause-on-event <true|false> - enable or disable pause on event",
			"step-by-step <true|false> - enable or disable step-by-step mode",
			"init-statistics <true|false> - enable or disable statistics reset between replications",
			"init-system <true|false> - enable or disable system reinitialization between replications",
			"pause-on-replication <true|false> - enable or disable pause between replications",
			"show-reports-after-replication <true|false> - enable or disable replication reports",
			"show-reports-after-simulation <true|false> - enable or disable simulation reports",
			"simulation-controls-in-report <true|false> - enable or disable simulation controls in reports",
			"simulation-responses-in-report <true|false> - enable or disable simulation responses in reports"
		});
		return;
	}
	if (topic == "plugin") {
		printTopic("plugin", "inspect and load installed plugins", {
			"list - show the installed plugins",
			"count - show how many plugins are installed",
			"info <plugin typename> - show detailed information for one plugin",
			"autoload <filename> - load plugins from a plugin list file",
			"template [plugintypename] - show language templates for plugins; the type filter is optional",
			"check <dynamic library filename> - validate whether a library looks like a plugin",
			"check-system-dependencies <dynamic library filename> - inspect missing system dependencies before loading",
			"discover - discover plugin filenames through the connector",
			"issues - show stored plugin load diagnostics",
			"clear-issues - remove all stored plugin load diagnostics",
			"clear-issue <dynamic library filename> - remove diagnostics for one plugin candidate",
			"remove <dynamic library filename> - unload a plugin by filename",
			"rank <n> - show the plugin stored at the given rank",
			"first - show the first plugin in the list",
			"next - show the next plugin in the list",
			"last - show the last plugin in the list"
		});
		return;
	}
	if (topic == "model") {
		printTopic("model", "create, inspect, load or save the current model", {
			"new - create a new current model",
			"remove - remove the current model",
			"check - validate the current model",
			"show - show the current model in language form",
			"load <filename> - load a model from a file",
			"save <filename> - save the current model to a file",
			"count - show how many models exist",
			"current - show the current model",
			"first - show the first model in the manager",
			"last - show the last model in the manager",
			"next - show the next model in the manager",
			"previous - show the previous model in the manager",
			"can-go-next - show whether there is a next model",
			"can-go-previous - show whether there is a previous model",
			"at <index> - show the model at a given index",
			"future-events - list the current model future events",
			"controls - list the current model simulation controls",
			"responses - list the current model simulation responses",
			"persistence - show whether the model has a persistence backend",
			"level - show the current model level"
		});
		return;
	}
	if (topic == "trace") {
		printTopic("trace", "inspect or update the trace level", {
			"show",
			"level <level 1-9>"
		});
		return;
	}
	if (topic == "execute-script") {
		printTopic("execute-script", "execute commands from a GenesysShell script file", {
			"<filename> - read the file line by line and execute each non-empty, non-comment line as a GenesysShell command"
		});
		return;
	}
	if (topic == "load") {
		printTopic("load", "load a model from a file", {
			"<filename> - path to the model file to load"
		});
		return;
	}
	if (topic == "save") {
		printTopic("save", "save the current model to a file", {
			"<filename> - path to the destination file"
		});
		return;
	}
	if (topic == "facade") {
		printTopic("facade", "query high-level simulator metadata", {
			"get-version",
			"get-version-number",
			"get-name"
		});
		return;
	}
	if (topic == "licence") {
		printTopic("licence", "inspect licence information", {
			"show",
			"limits",
			"activation-code",
			"lookfor-activation-code",
			"insert-activation-code",
			"remove-activation-code",
			"model-components-limit",
			"model-datas-limit",
			"entity-limit",
			"hosts-limit",
			"threads-limit"
		});
		return;
	}
	if (topic == "data") {
		printTopic("data", "inspect current model data definitions", {
			"count [type] - show the number of data definitions, optionally filtered by type",
			"show - show all current data definitions",
			"class-names - list all data definition class names",
			"list [type] - list definitions grouped by type or list one type; the type filter is optional",
			"get <type> <id|name> - find one definition by id or name",
			"rank-of <type> <name> - show the rank of a definition inside its type list",
			"clear - clear all current data definitions",
			"has-changed - show whether the data set changed",
			"set-has-changed <true|false> - change the data changed flag"
		});
		return;
	}
	if (topic == "component") {
		printTopic("component", "inspect current model components", {
			"count - show how many components exist",
			"show - show all components",
			"list - list all components",
			"clear - clear all components",
			"find <id|name> - find a component by id or name",
			"front - show the first component in the list",
			"next - show the next component in the list",
			"all - list every component using the manager snapshot",
			"has-changed - show whether the component set changed",
			"set-has-changed <true|false> - change the component changed flag"
		});
		return;
	}
	if (topic == "experiment") {
		printTopic("experiment", "inspect simulation experiments", {
			"count - show how many experiments exist",
			"show - show the current experiment",
			"current - show the current experiment",
			"new - create a new experiment",
			"save <filename> - save the current experiment to a file",
			"load <filename> - load an experiment from a file",
			"next - show the next experiment",
			"first - show the first experiment"
		});
		return;
	}
	cout << "No detailed help available for \"" << _typedWords->at(1) << "\"." << endl;
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
	if (_typedWords->size() != 3) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	const std::string action = lowercase(_typedWords->at(1));
	const std::string value = _typedWords->at(2);
	const bool run = action == "run" || action == "-r" || action == "-run";
	const bool show = action == "show" || action == "-s" || action == "--show";
	if (!run && !show) {
		cout<<"Syntax error on "<<_typedWords->at(1)<<endl;
		return;
	}
	if (show) {
		cout<<"Showing script "<<value<<endl;
	}
	if (run) {
		cout<<"Running script "<<value<<endl;
	}
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

void GenesysShell::cmdExecuteScript() {
	if (_typedWords->size() != 2) {
		cout << "Wrong number of parameters" << endl;
		return;
	}
	const std::string filename = _typedWords->at(1);
	ifstream file(filename);
	if (!file.is_open()) {
		cout << "Error: Could not load the script" << endl;
		return;
	}
	std::string line;
	while (getline(file, line)) {
		const std::string trimmed = Util::Trim(line);
		if (trimmed.empty() || trimmed[0] == '#') {
			continue;
		}
		cout << "$execute-script> " << line << endl;
		tryExecuteCommand(line);
	}
	file.close();
}

void GenesysShell::cmdTraceLevel() {
	if (_typedWords->size() == 1) {
		cout<<"Trace level is "<<static_cast<int> (facade.getTraceLevel())<<endl;
		return;
	}
	const std::string action = lowercase(_typedWords->at(1));
	if (action == "show" || action == "-s" || action == "--show") {
		cout<<"Trace level is "<<static_cast<int> (facade.getTraceLevel())<<endl;
		return;
	}
	if (action != "level" && action != "-l" && action != "--level") {
		cout<<"Syntax error on "<<_typedWords->at(1)<<endl;
		return;
	}
	if (_typedWords->size() != 3) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	TraceManager::Level level;
	if (parseTraceLevel(_typedWords->at(2), level)) {
		cout<<"Setting trace level to "<<_typedWords->at(2)<<endl;
		facade.setTraceLevel(level);
	} else {
		cout<<"Error: "<<_typedWords->at(2)<<" is not a valid level"<<endl;
	}
}

void GenesysShell::cmdPlugin() {
	if (_typedWords->size() == 1) {
		if (facade.pluginCount()==0) {
			cout<<"There is no installed plugins. Install some using the plugin autoload <filename>"<<endl;
			return;
		}
		cout<<facade.showPlugins()<<endl;
		return;
	}
	const std::string action = lowercase(_typedWords->at(1));
	if (action == "list") {
		if (facade.pluginCount()==0) {
			cout<<"There is no installed plugins. Install some using the plugin autoload <filename>"<<endl;
			return;
		}
		cout<<facade.showPlugins()<<endl;
		return;
	}
	if (action == "count") {
		cout<<"Installed plugins: "<<facade.pluginCount()<<endl;
		return;
	}
	if (action == "autoload") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		cout<<"Loading list of plugins from file "<<_typedWords->at(2)<<endl;
		facade.autoInsertPlugins(_typedWords->at(2), true, ShellPluginInsertionOptions());
		return;
	}
	if (action == "info") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		if (facade.pluginCount()==0) {
			cout<<"There is no installed plugins. Install some using the plugin autoload <filename>"<<endl;
			return;
		}
		Plugin* plugin = facade.findPlugin(_typedWords->at(2));
		if (plugin != nullptr) {
			cout<<"Information for plugin "<<_typedWords->at(2)<<":"<<endl;
			cout<<plugin->show()<<endl;
		} else {
			cout<<"Error: Could not find plugin of type "<<_typedWords->at(2)<<endl;
		}
		return;
	}
	if (action == "template") {
		const std::string value = _typedWords->size() >= 3 ? _typedWords->at(2) : "";
		if (facade.pluginCount()==0) {
			cout<<"There is no installed plugins. Install some using the plugin autoload <filename>"<<endl;
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
		return;
	}
	if (action == "check") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		cout << (facade.pluginCheck(_typedWords->at(2)) ? "true" : "false") << endl;
		return;
	}
	if (action == "check-system-dependencies") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		const SystemDependencyCheckResult result = facade.pluginCheckSystemDependencies(_typedWords->at(2));
		cout << result.diagnosticText() << endl;
		return;
	}
	if (action == "discover") {
		List<std::string>* files = facade.discoverPluginFilenames();
		if (files == nullptr || files->size() == 0) {
			cout << "No plugin filenames discovered." << endl;
			return;
		}
		for (const std::string& file : *files->list()) {
			cout << file << endl;
		}
		return;
	}
	if (action == "remove") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		cout << (facade.removePlugin(_typedWords->at(2)) ? "true" : "false") << endl;
		return;
	}
	if (action == "issues") {
		List<PluginLoadIssue>* issues = facade.getPluginLoadIssues();
		if (issues == nullptr || issues->size() == 0) {
			cout << "No plugin load issues." << endl;
			return;
		}
		for (const PluginLoadIssue& issue : *issues->list()) {
			cout << issue.diagnosticText() << endl;
		}
		return;
	}
	if (action == "clear-issues") {
		facade.clearPluginLoadIssues();
		cout << "Plugin load issues cleared." << endl;
		return;
	}
	if (action == "clear-issue") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		facade.clearPluginLoadIssue(_typedWords->at(2));
		cout << "Plugin load issue cleared for " << _typedWords->at(2) << endl;
		return;
	}
	if (action == "rank") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		unsigned int rank = 0;
		if (!parseUnsigned(_typedWords->at(2), rank)) {
			cout << "Error: " << _typedWords->at(2) << " is not a valid rank" << endl;
			return;
		}
		Plugin* plugin = facade.getPluginAtRank(rank);
		if (plugin != nullptr) {
			cout << plugin->show() << endl;
		} else {
			cout << "Plugin not found" << endl;
		}
		return;
	}
	if (action == "front" || action == "next" || action == "last") {
		Plugin* plugin = nullptr;
		if (action == "front") {
			plugin = facade.firstPlugin();
		} else if (action == "next") {
			plugin = facade.nextPlugin();
		} else {
			plugin = facade.lastPlugin();
		}
		if (plugin != nullptr) {
			cout << plugin->show() << endl;
		} else {
			cout << "No plugin available" << endl;
		}
		return;
	}
	if (action == "first" || action == "next" || action == "last") {
		Plugin* plugin = nullptr;
		if (action == "first") {
			plugin = facade.firstPlugin();
		} else if (action == "next") {
			plugin = facade.nextPlugin();
		} else {
			plugin = facade.lastPlugin();
		}
		if (plugin != nullptr) {
			cout << plugin->show() << endl;
		} else {
			cout << "No plugin available" << endl;
		}
		return;
	}
	cout<<"Syntax error on "<<_typedWords->at(1)<<endl;
}

void GenesysShell::cmdSimulation() {
	if (!requireCurrentModel("control the simulation")) {
		return;
	}
	if (_typedWords->size() != 2) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	const std::string parameter = lowercase(_typedWords->at(1));
	if (parameter=="start"||parameter=="-s"||parameter=="--start")
		facade.simStart();
	else if (parameter=="step"||parameter=="-p"||parameter=="--step")
		facade.simStep();
	else if (parameter=="show")
		cout<<facade.simShow()<<endl;
	else if (parameter=="pause")
		facade.simPause();
	else if (parameter=="stop")
		facade.simStop();
	else if (parameter=="current-event") {
		Event* event = facade.simGetCurrentEvent();
		if (event != nullptr) {
			cout << event->show() << endl;
		} else {
			cout << "No current event" << endl;
		}
	} else if (parameter=="reporter") {
		cout << (facade.simGetReporter() != nullptr ? "present" : "not set") << endl;
	} else if (parameter=="error-messages") {
		List<std::string>* messages = facade.traceGetErrorMessages();
		if (messages == nullptr || messages->size() == 0) {
			cout << "No trace error messages" << endl;
		} else {
			for (const std::string& message : *messages->list()) {
				cout << message << endl;
			}
		}
	} else if (parameter=="breakpoints") {
		Model* currentModel = facade.currentModel();
		if (currentModel == nullptr) {
			cout << "No current model" << endl;
			return;
		}
		ModelSimulation* simulation = currentModel->getSimulation();
		if (simulation == nullptr) {
			cout << "No current model simulation" << endl;
			return;
		}
		if (_typedWords->size() == 2) {
			cout << "Wrong number of parameters" << endl;
			return;
		}
		const std::string breakpointAction = lowercase(_typedWords->at(2));
		auto printTimeBreakpoints = [&]() {
			List<double>* items = simulation->getBreakpointsOnTime();
			if (items == nullptr || items->size() == 0) {
				cout << "No time breakpoints" << endl;
				return;
			}
			for (double value : *items->list()) {
				cout << value << endl;
			}
		};
		auto printEntityBreakpoints = [&]() {
			List<Entity*>* items = simulation->getBreakpointsOnEntity();
			if (items == nullptr || items->size() == 0) {
				cout << "No entity breakpoints" << endl;
				return;
			}
			for (Entity* entity : *items->list()) {
				if (entity != nullptr) {
					cout << entity->show() << endl;
				}
			}
		};
		auto printComponentBreakpoints = [&]() {
			List<ModelComponent*>* items = simulation->getBreakpointsOnComponent();
			if (items == nullptr || items->size() == 0) {
				cout << "No component breakpoints" << endl;
				return;
			}
			for (ModelComponent* component : *items->list()) {
				if (component != nullptr) {
					cout << component->show() << endl;
				}
			}
		};
		auto parseBreakpointTarget = [&](const std::string& kind, const std::string& valueText, bool add) -> bool {
			if (kind == "time") {
				double value = 0.0;
				if (!parseDouble(valueText, value)) {
					cout << "Error: " << valueText << " is not a valid time value" << endl;
					return false;
				}
				List<double>* items = simulation->getBreakpointsOnTime();
				if (add) items->insert(value); else items->remove(value);
				return true;
			}
			if (kind == "entity") {
				unsigned int id = 0;
				Entity* entity = nullptr;
				if (parseUnsigned(valueText, id)) {
					entity = dynamic_cast<Entity*>(facade.dataGetDataDefinition("Entity", id));
				} else {
					entity = dynamic_cast<Entity*>(facade.dataGetDataDefinition("Entity", valueText));
				}
				if (entity == nullptr) {
					cout << "Entity not found" << endl;
					return false;
				}
				List<Entity*>* items = simulation->getBreakpointsOnEntity();
				if (add) items->insert(entity); else items->remove(entity);
				return true;
			}
			if (kind == "component") {
				unsigned int id = 0;
				ModelComponent* component = nullptr;
				if (parseUnsigned(valueText, id)) {
					component = facade.componentFind(id);
				} else {
					component = facade.componentFind(valueText);
				}
				if (component == nullptr) {
					cout << "Component not found" << endl;
					return false;
				}
				List<ModelComponent*>* items = simulation->getBreakpointsOnComponent();
				if (add) items->insert(component); else items->remove(component);
				return true;
			}
			cout << "Syntax error on " << kind << endl;
			return false;
		};

		if (breakpointAction == "list") {
			if (_typedWords->size() != 4) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string kind = lowercase(_typedWords->at(3));
			if (kind == "time") printTimeBreakpoints();
			else if (kind == "entity") printEntityBreakpoints();
			else if (kind == "component") printComponentBreakpoints();
			else cout << "Syntax error on " << _typedWords->at(3) << endl;
			return;
		}
		if (breakpointAction == "clear") {
			if (_typedWords->size() != 4) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string kind = lowercase(_typedWords->at(3));
			if (kind == "time") simulation->getBreakpointsOnTime()->clear();
			else if (kind == "entity") simulation->getBreakpointsOnEntity()->clear();
			else if (kind == "component") simulation->getBreakpointsOnComponent()->clear();
			else cout << "Syntax error on " << _typedWords->at(3) << endl;
			return;
		}
		if (breakpointAction == "add" || breakpointAction == "remove") {
			if (_typedWords->size() != 5) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string kind = lowercase(_typedWords->at(3));
			if (!parseBreakpointTarget(kind, _typedWords->at(4), breakpointAction == "add")) {
				return;
			}
			return;
		}
		if (breakpointAction == "time") {
			printTimeBreakpoints();
			return;
		}
		if (breakpointAction == "entity") {
			printEntityBreakpoints();
			return;
		}
		if (breakpointAction == "component") {
			printComponentBreakpoints();
			return;
		}
		cout << "Syntax error on " << _typedWords->at(2) << endl;
	}
	else
		cout<<"Syntax error on "+_typedWords->at(1)<<endl;
}

void GenesysShell::cmdReplication() {
	if (!requireCurrentModel("setup the simulation")) {
		return;
	}
	if (_typedWords->size() == 1) {
		cout<<facade.simShow()<<endl;
		return;
	}
	const std::string action = lowercase(_typedWords->at(1));
	if (action == "show" || action == "-s" || action == "--show") {
		cout<<facade.simShow()<<endl;
		return;
	}
	if (action == "replications") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		unsigned int replications = 0;
		if (!parseUnsigned(_typedWords->at(2), replications)) {
			cout<<"Error: "<<_typedWords->at(2)<<" is not a valid number of replications"<<endl;
			return;
		}
		cout<<"Setting number of replications to "<<replications<<endl;
		facade.simSetNumberOfReplications(replications);
		return;
	}
	if (action == "length") {
		if (_typedWords->size() < 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		double length = 0.0;
		if (!parseDouble(_typedWords->at(2), length)) {
			cout<<"Error: "<<_typedWords->at(2)<<" is not a valid replication length"<<endl;
			return;
		}
		Util::TimeUnit timeUnit;
		if (_typedWords->size() >= 4 && parseTimeUnit(_typedWords->at(3), timeUnit)) {
			cout<<"Setting replication length to "<<length<<" "<<_typedWords->at(3)<<endl;
			facade.simSetReplicationLength(length, timeUnit);
			return;
		}
		cout<<"Setting replication length to "<<length<<endl;
		facade.simSetReplicationLength(length);
		return;
	}
	if (action == "base-length") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		Util::TimeUnit timeUnit;
		if (!parseTimeUnit(_typedWords->at(2), timeUnit)) {
			cout<<"Error: "<<_typedWords->at(2)<<" is not a valid time unit"<<endl;
			return;
		}
		facade.simSetReplicationReportBaseTimeUnit(timeUnit);
		return;
	}
	if (action == "warm-up") {
		if (_typedWords->size() < 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		double value = 0.0;
		if (!parseDouble(_typedWords->at(2), value)) {
			cout<<"Error: "<<_typedWords->at(2)<<" is not a valid warm-up value"<<endl;
			return;
		}
		Util::TimeUnit timeUnit = Util::TimeUnit::unknown;
		if (_typedWords->size() >= 4 && parseTimeUnit(_typedWords->at(3), timeUnit)) {
			facade.simSetWarmUpPeriod(value, timeUnit);
			return;
		}
		facade.simSetWarmUpPeriod(value);
		return;
	}
	if (action == "warm-up-unit") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		Util::TimeUnit timeUnit;
		if (!parseTimeUnit(_typedWords->at(2), timeUnit)) {
			cout<<"Error: "<<_typedWords->at(2)<<" is not a valid time unit"<<endl;
			return;
		}
		facade.simSetWarmUpPeriodTimeUnit(timeUnit);
		return;
	}
	if (action == "terminating-condition") {
		const std::string value = joinArguments(2);
		if (value.empty()) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		facade.simSetTerminatingCondition(value);
		return;
	}
	const auto parseBoolAction = [this](const std::string& valueText, void (SimulatorFacade::*setter)(bool)) -> bool {
		bool value = false;
		if (!parseBool(valueText, value)) {
			cout<<"Error: "<<valueText<<" is not a valid boolean"<<endl;
			return false;
		}
		(facade.*setter)(value);
		return true;
	};
	if (_typedWords->size() != 3) {
		cout<<"Wrong number of parameters"<<endl;
		return;
	}
	if (action == "pause-on-event") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetPauseOnEvent)) return;
	} else if (action == "step-by-step") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetStepByStep)) return;
	} else if (action == "init-statistics") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetInitializeStatistics)) return;
	} else if (action == "init-system") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetInitializeSystem)) return;
	} else if (action == "pause-on-replication") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetPauseOnReplication)) return;
	} else if (action == "show-reports-after-replication") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetShowReportsAfterReplication)) return;
	} else if (action == "show-reports-after-simulation") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetShowReportsAfterSimulation)) return;
	} else if (action == "simulation-controls-in-report") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetShowSimulationControlsInReport)) return;
	} else if (action == "simulation-responses-in-report") {
		if (!parseBoolAction(_typedWords->at(2), &SimulatorFacade::simSetShowSimulationResposesInReport)) return;
	} else if (action == "replications" || action == "length" || action == "base-length" || action == "warm-up" || action == "warm-up-unit" || action == "terminating-condition" || action == "pause-on-event" || action == "step-by-step" || action == "init-statistics" || action == "init-system" || action == "pause-on-replication" || action == "show-reports-after-replication" || action == "show-reports-after-simulation" || action == "simulation-controls-in-report" || action == "simulation-responses-in-report") {
		cout<<"Wrong number of parameters"<<endl;
		return;
	} else if (action == "-r" || action == "--replications" || action == "-l" || action == "--length" || action == "-t" || action == "--time" || action == "-s" || action == "--show") {
		cout<<"Legacy option syntax is no longer supported in this form. Use subcommands like config show, config replications 10, or config length 20 h."<<endl;
		return;
	}
	cout<<"Syntax error on "<<_typedWords->at(1)<<endl;
}

void GenesysShell::cmdModel() {
	if (_typedWords->size() == 1) {
		if (!requireCurrentModel("show the model")) {
			return;
		}
		cout<<facade.modelShowLanguage()<<endl;
		return;
	}
	const std::string action = lowercase(_typedWords->at(1));
	if (action == "new" || action == "-n" || action == "--new") {
		Model* currentModel = facade.currentModel();
		if (currentModel != nullptr) {
			facade.removeModel(currentModel);
			model = nullptr;
		}
		cout<<"Creating a new model"<<endl;
		model = facade.newModel();
		return;
	}
	if (action == "remove" || action == "-r" || action == "--remove") {
		if (!requireCurrentModel("close the model")) {
			return;
		}
		cout<<"Closing the model"<<endl;
		facade.removeModel(model);
		model = nullptr;
		return;
	}
	if (action == "check" || action == "-c" || action == "--check") {
		if (!requireCurrentModel("check the model")) {
			return;
		}
		if (!facade.modelCheck()) {
			cout<<"Model check returned errors."<<endl;
		}
		return;
	}
	if (action == "show" || action == "-s" || action == "--show") {
		if (!requireCurrentModel("show the model")) {
			return;
		}
		cout<<facade.modelShowLanguage()<<endl;
		return;
	}
	if (action == "load") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		const std::string parameter = _typedWords->at(2);
		cout<<"Loading model "<<parameter<<endl;
		model = facade.loadModel(parameter);
		if (model==nullptr) {
			cout<<"Error: Could not load the model"<<endl;
		} else {
			cout<<"Model loaded"<<endl;
		}
		return;
	}
	if (action == "save") {
		if (!requireCurrentModel("save the model")) {
			return;
		}
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		const std::string parameter = _typedWords->at(2);
		cout<<"Saving model "<<parameter<<endl;
		if (facade.saveModel(parameter)) {
			cout<<"Model saved"<<endl;
		} else {
			cout<<"Error: Could not save the model"<<endl;
		}
		return;
	}
	if (action == "count") {
		cout << facade.modelCount() << endl;
		return;
	}
	if (action == "current") {
		Model* current = facade.currentModel();
		if (current != nullptr) {
			cout << current->showLanguage() << endl;
		} else {
			cout << "No current model" << endl;
		}
		return;
	}
	if (action == "first" || action == "last" || action == "next" || action == "previous") {
		Model* selected = nullptr;
		if (action == "first") selected = facade.firstModel();
		else if (action == "last") selected = facade.lastModel();
		else if (action == "next") selected = facade.nextModel();
		else selected = facade.previousModel();
		if (selected != nullptr) {
			cout << selected->showLanguage() << endl;
		} else {
			cout << "No model available" << endl;
		}
		return;
	}
	if (action == "can-go-next") {
		cout << (facade.canGoNextModel() ? "true" : "false") << endl;
		return;
	}
	if (action == "can-go-previous") {
		cout << (facade.canGoPreviousModel() ? "true" : "false") << endl;
		return;
	}
	if (action == "at") {
		if (_typedWords->size() != 3) {
			cout<<"Wrong number of parameters"<<endl;
			return;
		}
		unsigned int index = 0;
		if (!parseUnsigned(_typedWords->at(2), index)) {
			cout << "Error: " << _typedWords->at(2) << " is not a valid index" << endl;
			return;
		}
		Model* selected = facade.modelAt(index);
		if (selected != nullptr) {
			cout << selected->showLanguage() << endl;
		} else {
			cout << "Model not found" << endl;
		}
		return;
	}
	if (action == "future-events") {
		List<Event*>* events = facade.modelGetFutureEvents();
		if (events == nullptr || events->size() == 0) {
			cout << "No future events" << endl;
			return;
		}
		for (Event* event : *events->list()) {
			if (event != nullptr) {
				cout << event->show() << endl;
			}
		}
		return;
	}
	if (action == "controls") {
		List<SimulationControl*>* controls = facade.modelGetControls();
		if (controls == nullptr || controls->size() == 0) {
			cout << "No simulation controls" << endl;
			return;
		}
		for (SimulationControl* control : *controls->list()) {
			if (control != nullptr) {
				cout << control->show() << endl;
			}
		}
		return;
	}
	if (action == "responses") {
		List<SimulationResponse*>* responses = facade.modelGetResponses();
		if (responses == nullptr || responses->size() == 0) {
			cout << "No simulation responses" << endl;
			return;
		}
		for (SimulationResponse* response : *responses->list()) {
			if (response != nullptr) {
				cout << response->show() << endl;
			}
		}
		return;
	}
	if (action == "persistence") {
		cout << (facade.modelGetPersistence() != nullptr ? "present" : "not set") << endl;
		return;
	}
	if (action == "level") {
		cout << facade.modelGetLevel() << endl;
		return;
	}
	cout<<"Syntax error on "<<_typedWords->at(1)<<endl;
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
		} else if (parameter == "get") {
			if (i + 2 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			const std::string type = _typedWords->at(i + 1);
			const std::string key = _typedWords->at(i + 2);
			ModelDataDefinition* item = nullptr;
			unsigned int id = 0;
			if (parseUnsigned(key, id)) {
				item = facade.dataGetDataDefinition(type, id);
			} else {
				item = facade.dataGetDataDefinition(type, key);
			}
			if (item != nullptr) {
				cout << item->show() << endl;
			} else {
				cout << "Data definition not found" << endl;
			}
			return;
		} else if (parameter == "rank-of") {
			if (i + 2 >= _typedWords->size()) {
				cout << "Wrong number of parameters" << endl;
				return;
			}
			cout << facade.dataGetRankOf(_typedWords->at(i + 1), _typedWords->at(i + 2)) << endl;
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
		} else if (parameter == "front") {
			ModelComponent* component = facade.componentFront();
			if (component != nullptr) {
				cout << component->show() << endl;
			} else {
				cout << "No component available" << endl;
			}
			return;
		} else if (parameter == "next") {
			ModelComponent* component = facade.componentNext();
			if (component != nullptr) {
				cout << component->show() << endl;
			} else {
				cout << "No next component" << endl;
			}
			return;
		} else if (parameter == "all") {
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
