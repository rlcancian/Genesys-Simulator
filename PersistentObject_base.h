/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   persistentObject_base.h
 * Author: rlcancian
 *
 * Created on 9 de abril de 2021, 15:26
 */

#ifndef PERSISTENTOBJECT_BASE_H
#define PERSISTENTOBJECT_BASE_H

#include <map>

class PersistentObject_base {
public:

	PersistentObject_base() {
	}
	virtual ~PersistentObject_base() = default;
protected: // must be overriden by derived classes
	virtual bool _loadInstance(std::map<std::string, std::string>* fields) = 0;
	virtual std::map<std::string, std::string>* _saveInstance() = 0;
};

#endif /* PERSISTENTOBJECT_BASE_H */

