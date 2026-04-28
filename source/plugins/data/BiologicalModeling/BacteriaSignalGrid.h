/*
 * File:   BacteriaSignalGrid.h
 * Author: GRO
 *
 * Created on 28 de Abril de 2026
 */

#ifndef BACTERIASIGNALGRID_H
#define BACTERIASIGNALGRID_H

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Plugin.h"

#include <string>
#include <vector>

/*!
 * \brief Reusable discrete signal-field definition for bacteria colonies.
 *
 * The data definition persists the spatial configuration and initial scalar
 * values of one signal field. Colonies copy this configuration into their
 * runtime state so model configuration and simulation mutation stay separated.
 */
class BacteriaSignalGrid : public ModelDataDefinition {
public:
	BacteriaSignalGrid(Model* model, std::string name = "");
	virtual ~BacteriaSignalGrid() override = default;

public:
	virtual std::string show() override;

public: // static plugin interface
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWidth(unsigned int width);
	unsigned int getWidth() const;
	void setHeight(unsigned int height);
	unsigned int getHeight() const;
	void setInitialSignal(double initialSignal);
	double getInitialSignal() const;
	void setDiffusionRate(double diffusionRate);
	double getDiffusionRate() const;
	void setDecayRate(double decayRate);
	double getDecayRate() const;
	void setInitialValues(std::string initialValues);
	std::string getInitialValues() const;
	unsigned int getCellCount() const;
	bool buildInitialField(std::vector<double>& values, std::string& errorMessage) const;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		const unsigned int width = 1;
		const unsigned int height = 1;
		const double initialSignal = 0.0;
		const double diffusionRate = 0.0;
		const double decayRate = 0.0;
		const std::string initialValues = "";
	} DEFAULT;

	unsigned int _width = DEFAULT.width;
	unsigned int _height = DEFAULT.height;
	double _initialSignal = DEFAULT.initialSignal;
	double _diffusionRate = DEFAULT.diffusionRate;
	double _decayRate = DEFAULT.decayRate;
	std::string _initialValues = DEFAULT.initialValues;
};

#endif /* BACTERIASIGNALGRID_H */
