/*
 * File:   Segment.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef SEGMENT_H
#define SEGMENT_H

#include <string>
#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/util/List.h"

/*!
 * \brief One (station, length-to-next-station) step of a Segment's ordered
 * conveyor path.
 */
class SegmentStep {
public:
	SegmentStep(std::string stationName, double lengthToNext) {
		this->stationName = stationName;
		this->lengthToNext = lengthToNext;
	}
public:
	std::string stationName;
	double lengthToNext;
};

/*!
 * \brief Data definition holding the ordered sequence of stations (and the
 * length between consecutive stations) that make up one Conveyor's
 * physical path.
 *
 * Arena correspondence: the "Segment module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Transfer Panel", p. 103), which
 * defines a Segment Set (an ordered list of stations along a conveyor with
 * the length of each segment).
 *
 * Known limitation of this initial implementation: distance lookup only
 * walks the path forward in declaration order (`getDistanceBetween()` sums
 * `lengthToNext` from the "from" station up to the "to" station); there is
 * no support for a conveyor whose stations are not visited in a single
 * consistent order.
 */
class Segment : public ModelDataDefinition {
public:
	Segment(Model* model, std::string name = "");
	virtual ~Segment() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual std::string show() override;
public:
	List<SegmentStep*>* getSteps() const;
	void insertStep(SegmentStep* step);
	/*! \brief Sums lengthToNext from fromStationName up to (not including) toStationName; returns -1 when either station is absent or out of order. */
	double getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const;
protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
private:
	List<SegmentStep*>* _steps = new List<SegmentStep*>();
};

#endif /* SEGMENT_H */
