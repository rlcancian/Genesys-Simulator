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
#include "../TraceManager.h"
#include <vector>

//namespace GenesysKernel {

/*!
 * \brief The ModelManager class
 */
class ModelManager {
public:
	/*!
	 * \brief Creates a model manager bound to a simulator instance.
	 * \param simulator Parent simulator that owns this manager.
	 */
	ModelManager(Simulator* simulator);
	/*!
	 * \brief Releases the model manager and any owned bookkeeping structures.
	 */
	virtual ~ModelManager();
public:
	/*!
	 * \brief Creates a new model, inserts it into the open-model list and makes it current.
	 * \return The newly opened current model.
	 */
	Model* newModel();
	/*!
	 * \brief Inserts an existing model into the open-model list.
	 * \param model Model to register.
	 */
	void insert(Model* model);
	/*!
	 * \brief Removes a model from the open-model list.
	 * \param model Model to remove.
	 */
	void remove(Model* model);
	/*!
	 * \brief Makes an open model the current one.
	 * \param model Model to activate.
	 * \return \c true when the model was found and selected.
	 */
	bool setCurrent(Model* model);
	/*!
	 * \brief Saves the current model to a file.
	 * \param filename Output filename.
	 * \return \c true when saving succeeds.
	 */
	bool saveModel(std::string filename);
	/*!
	 * \brief Loads a model from a file and opens it in the manager.
	 * \param filename Input filename.
	 * \return Loaded model instance, or \c nullptr on failure.
	 */
	Model* loadModel(std::string filename);
	/*!
	 * \brief Creates a model from a textual model specification.
	 * \param modelSpecification Source text in the model language.
	 * \return Newly created model instance, or \c nullptr on failure.
	 */
	Model* createFromLanguage(std::string modelSpecification);
	/*!
	 * \brief Returns the number of open models.
	 * \return Open-model count.
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
	 * \brief Returns the first open model.
	 * \return First model in open-model order, or \c nullptr when empty.
	 */
	Model* front();
	/*!
	 * \brief Returns the last open model.
	 * \return Last model in open-model order, or \c nullptr when empty.
	 */
	Model* last();
	/*!
	 * \brief Returns the current model.
	 * \return Currently selected model, or \c nullptr when none is selected.
	 */
	Model* current();
	/*!
	 * \brief Advances to the next open model.
	 * \return Next model in order, or \c nullptr when already at the end.
	 */
	Model* next();
	/*!
	 * \brief Moves to the previous open model.
	 * \return Previous model in order, or \c nullptr when already at the beginning.
	 */
	Model* previous();
	/*!
	 * \brief Indicates whether a next model exists.
	 * \return \c true when a call to \c next() can advance.
	 */
	bool canGoNext() const;
	/*!
	 * \brief Indicates whether a previous model exists.
	 * \return \c true when a call to \c previous() can retreat.
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
