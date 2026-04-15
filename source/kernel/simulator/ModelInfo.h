/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelInfo.h
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 18:13
 */

#ifndef MODELINFO_H
#define MODELINFO_H

#include <string>
#include "../util/Util.h"
#include "Persistence.h"

//namespace GenesysKernel {

/*!
 * ModelInfo stores basic model project information, such as the project and analyst names.
 */
class ModelInfo {
public:
	ModelInfo();
	virtual ~ModelInfo() = default;
public:
	std::string show();
public: // gets and sets
	/*!
	 * \brief setName
	 * \param _name
	 */
	void setName(std::string _name);
	/*!
	 * \brief getName
	 * \return
	 */
	std::string getName() const;
	/*!
	 * \brief setAnalystName
	 * \param _analystName
	 */
	void setAnalystName(std::string _analystName);
	/*!
	 * \brief getAnalystName
	 * \return
	 */
	std::string getAnalystName() const;
	/*!
	 * \brief setDescription
	 * \param _description
	 */
	void setDescription(std::string _description);
	/*!
	 * \brief getDescription
	 * \return
	 */
	std::string getDescription() const;
	/*!
	 * \brief setProjectTitle
	 * \param _projectTitle
	 */
	void setProjectTitle(std::string _projectTitle);
	/*!
	 * \brief getProjectTitle
	 * \return
	 */
	std::string getProjectTitle() const;
	/*!
	 * \brief setVersion
	 * \param _version
	 */
	void setVersion(std::string _version);
	/*!
	 * \brief getVersion
	 * \return
	 */
	std::string getVersion() const;
public:
	/*!
	 * \brief loadInstance
	 * \param fields
	 */
	void loadInstance(PersistenceRecord *fields);
	/*!
	 * \brief saveInstance
	 * \param fields
	 */
	void saveInstance(PersistenceRecord *fields);
	/*!
	 * \brief hasChanged
	 * \return
	 */
	bool hasChanged() const;
	/*! \brief Updates the changed flag for model metadata. */
	void setHasChanged(bool hasChanged);
private: // with public access (get & set)
	// model general information
	std::string _name;
	std::string _analystName = "";
	std::string _description = "";
	std::string _projectTitle = "";
	std::string _version = "1.0";
	bool _hasChanged = false;
};
//namespace\\}
#endif /* MODELINFO_H */
