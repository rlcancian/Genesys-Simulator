/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickUp.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#ifndef PICKUP_H
#define PICKUP_H

#include "plugins/components/Decisions/Remove.h"

/*!
 * \brief Removes a consecutive run of entities from a queue, appending them
 * to the incoming entity's group.
 *
 * Arena correspondence: the "Pickup module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 57). Implemented by
 * inheriting from Remove and reusing its Queue Name/Starting Rank plumbing
 * (a start/end rank range plays the role of Arena's Quantity).
 */
class PickUp : public Remove {
public: // constructors
	PickUp(Model* model, std::string name = "");
	virtual ~PickUp() = default;
public: // static
	static PluginInformation* GetPluginInformation();
	//static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
};


#endif /* PICKUP_H */

