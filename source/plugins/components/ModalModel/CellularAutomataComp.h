/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Dummy.h
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Maio de 2019, 18:41
 */

#pragma once

#include "../../../kernel/simulator/ModelComponent.h"
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomataBase.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet.h"

#include <cstdint>
#include <utility>
#include <vector>

class BoundaryCondition;
class CppCompiler;

/*!
 This component ...
 */
class CellularAutomataComp : public ModelComponent {
public: //! enums
	enum class CellularAutomataType : int {
		CLASSIC = 1, TIMED_1D = 2, ASYNCHRONOUS = 3, NONUNIFORMRULE = 4, NONUNIFORMNEIGHBOOR = 5, USERDEFINED = 6
	};

	enum class LatticeType : int {
		RETICULAR = 1, TRIANGULAR = 2, HEXAGONAL = 3, NETWORK = 4, USERDEFINED = 5
	};

	enum class NeighboorhoodType : int {
		CENTERED = 1, BACKWARD = 2, FORWARD = 3, VONNEUMANN = 4, MOORE = 5, USERDEFINED = 6
	};

	enum class BoundaryType : int {
		FIXED = 1, CLOSED = 2, REFLEXIVE = 3, ADIABATIC = 4, USERDEFINED = 5
	};

	enum class StateSetType : int {
		ENUMERATED = 1, INTEGERBASED = 2, BITBASED = 3, DOUBLEBASED = 4, USERDEFINED = 5
	};

	enum class LocalRuleType : int {
		ELEMENTAR_CA = 1, GAME_OF_LIFE = 2, BIASED_COMPETITION = 3, HPP = 4, USERDEFINED = 5
	};

	enum class AutomataType : int {
		Temporary = 0, Permanent = 1
	};

	//! Temporal update policy: how the lattice advances on each automaton step.
	enum class UpdatePolicyType : int {
		SYNCHRONOUS = 1, SEQUENTIAL = 2, RANDOM = 3, BLOCKS = 4
	};

public: //! constructors
	CellularAutomataComp(Model* model, std::string name = "");
	virtual ~CellularAutomataComp();

public: //! new public user methods for this component
	CellularAutomataComp::CellularAutomataType getCellularAutomataType() const;
	void setCellularAutomataType(CellularAutomataComp::CellularAutomataType newCellularAutomataType);

	CellularAutomataComp::LatticeType getLatticeType() const;
	void setLatticeType(CellularAutomataComp::LatticeType newLatticeStructure);

	CellularAutomataComp::NeighboorhoodType getNeighboorhoodType() const;
	void setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType newNeighboorhood);

	CellularAutomataComp::BoundaryType geBoundaryType() const;
	void setBoundaryType(CellularAutomataComp::BoundaryType newBoundary);

	CellularAutomataComp::StateSetType getStateSetType() const;
	void setStateSetType(CellularAutomataComp::StateSetType newStateSetType);

	//! Builds (checks + inits) the cellular automaton from the configured types. Returns false (and
	//! fills errorMessage, if given) when the configuration is invalid. Useful to drive the automaton
	//! directly from a terminal example or test, without running a full DES model.
	bool initializeCellularAutomata(std::string* errorMessage = nullptr);
	//! Advances the automaton one step, honoring the configured update policy.
	void stepCellularAutomata();
	bool setCellState(long cellNumber, int value);
	bool setCellState(long cellNumber, long value);
	bool setCellState(long cellNumber, double value);
	bool setCellState(const std::vector<int>& position, int value);
	bool setCellState(const std::vector<int>& position, long value);
	bool setCellState(const std::vector<int>& position, double value);
	std::string showCellularAutomata() const;
	void setElementaryRuleNumber(uint8_t ruleNumber);
	CellularAutomataComp::UpdatePolicyType getUpdatePolicyType() const;
	void setUpdatePolicyType(CellularAutomataComp::UpdatePolicyType updatePolicyType);
	unsigned int getUpdateBlockSize() const;
	void setUpdateBlockSize(unsigned int updateBlockSize);
	unsigned int getRandomSeed() const;
	void setRandomSeed(unsigned int randomSeed);
	void setNetworkEdges(const std::vector<std::pair<unsigned long, unsigned long>>& edges, bool undirected = true);
	std::vector<std::pair<unsigned long, unsigned long>> getNetworkEdges() const;
	bool getNetworkEdgesUndirected() const;

	//CellularAutomataBase *getcellularAutomata() const;
	Lattice *getlattice() const;
	Neighborhood *getNeighboorhood() const;
	//BoundaryCondition *getBoundary() const;
	StateSet *getStateSet() const;

public: //! virtual public methods
	virtual std::string show() override;

public: //! static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

	CellularAutomataComp::LocalRuleType getlocalRuleType() const;
	void setLocalRuleType(CellularAutomataComp::LocalRuleType newLocalRuleType);

	//! C++ source for a USERDEFINED local rule. The source must define the C-linkage function
	//! `extern "C" long nextState(long self, const long* neighbors, int numNeighbors)`. It is compiled
	//! at runtime (via CppCompiler) and loaded during model check. See LocalRule_UserDefined.
	void setUserDefinedRuleSource(const std::string& userDefinedRuleSource);
	std::string getUserDefinedRuleSource() const;

	LocalRule *getlocalRule() const;

protected: //! virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override; //!< This method is only for ModelComponents, not ModelDataElements


protected: //! virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	virtual bool _check(std::string* errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	virtual void _initBetweenReplications() override;
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addSimulationControl(SimulationControl* property);


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private: //! new private user methods
	//! Lazily instantiates the cellular automaton if it has not been created yet (so the setters can
	//! create lattice/neighborhood/state set/rule that need a parent automaton, in any order).
	void _ensureCellularAutomata();
	//! Compiles and loads the USERDEFINED local rule from _userDefinedRuleSource (called by _check).
	bool _buildUserDefinedRule(std::string* errorMessage);
	//! Decomposed semantic checks used by _check (best-of-each: structural + rule + policy coherence).
	bool _checkImplementedTypes(std::string* errorMessage) const;
	bool _checkLattice(std::string* errorMessage) const;
	bool _checkStateSet(std::string* errorMessage) const;
	bool _checkRuleCompatibility(std::string* errorMessage) const;
	bool _checkUpdatePolicy(std::string* errorMessage) const;
	//! Update-policy engine: advances the lattice one step under the configured policy.
	void _stepCellularAutomataByPolicy();
	void _stepSequential();
	void _stepRandom();
	void _stepBlocks();
	void _applyRuleAndUpdateCell(unsigned long cellNumber);

private: //! Attributes that should be loaded or saved with this component (Persistent Fields)

	// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const CellularAutomataType cellularAutomataType = CellularAutomataType::CLASSIC;
		const LatticeType latticeType = LatticeType::RETICULAR;
		const NeighboorhoodType neighboorhoodType = NeighboorhoodType::VONNEUMANN;
		const BoundaryType boundaryType = BoundaryType::FIXED;
		const StateSetType stateSetType = StateSetType::ENUMERATED;
		const LocalRuleType localRuleType = LocalRuleType::GAME_OF_LIFE;
		const UpdatePolicyType updatePolicyType = UpdatePolicyType::SYNCHRONOUS;
		const unsigned int updateBlockSize = 1;
		const unsigned int randomSeed = 1;
		const std::string userDefinedRuleSource = "";
	} DEFAULT;
	CellularAutomataComp::CellularAutomataType _cellularAutomataType = DEFAULT.cellularAutomataType;
	CellularAutomataComp::LatticeType _latticeType = DEFAULT.latticeType;
	CellularAutomataComp::NeighboorhoodType _neighboorhoodType = DEFAULT.neighboorhoodType;
	CellularAutomataComp::BoundaryType _boundaryType = DEFAULT.boundaryType;
	CellularAutomataComp::StateSetType _stateSetType = DEFAULT.stateSetType;
	CellularAutomataComp::LocalRuleType _localRuleType = DEFAULT.localRuleType;
	CellularAutomataComp::UpdatePolicyType _updatePolicyType = DEFAULT.updatePolicyType;
	unsigned int _updateBlockSize = DEFAULT.updateBlockSize;
	unsigned int _randomSeed = DEFAULT.randomSeed;
	uint8_t _elementaryRuleNumber = 30;
	std::string _userDefinedRuleSource = DEFAULT.userDefinedRuleSource;

private: //! Attributes that do not need to be loaded or saved with this component (Non Persistent Fields)
	CellularAutomataBase* _cellularAutomata = nullptr;
	Lattice* _lattice = nullptr;
	Neighborhood* _neighboorhood = nullptr;
	BoundaryCondition* _boundary = nullptr;
	StateSet* _stateSet = nullptr;
	LocalRule* _localRule = nullptr;
	CppCompiler* _ruleCompiler = nullptr; //!< owns runtime compilation for the USERDEFINED local rule
	unsigned int _randomStepCounter = 0;  //!< advances the RANDOM update-policy seed between steps
	// The two binary states of an ENUMERATED state set are owned here (handed by non-owning pointer to
	// StateSet_Enumerable, whose default destructor frees nothing) so reconfiguring leaks no State.
	State _enumeratedStateOff = State(0L);
	State _enumeratedStateOn = State(1L);

private: //! internal DataElements (Composition)

private: //! attached DataElements (Agrregation)
	// ...
};
