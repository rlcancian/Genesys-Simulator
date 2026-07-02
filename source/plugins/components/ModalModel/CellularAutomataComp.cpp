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
#include "../../../kernel/simulator/Model.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Adiabatic.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Reflexive.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_Classic.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_1DTimed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Elementary.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_Growty.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_UserDefined.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Hexagonal.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Network.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Triangular.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Bit.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Double.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Integer.h"
#include "plugins/data/ExternalIntegration/CppCompiler.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellularAutomataComp::GetPluginInformation;
}
#endif

namespace {
// Lattice dimensions are persisted as a comma-separated list (e.g. "5,5") so a saved automaton
// round-trips to a checkable configuration (persist the model's structural parameters).
std::string serializeDimensions(const std::vector<unsigned short>& dimensions) {
	std::string out;
	for (std::size_t i = 0; i < dimensions.size(); ++i) {
		if (i > 0)
			out += ',';
		out += std::to_string(dimensions.at(i));
	}
	return out;
}

std::vector<unsigned short> parseDimensions(const std::string& serialized) {
	std::vector<unsigned short> dimensions;
	std::stringstream stream(serialized);
	std::string item;
	while (std::getline(stream, item, ',')) {
		if (!item.empty())
			dimensions.emplace_back(static_cast<unsigned short>(std::stoul(item)));
	}
	return dimensions;
}

std::string serializeNetworkEdges(const std::vector<std::pair<unsigned long, unsigned long>>& edges) {
	std::string out;
	for (std::size_t i = 0; i < edges.size(); ++i) {
		if (i > 0)
			out += ',';
		out += std::to_string(edges.at(i).first) + '-' + std::to_string(edges.at(i).second);
	}
	return out;
}

std::vector<std::pair<unsigned long, unsigned long>> parseNetworkEdges(const std::string& serialized) {
	std::vector<std::pair<unsigned long, unsigned long>> edges;
	std::stringstream stream(serialized);
	std::string item;
	while (std::getline(stream, item, ',')) {
		const std::size_t separator = item.find('-');
		if (separator == std::string::npos)
			continue;
		edges.emplace_back(
			static_cast<unsigned long>(std::stoul(item.substr(0, separator))),
			static_cast<unsigned long>(std::stoul(item.substr(separator + 1))));
	}
	return edges;
}
} // namespace

ModelDataDefinition* CellularAutomataComp::NewInstance(Model* model, std::string name) {
	return new CellularAutomataComp(model, name);
}

CellularAutomataComp::CellularAutomataComp(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<CellularAutomataComp>(), name) {
}

CellularAutomataComp::~CellularAutomataComp() {
	// These sub-objects are plain classes (not ModelDataDefinition), so this component owns them and
	// must free them. Deleting _localRule first runs the LocalRule_UserDefined destructor, which
	// unloads its dynamic library. _ruleCompiler is a ModelDataDefinition owned by the model, so it
	// is intentionally NOT deleted here (the model frees it).
	delete _localRule;
	delete _cellularAutomata;
	delete _lattice;
	delete _neighboorhood;
	delete _boundary;
	delete _stateSet;
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
	_stepCellularAutomataByPolicy();
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool CellularAutomataComp::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// Recreate the sub-objects from the persisted type enums. The cellular-automata type must be
		// restored first because the lattice/neighborhood/boundary/state-set/rule reference it.
		setCellularAutomataType(static_cast<CellularAutomataType>(
			fields->loadField("cellularAutomataType", static_cast<int>(DEFAULT.cellularAutomataType))));
		setLatticeType(static_cast<LatticeType>(
			fields->loadField("latticeType", static_cast<int>(DEFAULT.latticeType))));
		// Restore the lattice dimensions (a structural model parameter) onto the freshly built lattice.
		const std::string serializedDimensions = fields->loadField("latticeDimensions", std::string(""));
		if (_lattice != nullptr && !serializedDimensions.empty())
			_lattice->setDimensions(parseDimensions(serializedDimensions));
		const std::string serializedEdges = fields->loadField("networkEdges", std::string(""));
		const bool networkEdgesUndirected = fields->loadField("networkEdgesUndirected", 1) != 0;
		if (_lattice != nullptr && !serializedEdges.empty())
			_lattice->setNetworkEdges(parseNetworkEdges(serializedEdges), networkEdgesUndirected);
		setNeighboorhoodType(static_cast<NeighboorhoodType>(
			fields->loadField("neighborhoodType", static_cast<int>(DEFAULT.neighboorhoodType))));
		if (_neighboorhood != nullptr)
			_neighboorhood->setRadius(static_cast<unsigned short>(fields->loadField("neighborhoodRadius", 1)));
		setBoundaryType(static_cast<BoundaryType>(
			fields->loadField("boundaryType", static_cast<int>(DEFAULT.boundaryType))));
		setStateSetType(static_cast<StateSetType>(
			fields->loadField("stateSetType", static_cast<int>(DEFAULT.stateSetType))));
		setUpdatePolicyType(static_cast<UpdatePolicyType>(
			fields->loadField("updatePolicyType", static_cast<int>(DEFAULT.updatePolicyType))));
		setUpdateBlockSize(static_cast<unsigned int>(
			fields->loadField("updateBlockSize", static_cast<int>(DEFAULT.updateBlockSize))));
		setRandomSeed(static_cast<unsigned int>(
			fields->loadField("randomSeed", static_cast<int>(DEFAULT.randomSeed))));
		// Set the elementary rule number before the rule type, so an ELEMENTAR_CA rule is built with it.
		setElementaryRuleNumber(static_cast<uint8_t>(fields->loadField("elementaryRuleNumber", 30)));
		// Load the user source before the rule type, so a USERDEFINED rule has its source available.
		_userDefinedRuleSource = fields->loadField("userDefinedRuleSource", DEFAULT.userDefinedRuleSource);
		setLocalRuleType(static_cast<LocalRuleType>(
			fields->loadField("localRuleType", static_cast<int>(DEFAULT.localRuleType))));
	}
	return res;
}

void CellularAutomataComp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("cellularAutomataType", static_cast<int>(_cellularAutomataType),
		static_cast<int>(DEFAULT.cellularAutomataType), saveDefaultValues);
	fields->saveField("latticeType", static_cast<int>(_latticeType),
		static_cast<int>(DEFAULT.latticeType), saveDefaultValues);
	fields->saveField("neighborhoodType", static_cast<int>(_neighboorhoodType),
		static_cast<int>(DEFAULT.neighboorhoodType), saveDefaultValues);
	fields->saveField("boundaryType", static_cast<int>(_boundaryType),
		static_cast<int>(DEFAULT.boundaryType), saveDefaultValues);
	fields->saveField("stateSetType", static_cast<int>(_stateSetType),
		static_cast<int>(DEFAULT.stateSetType), saveDefaultValues);
	fields->saveField("localRuleType", static_cast<int>(_localRuleType),
		static_cast<int>(DEFAULT.localRuleType), saveDefaultValues);
	fields->saveField("updatePolicyType", static_cast<int>(_updatePolicyType),
		static_cast<int>(DEFAULT.updatePolicyType), saveDefaultValues);
	fields->saveField("updateBlockSize", static_cast<int>(_updateBlockSize),
		static_cast<int>(DEFAULT.updateBlockSize), saveDefaultValues);
	fields->saveField("randomSeed", static_cast<int>(_randomSeed),
		static_cast<int>(DEFAULT.randomSeed), saveDefaultValues);
	fields->saveField("elementaryRuleNumber", static_cast<int>(_elementaryRuleNumber), 30, saveDefaultValues);
	fields->saveField("userDefinedRuleSource", _userDefinedRuleSource,
		DEFAULT.userDefinedRuleSource, saveDefaultValues);
	const std::string serializedDimensions = _lattice != nullptr ? serializeDimensions(_lattice->getDimensions()) : std::string("");
	fields->saveField("latticeDimensions", serializedDimensions, std::string(""), saveDefaultValues);
	const std::string serializedNetworkEdges = _lattice != nullptr ? serializeNetworkEdges(_lattice->getNetworkEdges()) : std::string("");
	fields->saveField("networkEdges", serializedNetworkEdges, std::string(""), saveDefaultValues);
	const int networkEdgesUndirected = (_lattice == nullptr || _lattice->getNetworkEdgesUndirected()) ? 1 : 0;
	fields->saveField("networkEdgesUndirected", networkEdgesUndirected, 1, saveDefaultValues);
	const int neighborhoodRadius = _neighboorhood != nullptr ? static_cast<int>(_neighboorhood->getRadius()) : 1;
	fields->saveField("neighborhoodRadius", neighborhoodRadius, 1, saveDefaultValues);
}

bool CellularAutomataComp::_buildUserDefinedRule(std::string* errorMessage) {
	if (_userDefinedRuleSource.empty()) {
		*errorMessage += "USERDEFINED local rule requires source code (use setUserDefinedRuleSource). ";
		return false;
	}
	if (_cellularAutomata == nullptr) {
		*errorMessage += "USERDEFINED local rule needs a cellular automata (set its type first). ";
		return false;
	}
	if (_ruleCompiler == nullptr) {
		_ruleCompiler = new CppCompiler(_parentModel, getName() + ".LocalRuleCompiler");
	}
	_ruleCompiler->setOutputDir(".temp/");
	_ruleCompiler->setTempDir(".temp/");
	_ruleCompiler->setFlagsGeneral("-w -std=c++14");
	if (_localRule != nullptr) {
		delete _localRule;
		_localRule = nullptr;
	}
	LocalRule_UserDefined* userRule = new LocalRule_UserDefined(_cellularAutomata, _ruleCompiler, _stateSet);
	std::string buildError;
	if (!userRule->build(_userDefinedRuleSource, buildError)) {
		*errorMessage += "USERDEFINED local rule failed to compile/load: " + buildError + " ";
		delete userRule;
		// userRule registered itself in the automaton (LocalRule ctor); clear the now-dangling pointer.
		_cellularAutomata->setLocalRule(nullptr);
		return false;
	}
	_localRule = userRule;
	// Don't leave the throwaway per-build temp paths in the compiler's persistent fields.
	_ruleCompiler->setSourceFilename("");
	_ruleCompiler->setOutputFilename("");
	return true;
}

bool CellularAutomataComp::_check(std::string* errorMessage) {
	// Decomposed structural/rule/policy validation, plus the USERDEFINED rule built lazily once
	// every structural sub-object is available.
	if (!_checkImplementedTypes(errorMessage))
		return false;
	if (_cellularAutomata == nullptr) {
		*errorMessage += "Cellular automata type is not set or is not supported. ";
		return false;
	}
	if (_lattice == nullptr) {
		*errorMessage += "Lattice type is not set. ";
		return false;
	}
	if (_neighboorhood == nullptr) {
		*errorMessage += "Neighborhood type is not set. ";
		return false;
	}
	if (_boundary == nullptr) {
		*errorMessage += "Boundary type is not set. ";
		return false;
	}
	if (_stateSet == nullptr) {
		*errorMessage += "State set type is not set. ";
		return false;
	}
	// Compile and load a user-defined local rule now, when the automaton, lattice,
	// neighborhood, boundary and state set all exist (see _buildUserDefinedRule).
	if (_localRuleType == LocalRuleType::USERDEFINED) {
		if (!_buildUserDefinedRule(errorMessage))
			return false;
	}
	if (_localRule == nullptr) {
		*errorMessage += "Local rule is not set (or its type is not supported yet). ";
		return false;
	}
	if (!_checkLattice(errorMessage))
		return false;
	if (!_checkStateSet(errorMessage))
		return false;
	if (!_checkRuleCompatibility(errorMessage))
		return false;
	if (!_checkUpdatePolicy(errorMessage))
		return false;
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
	_randomStepCounter = 0;
	if (_cellularAutomata != nullptr) // guard like _stepCellularAutomataByPolicy: never deref before _check succeeds
		_cellularAutomata->init();
}

bool CellularAutomataComp::initializeCellularAutomata(std::string* errorMessage) {
	std::string localErrorMessage;
	std::string* message = errorMessage != nullptr ? errorMessage : &localErrorMessage;
	if (!_check(message))
		return false;
	_randomStepCounter = 0;
	return _cellularAutomata->init();
}

void CellularAutomataComp::stepCellularAutomata() {
	_stepCellularAutomataByPolicy();
}

bool CellularAutomataComp::setCellState(long cellNumber, int value) {
	return setCellState(cellNumber, static_cast<long>(value));
}

bool CellularAutomataComp::setCellState(long cellNumber, long value) {
	return setCellState(cellNumber, static_cast<double>(value));
}

bool CellularAutomataComp::setCellState(long cellNumber, double value) {
	if (_lattice == nullptr)
		return false;
	State state;
	if (_stateSet != nullptr && !_stateSet->tryMakeState(value, &state))
		return false;
	else if (_stateSet == nullptr)
		state.setDoubleValue(value);
	return _lattice->setCellState(cellNumber, &state);
}

bool CellularAutomataComp::setCellState(const std::vector<int>& position, int value) {
	return setCellState(position, static_cast<long>(value));
}

bool CellularAutomataComp::setCellState(const std::vector<int>& position, long value) {
	return setCellState(position, static_cast<double>(value));
}

bool CellularAutomataComp::setCellState(const std::vector<int>& position, double value) {
	if (_lattice == nullptr)
		return false;
	State state;
	if (_stateSet != nullptr && !_stateSet->tryMakeState(value, &state))
		return false;
	else if (_stateSet == nullptr)
		state.setDoubleValue(value);
	return _lattice->setCellState(position, &state);
}

std::string CellularAutomataComp::showCellularAutomata() const {
	if (_lattice == nullptr)
		return "";
	std::ostringstream output;
	for (unsigned long cellNumber = 0; cellNumber < _lattice->getCellsSize(); ++cellNumber) {
		output << _lattice->getCell(static_cast<long>(cellNumber))->getCurrentState().getValue();
	}
	return output.str();
}

void CellularAutomataComp::setElementaryRuleNumber(uint8_t ruleNumber) {
	_elementaryRuleNumber = ruleNumber;
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA && _localRule != nullptr) {
		if (auto* elementaryRule = dynamic_cast<LocalRule_Elementary*>(_localRule))
			elementaryRule->setRuleNumber(ruleNumber);
	}
}

CellularAutomataComp::UpdatePolicyType CellularAutomataComp::getUpdatePolicyType() const {
	return _updatePolicyType;
}

void CellularAutomataComp::setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType updatePolicyType) {
	_updatePolicyType = updatePolicyType;
}

unsigned int CellularAutomataComp::getUpdateBlockSize() const {
	return _updateBlockSize;
}

void CellularAutomataComp::setUpdateBlockSize(unsigned int updateBlockSize) {
	_updateBlockSize = updateBlockSize == 0 ? 1 : updateBlockSize;
}

unsigned int CellularAutomataComp::getRandomSeed() const {
	return _randomSeed;
}

void CellularAutomataComp::setRandomSeed(unsigned int randomSeed) {
	_randomSeed = randomSeed;
	_randomStepCounter = 0;
}

void CellularAutomataComp::setNetworkEdges(const std::vector<std::pair<unsigned long, unsigned long>>& edges, bool undirected) {
	if (_lattice == nullptr)
		setLatticeType(LatticeType::NETWORK);
	_lattice->setNetworkEdges(edges, undirected);
}

std::vector<std::pair<unsigned long, unsigned long>> CellularAutomataComp::getNetworkEdges() const {
	if (_lattice == nullptr)
		return {};
	return _lattice->getNetworkEdges();
}

bool CellularAutomataComp::getNetworkEdgesUndirected() const {
	return _lattice == nullptr || _lattice->getNetworkEdgesUndirected();
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
	_ensureCellularAutomata();
	if (_localRule != nullptr) {
		delete _localRule;
		_localRule = nullptr; // avoid a dangling/double-freed pointer for types that build no rule here
		// The deleted rule registered itself in the automaton (LocalRule ctor); clear that pointer too.
		if (_cellularAutomata != nullptr)
			_cellularAutomata->setLocalRule(nullptr);
	}
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA) {
		_localRule = new LocalRule_Elementary(_cellularAutomata, _elementaryRuleNumber);
	} else if (_localRuleType == LocalRuleType::GAME_OF_LIFE) {
		_localRule = new LocalRule_GameOfLife(_cellularAutomata);
	} else if (_localRuleType == LocalRuleType::BIASED_COMPETITION) {
		_localRule = new LocalRule_Growty(_cellularAutomata);
	}
	// USERDEFINED is compiled and loaded lazily in _check(), where the cellular automata, state set
	// and user source are all available (see _buildUserDefinedRule).
}

void CellularAutomataComp::setUserDefinedRuleSource(const std::string& userDefinedRuleSource) {
	_userDefinedRuleSource = userDefinedRuleSource;
}

std::string CellularAutomataComp::getUserDefinedRuleSource() const {
	return _userDefinedRuleSource;
}

void CellularAutomataComp::setStateSetType(CellularAutomataComp::StateSetType newStateSetType)
{
	_stateSetType = newStateSetType;
	_ensureCellularAutomata();
	if (_stateSet != nullptr)
		delete _stateSet;
	_stateSet = nullptr;
	// Clear the automaton's now-dangling state-set pointer; an implemented type below re-wires it in
	// _check. For an unimplemented type no replacement is built, so this avoids a use-after-free if a
	// later LocalRule ctor reads parentCellularAutomata->getStateSet() (mirrors setLocalRuleType).
	if (_cellularAutomata != nullptr)
		_cellularAutomata->setStateSet(nullptr);
	if (_stateSetType == StateSetType::ENUMERATED)
		// The two binary states are owned by this component (members), so the enumerable leaks none.
		_stateSet = new StateSet_Enumerable(_cellularAutomata, {&_enumeratedStateOff, &_enumeratedStateOn});
	else if (_stateSetType == StateSetType::INTEGERBASED)
		_stateSet = new StateSet_Integer(_cellularAutomata);
	else if (_stateSetType == StateSetType::BITBASED)
		_stateSet = new StateSet_Bit(_cellularAutomata);
	else if (_stateSetType == StateSetType::DOUBLEBASED)
		_stateSet = new StateSet_Double(_cellularAutomata);
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
	if (_cellularAutomata != nullptr) {
		delete _cellularAutomata; // was an explicit destructor call (~CellularAutomataBase), which left a dangling pointer
		_cellularAutomata = nullptr;
	}
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
	_ensureCellularAutomata();
	const ::LatticeType latticeType = static_cast<::LatticeType>(static_cast<int>(_latticeType));
	if (_lattice == nullptr)
		_lattice = new Lattice(_cellularAutomata, nullptr, {}, latticeType);
	else
		_lattice->setLatticeType(latticeType);
	if (_neighboorhood != nullptr) {
		const unsigned short radius = _neighboorhood->getRadius();
		setNeighboorhoodType(_neighboorhoodType);
		if (_neighboorhood != nullptr)
			_neighboorhood->setRadius(radius);
	} else if (_latticeType == LatticeType::TRIANGULAR ||
			_latticeType == LatticeType::HEXAGONAL ||
			_latticeType == LatticeType::NETWORK) {
		setNeighboorhoodType(_neighboorhoodType);
	}
}

CellularAutomataComp::NeighboorhoodType CellularAutomataComp::getNeighboorhoodType() const
{
	return _neighboorhoodType;
}

void CellularAutomataComp::setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType newNeighboorhood)
{
	_neighboorhoodType = newNeighboorhood;
	_ensureCellularAutomata();
	if (_neighboorhood != nullptr)
		delete _neighboorhood;
	_neighboorhood = nullptr;
	// Clear the automaton's dangling neighborhood pointer; an implemented type below re-registers
	// itself (Neighborhood ctor), an unimplemented one leaves it safely null (avoids use-after-free).
	if (_cellularAutomata != nullptr)
		_cellularAutomata->setNeighborhood(nullptr);
	if (_latticeType == LatticeType::TRIANGULAR)
		_neighboorhood = new Neighborhood_Triangular(_cellularAutomata);
	else if (_latticeType == LatticeType::HEXAGONAL)
		_neighboorhood = new Neighborhood_Hexagonal(_cellularAutomata);
	else if (_latticeType == LatticeType::NETWORK)
		_neighboorhood = new Neighborhood_Network(_cellularAutomata);
	else if (_neighboorhoodType == NeighboorhoodType::CENTERED)
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
	_boundary = nullptr;
	if (_boundaryType == BoundaryType::CLOSED)
		_boundary = new Boundary_Closed();
	else if (_boundaryType == BoundaryType::FIXED)
		_boundary = new Boundary_Fixed();
	else if (_boundaryType == BoundaryType::REFLEXIVE)
		_boundary = new Boundary_Reflexive();
	else if (_boundaryType == BoundaryType::ADIABATIC)
		_boundary = new Boundary_Adiabatic();
}

void CellularAutomataComp::_ensureCellularAutomata() {
	if (_cellularAutomata == nullptr)
		setCellularAutomataType(_cellularAutomataType);
}

bool CellularAutomataComp::_checkImplementedTypes(std::string* errorMessage) const {
	// TIMED_1D is rejected: its applyLocalRule() is unfinished (dereferences an uninitialized State*)
	// and would crash. A 1D timed/elementary automaton is already covered by CLASSIC on a 1D lattice,
	// so only CLASSIC is offered until TIMED_1D is implemented and verified.
	if (_cellularAutomataType == CellularAutomataType::TIMED_1D ||
			_cellularAutomataType == CellularAutomataType::ASYNCHRONOUS ||
			_cellularAutomataType == CellularAutomataType::NONUNIFORMRULE ||
			_cellularAutomataType == CellularAutomataType::NONUNIFORMNEIGHBOOR ||
			_cellularAutomataType == CellularAutomataType::USERDEFINED) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured cellular automata type is not implemented yet. ";
		return false;
	}
	if (_latticeType == LatticeType::USERDEFINED) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured lattice type is not implemented yet. ";
		return false;
	}
	if (_neighboorhoodType == NeighboorhoodType::BACKWARD ||
			_neighboorhoodType == NeighboorhoodType::FORWARD ||
			_neighboorhoodType == NeighboorhoodType::USERDEFINED) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured neighborhood type is not implemented yet. ";
		return false;
	}
	if (_boundaryType == BoundaryType::USERDEFINED) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured boundary type is not implemented yet. ";
		return false;
	}
	if (_stateSetType == StateSetType::USERDEFINED) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured state set type is not implemented yet. ";
		return false;
	}
	// USERDEFINED local rule IS implemented (via CppCompiler); only HPP remains unimplemented.
	if (_localRuleType == LocalRuleType::HPP) {
		if (errorMessage != nullptr)
			*errorMessage += "Configured local rule type is not implemented yet. ";
		return false;
	}
	return true;
}

bool CellularAutomataComp::_checkLattice(std::string* errorMessage) const {
	const std::vector<unsigned short> dimensions = _lattice->getDimensions();
	if (dimensions.empty()) {
		if (errorMessage != nullptr)
			*errorMessage += "Lattice must have at least one dimension. ";
		return false;
	}
	for (unsigned short dimension : dimensions) {
		if (dimension == 0) {
			if (errorMessage != nullptr)
				*errorMessage += "All lattice dimensions must be greater than zero. ";
			return false;
		}
	}
	if ((_latticeType == LatticeType::TRIANGULAR || _latticeType == LatticeType::HEXAGONAL) &&
			dimensions.size() != 2) {
		if (errorMessage != nullptr)
			*errorMessage += "Triangular and hexagonal lattices require exactly two dimensions. ";
		return false;
	}
	if (_latticeType == LatticeType::NETWORK) {
		if (!_lattice->hasNetworkEdges()) {
			if (errorMessage != nullptr)
				*errorMessage += "Network lattice requires at least one edge. ";
			return false;
		}
		if (!_lattice->networkEdgesAreValid()) {
			if (errorMessage != nullptr)
				*errorMessage += "Network lattice edges must reference existing cells. ";
			return false;
		}
	}
	return true;
}

bool CellularAutomataComp::_checkStateSet(std::string* errorMessage) const {
	if (_stateSet == nullptr) {
		if (errorMessage != nullptr)
			*errorMessage += "State set type was not configured. ";
		return false;
	}
	return true;
}

bool CellularAutomataComp::_checkRuleCompatibility(std::string* errorMessage) const {
	const unsigned short numDimensions = _lattice->getNumDimensions();
	if ((_latticeType == LatticeType::TRIANGULAR || _latticeType == LatticeType::HEXAGONAL) &&
			(_neighboorhood == nullptr || _neighboorhood->getRadius() != 1)) {
		if (errorMessage != nullptr)
			*errorMessage += "Triangular and hexagonal lattices currently require radius-1 neighborhoods. ";
		return false;
	}
	if (_latticeType == LatticeType::RETICULAR && _neighboorhoodType == NeighboorhoodType::CENTERED && numDimensions != 1) {
		if (errorMessage != nullptr)
			*errorMessage += "Centered neighborhood is currently supported only for 1D lattices. ";
		return false;
	}
	if (_localRuleType == LocalRuleType::ELEMENTAR_CA) {
		if (_latticeType != LatticeType::RETICULAR) {
			if (errorMessage != nullptr)
				*errorMessage += "Elementary cellular automata rule requires a reticular lattice. ";
			return false;
		}
		if (numDimensions != 1) {
			if (errorMessage != nullptr)
				*errorMessage += "Elementary cellular automata rule requires a 1D lattice. ";
			return false;
		}
		if (_neighboorhood == nullptr || _neighboorhood->getRadius() != 1) {
			if (errorMessage != nullptr)
				*errorMessage += "Elementary cellular automata rule requires radius-1 neighborhood. ";
			return false;
		}
		if (_stateSetType != StateSetType::ENUMERATED && _stateSetType != StateSetType::BITBASED) {
			if (errorMessage != nullptr)
				*errorMessage += "Elementary cellular automata rule requires a binary state set. ";
			return false;
		}
	}
	if (_localRuleType == LocalRuleType::GAME_OF_LIFE) {
		if (_latticeType != LatticeType::RETICULAR ||
				numDimensions != 2 ||
				_neighboorhoodType != NeighboorhoodType::MOORE ||
				_neighboorhood->getRadius() != 1) {
			if (errorMessage != nullptr)
				*errorMessage += "Game of Life requires a 2D reticular lattice with Moore radius-1 neighborhood. ";
			return false;
		}
		StateSet_Enumerable* enumerableStateSet = dynamic_cast<StateSet_Enumerable*>(_stateSet);
		const bool isEnumeratedBinary = _stateSetType == StateSetType::ENUMERATED && enumerableStateSet != nullptr && enumerableStateSet->getStatesSize() == 2;
		const bool isBitBased = _stateSetType == StateSetType::BITBASED;
		if (!isEnumeratedBinary && !isBitBased) {
			if (errorMessage != nullptr)
				*errorMessage += "Game of Life requires a binary state set. ";
			return false;
		}
	}
	return true;
}

bool CellularAutomataComp::_checkUpdatePolicy(std::string* errorMessage) const {
	if (_updatePolicyType == UpdatePolicyType::BLOCKS && _updateBlockSize == 0) {
		if (errorMessage != nullptr)
			*errorMessage += "Block update policy requires update block size greater than zero. ";
		return false;
	}
	return true;
}

void CellularAutomataComp::_stepCellularAutomataByPolicy() {
	if (_cellularAutomata == nullptr || _lattice == nullptr || _localRule == nullptr)
		return;
	if (_updatePolicyType == UpdatePolicyType::SYNCHRONOUS)
		_cellularAutomata->step();
	else if (_updatePolicyType == UpdatePolicyType::SEQUENTIAL)
		_stepSequential();
	else if (_updatePolicyType == UpdatePolicyType::RANDOM)
		_stepRandom();
	else if (_updatePolicyType == UpdatePolicyType::BLOCKS)
		_stepBlocks();
}

void CellularAutomataComp::_stepSequential() {
	for (unsigned long cellNumber = 0; cellNumber < _lattice->getCellsSize(); ++cellNumber)
		_applyRuleAndUpdateCell(cellNumber);
}

void CellularAutomataComp::_stepRandom() {
	std::vector<unsigned long> cellNumbers(_lattice->getCellsSize());
	std::iota(cellNumbers.begin(), cellNumbers.end(), 0);
	// Portable Fisher-Yates. std::shuffle's consumption of the engine is implementation-defined
	// (libc++ and libstdc++ produce different permutations from the same mt19937 state), but
	// std::uniform_int_distribution is specified, so this yields the SAME random order on every
	// platform for a given seed -> the RANDOM update policy is reproducible (REGRA 3: comprovável).
	std::mt19937 randomEngine(_randomSeed + _randomStepCounter++);
	for (std::size_t i = cellNumbers.size(); i > 1; --i) {
		std::uniform_int_distribution<std::size_t> distribution(0, i - 1);
		std::swap(cellNumbers[i - 1], cellNumbers[distribution(randomEngine)]);
	}
	for (unsigned long cellNumber : cellNumbers)
		_applyRuleAndUpdateCell(cellNumber);
}

void CellularAutomataComp::_stepBlocks() {
	const unsigned int blockSize = _updateBlockSize == 0 ? 1 : _updateBlockSize;
	for (unsigned long firstCellNumber = 0; firstCellNumber < _lattice->getCellsSize(); firstCellNumber += blockSize) {
		const unsigned long lastCellNumber = std::min<unsigned long>(firstCellNumber + blockSize, _lattice->getCellsSize());
		for (unsigned long cellNumber = firstCellNumber; cellNumber < lastCellNumber; ++cellNumber)
			_localRule->applyRule(_lattice->getCell(static_cast<long>(cellNumber)));
		for (unsigned long cellNumber = firstCellNumber; cellNumber < lastCellNumber; ++cellNumber)
			_lattice->getCell(static_cast<long>(cellNumber))->updateState();
	}
}

void CellularAutomataComp::_applyRuleAndUpdateCell(unsigned long cellNumber) {
	Cell* cell = _lattice->getCell(static_cast<long>(cellNumber));
	_localRule->applyRule(cell);
	cell->updateState();
}

PluginInformation* CellularAutomataComp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellularAutomataComp>(), &CellularAutomataComp::LoadInstance, &CellularAutomataComp::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Universal cellular automaton: configurable lattice (reticular/triangular/hexagonal/network), neighborhood "
		"(centered/Moore/Von Neumann/lattice-specific), boundary (fixed/closed/reflexive/adiabatic), state set "
		"(enumerated/integer/bit/double), update policy (synchronous/sequential/random/blocks) and local "
		"rule (elementary/Game of Life/biased-competition or a user-defined rule compiled at runtime).");
	return info;
}

// void CellularAutomataComp::_createInternalStatisticReporters() { }

// void CellularAutomataComp::_createEditableDataDefinitions() { }

// void CellularAutomataComp::_createAttachedAttributes() { }
