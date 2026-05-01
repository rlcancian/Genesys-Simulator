/*
 * File:   GroProgram.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"
#include "kernel/simulator/Model.h"

#include <fstream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GroProgram::GetPluginInformation;
}
#endif

namespace {

std::string buildVisibleColonyStarterGroProgram() {
	return R"(include gro;
set ( "dt", 0.18 );
set ( "population_max", 256 );

wave := signal ( 0.85, 0.035 );
REST := 0;
PULSE := 1;

program leader() := {
  p.phase := REST;
  p.timer := 0;

  true : {
    p.timer := p.timer + dt,
    volume := volume + 0.05,
    size := volume,
    speed := 0.22,
    gfp := 30 + 18 * volume,
    direction := direction + 0.08,
    emit_signal ( wave, 8 ),
    rfp := 6 + 8 * get_signal ( wave )
  }

  p.phase = REST & p.timer > 4 : {
    p.phase := PULSE,
    p.timer := 0
  }

  p.phase = PULSE : { emit_signal ( wave, 22 ) }

  p.phase = PULSE & p.timer > 1.2 : {
    p.phase := REST,
    p.timer := 0
  }

  volume > 2.2 : {
    divide(),
    volume := 1.1
  }
};

program follower() := {
  p.t := 0;
  sense := get_signal ( wave );

  true : {
    p.t := p.t + dt,
    volume := volume + 0.035 + 0.02 * sense,
    size := volume,
    speed := 0.08 + 0.12 * sense,
    yfp := 12 + 24 * sense,
    cfp := 8 + 14 * p.t
  }

  sense > 0.08 : {
    direction := direction + 0.22,
    emit_signal ( wave, 6 )
  }

  sense > 0.20 : {
    rfp := 24 + 40 * sense
  }

  volume > 2.05 : {
    divide(),
    volume := 1.0
  }
};

ecoli ( [ x := 16, y := 16 ], program leader() );
ecoli ( [ x := 13, y := 15 ], program follower() );
ecoli ( [ x := 19, y := 17 ], program follower() );
ecoli ( [ x := 15, y := 20 ], program follower() );
ecoli ( [ x := 18, y := 13 ], program follower() );

program main() := {
  skip();
};
)";
}

} // namespace

ModelDataDefinition* GroProgram::NewInstance(Model* model, std::string name) {
	return new GroProgram(model, name);
}

GroProgram::GroProgram(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<GroProgram>(), name) {
	SimulationControlSourceCodeString* propSourceCode = new SimulationControlSourceCodeString(
			std::bind(&GroProgram::getSourceCodeProperty, this),
			std::bind(&GroProgram::setSourceCodeProperty, this, std::placeholders::_1),
			Util::TypeOf<GroProgram>(), getName(), "SourceCode",
			"Gro source code associated with this reusable program",
			false,
			false,
			false,
			std::bind(&GroProgram::_validateSourceCodeSyntax, this, std::placeholders::_1, std::placeholders::_2));
	_parentModel->getControls()->insert(propSourceCode);
	_addSimulationControl(propSourceCode);
	// Keep new GroProgram instances immediately usable in the GUI with a
	// starter that visibly emits signals, moves, and divides.
	_sourceCode = buildVisibleColonyStarterGroProgram();
}

std::string GroProgram::show() {
	return ModelDataDefinition::show() + ", sourceLength=" + std::to_string(_sourceCode.size());
}

PluginInformation* GroProgram::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GroProgram>(), &GroProgram::LoadInstance,
	                                                &GroProgram::NewInstance);
	info->setCategory("BiologicalModeling");
	info->setDescriptionHelp("Stores reusable Gro source code for biological simulation components. "
	                         "The default starter now seeds a small signaling colony so GUI work can "
	                         "show motion, division, diffusion, and fluorescence without extra setup.");
	return info;
}

ModelDataDefinition* GroProgram::LoadInstance(Model* model, PersistenceRecord* fields) {
	GroProgram* newElement = new GroProgram(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

void GroProgram::setSourceCode(std::string sourceCode) {
	_sourceCode = sourceCode;
}

void GroProgram::setSourceCodeProperty(SourceCodeString sourceCode) {
	_sourceCode = sourceCode.str();
}

std::string GroProgram::getSourceCode() const {
	return _sourceCode;
}

SourceCodeString GroProgram::getSourceCodeProperty() const {
	return SourceCodeString(_sourceCode);
}

bool GroProgram::createDefaultGroProgram(const std::string& filename) {
	// Keep the model-side source synchronized with the visible colony starter
	// even when the caller does not request an external file yet.
	_sourceCode = buildVisibleColonyStarterGroProgram();

	if (filename.empty()) {
		return true;
	}

	std::ofstream output(filename, std::ios::out | std::ios::trunc);
	if (!output.is_open()) {
		return false;
	}

	output << _sourceCode;
	return output.good();
}

bool GroProgram::validateSyntax(std::string& errorMessage) const {
	// An empty source is a valid "not created yet" state for GUI-driven flows.
	return _validateSourceCodeSyntax(_sourceCode, errorMessage);
}

bool GroProgram::_validateSourceCodeSyntax(const std::string& sourceCode, std::string& errorMessage) const {
	// An empty source is a valid "not created yet" state for GUI-driven flows.
	if (sourceCode.empty()) {
		errorMessage.clear();
		return true;
	}

	errorMessage.clear();
	const GroProgramParser::Result result = GroProgramParser().parse(sourceCode);
	if (!result.accepted) {
		errorMessage = result.errorMessage;
	}
	return result.accepted;
}

bool GroProgram::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_sourceCode = fields->loadField("sourceCode", DEFAULT.sourceCode);
	}
	return res;
}

void GroProgram::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("sourceCode", _sourceCode, DEFAULT.sourceCode, saveDefaultValues);
}

bool GroProgram::_check(std::string& errorMessage) {
	return validateSyntax(errorMessage);
}

void GroProgram::_createInternalStatisticReporters() {
}

void GroProgram::_createEditableDataDefinitions() {
}

void GroProgram::_createAttachedAttributes() {
}
