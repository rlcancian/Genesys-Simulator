#ifndef TRANSPORTER_H
#define TRANSPORTER_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/Event.h"
#include "kernel/simulator/PluginInformation.h"
#include "plugins/data/MaterialHandling/Distance.h"
#include "plugins/data/MaterialHandling/Station.h"

class Transporter : public ModelDataDefinition {
public:
	struct TravelCompletion {
		Station* destination = nullptr;
	};
public:
	Transporter(Model* model, std::string name = "");
	virtual ~Transporter() override = default;
public:
	virtual std::string show() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setDistance(Distance* distance);
	Distance* getDistance() const;
	void setInitialStation(Station* station);
	Station* getInitialStation() const;
	Station* getCurrentStation() const;
	void setSpeed(double speed);
	double getSpeed() const;
	void setInitiallyActive(bool initiallyActive);
	bool isInitiallyActive() const;
	void setActive(bool active);
	bool isActive() const;
	bool isBusy() const;
	bool reserve();
	void releaseAt(Station* station);
	double getTravelTime(const Station* fromStation, const Station* toStation) const;
	void handleTravelCompletion(void* parameter);
protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _createEditableDataDefinitions() override;
private:
	const struct DEFAULT_VALUES {
		const double speed = 1.0;
		const bool initiallyActive = true;
	} DEFAULT;
	Distance* _distance = nullptr;
	Station* _initialStation = nullptr;
	Station* _currentStation = nullptr;
	double _speed = DEFAULT.speed;
	bool _initiallyActive = DEFAULT.initiallyActive;
	bool _active = DEFAULT.initiallyActive;
	bool _busy = false;
};

#endif /* TRANSPORTER_H */
