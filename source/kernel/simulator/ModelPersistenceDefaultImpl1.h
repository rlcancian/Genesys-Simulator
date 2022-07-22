/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelPersistenceDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 20 de Maio de 2019, 20:41
 */

#ifndef MODELPERSISTENCEDEFAULTIMPL1_H
#define MODELPERSISTENCEDEFAULTIMPL1_H

#include "ModelPersistence_if.h"
#include "Model.h"

//namespace GenesysKernel {

class ModelPersistenceDefaultImpl1 : public ModelPersistence_if {
public:
	ModelPersistenceDefaultImpl1(Model* model);
	virtual ~ModelPersistenceDefaultImpl1() = default;
public: // interface
	virtual bool save(std::string filename);
	virtual bool load(std::string filename);
	virtual bool getOption(ModelPersistence_if::Options option);
	virtual void setOption(ModelPersistence_if::Options option, bool value);
	virtual std::string getFormatedField(PersistenceRecord *fields);
public:
	virtual bool hasChanged();
private:
	void _saveContent(const std::list<std::string>& content, std::ofstream* file);
	bool _loadFields(std::list<std::map<std::string, std::string>>& componentFields, std::string line);
	void _loadSimulatorInfoFields(PersistenceRecord *fields);
	std::list<std::string> _adjustFieldsToSave(PersistenceRecord *fields);
	friend class Simulator; // @TODO
	std::string _convertLineseparatorToLineseparatorReplacement(std::string str);
	std::string _convertLineseparatorReplacementBacktoLineseparator(std::string str);
	void _getSimulatorInfoFieldsToSave(PersistenceRecord *fields);
private:
	Model* _model = nullptr;
	bool _hasChanged = false;
	bool _saveDefaultValues = true;
	unsigned int _options = 0;
	std::string _fieldseparator = " "; // fields are separated by space
	std::string _fieldseparatorReplacement = "\\_"; // in cases where spaces are in data to be saved, they are replaced by this pattern, so there will not be separators inside data (replacement is \_)
};
//namespace\\}
#endif /* MODELPERSISTENCEDEFAULTIMPL1_H */

