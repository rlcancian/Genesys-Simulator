/*
 * File:   SystemDependencyResolver.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 16 de Abril de 2026
 */

#include "SystemDependencyResolver.h"

#include <array>
#include <cstdio>
#include <sstream>
#include <utility>

#ifdef _WIN32
#define GENESYS_POPEN _popen
#define GENESYS_PCLOSE _pclose
#else
#include <sys/wait.h>
#define GENESYS_POPEN popen
#define GENESYS_PCLOSE pclose
#endif

bool SystemCommandResult::succeeded() const {
	return started && exitCode == 0;
}

SystemCommandResult SystemShellCommandExecutor::run(const std::string& command) {
	SystemCommandResult result;
	if (command.empty()) {
		result.errorMessage = "Empty command.";
		return result;
	}

	// Capture stderr together with stdout so the GUI and trace log can explain failed checks.
	const std::string commandWithStderr = command + " 2>&1";
	FILE* pipe = GENESYS_POPEN(commandWithStderr.c_str(), "r");
	if (pipe == nullptr) {
		result.errorMessage = "Could not start command.";
		return result;
	}

	result.started = true;
	std::array<char, 256> buffer{};
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
		result.output += buffer.data();
	}
	const int closeStatus = GENESYS_PCLOSE(pipe);
#ifdef _WIN32
	result.exitCode = closeStatus;
#else
	result.exitCode = WIFEXITED(closeStatus) ? WEXITSTATUS(closeStatus) : closeStatus;
#endif
	return result;
}

SystemDependencyCheckEntry::SystemDependencyCheckEntry(SystemDependency dependency, Status status, SystemCommandResult checkResult, std::string message)
	: _dependency(std::move(dependency)),
	  _status(status),
	  _checkResult(std::move(checkResult)),
	  _message(std::move(message)) {
}

const SystemDependency& SystemDependencyCheckEntry::dependency() const {
	return _dependency;
}

SystemDependencyCheckEntry::Status SystemDependencyCheckEntry::status() const {
	return _status;
}

const SystemCommandResult& SystemDependencyCheckEntry::checkResult() const {
	return _checkResult;
}

const std::string& SystemDependencyCheckEntry::message() const {
	return _message;
}

bool SystemDependencyCheckEntry::blocksInsertion() const {
	return _status == Status::Missing || _status == Status::NotVerifiable;
}

bool SystemDependencyCheckEntry::canAttemptInstall() const {
	return _status == Status::Missing && !_dependency.getInstallCommand().empty();
}

std::string SystemDependencyCheckEntry::diagnosticText() const {
	std::ostringstream text;
	text << "Dependency: " << _dependency.getName() << "\n";
	text << "  OS: " << SystemDependency::osToString(_dependency.getOS()) << "\n";
	text << "  Status: " << statusToString(_status) << "\n";
	text << "  Check command: " << (_dependency.getCheckCommand().empty() ? "<not declared>" : _dependency.getCheckCommand()) << "\n";
	text << "  Install command: " << (_dependency.getInstallCommand().empty() ? "<not declared>" : _dependency.getInstallCommand()) << "\n";

	// Command details are preserved so headless callers can explain what failed without rerunning commands.
	if (_checkResult.started) {
		text << "  Check exit code: " << _checkResult.exitCode << "\n";
		if (!_checkResult.output.empty()) {
			text << "  Check output: " << _checkResult.output;
			if (_checkResult.output.back() != '\n') {
				text << "\n";
			}
		}
	}
	if (!_checkResult.errorMessage.empty()) {
		text << "  Check error: " << _checkResult.errorMessage << "\n";
	}
	if (!_message.empty()) {
		text << "  Diagnostic: " << _message << "\n";
	}
	return text.str();
}

std::string SystemDependencyCheckEntry::statusToString(Status status) {
	switch (status) {
		case Status::Satisfied:
			return "satisfied";
		case Status::Missing:
			return "missing";
		case Status::NotVerifiable:
			return "not verifiable";
		case Status::IgnoredDifferentOS:
			return "ignored for current OS";
	}
	return "unknown";
}

void SystemDependencyCheckResult::add(SystemDependencyCheckEntry entry) {
	_entries.push_back(std::move(entry));
}

const std::list<SystemDependencyCheckEntry>& SystemDependencyCheckResult::entries() const {
	return _entries;
}

bool SystemDependencyCheckResult::canInsertPlugin() const {
	return !hasBlockingEntries();
}

bool SystemDependencyCheckResult::hasBlockingEntries() const {
	for (const SystemDependencyCheckEntry& entry : _entries) {
		if (entry.blocksInsertion()) {
			return true;
		}
	}
	return false;
}

bool SystemDependencyCheckResult::canAttemptInstallForAllMissing() const {
	for (const SystemDependencyCheckEntry& entry : _entries) {
		if (entry.status() == SystemDependencyCheckEntry::Status::Missing && !entry.canAttemptInstall()) {
			return false;
		}
		if (entry.status() == SystemDependencyCheckEntry::Status::NotVerifiable) {
			return false;
		}
	}
	return true;
}

std::string SystemDependencyCheckResult::summary() const {
	std::ostringstream text;
	bool first = true;
	for (const SystemDependencyCheckEntry& entry : _entries) {
		if (!first) {
			text << "; ";
		}
		first = false;
		text << entry.dependency().getName() << "=" << SystemDependencyCheckEntry::statusToString(entry.status());
		if (!entry.message().empty()) {
			text << " (" << entry.message() << ")";
		}
	}
	return text.str();
}

std::string SystemDependencyCheckResult::diagnosticText(bool includeSatisfied) const {
	std::ostringstream text;
	bool first = true;
	for (const SystemDependencyCheckEntry& entry : _entries) {
		if (!includeSatisfied && !entry.blocksInsertion()) {
			continue;
		}
		if (!first) {
			text << "\n";
		}
		first = false;
		text << entry.diagnosticText();
	}
	if (first) {
		return summary();
	}
	return text.str();
}

SystemDependencyInstallEntry::SystemDependencyInstallEntry(SystemDependency dependency, SystemCommandResult installResult)
	: _dependency(std::move(dependency)),
	  _installResult(std::move(installResult)) {
}

const SystemDependency& SystemDependencyInstallEntry::dependency() const {
	return _dependency;
}

const SystemCommandResult& SystemDependencyInstallEntry::installResult() const {
	return _installResult;
}

std::string SystemDependencyInstallEntry::diagnosticText() const {
	std::ostringstream text;
	text << "Dependency: " << _dependency.getName() << "\n";
	text << "  Install command: " << (_dependency.getInstallCommand().empty() ? "<not declared>" : _dependency.getInstallCommand()) << "\n";
	text << "  Install started: " << (_installResult.started ? "yes" : "no") << "\n";
	text << "  Install exit code: " << _installResult.exitCode << "\n";
	if (!_installResult.output.empty()) {
		text << "  Install output: " << _installResult.output;
		if (_installResult.output.back() != '\n') {
			text << "\n";
		}
	}
	if (!_installResult.errorMessage.empty()) {
		text << "  Install error: " << _installResult.errorMessage << "\n";
	}
	return text.str();
}

void SystemDependencyInstallResult::add(SystemDependencyInstallEntry entry) {
	_entries.push_back(std::move(entry));
}

const std::list<SystemDependencyInstallEntry>& SystemDependencyInstallResult::entries() const {
	return _entries;
}

bool SystemDependencyInstallResult::succeeded() const {
	for (const SystemDependencyInstallEntry& entry : _entries) {
		if (!entry.installResult().succeeded()) {
			return false;
		}
	}
	return true;
}

std::string SystemDependencyInstallResult::summary() const {
	std::ostringstream text;
	bool first = true;
	for (const SystemDependencyInstallEntry& entry : _entries) {
		if (!first) {
			text << "; ";
		}
		first = false;
		text << entry.dependency().getName() << " install exit=" << entry.installResult().exitCode;
	}
	return text.str();
}

std::string SystemDependencyInstallResult::diagnosticText() const {
	std::ostringstream text;
	bool first = true;
	for (const SystemDependencyInstallEntry& entry : _entries) {
		if (!first) {
			text << "\n";
		}
		first = false;
		text << entry.diagnosticText();
	}
	return first ? summary() : text.str();
}

SystemDependency::OS SystemDependencyResolver::currentOS() {
#if defined(_WIN32)
	return SystemDependency::OS::Windows;
#elif defined(__APPLE__) && defined(__MACH__)
	return SystemDependency::OS::MacOS;
#elif defined(__linux__)
	return SystemDependency::OS::Linux;
#else
	return SystemDependency::OS::Unknown;
#endif
}

bool SystemDependencyResolver::appliesToCurrentOS(const SystemDependency& dependency) {
	const SystemDependency::OS dependencyOS = dependency.getOS();
	const SystemDependency::OS hostOS = currentOS();

	// OS::Any is intentionally broad; OS-specific declarations are ignored on other hosts.
	return dependencyOS == SystemDependency::OS::Any || dependencyOS == hostOS;
}

SystemDependencyCheckResult SystemDependencyResolver::evaluate(const std::list<SystemDependency>* dependencies,
                                                               SystemCommandExecutor_if& executor) {
	SystemDependencyCheckResult result;
	if (dependencies == nullptr) {
		return result;
	}

	for (const SystemDependency& dependency : *dependencies) {
		// Dependencies for a different OS are preserved in diagnostics but never executed.
		if (!appliesToCurrentOS(dependency)) {
			result.add(SystemDependencyCheckEntry(
				dependency,
				SystemDependencyCheckEntry::Status::IgnoredDifferentOS,
				{},
				"Declared for " + SystemDependency::osToString(dependency.getOS()) + "."));
			continue;
		}

		if (dependency.getCheckCommand().empty()) {
			result.add(SystemDependencyCheckEntry(
				dependency,
				SystemDependencyCheckEntry::Status::NotVerifiable,
				{},
				"No check command was declared."));
			continue;
		}

		// A zero exit code is the contract for a satisfied check command.
		SystemCommandResult check = executor.run(dependency.getCheckCommand());
		const bool satisfied = check.succeeded();
		result.add(SystemDependencyCheckEntry(
			dependency,
			satisfied ? SystemDependencyCheckEntry::Status::Satisfied : SystemDependencyCheckEntry::Status::Missing,
			check,
			satisfied ? "Check command succeeded." : "Check command failed."));
	}
	return result;
}

SystemDependencyInstallResult SystemDependencyResolver::installMissingDependencies(const SystemDependencyCheckResult& checkResult,
                                                                                  SystemCommandExecutor_if& executor) {
	SystemDependencyInstallResult result;
	for (const SystemDependencyCheckEntry& entry : checkResult.entries()) {
		if (!entry.canAttemptInstall()) {
			continue;
		}

		// Installation is intentionally separate from evaluation; callers must ask the user first.
		result.add(SystemDependencyInstallEntry(
			entry.dependency(),
			executor.run(entry.dependency().getInstallCommand())));
	}
	return result;
}
