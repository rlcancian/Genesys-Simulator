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
#include <vector>

//namespace GenesysKernel {

/*!
 * \brief The ModelManager class
 */
class ModelManager {
public:
	ModelManager(Simulator* simulator);
	virtual ~ModelManager();
public:
	/*!
	 * \brief Creates a new model, inserts it into the open-model list and makes it current.
	 * \return The newly opened current model.
	 */
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
	bool setCurrent(Model* model);
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
	 * \brief Returns a stable snapshot of the currently open models in tab/navigation order.
	 * \return Vector with the open model pointers owned by this manager.
	 */
	std::vector<Model*> models() const;
	/*!
	 * \brief Checks whether a model is owned by this manager.
	 * \param model Model pointer to test.
	 * \return True when model is in the open-model list.
	 */
	bool hasModel(Model* model) const;
	/*!
	 * \brief Returns the open-model index of a model.
	 * \param model Model pointer to locate.
	 * \return Zero-based index, or -1 when not open.
	 */
	int indexOf(Model* model) const;
	/*!
	 * \brief Returns the model at a given open-model index.
	 * \param index Zero-based model index.
	 * \return Model pointer, or nullptr when index is out of range.
	 */
	Model* modelAt(unsigned int index) const;
	/*!
	 * \brief Returns the index of the current model.
	 * \return Zero-based current model index, or -1 when there is no current model.
	 */
	int currentIndex() const;
	/*!
	 * \brief front
	 * \return
	 */
	Model* front();
	/*!
	 * \brief last
	 * \return
	 */
	Model* last();
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
	/*!
	 * \brief previous
	 * \return
	 */
	Model* previous();
	/*!
	 * \brief canGoNext
	 * \return
	 */
	bool canGoNext() const;
	/*!
	 * \brief canGoPrevious
	 * \return
	 */
	bool canGoPrevious() const;
	//Model* end();
private:
	List<Model*>* _models = new List<Model*>();
    Model* _currentModel = nullptr;
private:
	Simulator* _simulator;
};
//namespace\\}
#endif /* MODELMANAGER_H */
