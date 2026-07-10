/*
 * File:   DiffusionSimulate.h
 * Author: GenESyS DCS (Tema 8.2)
 *
 * Component that advances a DiffusionField (continuous N-D diffusion) when
 * entities arrive, mirroring the BioSimulate -> BioNetwork pattern.
 */

#ifndef DIFFUSIONSIMULATE_H
#define DIFFUSIONSIMULATE_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/Continuous/DiffusionField.h"

/*!
 * \brief Component that triggers a DiffusionField solve when entities arrive.
 *
 * DiffusionSimulate delegates the continuous diffusion solve to a referenced
 * DiffusionField (a ModelDataDefinition in plugins/data/Continuous). Each arriving
 * entity runs the field either with its own configured time window or with
 * component-provided start/stop/step values, then forwards the entity. This is the
 * discrete->continuous bridge: a discrete-event component drives the continuous PDE
 * subsystem. For event-clock-synchronized advance, enable AutoSchedule on the
 * DiffusionField instead, so it steps once per kernel InternalEvent.
 */
class DiffusionSimulate : public ModelComponent {
public:
	DiffusionSimulate(Model* model, std::string name = "");
	virtual ~DiffusionSimulate() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setDiffusionField(DiffusionField* diffusionField);
	DiffusionField* getDiffusionField() const;
	void setUseFieldTimeWindow(bool useFieldTimeWindow);
	bool getUseFieldTimeWindow() const;
	void setStartTime(double startTime);
	double getStartTime() const;
	void setStopTime(double stopTime);
	double getStopTime() const;
	void setStepSize(double stepSize);
	double getStepSize() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	virtual void _createEditableDataDefinitions() override;

private:
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string diffusionFieldName = "";
		bool useFieldTimeWindow = true;
		double startTime = 0.0;
		double stopTime = 1.0;
		double stepSize = 0.1;
		bool lastSucceeded = false;
		std::string lastMessage = "";
	} DEFAULT;

	DiffusionField* _diffusionField = nullptr;
	bool _useFieldTimeWindow = DEFAULT.useFieldTimeWindow;
	double _startTime = DEFAULT.startTime;
	double _stopTime = DEFAULT.stopTime;
	double _stepSize = DEFAULT.stepSize;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* DIFFUSIONSIMULATE_H */
