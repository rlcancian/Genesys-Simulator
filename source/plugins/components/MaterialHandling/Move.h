#ifndef MOVE_H
#define MOVE_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../data/MaterialHandling/Station.h"
#include "../../data/MaterialHandling/Transporter.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"

/*!
 * \brief Minimal transporter movement component.
 *
 * This component intentionally collapses Arena's request/transport/free chain
 * into one atomic operation suitable for the current closeout: reserve one
 * transporter, move the entity to a destination station using the attached
 * Distance set, and free the transporter on arrival.
 */
class Move : public ModelComponent {
public:
	Move(Model* model, std::string name = "");
	virtual ~Move() override = default;
public:
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setTransporter(Transporter* transporter);
	Transporter* getTransporter() const;
	void setStation(Station* station);
	Station* getStation() const;
	void setStationExpression(std::string stationExpression);
	std::string getStationExpression() const;
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalStatisticReporters() override;
	virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;
private:
	Station* _resolveDestination() const;
	Station* _resolveSource(Entity* entity) const;
private:
	const struct DEFAULT_VALUES {
		const std::string stationExpression = "";
	} DEFAULT;
	Transporter* _transporter = nullptr;
	Station* _station = nullptr;
	std::string _stationExpression = DEFAULT.stationExpression;
	Counter* _numberIn = nullptr;
};

#endif /* MOVE_H */
