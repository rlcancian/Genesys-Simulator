/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelPersistence_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 24 de Agosto de 2018, 19:22
 */

#ifndef MODELPERSISTENCE_IF_H
#define MODELPERSISTENCE_IF_H

#include <string>
#include <map>


// forward decl
class PersistenceRecord;

class ModelPersistence_if {
public:
	enum class Options : int {
		SAVEDEFAULTS = 1, HIDEIDKEY = 2, HIDETYPEKEY = 4, HIDENAMEKEY = 8, SORTALPHLY = 16
	};

public:
	virtual bool save(std::string filename) = 0;
	virtual bool load(std::string filename) = 0;

public:
	virtual bool hasChanged() = 0;
	virtual bool getOption(ModelPersistence_if::Options option) = 0;
	virtual void setOption(ModelPersistence_if::Options option, bool value) = 0;

public:
	virtual std::string getFormatedField(PersistenceRecord *fields) = 0;
};

#endif /* MODELPERSISTENCE_IF_H */

