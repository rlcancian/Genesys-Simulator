/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Search.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "kernel/simulator/ModelComponent.h"

/*!
Search module
DESCRIPTION
The Search module searches a queue, a group (batch), or an expression to find the
entity rank (for entities in a queue or group) or the value of the global variable J that
satisfies the specified search condition. When searching a queue or group, the value
of the global system variable J is set to the rank of the first entity that satisfies Search
Condition, or to 0 if Search Condition is not satisfied. When searching an expression,
the global system variable J is set to the value of the first index value that satisfies the
search condition or to zero if no value of J in the specified range satisfies the search
condition.
When an entity arrives at a Search module, the index J is set to the starting index and
the search condition is then checked. If the search condition is satisfied, the search
ends and the current value of J is retained. Otherwise, the value of J is increased or
decreased and the condition is rechecked. This process repeats until the search
condition is satisfied or the ending value is reached. If the condition is not met or
there are no entities in the queue or group, J is set equal to 0.
TYPICAL USES
 * Looking for a particular order number in a queue
 * Searching a group for a certain part type
 * Determining which process to enter based on availability of resources (search an
expression)
 Prompt Description
Name Unique module identifier displayed on the module shape.
Type Determination of what will be searched. Search options include
entities in a queue, entities within a group (batch) or some
expression(s).
Queue Name Name of the queue that will be searched. Applies only when the
Type is Search a Queue.
Starting Value Starting rank in the queue or group or starting value for J in an
expression.
Ending Value Ending rank in the queue or group or ending value for J in an
expression.
Search Condition Condition containing the index J for searching expressions or
containing an attribute name(s) for searching queues or batches.
 */
class Search : public ModelComponent {
public:
	enum class SearchInType : int {
		QUEUE = 0, ENTITYGROUP = 1, num_elements = 2
	};
public:
	static std::string convertEnumToStr(SearchInType type);
public: // constructors
	Search(Model* model, std::string name = "");
	virtual ~Search() = default;
public: // virtual
	virtual std::string show() override;
public:
	void setSaveFounRankAttribute(std::string _saveFounRankAttribute);
	std::string getSaveFounRankAttribute() const;
	void setSearchCondition(std::string _searchCondition);
	std::string getSearchCondition() const;
	void setEndRank(std::string _endRank);
	std::string getEndRank() const;
	void setStartRank(std::string _startRank);
	std::string getStartRank() const;
	void setSearchInName(std::string searchInName);
	std::string getSearchInName() const;
    void setSearchIn(ModelDataDefinition* _searchIn);
    ModelDataDefinition* getSearchIn() const;
    void setSearchInType(Search::SearchInType _searchInType);
    Search::SearchInType getSearchInType() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	//virtual void _addSimulationControl(SimulationControl* property);

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const SearchInType searchInType = SearchInType::QUEUE; 		
		const std::string startRank = "";
		const std::string endRank = "";
		const std::string searchCondition = "";
		const std::string saveFounRankAttribute = "";
	} DEFAULT;
	ModelDataDefinition* _searchIn = nullptr;
	Search::SearchInType _searchInType = DEFAULT.searchInType;
	std::string _startRank = DEFAULT.startRank;
	std::string _endRank = DEFAULT.endRank;
	std::string _searchCondition = DEFAULT.searchCondition;
	std::string _saveFounRankAttribute = DEFAULT.saveFounRankAttribute;

private: // attributes 1:n
};


#endif /* SEARCH_H */
