/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Write.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:06
 */

#ifndef WRITE_H
#define WRITE_H

#include <list>
#include "../../kernel/simulator/ModelComponent.h"

/*!
 This component ...
 */
class Write : public ModelComponent {
public:

	enum class WriteToType : int {
		SCREEN = 1, FILE = 2
	};



public: // constructors
	Write(Model* model, std::string name = "");
	virtual ~Write() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void insertText(std::list<std::string> texts);
	void setFilename(std::string _filename);
	std::string filename() const;
	void setWriteToType(WriteToType _writeToType);
	Write::WriteToType writeToType() const;

protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual void _initBetweenReplications();
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private: // methods
private: // attributes 1:1
	const struct DEFAULT_VALUES {
		WriteToType writeToType = Write::WriteToType::SCREEN;
		std::string filename = "";
	} DEFAULT;
	WriteToType _writeToType = DEFAULT.writeToType;
	std::string _filename = DEFAULT.filename;
private: // attributes 1:n
	List<std::string>* _writeElements = new List<std::string>();
};


#endif /* WRITE_H */

