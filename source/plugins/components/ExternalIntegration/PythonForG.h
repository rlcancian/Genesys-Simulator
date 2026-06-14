#ifndef PYTHONFORG_H
#define PYTHONFORG_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/SimulationControlAndResponse.h"
#include "plugins/data/ExternalIntegration/PythonRuntime.h"

class PythonForG : public ModelComponent {
public:
	PythonForG(Model* model, std::string name = "");
	virtual ~PythonForG() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setInitBetweenReplicationCode(std::string initBetweenReplicationCode);
	std::string getInitBetweenReplicationCode() const;
	void setInitBetweenReplicationCodeProperty(SourceCodeString initBetweenReplicationCode);
	SourceCodeString getInitBetweenReplicationCodeProperty() const;
	void setOnDispatchEventCode(std::string onDispatchEventCode);
	std::string getOnDispatchEventCode() const;
	void setOnDispatchEventCodeProperty(SourceCodeString onDispatchEventCode);
	SourceCodeString getOnDispatchEventCodeProperty() const;
	void setForwardEntityOnError(bool forwardEntityOnError);
	bool isForwardEntityOnError() const;
	PythonRuntime* getPythonRuntime() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createNonEditableDataDefinitions() override;

private:
	bool _validateInitCode(const std::string& candidate, std::string& errorMessage) const;
	bool _validateDispatchCode(const std::string& candidate, std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		std::string initBetweenReplicationCode = "";
		std::string onDispatchEventCode = "";
		bool forwardEntityOnError = true;
	} DEFAULT;

	std::string _initBetweenReplicationCode = DEFAULT.initBetweenReplicationCode;
	std::string _onDispatchEventCode = DEFAULT.onDispatchEventCode;
	bool _forwardEntityOnError = DEFAULT.forwardEntityOnError;
	PythonRuntime* _pythonRuntime = nullptr;
};

#endif /* PYTHONFORG_H */
