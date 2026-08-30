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

#include "../../../kernel/simulator/model/ModelComponent.h"

/*!
 * \brief Scans a Queue or an EntityGroup rank range for the first entity
 * satisfying a condition, saving the found rank (or 0) to an attribute.
 *
 * Arena correspondence: the "Search module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 64), which also
 * supports a third Type: searching a free-standing index range via an
 * expression (setting the global system variable `J`), with no queue or
 * group involved at all.
 *
 * `SearchInType` (`QUEUE/ENTITYGROUP`) matches two of Arena's three Types;
 * `_startRank`/`_endRank`/`_searchCondition`/`_saveFounRankAttribute` match
 * Starting/Ending Value and Search Condition.
 *
 * Known difference from Arena: the free-standing expression-only search
 * Type has no corresponding `SearchInType` enumerator.
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
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	 virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData() override;
	//virtual void _addSimulationControl(SimulationControl* property);

protected:

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
