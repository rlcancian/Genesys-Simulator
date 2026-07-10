#ifndef PYTHONRUNTIME_H
#define PYTHONRUNTIME_H

#include <string>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

class Entity;
class ModelComponent;

class PythonRuntime : public ModelDataDefinition {
public:
	PythonRuntime(Model* model, std::string name = "");
	virtual ~PythonRuntime() override;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setPythonExecutable(std::string pythonExecutable);
	std::string getPythonExecutable() const;
	void setFacadeObjectName(std::string facadeObjectName);
	std::string getFacadeObjectName() const;
	void setCaptureOutput(bool captureOutput);
	bool isCaptureOutput() const;
	void setLastStatus(std::string lastStatus);
	std::string getLastStatus() const;
	void setLastErrorMessage(std::string lastErrorMessage);
	std::string getLastErrorMessage() const;
	void setLastStdout(std::string lastStdout);
	std::string getLastStdout() const;
	void setLastStderr(std::string lastStderr);
	std::string getLastStderr() const;

	bool validateHooks(const std::string& initCode,
	                   const std::string& onDispatchCode,
	                   std::string& errorMessage) const;
	bool executeInitHook(ModelComponent* component,
	                     const std::string& initCode,
	                     const std::string& onDispatchCode,
	                     std::string& errorMessage);
	bool executeOnDispatchHook(ModelComponent* component,
	                           Entity* entity,
	                           const std::string& initCode,
	                           const std::string& onDispatchCode,
	                           std::string& errorMessage);
	void resetExecutionState();

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	bool _validateIdentifier(std::string& errorMessage) const;
	void _invalidatePreparedHooks();
	bool _prepareHooks(ModelComponent* component,
	                   const std::string& initCode,
	                   const std::string& onDispatchCode,
	                   std::string& errorMessage);

private:
	const struct DEFAULT_VALUES {
		std::string pythonExecutable = "python3";
		std::string facadeObjectName = "simulator";
		bool captureOutput = true;
		std::string lastStatus = "Idle";
		std::string lastErrorMessage = "";
		std::string lastStdout = "";
		std::string lastStderr = "";
	} DEFAULT;

	std::string _pythonExecutable = DEFAULT.pythonExecutable;
	std::string _facadeObjectName = DEFAULT.facadeObjectName;
	bool _captureOutput = DEFAULT.captureOutput;
	std::string _lastStatus = DEFAULT.lastStatus;
	std::string _lastErrorMessage = DEFAULT.lastErrorMessage;
	std::string _lastStdout = DEFAULT.lastStdout;
	std::string _lastStderr = DEFAULT.lastStderr;
	std::string _preparedSignature = "";

#if GENESYS_HAS_PYTHON_INTEGRATION
	void* _preparedGlobals = nullptr;
	void* _preparedInitFunction = nullptr;
	void* _preparedDispatchFunction = nullptr;
	void* _preparedFacadeObject = nullptr;
#endif
};

#endif /* PYTHONRUNTIME_H */
