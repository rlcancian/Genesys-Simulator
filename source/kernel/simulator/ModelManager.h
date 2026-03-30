/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Maio de 2019, 08:37
 */

#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "Model.h"
#include "TraceManager.h"

//namespace GenesysKernel {

/*!
 * \brief The ModelManager class
 */
class ModelManager {
public:
	ModelManager(Simulator* simulator);
	virtual ~ModelManager();
public:
	Model* newModel();
	/*!
	 * \brief insert
	 * \param model
	 */
	void insert(Model* model);
	/*!
	 * \brief remove
	 * \param model
	 */
	void remove(Model* model);
	/*!
	 * \brief setCurrent
	 * \param model
	 */
	void setCurrent(Model* model);
	/*!
	 * \brief saveModel
	 * \param filename
	 * \return
	 */
	bool saveModel(std::string filename);
	/*!
	 * \brief loadModel
	 * \param filename
	 * \return
	 */
	Model* loadModel(std::string filename);
	/*!
	 * \brief createFromLanguage
	 * \param modelSpecification
	 * \return
	 */
	Model* createFromLanguage(std::string modelSpecification);
	/*!
	 * \brief size
	 * \return
	 */
	unsigned int size();
public:
	/*!
	 * \brief front
	 * \return
	 */
	Model* front();
	/*!
	 * \brief current
	 * \return
	 */
	Model* current();
	/*!
	 * \brief next
	 * \return
	 */
	Model* next();
	//Model* end();
private:
	List<Model*>* _models = new List<Model*>();
	Model* _currentModel;
private:
	Simulator* _simulator;
};
//namespace\\}
#endif /* MODELMANAGER_H */
