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
#include <string>

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
	/*! \brief Creates a small starter Gro program and optionally writes it to a file. */
	bool createDefaultGroProgram(const std::string& filename = "");
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
	bool _validateSourceCodeSyntax(const std::string& sourceCode, std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		const std::string sourceCode = R"(
include gro
set ("dt",0.1);
s := signal (1,1.05);
k0 := 2.5;  // base rate of oscillation
kb := 5;    // cell-cell feedback strength
tr := 10;   // refractory period length
se := 650;  // signal emit magnitude
GO := 0;
WAIT := 1;
program oscillator(g0) := {
  gfp := 0.5*volume*g0;
  p := [mode := GO, t := 0, x := g0];
  true : {gfp := 0.5*volume*(g0 + p.x/150)}
  // advance the oscillation phase
  p.mode = GO & rate (k0) : {p.x := p.x + 0.01*(150-p.x)}
  p.mode = GO & rate(kb*get_signal(s)) : {p.x := p.x + 0.01*(150-p.x)}
  // phase reset
  p.x > 100 : {
    p.x := 0,
    p.mode := WAIT,
    emit_signal(s, se)
  }
  // refractory period timer
  p.mode = WAIT : {p.t := p.t+dt}
  p.mode = WAIT & p.t > tr : {
    p.mode := GO,
    p.t := 0
  }
};
ecoli ( [x:=0, y:=0], program oscillator(1));
program main() := {
  //n := 0;
  //true : {snapshot("movie/oscillator/" <> tostring(n) <> ".tif"), n:=n+1}
  skip();
};
)";

	} DEFAULT;

	std::string _sourceCode = DEFAULT.sourceCode;
};

#endif /* GROPROGRAM_H */
