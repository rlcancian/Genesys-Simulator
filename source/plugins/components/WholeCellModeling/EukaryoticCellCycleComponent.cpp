#include "plugins/components/WholeCellModeling/EukaryoticCellCycleComponent.h"

#include <algorithm>
#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &EukaryoticCellCycleComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* EukaryoticCellCycleComponent::NewInstance(Model* model, std::string name) {
	return new EukaryoticCellCycleComponent(model, name);
}

EukaryoticCellCycleComponent::EukaryoticCellCycleComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<EukaryoticCellCycleComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&EukaryoticCellCycleComponent::getWholeCellState, this),
			std::bind(&EukaryoticCellCycleComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "WholeCellState", "");
	auto* propDeltaT = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getDeltaT, this),
			std::bind(&EukaryoticCellCycleComponent::setDeltaT, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "DeltaT", "");
	auto* propAdvanceClock = new SimulationControlBool(
			std::bind(&EukaryoticCellCycleComponent::getAdvanceWholeCellClock, this),
			std::bind(&EukaryoticCellCycleComponent::setAdvanceWholeCellClock, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "AdvanceWholeCellClock", "");
	auto* propCytosol = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getCytosolCompartmentName, this),
			std::bind(&EukaryoticCellCycleComponent::setCytosolCompartmentName, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "CytosolCompartmentName", "");
	auto* propNucleus = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getNucleusCompartmentName, this),
			std::bind(&EukaryoticCellCycleComponent::setNucleusCompartmentName, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "NucleusCompartmentName", "");
	auto* propMito = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getMitochondrionCompartmentName, this),
			std::bind(&EukaryoticCellCycleComponent::setMitochondrionCompartmentName, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitochondrionCompartmentName", "");
	auto* propBud = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getBudCompartmentName, this),
			std::bind(&EukaryoticCellCycleComponent::setBudCompartmentName, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudCompartmentName", "");
	auto* propEnergyKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getEnergyMetaboliteKey, this),
			std::bind(&EukaryoticCellCycleComponent::setEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "EnergyMetaboliteKey", "");
	auto* propCytEnergyKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getCytosolicEnergyMetaboliteKey, this),
			std::bind(&EukaryoticCellCycleComponent::setCytosolicEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "CytosolicEnergyMetaboliteKey", "");
	auto* propMitoEnergyKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getMitochondrialEnergyMetaboliteKey, this),
			std::bind(&EukaryoticCellCycleComponent::setMitochondrialEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitochondrialEnergyMetaboliteKey", "");
	auto* propBudEnergyKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getBudEnergyMetaboliteKey, this),
			std::bind(&EukaryoticCellCycleComponent::setBudEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudEnergyMetaboliteKey", "");
	auto* propGrowthFluxKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getGrowthFluxKey, this),
			std::bind(&EukaryoticCellCycleComponent::setGrowthFluxKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "GrowthFluxKey", "");
	auto* propRespFluxKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getRespirationFluxKey, this),
			std::bind(&EukaryoticCellCycleComponent::setRespirationFluxKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "RespirationFluxKey", "");
	auto* propBudProgressKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getBudProgressKey, this),
			std::bind(&EukaryoticCellCycleComponent::setBudProgressKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudProgressKey", "");
	auto* propDnaProgressKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getDnaReplicationProgressKey, this),
			std::bind(&EukaryoticCellCycleComponent::setDnaReplicationProgressKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "DnaReplicationProgressKey", "");
	auto* propSpindleProgressKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getSpindleAssemblyProgressKey, this),
			std::bind(&EukaryoticCellCycleComponent::setSpindleAssemblyProgressKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "SpindleAssemblyProgressKey", "");
	auto* propMitoticExitProgressKey = new SimulationControlString(
			std::bind(&EukaryoticCellCycleComponent::getMitoticExitProgressKey, this),
			std::bind(&EukaryoticCellCycleComponent::setMitoticExitProgressKey, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitoticExitProgressKey", "");
	auto* propGlobalAtpThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getGlobalAtpThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setGlobalAtpThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "GlobalAtpThreshold", "");
	auto* propCytAtpThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getCytosolicAtpThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setCytosolicAtpThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "CytosolicAtpThreshold", "");
	auto* propMitoAtpThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getMitochondrialAtpThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setMitochondrialAtpThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitochondrialAtpThreshold", "");
	auto* propBudAtpThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getBudAtpThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setBudAtpThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudAtpThreshold", "");
	auto* propGrowthFluxThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getGrowthFluxThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setGrowthFluxThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "GrowthFluxThreshold", "");
	auto* propRespFluxThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getRespirationFluxThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setRespirationFluxThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "RespirationFluxThreshold", "");
	auto* propBudRate = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getBudProgressRate, this),
			std::bind(&EukaryoticCellCycleComponent::setBudProgressRate, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudProgressRate", "");
	auto* propDnaRate = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getDnaReplicationRate, this),
			std::bind(&EukaryoticCellCycleComponent::setDnaReplicationRate, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "DnaReplicationRate", "");
	auto* propSpindleRate = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getSpindleAssemblyRate, this),
			std::bind(&EukaryoticCellCycleComponent::setSpindleAssemblyRate, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "SpindleAssemblyRate", "");
	auto* propMitoticExitRate = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getMitoticExitRate, this),
			std::bind(&EukaryoticCellCycleComponent::setMitoticExitRate, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitoticExitRate", "");
	auto* propBudThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getBudProgressThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setBudProgressThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "BudProgressThreshold", "");
	auto* propDnaThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getDnaReplicationThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setDnaReplicationThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "DnaReplicationThreshold", "");
	auto* propSpindleThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getSpindleAssemblyThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setSpindleAssemblyThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "SpindleAssemblyThreshold", "");
	auto* propMitoticExitThreshold = new SimulationControlDouble(
			std::bind(&EukaryoticCellCycleComponent::getMitoticExitThreshold, this),
			std::bind(&EukaryoticCellCycleComponent::setMitoticExitThreshold, this, std::placeholders::_1),
			Util::TypeOf<EukaryoticCellCycleComponent>(), getName(), "MitoticExitThreshold", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propDeltaT);
	_parentModel->getControls()->insert(propAdvanceClock);
	_parentModel->getControls()->insert(propCytosol);
	_parentModel->getControls()->insert(propNucleus);
	_parentModel->getControls()->insert(propMito);
	_parentModel->getControls()->insert(propBud);
	_parentModel->getControls()->insert(propEnergyKey);
	_parentModel->getControls()->insert(propCytEnergyKey);
	_parentModel->getControls()->insert(propMitoEnergyKey);
	_parentModel->getControls()->insert(propBudEnergyKey);
	_parentModel->getControls()->insert(propGrowthFluxKey);
	_parentModel->getControls()->insert(propRespFluxKey);
	_parentModel->getControls()->insert(propBudProgressKey);
	_parentModel->getControls()->insert(propDnaProgressKey);
	_parentModel->getControls()->insert(propSpindleProgressKey);
	_parentModel->getControls()->insert(propMitoticExitProgressKey);
	_parentModel->getControls()->insert(propGlobalAtpThreshold);
	_parentModel->getControls()->insert(propCytAtpThreshold);
	_parentModel->getControls()->insert(propMitoAtpThreshold);
	_parentModel->getControls()->insert(propBudAtpThreshold);
	_parentModel->getControls()->insert(propGrowthFluxThreshold);
	_parentModel->getControls()->insert(propRespFluxThreshold);
	_parentModel->getControls()->insert(propBudRate);
	_parentModel->getControls()->insert(propDnaRate);
	_parentModel->getControls()->insert(propSpindleRate);
	_parentModel->getControls()->insert(propMitoticExitRate);
	_parentModel->getControls()->insert(propBudThreshold);
	_parentModel->getControls()->insert(propDnaThreshold);
	_parentModel->getControls()->insert(propSpindleThreshold);
	_parentModel->getControls()->insert(propMitoticExitThreshold);

	_addSimulationControl(propState);
	_addSimulationControl(propDeltaT);
	_addSimulationControl(propAdvanceClock);
	_addSimulationControl(propCytosol);
	_addSimulationControl(propNucleus);
	_addSimulationControl(propMito);
	_addSimulationControl(propBud);
	_addSimulationControl(propEnergyKey);
	_addSimulationControl(propCytEnergyKey);
	_addSimulationControl(propMitoEnergyKey);
	_addSimulationControl(propBudEnergyKey);
	_addSimulationControl(propGrowthFluxKey);
	_addSimulationControl(propRespFluxKey);
	_addSimulationControl(propBudProgressKey);
	_addSimulationControl(propDnaProgressKey);
	_addSimulationControl(propSpindleProgressKey);
	_addSimulationControl(propMitoticExitProgressKey);
	_addSimulationControl(propGlobalAtpThreshold);
	_addSimulationControl(propCytAtpThreshold);
	_addSimulationControl(propMitoAtpThreshold);
	_addSimulationControl(propBudAtpThreshold);
	_addSimulationControl(propGrowthFluxThreshold);
	_addSimulationControl(propRespFluxThreshold);
	_addSimulationControl(propBudRate);
	_addSimulationControl(propDnaRate);
	_addSimulationControl(propSpindleRate);
	_addSimulationControl(propMitoticExitRate);
	_addSimulationControl(propBudThreshold);
	_addSimulationControl(propDnaThreshold);
	_addSimulationControl(propSpindleThreshold);
	_addSimulationControl(propMitoticExitThreshold);
}

PluginInformation* EukaryoticCellCycleComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<EukaryoticCellCycleComponent>(), &EukaryoticCellCycleComponent::LoadInstance, &EukaryoticCellCycleComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Coordinates a didactic eukaryotic cell cycle using WholeCellState, compartment-specific energy pools, and progress variables for budding, DNA replication, spindle assembly, and mitotic exit.");
	return info;
}

ModelComponent* EukaryoticCellCycleComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	EukaryoticCellCycleComponent* newComponent = new EukaryoticCellCycleComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load EukaryoticCellCycleComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string EukaryoticCellCycleComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",deltaT=" + std::to_string(_deltaT) +
			",cytosolCompartmentName=\"" + _cytosolCompartmentName + "\"" +
			",nucleusCompartmentName=\"" + _nucleusCompartmentName + "\"" +
			",mitochondrionCompartmentName=\"" + _mitochondrionCompartmentName + "\"" +
			",budCompartmentName=\"" + _budCompartmentName + "\"" +
			",energyMetaboliteKey=\"" + _energyMetaboliteKey + "\"" +
			",growthFluxKey=\"" + _growthFluxKey + "\"" +
			",respirationFluxKey=\"" + _respirationFluxKey + "\"";
}

bool EukaryoticCellCycleComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_deltaT = fields->loadField("deltaT", DEFAULT.deltaT);
		_advanceWholeCellClock = fields->loadField("advanceWholeCellClock", DEFAULT.advanceWholeCellClock);
		_cytosolCompartmentName = fields->loadField("cytosolCompartmentName", DEFAULT.cytosolCompartmentName);
		_nucleusCompartmentName = fields->loadField("nucleusCompartmentName", DEFAULT.nucleusCompartmentName);
		_mitochondrionCompartmentName = fields->loadField("mitochondrionCompartmentName", DEFAULT.mitochondrionCompartmentName);
		_budCompartmentName = fields->loadField("budCompartmentName", DEFAULT.budCompartmentName);
		_energyMetaboliteKey = fields->loadField("energyMetaboliteKey", DEFAULT.energyMetaboliteKey);
		_cytosolicEnergyMetaboliteKey = fields->loadField("cytosolicEnergyMetaboliteKey", DEFAULT.cytosolicEnergyMetaboliteKey);
		_mitochondrialEnergyMetaboliteKey = fields->loadField("mitochondrialEnergyMetaboliteKey", DEFAULT.mitochondrialEnergyMetaboliteKey);
		_budEnergyMetaboliteKey = fields->loadField("budEnergyMetaboliteKey", DEFAULT.budEnergyMetaboliteKey);
		_growthFluxKey = fields->loadField("growthFluxKey", DEFAULT.growthFluxKey);
		_respirationFluxKey = fields->loadField("respirationFluxKey", DEFAULT.respirationFluxKey);
		_budProgressKey = fields->loadField("budProgressKey", DEFAULT.budProgressKey);
		_dnaReplicationProgressKey = fields->loadField("dnaReplicationProgressKey", DEFAULT.dnaReplicationProgressKey);
		_spindleAssemblyProgressKey = fields->loadField("spindleAssemblyProgressKey", DEFAULT.spindleAssemblyProgressKey);
		_mitoticExitProgressKey = fields->loadField("mitoticExitProgressKey", DEFAULT.mitoticExitProgressKey);
		_globalAtpThreshold = fields->loadField("globalAtpThreshold", DEFAULT.globalAtpThreshold);
		_cytosolicAtpThreshold = fields->loadField("cytosolicAtpThreshold", DEFAULT.cytosolicAtpThreshold);
		_mitochondrialAtpThreshold = fields->loadField("mitochondrialAtpThreshold", DEFAULT.mitochondrialAtpThreshold);
		_budAtpThreshold = fields->loadField("budAtpThreshold", DEFAULT.budAtpThreshold);
		_growthFluxThreshold = fields->loadField("growthFluxThreshold", DEFAULT.growthFluxThreshold);
		_respirationFluxThreshold = fields->loadField("respirationFluxThreshold", DEFAULT.respirationFluxThreshold);
		_budProgressRate = fields->loadField("budProgressRate", DEFAULT.budProgressRate);
		_dnaReplicationRate = fields->loadField("dnaReplicationRate", DEFAULT.dnaReplicationRate);
		_spindleAssemblyRate = fields->loadField("spindleAssemblyRate", DEFAULT.spindleAssemblyRate);
		_mitoticExitRate = fields->loadField("mitoticExitRate", DEFAULT.mitoticExitRate);
		_budProgressThreshold = fields->loadField("budProgressThreshold", DEFAULT.budProgressThreshold);
		_dnaReplicationThreshold = fields->loadField("dnaReplicationThreshold", DEFAULT.dnaReplicationThreshold);
		_spindleAssemblyThreshold = fields->loadField("spindleAssemblyThreshold", DEFAULT.spindleAssemblyThreshold);
		_mitoticExitThreshold = fields->loadField("mitoticExitThreshold", DEFAULT.mitoticExitThreshold);
	}
	return res;
}

void EukaryoticCellCycleComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("deltaT", _deltaT, DEFAULT.deltaT, saveDefaultValues);
	fields->saveField("advanceWholeCellClock", _advanceWholeCellClock, DEFAULT.advanceWholeCellClock, saveDefaultValues);
	fields->saveField("cytosolCompartmentName", _cytosolCompartmentName, DEFAULT.cytosolCompartmentName, saveDefaultValues);
	fields->saveField("nucleusCompartmentName", _nucleusCompartmentName, DEFAULT.nucleusCompartmentName, saveDefaultValues);
	fields->saveField("mitochondrionCompartmentName", _mitochondrionCompartmentName, DEFAULT.mitochondrionCompartmentName, saveDefaultValues);
	fields->saveField("budCompartmentName", _budCompartmentName, DEFAULT.budCompartmentName, saveDefaultValues);
	fields->saveField("energyMetaboliteKey", _energyMetaboliteKey, DEFAULT.energyMetaboliteKey, saveDefaultValues);
	fields->saveField("cytosolicEnergyMetaboliteKey", _cytosolicEnergyMetaboliteKey, DEFAULT.cytosolicEnergyMetaboliteKey, saveDefaultValues);
	fields->saveField("mitochondrialEnergyMetaboliteKey", _mitochondrialEnergyMetaboliteKey, DEFAULT.mitochondrialEnergyMetaboliteKey, saveDefaultValues);
	fields->saveField("budEnergyMetaboliteKey", _budEnergyMetaboliteKey, DEFAULT.budEnergyMetaboliteKey, saveDefaultValues);
	fields->saveField("growthFluxKey", _growthFluxKey, DEFAULT.growthFluxKey, saveDefaultValues);
	fields->saveField("respirationFluxKey", _respirationFluxKey, DEFAULT.respirationFluxKey, saveDefaultValues);
	fields->saveField("budProgressKey", _budProgressKey, DEFAULT.budProgressKey, saveDefaultValues);
	fields->saveField("dnaReplicationProgressKey", _dnaReplicationProgressKey, DEFAULT.dnaReplicationProgressKey, saveDefaultValues);
	fields->saveField("spindleAssemblyProgressKey", _spindleAssemblyProgressKey, DEFAULT.spindleAssemblyProgressKey, saveDefaultValues);
	fields->saveField("mitoticExitProgressKey", _mitoticExitProgressKey, DEFAULT.mitoticExitProgressKey, saveDefaultValues);
	fields->saveField("globalAtpThreshold", _globalAtpThreshold, DEFAULT.globalAtpThreshold, saveDefaultValues);
	fields->saveField("cytosolicAtpThreshold", _cytosolicAtpThreshold, DEFAULT.cytosolicAtpThreshold, saveDefaultValues);
	fields->saveField("mitochondrialAtpThreshold", _mitochondrialAtpThreshold, DEFAULT.mitochondrialAtpThreshold, saveDefaultValues);
	fields->saveField("budAtpThreshold", _budAtpThreshold, DEFAULT.budAtpThreshold, saveDefaultValues);
	fields->saveField("growthFluxThreshold", _growthFluxThreshold, DEFAULT.growthFluxThreshold, saveDefaultValues);
	fields->saveField("respirationFluxThreshold", _respirationFluxThreshold, DEFAULT.respirationFluxThreshold, saveDefaultValues);
	fields->saveField("budProgressRate", _budProgressRate, DEFAULT.budProgressRate, saveDefaultValues);
	fields->saveField("dnaReplicationRate", _dnaReplicationRate, DEFAULT.dnaReplicationRate, saveDefaultValues);
	fields->saveField("spindleAssemblyRate", _spindleAssemblyRate, DEFAULT.spindleAssemblyRate, saveDefaultValues);
	fields->saveField("mitoticExitRate", _mitoticExitRate, DEFAULT.mitoticExitRate, saveDefaultValues);
	fields->saveField("budProgressThreshold", _budProgressThreshold, DEFAULT.budProgressThreshold, saveDefaultValues);
	fields->saveField("dnaReplicationThreshold", _dnaReplicationThreshold, DEFAULT.dnaReplicationThreshold, saveDefaultValues);
	fields->saveField("spindleAssemblyThreshold", _spindleAssemblyThreshold, DEFAULT.spindleAssemblyThreshold, saveDefaultValues);
	fields->saveField("mitoticExitThreshold", _mitoticExitThreshold, DEFAULT.mitoticExitThreshold, saveDefaultValues);
}

bool EukaryoticCellCycleComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "EukaryoticCellCycleComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_deltaT <= 0.0) {
		errorMessage += "EukaryoticCellCycleComponent \"" + getName() + "\" deltaT must be > 0. ";
		resultAll = false;
	}
	if (_budProgressKey.empty() || _dnaReplicationProgressKey.empty() ||
	    _spindleAssemblyProgressKey.empty() || _mitoticExitProgressKey.empty()) {
		errorMessage += "EukaryoticCellCycleComponent \"" + getName() + "\" requires non-empty progress keys. ";
		resultAll = false;
	}
	const double numericValues[] = {
		_globalAtpThreshold, _cytosolicAtpThreshold, _mitochondrialAtpThreshold, _budAtpThreshold,
		_growthFluxThreshold, _respirationFluxThreshold, _budProgressRate, _dnaReplicationRate,
		_spindleAssemblyRate, _mitoticExitRate, _budProgressThreshold, _dnaReplicationThreshold,
		_spindleAssemblyThreshold, _mitoticExitThreshold
	};
	for (double value : numericValues) {
		if (value < 0.0) {
			errorMessage += "EukaryoticCellCycleComponent \"" + getName() + "\" requires all thresholds and rates to be >= 0. ";
			resultAll = false;
			break;
		}
	}
	resultAll = _validateReferencedCompartment(_cytosolCompartmentName, &errorMessage) && resultAll;
	resultAll = _validateReferencedCompartment(_nucleusCompartmentName, &errorMessage) && resultAll;
	resultAll = _validateReferencedCompartment(_mitochondrionCompartmentName, &errorMessage) && resultAll;
	resultAll = _validateReferencedCompartment(_budCompartmentName, &errorMessage) && resultAll;
	_createEditableDataDefinitions();
	return resultAll;
}

void EukaryoticCellCycleComponent::_initBetweenReplications() {
	if (_wholeCellState == nullptr) {
		return;
	}
	_wholeCellState->setPathwayActivity(_budProgressKey, 0.0);
	_wholeCellState->setPathwayActivity(_dnaReplicationProgressKey, 0.0);
	_wholeCellState->setPathwayActivity(_spindleAssemblyProgressKey, 0.0);
	_wholeCellState->setPathwayActivity(_mitoticExitProgressKey, 0.0);
}

void EukaryoticCellCycleComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"EukaryoticCellCycleComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}
	if (_advanceWholeCellClock) {
		_wholeCellState->setCurrentTime(_wholeCellState->getCurrentTime() + _deltaT);
		_wholeCellState->incrementStep();
	}
	_updateLifecyclePhase();
	_forwardEntity(entity);
}

void EukaryoticCellCycleComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
	const std::pair<const char*, std::string> compartments[] = {
		{"CytosolCompartment", _cytosolCompartmentName},
		{"NucleusCompartment", _nucleusCompartmentName},
		{"MitochondrionCompartment", _mitochondrionCompartmentName},
		{"BudCompartment", _budCompartmentName}
	};
	for (const auto& entry : compartments) {
		auto* def = _findCompartment(entry.second);
		if (def != nullptr) {
			_optionalEditableDataDefinitionInsert(entry.first, def);
		} else {
			_optionalEditableDataDefinitionRemove(entry.first);
		}
	}
}

double EukaryoticCellCycleComponent::_normalizedSignal(double value, double threshold) const {
	if (threshold <= 0.0) {
		return value > 0.0 ? 1.0 : 0.0;
	}
	return std::clamp(value / threshold, 0.0, 1.5);
}

double EukaryoticCellCycleComponent::_averageSignal(double a, double b) const {
	return 0.5 * (a + b);
}

double EukaryoticCellCycleComponent::_advanceProgress(const std::string& key, double increment) const {
	const double current = _wholeCellState->getPathwayActivity(key);
	const double next = std::max(0.0, current + increment);
	_wholeCellState->setPathwayActivity(key, next);
	return next;
}

void EukaryoticCellCycleComponent::_updateLifecyclePhase() {
	if (!_wholeCellState->isViable()) {
		_wholeCellState->setLifecyclePhase("dead");
		return;
	}

	const double globalAtp = _energyMetaboliteKey.empty() ? 0.0 : _wholeCellState->getMetaboliteAmount(_energyMetaboliteKey);
	const double cytAtp = (!_cytosolCompartmentName.empty() && !_cytosolicEnergyMetaboliteKey.empty())
			? _wholeCellState->getCompartmentMetaboliteAmount(_cytosolCompartmentName, _cytosolicEnergyMetaboliteKey) : 0.0;
	const double mitoAtp = (!_mitochondrionCompartmentName.empty() && !_mitochondrialEnergyMetaboliteKey.empty())
			? _wholeCellState->getCompartmentMetaboliteAmount(_mitochondrionCompartmentName, _mitochondrialEnergyMetaboliteKey) : 0.0;
	const double budAtp = (!_budCompartmentName.empty() && !_budEnergyMetaboliteKey.empty())
			? _wholeCellState->getCompartmentMetaboliteAmount(_budCompartmentName, _budEnergyMetaboliteKey) : 0.0;
	const double growthFlux = _growthFluxKey.empty() ? 0.0 : _wholeCellState->getPathwayActivity(_growthFluxKey);
	const double respirationFlux = _respirationFluxKey.empty() ? 0.0 : _wholeCellState->getPathwayActivity(_respirationFluxKey);

	const double globalEnergySupport = _normalizedSignal(globalAtp, _globalAtpThreshold);
	const double cytEnergySupport = _normalizedSignal(cytAtp, _cytosolicAtpThreshold);
	const double mitoEnergySupport = _normalizedSignal(mitoAtp, _mitochondrialAtpThreshold);
	const double budEnergySupport = _normalizedSignal(budAtp, _budAtpThreshold);
	const double growthSupport = _normalizedSignal(growthFlux, _growthFluxThreshold);
	const double respirationSupport = _normalizedSignal(respirationFlux, _respirationFluxThreshold);

	if (globalEnergySupport < 1.0 || cytEnergySupport < 1.0 || growthSupport < 1.0) {
		_wholeCellState->setLifecyclePhase("arrested");
		return;
	}

	if (_wholeCellState->getGenerationCount() == 0 && _wholeCellState->getStepCount() <= 1) {
		_wholeCellState->setLifecyclePhase("newborn");
	}

	const double budProgress = _advanceProgress(_budProgressKey, _budProgressRate * _averageSignal(cytEnergySupport, growthSupport));
	if (budProgress < _budProgressThreshold) {
		_wholeCellState->setLifecyclePhase("g1_budding");
		return;
	}

	const double dnaProgress = _advanceProgress(_dnaReplicationProgressKey, _dnaReplicationRate * _averageSignal(globalEnergySupport, cytEnergySupport));
	if (dnaProgress < _dnaReplicationThreshold) {
		_wholeCellState->setLifecyclePhase("s_phase");
		return;
	}
	if (respirationSupport < 1.0 || mitoEnergySupport < 1.0) {
		_wholeCellState->setLifecyclePhase("arrested");
		return;
	}

	const double spindleProgress = _advanceProgress(_spindleAssemblyProgressKey, _spindleAssemblyRate * _averageSignal(respirationSupport, mitoEnergySupport));
	if (spindleProgress < _spindleAssemblyThreshold) {
		_wholeCellState->setLifecyclePhase("g2_phase");
		return;
	}
	if (budEnergySupport < 1.0) {
		_wholeCellState->setLifecyclePhase("arrested");
		return;
	}

	const double mitoticExitProgress = _advanceProgress(_mitoticExitProgressKey, _mitoticExitRate * _averageSignal(budEnergySupport, growthSupport));
	if (mitoticExitProgress < _mitoticExitThreshold) {
		_wholeCellState->setLifecyclePhase("m_phase");
		return;
	}

	_wholeCellState->setLifecyclePhase("division_ready");
}

void EukaryoticCellCycleComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "EukaryoticCellCycleComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

bool EukaryoticCellCycleComponent::_validateReferencedCompartment(const std::string& name, std::string* errorMessage) const {
	if (name.empty()) {
		return true;
	}
	if (_findCompartment(name) != nullptr) {
		return true;
	}
	if (errorMessage != nullptr) {
		*errorMessage += "EukaryoticCellCycleComponent \"" + getName() + "\" requires BioCompartment \"" + name + "\". ";
	}
	return false;
}

BioCompartment* EukaryoticCellCycleComponent::_findCompartment(const std::string& name) const {
	if (name.empty()) {
		return nullptr;
	}
	auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioCompartment>(), name);
	return dynamic_cast<BioCompartment*>(def);
}

void EukaryoticCellCycleComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* EukaryoticCellCycleComponent::getWholeCellState() const { return _wholeCellState; }
void EukaryoticCellCycleComponent::setDeltaT(double deltaT) { _deltaT = deltaT; }
double EukaryoticCellCycleComponent::getDeltaT() const { return _deltaT; }
void EukaryoticCellCycleComponent::setAdvanceWholeCellClock(bool advance) { _advanceWholeCellClock = advance; }
bool EukaryoticCellCycleComponent::getAdvanceWholeCellClock() const { return _advanceWholeCellClock; }
void EukaryoticCellCycleComponent::setCytosolCompartmentName(std::string name) { _cytosolCompartmentName = std::move(name); }
std::string EukaryoticCellCycleComponent::getCytosolCompartmentName() const { return _cytosolCompartmentName; }
void EukaryoticCellCycleComponent::setNucleusCompartmentName(std::string name) { _nucleusCompartmentName = std::move(name); }
std::string EukaryoticCellCycleComponent::getNucleusCompartmentName() const { return _nucleusCompartmentName; }
void EukaryoticCellCycleComponent::setMitochondrionCompartmentName(std::string name) { _mitochondrionCompartmentName = std::move(name); }
std::string EukaryoticCellCycleComponent::getMitochondrionCompartmentName() const { return _mitochondrionCompartmentName; }
void EukaryoticCellCycleComponent::setBudCompartmentName(std::string name) { _budCompartmentName = std::move(name); }
std::string EukaryoticCellCycleComponent::getBudCompartmentName() const { return _budCompartmentName; }
void EukaryoticCellCycleComponent::setEnergyMetaboliteKey(std::string key) { _energyMetaboliteKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getEnergyMetaboliteKey() const { return _energyMetaboliteKey; }
void EukaryoticCellCycleComponent::setCytosolicEnergyMetaboliteKey(std::string key) { _cytosolicEnergyMetaboliteKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getCytosolicEnergyMetaboliteKey() const { return _cytosolicEnergyMetaboliteKey; }
void EukaryoticCellCycleComponent::setMitochondrialEnergyMetaboliteKey(std::string key) { _mitochondrialEnergyMetaboliteKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getMitochondrialEnergyMetaboliteKey() const { return _mitochondrialEnergyMetaboliteKey; }
void EukaryoticCellCycleComponent::setBudEnergyMetaboliteKey(std::string key) { _budEnergyMetaboliteKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getBudEnergyMetaboliteKey() const { return _budEnergyMetaboliteKey; }
void EukaryoticCellCycleComponent::setGrowthFluxKey(std::string key) { _growthFluxKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getGrowthFluxKey() const { return _growthFluxKey; }
void EukaryoticCellCycleComponent::setRespirationFluxKey(std::string key) { _respirationFluxKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getRespirationFluxKey() const { return _respirationFluxKey; }
void EukaryoticCellCycleComponent::setBudProgressKey(std::string key) { _budProgressKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getBudProgressKey() const { return _budProgressKey; }
void EukaryoticCellCycleComponent::setDnaReplicationProgressKey(std::string key) { _dnaReplicationProgressKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getDnaReplicationProgressKey() const { return _dnaReplicationProgressKey; }
void EukaryoticCellCycleComponent::setSpindleAssemblyProgressKey(std::string key) { _spindleAssemblyProgressKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getSpindleAssemblyProgressKey() const { return _spindleAssemblyProgressKey; }
void EukaryoticCellCycleComponent::setMitoticExitProgressKey(std::string key) { _mitoticExitProgressKey = std::move(key); }
std::string EukaryoticCellCycleComponent::getMitoticExitProgressKey() const { return _mitoticExitProgressKey; }
void EukaryoticCellCycleComponent::setGlobalAtpThreshold(double threshold) { _globalAtpThreshold = threshold; }
double EukaryoticCellCycleComponent::getGlobalAtpThreshold() const { return _globalAtpThreshold; }
void EukaryoticCellCycleComponent::setCytosolicAtpThreshold(double threshold) { _cytosolicAtpThreshold = threshold; }
double EukaryoticCellCycleComponent::getCytosolicAtpThreshold() const { return _cytosolicAtpThreshold; }
void EukaryoticCellCycleComponent::setMitochondrialAtpThreshold(double threshold) { _mitochondrialAtpThreshold = threshold; }
double EukaryoticCellCycleComponent::getMitochondrialAtpThreshold() const { return _mitochondrialAtpThreshold; }
void EukaryoticCellCycleComponent::setBudAtpThreshold(double threshold) { _budAtpThreshold = threshold; }
double EukaryoticCellCycleComponent::getBudAtpThreshold() const { return _budAtpThreshold; }
void EukaryoticCellCycleComponent::setGrowthFluxThreshold(double threshold) { _growthFluxThreshold = threshold; }
double EukaryoticCellCycleComponent::getGrowthFluxThreshold() const { return _growthFluxThreshold; }
void EukaryoticCellCycleComponent::setRespirationFluxThreshold(double threshold) { _respirationFluxThreshold = threshold; }
double EukaryoticCellCycleComponent::getRespirationFluxThreshold() const { return _respirationFluxThreshold; }
void EukaryoticCellCycleComponent::setBudProgressRate(double rate) { _budProgressRate = rate; }
double EukaryoticCellCycleComponent::getBudProgressRate() const { return _budProgressRate; }
void EukaryoticCellCycleComponent::setDnaReplicationRate(double rate) { _dnaReplicationRate = rate; }
double EukaryoticCellCycleComponent::getDnaReplicationRate() const { return _dnaReplicationRate; }
void EukaryoticCellCycleComponent::setSpindleAssemblyRate(double rate) { _spindleAssemblyRate = rate; }
double EukaryoticCellCycleComponent::getSpindleAssemblyRate() const { return _spindleAssemblyRate; }
void EukaryoticCellCycleComponent::setMitoticExitRate(double rate) { _mitoticExitRate = rate; }
double EukaryoticCellCycleComponent::getMitoticExitRate() const { return _mitoticExitRate; }
void EukaryoticCellCycleComponent::setBudProgressThreshold(double threshold) { _budProgressThreshold = threshold; }
double EukaryoticCellCycleComponent::getBudProgressThreshold() const { return _budProgressThreshold; }
void EukaryoticCellCycleComponent::setDnaReplicationThreshold(double threshold) { _dnaReplicationThreshold = threshold; }
double EukaryoticCellCycleComponent::getDnaReplicationThreshold() const { return _dnaReplicationThreshold; }
void EukaryoticCellCycleComponent::setSpindleAssemblyThreshold(double threshold) { _spindleAssemblyThreshold = threshold; }
double EukaryoticCellCycleComponent::getSpindleAssemblyThreshold() const { return _spindleAssemblyThreshold; }
void EukaryoticCellCycleComponent::setMitoticExitThreshold(double threshold) { _mitoticExitThreshold = threshold; }
double EukaryoticCellCycleComponent::getMitoticExitThreshold() const { return _mitoticExitThreshold; }
