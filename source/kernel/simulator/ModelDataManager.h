/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ElementManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 12:48
 */

#ifndef ELEMENTMANAGER_H
#define ELEMENTMANAGER_H

#include <list>
#include <map>
#include "../util/List.h"
#include "ModelDataDefinition.h"

//namespace GenesysKernel {
class Model;

/*!
 * \brief Central registry for \c ModelDataDefinition instances owned by a model.
 *
 * This manager groups model data by typename, supports insertion/removal/lookup
 * operations and offers consistency checks used by components and model validation.
 */
class ModelDataManager {
public:
	/*! \brief Creates a data manager attached to \p model. */
	ModelDataManager(Model* model);
	virtual ~ModelDataManager();
public:
	/*!
	 * \brief insert
	 * \param anElement
	 * \return
	 */
	bool insert(ModelDataDefinition* anElement);
	/*!
	 * \brief remove
	 * \param anElement
	 */
	void remove(ModelDataDefinition* anElement); //!< Deprected
	/*!
	 * \brief insert
	 * \param datadefinitionTypename
	 * \param anElement
	 * \return
	 */
	bool insert(std::string datadefinitionTypename, ModelDataDefinition* anElement); //!< Deprected
	/*!
	 * \brief remove
	 * \param datadefinitionTypename
	 * \param anElement
	 */
	void remove(std::string datadefinitionTypename, ModelDataDefinition* anElement);
	/*!
	 * \brief check
	 * \param datadefinitionTypename
	 * \param anElement
	 * \param expressionName
	 * \param errorMessage
	 * \return
	 */
    bool check(std::string datadefinitionTypename, ModelDataDefinition* anElement, std::string expressionName, std::string& errorMessage);
	/*!
	 * \brief check
	 * \param datadefinitionTypename
	 * \param elementName
	 * \param expressionName
	 * \param mandatory
	 * \param errorMessage
	 * \return
	 */
    bool check(std::string datadefinitionTypename, std::string elementName, std::string expressionName, bool mandatory, std::string& errorMessage);
	/*!
	 * \brief clear
	 */
	void clear();
public:
	/*!
	 * \brief getDataDefinition
	 * \param datadefinitionTypename
	 * \param id
	 * \return
	 */
	ModelDataDefinition* getDataDefinition(std::string datadefinitionTypename, Util::identification id);
	/*!
	 * \brief getDataDefinition
	 * \param datadefinitionTypename
	 * \param name
	 * \return
	 */
	ModelDataDefinition* getDataDefinition(std::string datadefinitionTypename, std::string name);
	/*!
	 * \brief getNumberOfDataDefinitions
	 * \param datadefinitionTypename
	 * \return
	 */
	unsigned int getNumberOfDataDefinitions(std::string datadefinitionTypename);
	/*!
	 * \brief getNumberOfDataDefinitions
	 * \return
	 */
	unsigned int getNumberOfDataDefinitions();
	/*!
	 * \brief getRankOf
	 * \param datadefinitionTypename
	 * \param name
	 * \return
	 */
	int getRankOf(std::string datadefinitionTypename, std::string name); //!< returns the position (1st position=0) of the modeldatum if found, or negative value if not found
	/*!
	 * \brief getDataDefinitionClassnames
	 * \return
	 */
	std::list<std::string>* getDataDefinitionClassnames() const;

	//private:
public:
	// @TODO: Should be removed and replaced by GetElement(elementType, rank)
	List<ModelDataDefinition*>* getDataDefinitionList(std::string datadefinitionTypename) const;
public:
	/*! \brief Prints a textual summary of registered model data definitions. */
	void show();
	/*! \brief Returns the model that owns this data manager. */
	Model* getParentModel() const;
	/*! \brief Indicates whether managed data collections were modified. */
	bool hasChanged() const;
	/*! \brief Updates the internal changed flag. */
	void setHasChanged(bool _hasChanged);
private:
	std::map<std::string, List<ModelDataDefinition*>*>* _datadefinitions;
	Model* _parentModel;
	bool _hasChanged = false;
};
//namespace\\}
#endif /* ELEMENTMANAGER_H */
