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
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_FlorestalFire.h"
#include "plugins/components/ModalModel/CellularAutomata/semantic/CompiledRuleLogic.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/semantic/LocalRuleFactory.h"
#include "plugins/components/ModalModel/CellularAutomata/temporal/UpdatePolicy.h"
#include "plugins/components/ModalModel/CellularAutomata/persistence/CellularAutomataSerializer.h"

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
	if (!res) {
		return false;
	}

	// Carrega configuração básica (enums)
	_cellularAutomataType = static_cast<CellularAutomataType>(
		fields->loadField("ca.cellularAutomataType", static_cast<int>(CellularAutomataType::CLASSIC)));
	_latticeType = static_cast<LatticeType>(
		fields->loadField("ca.latticeType", static_cast<int>(LatticeType::RETICULAR)));
	_neighboorhoodType = static_cast<NeighboorhoodType>(
		fields->loadField("ca.neighboorhoodType", static_cast<int>(NeighboorhoodType::VONNEUMANN)));
	_boundaryType = static_cast<BoundaryType>(
		fields->loadField("ca.boundaryType", static_cast<int>(BoundaryType::FIXED)));
	_stateSetType = static_cast<StateSetType>(
		fields->loadField("ca.stateSetType", static_cast<int>(StateSetType::ENUMERATED)));
	_localRuleType = static_cast<LocalRuleType>(
		fields->loadField("ca.localRuleType", static_cast<int>(LocalRuleType::GAME_OF_LIFE)));

	// Carrega código-fonte da regra definida pelo usuário
	_userRuleSourceCode = fields->loadField("ca.userRule.sourceCode", std::string(""));

	// Reconstrói o autômato
	setCellularAutomataType(_cellularAutomataType);
	setLatticeType(_latticeType);
	setNeighboorhoodType(_neighboorhoodType);
	setBoundaryType(_boundaryType);
	setStateSetType(_stateSetType);
	setLocalRuleType(_localRuleType);

	// Se há código de regra definido pelo usuário, compila
	if (!_userRuleSourceCode.empty() && _localRuleType == LocalRuleType::USERDEFINED) {
		setUserRuleCode(_userRuleSourceCode);
	}

	return res;
}

void CellularAutomataComp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);

	// Salva configuração
	fields->saveField("ca.cellularAutomataType", static_cast<int>(_cellularAutomataType),
					  static_cast<int>(DEFAULT.cellularAutomataType), saveDefaultValues);
	fields->saveField("ca.latticeType", static_cast<int>(_latticeType),
					  static_cast<int>(DEFAULT.latticeType), saveDefaultValues);
	fields->saveField("ca.neighboorhoodType", static_cast<int>(_neighboorhoodType),
					  static_cast<int>(DEFAULT.neighboorhoodType), saveDefaultValues);
	fields->saveField("ca.boundaryType", static_cast<int>(_boundaryType),
					  static_cast<int>(DEFAULT.boundaryType), saveDefaultValues);
	fields->saveField("ca.stateSetType", static_cast<int>(_stateSetType),
					  static_cast<int>(DEFAULT.stateSetType), saveDefaultValues);
	fields->saveField("ca.localRuleType", static_cast<int>(_localRuleType),
					  static_cast<int>(DEFAULT.localRuleType), saveDefaultValues);

	// Salva tipo da regra por nome (para factory)
	if (_localRule != nullptr) {
		fields->saveField("ca.semantic.ruleType", _localRule->getRuleType());
	}

	// Salva código-fonte da regra definida pelo usuário
	if (!_userRuleSourceCode.empty()) {
		fields->saveField("ca.userRule.sourceCode", _userRuleSourceCode, std::string(""), saveDefaultValues);
	}

	// Salva estado do autômato via serializer
	if (_cellularAutomata != nullptr && _lattice != nullptr && _neighboorhood != nullptr && _localRule != nullptr) {
		CellularAutomataSerializer::save(_lattice, _neighboorhood, _localRule, _cellularAutomata, fields);
	}
}

bool CellularAutomataComp::_check(std::string* errorMessage) {
	*errorMessage = "";

	// Validar existência dos componentes básicos
	if (_cellularAutomata == nullptr) {
		*errorMessage += "CellularAutomata is nullptr; ";
		return false;
	}
	if (_lattice == nullptr) {
		*errorMessage += "Lattice is nullptr; ";
		return false;
	}
	if (_neighboorhood == nullptr) {
		*errorMessage += "Neighborhood is nullptr; ";
		return false;
	}
	if (_localRule == nullptr) {
		*errorMessage += "LocalRule is nullptr; ";
		return false;
	}
	if (_stateSet == nullptr) {
		*errorMessage += "StateSet is nullptr; ";
		return false;
	}
	if (_boundary == nullptr) {
		*errorMessage += "Boundary is nullptr; ";
		return false;
	}

	// Validar compatibilidade dimensão lattice
	unsigned short latticeDims = _lattice->getNumDimensions();
	if (latticeDims == 0) {
		*errorMessage += "Lattice has no dimensions; ";
		return false;
	}

	// Validar compatibilidade StateSet com regra local
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA && latticeDims != 1) {
		*errorMessage += "Elementary CA requires 1D lattice; ";
		return false;
	}

	// Validar regra definida pelo usuário
	if (_localRuleType == LocalRuleType::USERDEFINED) {
		if (_userRuleSourceCode.empty()) {
			*errorMessage += "UserDefined rule requires source code; ";
			return false;
		}
	}

	// Validar política de atualização
	if (_updatePolicyType != UpdatePolicyType::SYNCHRONOUS &&
		_updatePolicyType != UpdatePolicyType::RANDOM_ASYNCHRONOUS &&
		_updatePolicyType != UpdatePolicyType::SEQUENTIAL) {
		*errorMessage += "Invalid UpdatePolicyType; ";
		return false;
	}

	// Tudo OK - configurar referências cruzadas
	_cellularAutomata->setLattice(_lattice);
	_cellularAutomata->setLocalRule(_localRule);
	_cellularAutomata->setNeighborhood(_neighboorhood);
	_cellularAutomata->setStateSet(_stateSet);
	_localRule->setStateSet(_stateSet);
	_neighboorhood->setBoundary(_boundary);
	_boundary->setLattice(_lattice);
	_boundary->setNeighborhood(_neighboorhood);

	return true;
}

void CellularAutomataComp::_initBetweenReplications() {
	// Registra todas as regras concretas na factory
	LocalRuleFactory::registerRule("GameOfLife", [](CellularAutomataBase* parent) -> std::unique_ptr<LocalRule> {
		return std::make_unique<LocalRule_GameOfLife>(parent);
	});
	LocalRuleFactory::registerRule("Elementary", [](CellularAutomataBase* parent) -> std::unique_ptr<LocalRule> {
		return std::make_unique<LocalRule_Elementary>(parent);
	});
	LocalRuleFactory::registerRule("Growty", [](CellularAutomataBase* parent) -> std::unique_ptr<LocalRule> {
		return std::make_unique<LocalRule_Growty>(parent);
	});
	LocalRuleFactory::registerRule("ForestFire", [](CellularAutomataBase* parent) -> std::unique_ptr<LocalRule> {
		return std::make_unique<LocalRule_FlorestalFire>(parent);
	});

	_cellularAutomata->init();
}

void CellularAutomataComp::setUserRuleCode(const std::string& sourceCode) {
	_userRuleSourceCode = sourceCode;

	if (sourceCode.empty()) {
		return;
	}

	// Se o tipo é USERDEFINED, compila o código
	if (_localRuleType == LocalRuleType::USERDEFINED) {
		// Remove regra anterior se existir
		if (_localRule != nullptr) {
			delete _localRule;
			_localRule = nullptr;
		}

		// Cria nova regra compilada
		auto* compiledRule = new CompiledRuleLogic(_cellularAutomata, _stateSet);
		if (compiledRule->compile(sourceCode)) {
			_localRule = compiledRule;
		} else {
			// Se falhou, armazena erro e mantém regra anterior
			delete compiledRule;
			// Em produção, deveria logar o erro
		}
	}
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

	// Usa a factory para criar a regra pelo tipo
	switch (_localRuleType) {
		case LocalRuleType::ELEMENTAR_CA:
			_localRule = LocalRuleFactory::create("Elementary", _cellularAutomata).release();
			break;
		case LocalRuleType::GAME_OF_LIFE:
			_localRule = LocalRuleFactory::create("GameOfLife", _cellularAutomata).release();
			break;
		case LocalRuleType::BIASED_COMPETITION:
			_localRule = LocalRuleFactory::create("Growty", _cellularAutomata).release();
			break;
		case LocalRuleType::USERDEFINED:
			// Para USERDEFINED, o código deve ser compilado via setUserRuleCode()
			// Se já há código-fonte, compila agora
			if (!_userRuleSourceCode.empty()) {
				auto* compiledRule = new CompiledRuleLogic(_cellularAutomata, _stateSet);
				if (compiledRule->compile(_userRuleSourceCode)) {
					_localRule = compiledRule;
				} else {
					delete compiledRule;
					_localRule = nullptr;
				}
			} else {
				_localRule = nullptr;
			}
			break;
		default:
			_localRule = nullptr;
			break;
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
