/*
 * File:   Distance.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef DISTANCE_H
#define DISTANCE_H

#include <string>
#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/util/List.h"

/*!
 * \brief One directed (from-station, to-station, length) entry of a
 * Distance set, used to look up a free-path Transporter's travel length
 * between two stations.
 */
class DistanceEntry {
public:
	DistanceEntry(std::string fromStationName, std::string toStationName, double length, bool bidirectional = true) {
		this->fromStationName = fromStationName;
		this->toStationName = toStationName;
		this->length = length;
		this->bidirectional = bidirectional;
	}
public:
	std::string fromStationName;
	std::string toStationName;
	double length;
	bool bidirectional;
};

/*!
 * \brief Data definition holding a set of direct station-to-station
 * distances, used by free-path (non-guided) `Transporter` units to compute
 * travel time.
 *
 * Arena correspondence: the "Distance module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Advanced Transfer Panel", p. 106),
 * which defines a Distance Set of (Beginning Station, Ending Station,
 * Distance) triples for free-path transporters.
 *
 * Known limitation of this initial implementation: only a direct
 * from/to (or to/from, when `bidirectional`) lookup is supported — there is
 * no multi-hop shortest-path resolution when two stations are not directly
 * listed.
 */
class Distance : public ModelDataDefinition {
public:
	Distance(Model* model, std::string name = "");
	virtual ~Distance() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual std::string show() override;
public:
	List<DistanceEntry*>* getEntries() const;
	void insertEntry(DistanceEntry* entry);
	/*! \brief Returns the direct distance between two stations, or -1 when no matching entry exists. */
	double getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const;
protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
private:
	List<DistanceEntry*>* _entries = new List<DistanceEntry*>();
};

#endif /* DISTANCE_H */
