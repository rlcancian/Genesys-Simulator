/*
 * File:   BacteriaColony.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef BACTERIACOLONY_H
#define BACTERIACOLONY_H

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"

/*!
 * \brief First GenESyS component for Gro-inspired bacteria colony simulation.
 *
 * BacteriaColony owns an internal biological simulation clock and a population
 * summary that are independent from ModelSimulation global event time in this
 * first slice. The component references a reusable GroProgram but currently
 * only validates and stores the program; complete Gro execution semantics are
 * intentionally deferred to plugin-side parser/runtime helpers.
 */
class BacteriaColony : public ModelComponent {
public:
	BacteriaColony(Model* model, std::string name = "");
	virtual ~BacteriaColony() override = default;

public:
	virtual std::string show() override;

public: // static plugin interface
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*! \brief Sets the reusable Gro program used by this colony. */
	void setGroProgram(GroProgram* groProgram);
	/*! \brief Returns the reusable Gro program used by this colony. */
	GroProgram* getGroProgram() const;
	/*! \brief Sets the internal colony time increment used by one colony step. */
	void setSimulationStep(double simulationStep);
	/*! \brief Returns the internal colony time increment. */
	double getSimulationStep() const;
	/*! \brief Sets the initial internal colony time for each replication. */
	void setInitialColonyTime(double initialColonyTime);
	/*! \brief Returns the initial internal colony time. */
	double getInitialColonyTime() const;
	/*! \brief Returns the current internal colony time. */
	double getColonyTime() const;
	/*! \brief Sets the initial bacteria population summary for each replication. */
	void setInitialPopulation(unsigned int initialPopulation);
	/*! \brief Returns the initial bacteria population summary. */
	unsigned int getInitialPopulation() const;
	/*! \brief Returns the current bacteria population summary. */
	unsigned int getPopulationSize() const;
	/*! \brief Sets the discrete spatial grid width reserved for the first colony model. */
	void setGridWidth(unsigned int gridWidth);
	/*! \brief Returns the discrete spatial grid width. */
	unsigned int getGridWidth() const;
	/*! \brief Sets the discrete spatial grid height reserved for the first colony model. */
	void setGridHeight(unsigned int gridHeight);
	/*! \brief Returns the discrete spatial grid height. */
	unsigned int getGridHeight() const;
	/*! \brief Advances the internal colony clock by one configured simulation step. */
	double advanceColonyTime();
	/*! \brief Executes the configured Gro program once against the current colony state. */
	GroProgramRuntime::ExecutionResult executeGroProgram();

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createInternalAndAttachedData() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

private:
	const struct DEFAULT_VALUES {
		const std::string groProgramName = "";
		const double simulationStep = 1.0;
		const double initialColonyTime = 0.0;
		const unsigned int initialPopulation = 1;
		const unsigned int gridWidth = 1;
		const unsigned int gridHeight = 1;
	} DEFAULT;

	GroProgram* _groProgram = nullptr;
	double _simulationStep = DEFAULT.simulationStep;
	double _initialColonyTime = DEFAULT.initialColonyTime;
	double _colonyTime = DEFAULT.initialColonyTime;
	unsigned int _initialPopulation = DEFAULT.initialPopulation;
	unsigned int _populationSize = DEFAULT.initialPopulation;
	unsigned int _gridWidth = DEFAULT.gridWidth;
	unsigned int _gridHeight = DEFAULT.gridHeight;
};

#endif /* BACTERIACOLONY_H */
