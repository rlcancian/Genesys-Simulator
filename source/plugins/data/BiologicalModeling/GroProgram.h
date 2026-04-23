/*
 * File:   GroProgram.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAM_H
#define GROPROGRAM_H

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Plugin.h"

/*!
 * \brief Reusable Gro source program attached to a GenESyS model.
 *
 * GroProgram is intentionally a ModelDataDefinition so several biological
 * components can share the same Gro source text. This first implementation only
 * stores the source and performs a small lexical sanity check; complete Gro
 * parsing and executable semantics belong to later plugin-side helper classes.
 */
class GroProgram : public ModelDataDefinition {
public:
	GroProgram(Model* model, std::string name = "");
	virtual ~GroProgram() override = default;

public:
	virtual std::string show() override;

public: // static plugin interface
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Replaces the stored Gro source text. */
	void setSourceCode(std::string sourceCode);
	/*! \brief Returns the stored Gro source text. */
	std::string getSourceCode() const;
	/*! \brief Performs a permissive lexical sanity check for the stored source. */
	bool validateSyntax(std::string& errorMessage) const;

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
		const std::string sourceCode = "";
	} DEFAULT;

	std::string _sourceCode = DEFAULT.sourceCode;
};

#endif /* GROPROGRAM_H */
