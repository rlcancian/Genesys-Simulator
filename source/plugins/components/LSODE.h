/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LSODE.h
 * Author: rlcancian
 *
 * Created on 22 de Outubro de 2019, 22:28
 */

#ifndef LSODE_H
#define LSODE_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../data/Formula.h"
#include "../data/Variable.h"

/*!
 This component ...
 */
class LSODE : public ModelComponent {
public: // constructors
	LSODE(Model* model, std::string name = "");
	virtual ~LSODE() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // g&s
	void setTimeVariable(Variable* _timeVariable);
	Variable* getTimeVariable() const;
	void setStep(double _step);
	double getStep() const;
	void setVariable(Variable* _variables);
	Variable* getVariable() const;
	List<std::string>* getDiffEquations() const;
	void setFilename(std::string filename);
	std::string getFilename() const;
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
	//virtual void _createInternalAndAttachedData();
private: // methods
	bool _doStep();
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		std::string filename = "";
	} DEFAULT;
	List<std::string>* _diffEquations = new List<std::string>();
	Variable* _variable;
	Variable* _timeVariable;
	std::string _filename = DEFAULT.filename;
	double _step;
private: // attributes 1:n
};

#endif /* LSODE_H */

