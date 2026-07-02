/*
 * File:   ODESolver.h
 * Author: GenESyS
 *
 * Created on 13 de maio de 2026
 */

#ifndef ODESOLVER_H
#define ODESOLVER_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "tools/OdeSolver_if.h"
#include "tools/OdeSystem_if.h"
#include "tools/RungeKutta4OdeSolver.h"

class ODESolver : public ModelDataDefinition {
public:
	ODESolver(Model* model, std::string name = "");
	virtual ~ODESolver() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	static void SaveInstance(PersistenceRecord *fields, ModelDataDefinition* dataDef);
	static bool Check(ModelDataDefinition* dataDef, std::string& errorMessage);
	static void InitBetweenReplications(ModelDataDefinition* dataDef);
	static void CreateInternalData(ModelDataDefinition* dataDef);
public:
	virtual std::string show() override;
public: // gets & sets
	void setTimeVariableName(std::string timeVariableName);
	std::string getTimeVariableName() const;
	void setStateVariableNames(std::vector<std::string> stateVariableNames);
	std::vector<std::string> getStateVariableNames() const;
	void setEquationExpressions(std::vector<std::string> equationExpressions);
	std::vector<std::string> getEquationExpressions() const;
	void setStep(double step);
	double getStep() const;
	void setPrecision(double precision);
	double getPrecision() const;
	void setMaxSteps(int maxSteps);
	int getMaxSteps() const;
	void setCurrentTime(double currentTime);
	double getCurrentTime() const;
	void setStateValues(std::vector<double> stateValues);
	std::vector<double> getStateValues() const;
	void setInitialStateValues(std::vector<double> initialStateValues);
	std::vector<double> getInitialStateValues() const;
public: // new methods
	void integrate(double targetTime);
	void resetState();
	double getStateValue(unsigned int index) const;
	void setStateValue(unsigned int index, double value);
	void synchronizeStateFromVariables();
	void writeStateToVariables() const;

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createEditableDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		const std::string timeVariableName = "t";
		const double step = 0.01;
		const double precision = 1e-6;
		const int maxSteps = 100000;
		const double currentTime = 0.0;
	} DEFAULT;
	std::string _timeVariableName = DEFAULT.timeVariableName;
	std::vector<std::string> _stateVariableNames;
	std::vector<std::string> _equationExpressions;
	double _step = DEFAULT.step;
	double _precision = DEFAULT.precision;
	int _maxSteps = DEFAULT.maxSteps;
	double _currentTime = DEFAULT.currentTime;
	std::vector<double> _stateValues;
	std::vector<double> _initialStateValues;
	RungeKutta4OdeSolver _solver;

	std::vector<double> _loadStateVector(PersistenceRecord* fields, const std::string& prefix) const;
	void _saveStateVector(PersistenceRecord* fields,
	                      const std::string& prefix,
	                      const std::vector<double>& values,
	                      bool saveDefaultValues) const;
	double _readVariableValue(const std::string& variableName) const;
};

#endif /* ODESOLVER_H */
