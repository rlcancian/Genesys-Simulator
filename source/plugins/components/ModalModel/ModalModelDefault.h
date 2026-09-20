/*
 * File:    ModalModelDefault.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 01 de Julho de 2025, 14:26
 */

#pragma once

#include "../../../kernel/simulator/model/ModelComponent.h"
#include <string>
#include <vector>

class DefaultNetwork;
class NetworkActivationResult;

/*!
 * Process-world adapter that activates an attached `DefaultNetwork`.
 *
 * Network topology and formalism state live in the attached network. This
 * component only maps entity arrivals to network activations and routes
 * network outputs back into the process model.
 */
class ModalModelDefault : public ModelComponent {
public:
	ModalModelDefault(Model* model, std::string name = "");
	virtual ~ModalModelDefault() override = default;

public:
	virtual void setMaxTransitionsPerDispatch(unsigned int maxTransitionsPerDispatch);
	virtual unsigned int getMaxTransitionsPerDispatch() const;
	std::string getTimeDelayExpressionPerDispatch();
	void setTimeDelayExpressionPerDispatch(const std::string time_delay_expression_per_dispatch);
	Util::TimeUnit getTimeDelayPerDispatchTimeUnit();
	void setTimeDelayPerDispatchTimeUnit(const Util::TimeUnit time_delay_per_dispatch_time_unit);
	DefaultNetwork* getNetwork() const;
	void setNetwork(DefaultNetwork* network);
	std::string getNetworkName() const;
	void setNetworkName(const std::string& networkName);
	void setInputBinding(unsigned int port, const std::string& expression);
	std::string getInputBinding(unsigned int port) const;
	void setOutputBinding(unsigned int port, const std::string& attributeName);
	std::string getOutputBinding(unsigned int port) const;

public:
	virtual void addOutputExpressionReference(ModelDataDefinition* expressionReference);

public:
	virtual std::string show() override;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createAttachedAttributes() override;

protected:
	const struct DEFAULT_VALUES {
		const std::string timeDelayExpressionPerDispatch = "0";
		const Util::TimeUnit timeDelayPerDispatchTimeUnit = Util::TimeUnit::second;
		const unsigned int maxTransitionsPerDispatch = 1;
	} DEFAULT;
	unsigned int _maxTransitionsPerDispatch = DEFAULT.maxTransitionsPerDispatch;
	std::string _timeDelayExpressionPerDispatch = DEFAULT.timeDelayExpressionPerDispatch;
	Util::TimeUnit _timeDelayPerDispatchTimeUnit = DEFAULT.timeDelayPerDispatchTimeUnit;

private:
	DefaultNetwork* _resolveNetworkReference();
	void _syncBindingsToNetwork();
	bool _dispatchNetworkActivation(Entity* entity, unsigned int inputPortNumber);
	Entity* _cloneEntity(Entity* entity);
	void _writeOutputBinding(Entity* entity, unsigned int outputPort, double value);
	void _routeNetworkOutputs(Entity* entity, const NetworkActivationResult& result);

private:
	DefaultNetwork* _network = nullptr;
	std::string _networkName = "";
	std::vector<std::string> _inputBindings;
	std::vector<std::string> _outputBindings;
};
