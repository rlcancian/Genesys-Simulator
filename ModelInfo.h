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
#include "Util.h"

//namespace GenesysKernel {

	/*!
	 * ModelInfo stores basic model project information.
	 */
	class ModelInfo {
	public:
		ModelInfo();
		virtual ~ModelInfo() = default;
	public:
		std::string show();
	public: // gets and sets
		void setName(std::string _name);
		std::string getName() const;
		void setAnalystName(std::string _analystName);
		std::string getAnalystName() const;
		void setDescription(std::string _description);
		std::string getDescription() const;
		void setProjectTitle(std::string _projectTitle);
		std::string getProjectTitle() const;
		void setVersion(std::string _version);
		std::string getVersion() const;
	public:
		void loadInstance(std::map<std::string, std::string>* fields);
		std::map<std::string, std::string>* saveInstance();
		bool hasChanged() const;
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

