#ifndef BIONETWORKPROJECTIONCOMPONENT_H
#define BIONETWORKPROJECTIONCOMPONENT_H

#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

class BioSpecies;

/*!
 * \brief Projects selected BioSpecies values into a WholeCellState.
 *
 * BioStateProjectionComponent bridges the biochemical and whole-cell layers
 * by copying chosen BioSpecies amounts into WholeCellState molecule counts or
 * metabolite pools at simulation event boundaries. It is intentionally small
 * and explicit: the caller decides which source species feed which whole-cell
 * fields and which scale/offset transform applies to each projection.
 */
class BioStateProjectionComponent : public ModelComponent {
public:
	enum class ProjectionKind {
		MoleculeCount,
		MetaboliteAmount
	};

	struct Projection {
		std::string sourceSpeciesName;
		std::string targetKey;
		ProjectionKind kind = ProjectionKind::MoleculeCount;
		double scale = 1.0;
		double offset = 0.0;
	};

public:
	BioStateProjectionComponent(Model* model, std::string name = "");
	virtual ~BioStateProjectionComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void clearProjections();
	void addMoleculeProjection(const std::string& sourceSpeciesName, const std::string& targetKey = "",
	                           double scale = 1.0, double offset = 0.0);
	void addMetaboliteProjection(const std::string& sourceSpeciesName, const std::string& targetKey = "",
	                             double scale = 1.0, double offset = 0.0);
	unsigned int getProjectionCount() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual void _createEditableDataDefinitions() override;

private:
	void _forwardEntity(Entity* entity);
	void _syncEditableDataDefinitions();

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		bool lastSucceeded = false;
		std::string lastMessage = "";
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	std::vector<Projection> _projections;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* BIONETWORKPROJECTIONCOMPONENT_H */
