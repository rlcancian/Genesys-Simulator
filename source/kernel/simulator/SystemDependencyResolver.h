/*
 * File:   SystemDependencyResolver.h
 * Author: rafael.luiz.cancian
 *
 * Created on 16 de Abril de 2026
 */

#ifndef SYSTEMDEPENDENCYRESOLVER_H
#define SYSTEMDEPENDENCYRESOLVER_H

#include "PluginInformation.h"

#include <list>
#include <string>

/*!
 * \brief Captures the result of executing a system command used by dependency checks.
 */
struct SystemCommandResult {
	/*! \brief True when the command process was started by the executor. */
	bool started = false;
	/*! \brief Process exit code, or -1 when the command could not be started. */
	int exitCode = -1;
	/*! \brief Combined standard output and diagnostic text produced by the command. */
	std::string output = "";
	/*! \brief Internal executor error message, when starting or collecting the command failed. */
	std::string errorMessage = "";

	/*! \brief Returns true when the command started and exited successfully. */
	bool succeeded() const;
};

/*!
 * \brief Minimal command execution interface used to make system dependency checks testable.
 */
class SystemCommandExecutor_if {
public:
	/*! \brief Destroys the command executor abstraction. */
	virtual ~SystemCommandExecutor_if() = default;
	/*! \brief Runs a shell command and returns its observable result. */
	virtual SystemCommandResult run(const std::string& command) = 0;
};

/*!
 * \brief Shell-based executor used by production plugin dependency preflight.
 */
class SystemShellCommandExecutor : public SystemCommandExecutor_if {
public:
	/*! \brief Executes a command through the host shell and captures combined output. */
	SystemCommandResult run(const std::string& command) override;
};

/*!
 * \brief One dependency entry produced by a system dependency preflight.
 */
class SystemDependencyCheckEntry {
public:
	/*! \brief Classification assigned to one declared dependency after preflight. */
	enum class Status {
		Satisfied,
		Missing,
		NotVerifiable,
		IgnoredDifferentOS
	};

	/*! \brief Builds a preflight entry for one declared dependency. */
	SystemDependencyCheckEntry(SystemDependency dependency, Status status, SystemCommandResult checkResult = {}, std::string message = "");

	/*! \brief Returns the dependency declaration evaluated by this entry. */
	const SystemDependency& dependency() const;
	/*! \brief Returns the preflight status for this dependency. */
	Status status() const;
	/*! \brief Returns the result of the check command, when one was executed. */
	const SystemCommandResult& checkResult() const;
	/*! \brief Returns human-readable diagnostic context for this entry. */
	const std::string& message() const;
	/*! \brief Returns true when this entry should prevent plugin insertion. */
	bool blocksInsertion() const;
	/*! \brief Returns true when this missing dependency has an installation command. */
	bool canAttemptInstall() const;
	/*! \brief Returns a multi-line diagnostic with check/install commands and command results. */
	std::string diagnosticText() const;
	/*! \brief Converts a status value into a stable textual representation. */
	static std::string statusToString(Status status);

private:
	SystemDependency _dependency;
	Status _status = Status::NotVerifiable;
	SystemCommandResult _checkResult;
	std::string _message = "";
};

/*!
 * \brief Full preflight result for the system dependencies declared by one plugin.
 */
class SystemDependencyCheckResult {
public:
	/*! \brief Adds one dependency evaluation entry. */
	void add(SystemDependencyCheckEntry entry);
	/*! \brief Returns every evaluated dependency entry, including ignored entries. */
	const std::list<SystemDependencyCheckEntry>& entries() const;
	/*! \brief Returns true when no applicable dependency blocks insertion. */
	bool canInsertPlugin() const;
	/*! \brief Returns true when at least one dependency blocks insertion. */
	bool hasBlockingEntries() const;
	/*! \brief Returns true when every blocking missing dependency has an installation command. */
	bool canAttemptInstallForAllMissing() const;
	/*! \brief Returns a compact diagnostic summary suitable for trace logs. */
	std::string summary() const;
	/*! \brief Returns a multi-line diagnostic suitable for terminal, GUI and trace details. */
	std::string diagnosticText(bool includeSatisfied = false) const;

private:
	std::list<SystemDependencyCheckEntry> _entries;
};

/*!
 * \brief One command result from an attempted system dependency installation.
 */
class SystemDependencyInstallEntry {
public:
	/*! \brief Builds an installation result for a single dependency. */
	SystemDependencyInstallEntry(SystemDependency dependency, SystemCommandResult installResult);
	/*! \brief Returns the dependency whose install command was executed. */
	const SystemDependency& dependency() const;
	/*! \brief Returns the result produced by the install command. */
	const SystemCommandResult& installResult() const;
	/*! \brief Returns command, exit code and captured output for trace or GUI feedback. */
	std::string diagnosticText() const;

private:
	SystemDependency _dependency;
	SystemCommandResult _installResult;
};

/*!
 * \brief Aggregates commands executed while trying to satisfy missing system dependencies.
 */
class SystemDependencyInstallResult {
public:
	/*! \brief Adds one installation command result. */
	void add(SystemDependencyInstallEntry entry);
	/*! \brief Returns all attempted installation command results. */
	const std::list<SystemDependencyInstallEntry>& entries() const;
	/*! \brief Returns true when every attempted installation command succeeded. */
	bool succeeded() const;
	/*! \brief Returns a compact diagnostic summary suitable for trace logs. */
	std::string summary() const;
	/*! \brief Returns detailed diagnostic text for every executed install command. */
	std::string diagnosticText() const;

private:
	std::list<SystemDependencyInstallEntry> _entries;
};

/*!
 * \brief Evaluates and optionally installs plugin system dependencies.
 *
 * The resolver deliberately contains no user-interface code. It only decides
 * whether dependencies apply to the current OS, runs declared check/install
 * commands through an executor, and reports diagnostics to the caller.
 */
class SystemDependencyResolver {
public:
	/*! \brief Returns the operating system represented by the current executable. */
	static SystemDependency::OS currentOS();
	/*! \brief Returns true when a dependency declaration applies to the current OS. */
	static bool appliesToCurrentOS(const SystemDependency& dependency);
	/*! \brief Evaluates all dependency declarations using check commands when available. */
	static SystemDependencyCheckResult evaluate(const std::list<SystemDependency>* dependencies,
	                                            SystemCommandExecutor_if& executor);
	/*! \brief Executes install commands for missing dependencies from a preflight result. */
	static SystemDependencyInstallResult installMissingDependencies(const SystemDependencyCheckResult& checkResult,
	                                                               SystemCommandExecutor_if& executor);
};

#endif /* SYSTEMDEPENDENCYRESOLVER_H */
