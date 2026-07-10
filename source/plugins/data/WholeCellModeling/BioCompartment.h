#ifndef BIOCOMPARTMENT_H
#define BIOCOMPARTMENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/*!
 * \brief Defines a biological compartment for biochemical and whole-cell models.
 *
 * BioCompartment represents a named physical or logical cellular region such as
 * cytosol, nucleus, mitochondrion, bud, membrane, or extracellular medium.
 * It is a ModelDataDefinition and therefore stores structural metadata used by
 * other plugins, but it does not execute simulation behavior itself.
 *
 * The class is intentionally generic so it can support both prokaryotic and
 * eukaryotic cells. Parent-child relationships allow hierarchical structures
 * such as:
 * - extracellular -> cell surface
 * - cytosol -> nucleus
 * - cytosol -> mitochondrion
 * - cytosol -> bud
 */
class BioCompartment : public ModelDataDefinition {
public:
	BioCompartment(Model* model, std::string name = "");
	virtual ~BioCompartment() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setCompartmentType(std::string type);
	std::string getCompartmentType() const;
	void setParentCompartmentName(std::string parentName);
	std::string getParentCompartmentName() const;
	void setMembraneBounded(bool membraneBounded);
	bool isMembraneBounded() const;
	void setVolumeFraction(double volumeFraction);
	double getVolumeFraction() const;
	void setCopyNumber(unsigned int copyNumber);
	unsigned int getCopyNumber() const;
	void setRole(std::string role);
	std::string getRole() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createEditableDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		std::string compartmentType = "cytosol";
		std::string parentCompartmentName = "";
		bool membraneBounded = false;
		double volumeFraction = 1.0;
		unsigned int copyNumber = 1u;
		std::string role = "generic";
	} DEFAULT;

	std::string _compartmentType = DEFAULT.compartmentType;
	std::string _parentCompartmentName = DEFAULT.parentCompartmentName;
	bool _membraneBounded = DEFAULT.membraneBounded;
	double _volumeFraction = DEFAULT.volumeFraction;
	unsigned int _copyNumber = DEFAULT.copyNumber;
	std::string _role = DEFAULT.role;
};

#endif /* BIOCOMPARTMENT_H */
