/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ComponentManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 28 de Maio de 2019, 10:41
 */

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "ModelComponent.h"
#include "SourceModelComponent.h"

//namespace GenesysKernel {
//class Model;

/*!
 * \brief Owns and organizes the set of \c ModelComponent objects in a model.
 *
 * Provides insertion/removal, lookup and iteration helpers while tracking
 * structural changes relevant to model validation/persistence workflows.
 */
class ComponentManager {
public:
	/*! \brief Creates a component manager attached to a model. */
	ComponentManager(Model* model);
	virtual ~ComponentManager() = default;
public:
	/*!
	 * \brief insert
	 * \param comp
	 * \return True when insertion succeeds.
	 */
	bool insert(ModelComponent* comp);
	/*!
	 * \brief remove
	 * \param comp
     */
    void remove(ModelComponent* comp);
	/*!
	 * \brief find
	 * \param name
	 * \return Pointer to the component, or \c nullptr when not found.
	 */
	ModelComponent* find(std::string name);
	/*!
	 * \brief find
	 * \param id
	 * \return Pointer to the component, or \c nullptr when not found.
	 */
	ModelComponent* find(Util::identification id);
	/*!
	 * \brief clear
	 * \details Removes all managed components from the model.
	*/
	void clear();
public:
	/*!
	 * \brief getNumberOfComponents
	 * \return Number of managed components.
	 */
	unsigned int getNumberOfComponents();
	/*!
	 * \brief begin
	 * \return Iterator to the first component.
	 */
	std::list<ModelComponent*>::iterator begin();
	/*!
	 * \brief end
	 * \return Iterator past the last component.
	 */
	std::list<ModelComponent*>::iterator end();
	/*!
	 * \brief front
	 * \return First component in iteration order.
	 */
	ModelComponent* front();
	/*!
	 * \brief next
	 * \return Next component in iteration order.
	 */
	ModelComponent* next();
	/*!
	 * \brief hasChanged
	 * \return
	*/
	bool hasChanged() const;
	/*!
	 * \brief setHasChanged
	 * \param _hasChanged
	 */
	void setHasChanged(bool _hasChanged);
public:
	/*!
	 * \brief getSourceComponents
	 * \return
	 */
	std::list<SourceModelComponent*>* getSourceComponents();
	/*!
	 * \brief getTransferInComponents
	 * \return
	 */
	std::list<ModelComponent*>* getTransferInComponents();
	/*!
	 * \brief getAllComponents
	 * \return
	 */
	std::list<ModelComponent*>* getAllComponents() const;
private:
	List<ModelComponent*>* _components;
	Model* _parentModel;
	bool _hasChanged = false;
	std::list<ModelComponent*>::iterator _componentIterator;
};
//namespace\\}
#endif /* COMPONENTMANAGER_H */
