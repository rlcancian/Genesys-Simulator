/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Record.h
 * Author: rafael.luiz.cancian
 *
 * Created on 9 de Agosto de 2018, 13:52
 */

#ifndef RECORD_H
#define RECORD_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include <string>

/*!
 * \brief Evaluates a parser expression each time an entity passes through
 * and records the value into a StatisticsCollector and/or an external file.
 *
 * Arena correspondence: the "Record module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 41-42), which offers
 * five Types: Count, Entity Statistics, Time Interval, Time Between, and
 * Expression, plus optional Tally/Counter Sets.
 *
 * Only the Expression Type is implemented — there is no \c Type field at
 * all. GenESyS extends this single mode with dataset-export metadata that
 * has no Arena counterpart (\c _datasetName, \c _randomVariableName,
 * \c _variableType, \c _description) and direct file recording
 * (\c _filename, \c _timeDependent) intended for external tooling such as
 * the Data Analyser.
 *
 * Known differences from Arena: Count, Entity Statistics, Time Interval and
 * Time Between have no equivalent Type here, and there is no Tally/Counter
 * Set support ("Record into Set"). This does not mean statistics collection
 * itself is unavailable in GenESyS — \c Counter and \c StatisticsCollector
 * are used directly by several other components/data definitions — only
 * that this specific component only reaches the Expression case.
 */
class Record : public ModelComponent {
public:
	Record(Model* model, std::string name = "");
	virtual ~Record();

public:
	void setFilename(std::string filename);
	std::string getFileName() const;
	void setExpression(const std::string expression);
	std::string getExpression() const;
	/*!
	 Sets the legacy expression-name metadata kept for old Record datasets.
	 */
	void setExpressionName(std::string expressionName);
	std::string getExpressionName() const;
	/*!
	 Sets the analytical dataset name emitted in enriched Record file headers.
	 */
	void setDatasetName(std::string datasetName);
	std::string getDatasetName() const;
	/*!
	 Sets the random variable name emitted in enriched Record file headers.
	 */
	void setRandomVariableName(std::string randomVariableName);
	std::string getRandomVariableName() const;
	/*!
	 Sets the canonical random variable type for enriched Record file headers.
	 */
	void setVariableType(std::string variableType);
	std::string getVariableType() const;
	/*!
	 Sets the free-text dataset description emitted in enriched Record file headers.
	 */
	void setDatasetDescription(std::string description);
	std::string getDatasetDescription() const;
	StatisticsCollector* getCstatExpression() const;	
	bool getTimeDependent() const;
	void setTimeDependent(bool timeDependent);

public:
	virtual std::string show() override;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

protected:
	virtual void _initBetweenReplications() override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;


protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	const struct DEFAULT_VALUES {
		const bool timeDependent = false;
		const std::string expression = "";
		const std::string expressionName = "";
		const std::string datasetName = "";
		const std::string randomVariableName = "";
		const std::string variableType = "Continuous numeric";
		const std::string description = "";
		const std::string filename = "";
	} DEFAULT;
	bool _timeDependent = DEFAULT.timeDependent;
	std::string _expression = DEFAULT.expression;
	std::string _expressionName = DEFAULT.expressionName;
	std::string _datasetName = DEFAULT.datasetName;
	std::string _randomVariableName = DEFAULT.randomVariableName;
	std::string _variableType = DEFAULT.variableType;
	std::string _description = DEFAULT.description;
	std::string _filename = DEFAULT.filename;
private:
	const std::string _separator = " ";
	StatisticsCollector* _cstatExpression = nullptr;
	/* @TODO: Create an internal class to agregate ExpressionStatisticsColelctor, and change Record to got a list of it, so Record can record a set of expressions into a set of files */
};

#endif /* RECORD_H */
