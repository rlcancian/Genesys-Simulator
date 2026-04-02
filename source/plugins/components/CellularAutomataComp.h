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

#include "../../kernel/simulator/ModelComponent.h"
#include "CellularAutomata/CellularAutomataBase.h"
#include "CellularAutomata/Lattice.h"
#include "CellularAutomata/Neighborhood.h"
#include "CellularAutomata/StateSet.h"

class BoundaryCondition;

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
	
public: //! constructors
	CellularAutomataComp(Model* model, std::string name = "");
	virtual ~CellularAutomataComp() = default;

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

	LocalRule *getlocalRule() const;

protected: //! virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override; //!< This method is only for ModelComponents, not ModelDataElements


protected: //! virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	virtual bool _check(std::string* errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	virtual void _initBetweenReplications() override;
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addProperty(PropertyBase* property);

private: //! new private user methods
	// ...

private: //! Attributes that should be loaded or saved with this component (Persistent Fields)

	// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const CellularAutomataType cellularAutomataType = CellularAutomataType::CLASSIC;
		const LatticeType latticeType = LatticeType::RETICULAR;
		const NeighboorhoodType neighboorhoodType = NeighboorhoodType::VONNEUMANN;
		const BoundaryType boundaryType = BoundaryType::FIXED;
		const StateSetType stateSetType = StateSetType::ENUMERATED;
		const LocalRuleType localRuleType = LocalRuleType::GAME_OF_LIFE;
	} DEFAULT;
	CellularAutomataComp::CellularAutomataType _cellularAutomataType = DEFAULT.cellularAutomataType;
	CellularAutomataComp::LatticeType _latticeType = DEFAULT.latticeType;
	CellularAutomataComp::NeighboorhoodType _neighboorhoodType = DEFAULT.neighboorhoodType;
	CellularAutomataComp::BoundaryType _boundaryType = DEFAULT.boundaryType;
	CellularAutomataComp::StateSetType _stateSetType = DEFAULT.stateSetType;
	CellularAutomataComp::LocalRuleType _localRuleType = DEFAULT.localRuleType;

private: //! Attributes that do not need to be loaded or saved with this component (Non Persistent Fields)
	CellularAutomataBase* _cellularAutomata = nullptr;
	Lattice* _lattice = nullptr;
	Neighborhood* _neighboorhood = nullptr;
	BoundaryCondition* _boundary = nullptr;
	StateSet* _stateSet = nullptr;
	LocalRule* _localRule = nullptr;

private: //! internal DataElements (Composition)

private: //! attached DataElements (Agrregation)
	// ...
};
