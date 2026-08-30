#ifndef CONVEYOR_H
#define CONVEYOR_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "plugins/data/MaterialHandling/Segment.h"

/*!
 * \brief Minimal conveyor data definition built on top of one Segment path.
 *
 * This implementation intentionally keeps only the semantics required by the
 * current Advanced Transfer closeout: active/inactive state, a single ordered
 * path (Segment), velocity and a simplified concurrent-allocation capacity.
 * It does not model Arena's cell-by-cell occupancy contract.
 */
class Conveyor : public ModelDataDefinition {
public:
	Conveyor(Model* model, std::string name = "");
	virtual ~Conveyor() override = default;
public:
	virtual std::string show() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setSegment(Segment* segment);
	Segment* getSegment() const;
	void setVelocity(double velocity);
	double getVelocity() const;
	void setCapacity(unsigned int capacity);
	unsigned int getCapacity() const;
	void setInitiallyActive(bool initiallyActive);
	bool isInitiallyActive() const;
	void setActive(bool active);
	bool isActive() const;
	unsigned int getCurrentAllocation() const;
	bool access(unsigned int quantity = 1);
	bool exit(unsigned int quantity = 1);
	double getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const;
protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createEditableDataDefinitions() override;
private:
	const struct DEFAULT_VALUES {
		const double velocity = 1.0;
		const unsigned int capacity = 1;
		const bool initiallyActive = true;
	} DEFAULT;
	Segment* _segment = nullptr;
	double _velocity = DEFAULT.velocity;
	unsigned int _capacity = DEFAULT.capacity;
	bool _initiallyActive = DEFAULT.initiallyActive;
	bool _active = DEFAULT.initiallyActive;
	unsigned int _currentAllocation = 0;
};

#endif /* CONVEYOR_H */
