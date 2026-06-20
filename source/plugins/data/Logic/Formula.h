/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.h
 * Author: rlcancian
 *
 * Created on 20 de Junho de 2019, 00:56
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/Plugin.h"
//#include "kernel/simulator/Parser_if.h"

class Formula : public ModelDataDefinition {
public:
	/*!
	 * \brief Creates an empty formula definition.
	 * \param model Parent model.
	 * \param name Initial formula name.
	 */
	Formula(Model* model, std::string name = "");
	/*!
	 * \brief Releases formula-owned resources.
	 */
	virtual ~Formula() = default;
public: // virtual
	/*!
	 * \brief Returns a textual representation of the formula.
	 * \return Human-readable formula summary.
	 */
	virtual std::string show() override;
public:
	/*!
	 * \brief Returns the number of indexed expressions stored in this formula.
	 * \return Number of expression slots.
	 */
	unsigned int size();
	/*!
	 * \brief Stores or updates the expression associated with an index.
	 * \param formulaExpression Formula text.
	 * \param index Optional index key used for vectorized formulas.
	 */
	void setExpression(std::string formulaExpression, std::string index = "");
	//void setExpression(std::string formulaExpression);
	/*!
	 * \brief Returns the expression associated with an index.
	 * \param index Optional index key used for vectorized formulas.
	 * \return Stored formula text, or an empty string when absent.
	 */
	std::string getExpression(std::string index = "");
	//std::string getExpression();
	//double getValue();
	/*!
	 * \brief Evaluates the expression associated with an index.
	 * \param index Optional index key used for vectorized formulas.
	 * \return Numeric value of the indexed expression.
	 */
	double getValue(std::string index);
public: // statics
	/*!
	 * \brief Returns plugin metadata for this formula type.
	 * \return Plugin information block.
	 */
	static PluginInformation* GetPluginInformation();
	/*!
	 * \brief Loads a formula from serialized fields.
	 * \param model Parent model.
	 * \param fields Serialized fields.
	 * \return Newly created formula instance.
	 */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	/*!
	 * \brief Creates a new empty formula instance.
	 * \param model Parent model.
	 * \param name Initial formula name.
	 * \return Newly created formula instance.
	 */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	//virtual void _addSimulationControl(SimulationControl* property);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private:
	std::map<std::string, std::string>* _formulaExpressions = new std::map<std::string, std::string>(); // map<index,formula>
private:
	///*static*/ Parser_if* _myPrivateParser;
};

#endif /* FORMULA_H */
