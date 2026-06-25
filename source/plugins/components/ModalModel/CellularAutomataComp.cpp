/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CelularAutomata.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:14
 */

#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "../../../kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_1DTimed.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformRule.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformNeighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Custom.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellularAutomataComp::GetPluginInformation;
}
#endif

ModelDataDefinition* CellularAutomataComp::NewInstance(Model* model, std::string name) {
	return new CellularAutomataComp(model, name);
}

CellularAutomataComp::CellularAutomataComp(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<CellularAutomataComp>(), name) {
}

std::string CellularAutomataComp::show() {
	return ModelComponent::show() + "";
}


ModelComponent* CellularAutomataComp::LoadInstance(Model* model, PersistenceRecord *fields) {
	CellularAutomataComp* newComponent = new CellularAutomataComp(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void CellularAutomataComp::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_cellularAutomata->step();
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool CellularAutomataComp::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		setCellularAutomataType(static_cast<CellularAutomataType>(
			fields->loadField("caType", static_cast<int>(DEFAULT.cellularAutomataType))));
		setLatticeType(static_cast<LatticeType>(
			fields->loadField("latticeType", static_cast<int>(DEFAULT.latticeType))));
		setNeighboorhoodType(static_cast<NeighboorhoodType>(
			fields->loadField("neighborhoodType", static_cast<int>(DEFAULT.neighboorhoodType))));
		setBoundaryType(static_cast<BoundaryType>(
			fields->loadField("boundaryType", static_cast<int>(DEFAULT.boundaryType))));
		setStateSetType(static_cast<StateSetType>(
			fields->loadField("stateSetType", static_cast<int>(DEFAULT.stateSetType))));
		setLocalRuleType(static_cast<LocalRuleType>(
			fields->loadField("localRuleType", static_cast<int>(DEFAULT.localRuleType))));
	}
	return res;
}

void CellularAutomataComp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("caType",           static_cast<int>(_cellularAutomataType), static_cast<int>(DEFAULT.cellularAutomataType), saveDefaultValues);
	fields->saveField("latticeType",      static_cast<int>(_latticeType),          static_cast<int>(DEFAULT.latticeType),          saveDefaultValues);
	fields->saveField("neighborhoodType", static_cast<int>(_neighboorhoodType),    static_cast<int>(DEFAULT.neighboorhoodType),    saveDefaultValues);
	fields->saveField("boundaryType",     static_cast<int>(_boundaryType),         static_cast<int>(DEFAULT.boundaryType),         saveDefaultValues);
	fields->saveField("stateSetType",     static_cast<int>(_stateSetType),         static_cast<int>(DEFAULT.stateSetType),         saveDefaultValues);
	fields->saveField("localRuleType",    static_cast<int>(_localRuleType),        static_cast<int>(DEFAULT.localRuleType),        saveDefaultValues);
}

bool CellularAutomataComp::_check(std::string* errorMessage) {
	bool isValid = true;
	if (_cellularAutomata == nullptr) {
		*errorMessage += "Cellular automata type not set. ";
		isValid = false;
	}
	if (_lattice == nullptr) {
		*errorMessage += "Lattice not set. ";
		isValid = false;
	}
	if (_neighboorhood == nullptr) {
		*errorMessage += "Neighborhood not set. ";
		isValid = false;
	}
	if (_boundary == nullptr) {
		*errorMessage += "Boundary condition not set. ";
		isValid = false;
	}
	if (_stateSet == nullptr) {
		*errorMessage += "State set not set. ";
		isValid = false;
	}
	if (_localRule == nullptr &&
		_cellularAutomataType != CellularAutomataType::NONUNIFORMRULE &&
		_cellularAutomataType != CellularAutomataType::NONUNIFORM) {
		*errorMessage += "Local rule not set. ";
		isValid = false;
	}
	if (isValid) {
		_cellularAutomata->setLattice(_lattice);
		_cellularAutomata->setLocalRule(_localRule);
		_cellularAutomata->setNeighborhood(_neighboorhood);
		_cellularAutomata->setStateSet(_stateSet);
		if (_localRule != nullptr)
			_localRule->setStateSet(_stateSet);
		_neighboorhood->setBoundary(_boundary);
		_boundary->setLattice(_lattice);
		_boundary->setNeighborhood(_neighboorhood);
	}
	return isValid;
}

void CellularAutomataComp::_initBetweenReplications() {
	_cellularAutomata->init();
}

LocalRule *CellularAutomataComp::getlocalRule() const
{
	return _localRule;
}

CellularAutomataComp::LocalRuleType CellularAutomataComp::getlocalRuleType() const
{
	return _localRuleType;
}

void CellularAutomataComp::setLocalRuleType(CellularAutomataComp::LocalRuleType newLocalRuleType)
{
	_localRuleType = newLocalRuleType;
	if (_localRule != nullptr)
		delete _localRule;
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA) {
		_localRule = new LocalRule_Elementary(_cellularAutomata, 30);
	} else if (_localRuleType == LocalRuleType::GAME_OF_LIFE) {
		_localRule = new LocalRule_GameOfLife(_cellularAutomata);
	} else if (_localRuleType == LocalRuleType::BIASED_COMPETITION) {
		_localRule = new LocalRule_Growty(_cellularAutomata);
	}
}

void CellularAutomataComp::setStateSetType(CellularAutomataComp::StateSetType newStateSetType)
{
	_stateSetType = newStateSetType;
	if (_stateSet == nullptr)
		_stateSet = new StateSet(_cellularAutomata);
}


CellularAutomataComp::StateSetType CellularAutomataComp::getStateSetType() const
{
	return _stateSetType;
}

//CellularAutomataBase *CellularAutomataComp::getcellularAutomata() const{
//	return _cellularAutomata;
//}

Lattice *CellularAutomataComp::getlattice() const
{
	return _lattice;
}

Neighborhood *CellularAutomataComp::getNeighboorhood() const
{
	return _neighboorhood;
}

//BoundaryCondition *CellularAutomataComp::getBoundary() const{
//	return _boundary;
//}

StateSet *CellularAutomataComp::getStateSet() const
{
	return _stateSet;
}

CellularAutomataComp::CellularAutomataType CellularAutomataComp::getCellularAutomataType() const
{
	return _cellularAutomataType;
}

void CellularAutomataComp::setCellularAutomataType(CellularAutomataComp::CellularAutomataType newCellularAutomataType)
{
	_cellularAutomataType = newCellularAutomataType;
	if (_cellularAutomata != nullptr)
		_cellularAutomata->~CellularAutomataBase();
	if (_cellularAutomataType == CellularAutomataType::CLASSIC)
		_cellularAutomata = new CellularAutomata_Classic();
	else if (_cellularAutomataType == CellularAutomataType::TIMED_1D)
		_cellularAutomata = new CellularAutomata_1DTimed();
	else if (_cellularAutomataType == CellularAutomataType::NONUNIFORMRULE)
		_cellularAutomata = new CellularAutomata_NonUniformRule();
	else if (_cellularAutomataType == CellularAutomataType::NONUNIFORMNEIGHBOOR)
		_cellularAutomata = new CellularAutomata_NonUniformNeighborhood();
	else if (_cellularAutomataType == CellularAutomataType::NONUNIFORM)
		_cellularAutomata = new CellularAutomata_NonUniform();
}

bool CellularAutomataComp::setCellLocalRule(long cellNumber, LocalRule* rule) {
	auto* nr = dynamic_cast<CellularAutomata_NonUniformRule*>(_cellularAutomata);
	if (nr != nullptr) { nr->setCellRule(cellNumber, rule); return true; }
	auto* nu = dynamic_cast<CellularAutomata_NonUniform*>(_cellularAutomata);
	if (nu != nullptr) { nu->setCellRule(cellNumber, rule); return true; }
	return false;
}

bool CellularAutomataComp::setCellLocalRule(std::vector<int> position, LocalRule* rule) {
	auto* nr = dynamic_cast<CellularAutomata_NonUniformRule*>(_cellularAutomata);
	if (nr != nullptr) { nr->setCellRule(position, rule); return true; }
	auto* nu = dynamic_cast<CellularAutomata_NonUniform*>(_cellularAutomata);
	if (nu != nullptr) { nu->setCellRule(position, rule); return true; }
	return false;
}

bool CellularAutomataComp::setCellNeighborhood(long cellNumber, Neighborhood* hood) {
	auto* nn = dynamic_cast<CellularAutomata_NonUniformNeighborhood*>(_cellularAutomata);
	if (nn != nullptr) { nn->setCellNeighborhood(cellNumber, hood); return true; }
	return false;
}

bool CellularAutomataComp::setCellNeighborhood(std::vector<int> position, Neighborhood* hood) {
	auto* nn = dynamic_cast<CellularAutomata_NonUniformNeighborhood*>(_cellularAutomata);
	if (nn != nullptr) { nn->setCellNeighborhood(position, hood); return true; }
	return false;
}

CellularAutomataComp::LatticeType CellularAutomataComp::getLatticeType() const
{
	return _latticeType;
}

void CellularAutomataComp::setLatticeType(CellularAutomataComp::LatticeType newLatticeStructure)
{
	_latticeType = newLatticeStructure;
	if (_lattice == nullptr)
		_lattice = new Lattice(_cellularAutomata);
}

CellularAutomataComp::NeighboorhoodType CellularAutomataComp::getNeighboorhoodType() const
{
	return _neighboorhoodType;
}

void CellularAutomataComp::setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType newNeighboorhood)
{
	_neighboorhoodType = newNeighboorhood;
	if (_neighboorhood != nullptr)
		delete _neighboorhood;
	if (_neighboorhoodType == NeighboorhoodType::CENTERED)
		_neighboorhood = new Neighborhood_Center(_cellularAutomata);
	else if (_neighboorhoodType == NeighboorhoodType::MOORE)
		_neighboorhood = new Neighborhood_Moore(_cellularAutomata);
	else if (_neighboorhoodType == NeighboorhoodType::VONNEUMANN)
		_neighboorhood = new Neighborhood_VonNeumann(_cellularAutomata);
}

CellularAutomataComp::BoundaryType CellularAutomataComp::geBoundaryType() const
{
	return _boundaryType;
}

void CellularAutomataComp::setBoundaryType(CellularAutomataComp::BoundaryType newBoundary)
{
	_boundaryType = newBoundary;
	if (_boundary != nullptr)
		delete _boundary;
	if (_boundaryType == BoundaryType::CLOSED)
		_boundary = new Boundary_Closed();
	else if (_boundaryType == BoundaryType::FIXED)
		_boundary = new Boundary_Fixed();
}

PluginInformation* CellularAutomataComp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellularAutomataComp>(), &CellularAutomataComp::LoadInstance, &CellularAutomataComp::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("//@TODO");
	return info;
}

// void CellularAutomataComp::_createInternalStatisticReporters() { }

// void CellularAutomataComp::_createEditableDataDefinitions() { }

// void CellularAutomataComp::_createAttachedAttributes() { }
