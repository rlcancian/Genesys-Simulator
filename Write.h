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

#include "ModelComponent.h"

class WriteElement {
public:

	WriteElement(std::string text = "", bool isExpression = false, bool newline = false) {
		this->text = text;
		this->isExpression = isExpression;
		this->newline = newline;
	}
	const struct DEFAULT_VALUES {
		const std::string text = "";
		const bool isExpression = false;
		const bool newline = false;
	} DEFAULT;
	std::string text;
	bool isExpression;
	bool newline;
};

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
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	List<WriteElement*>* writeElements() const;
	void setFilename(std::string _filename);
	std::string filename() const;
	void setWriteToType(WriteToType _writeToType);
	Write::WriteToType writeToType() const;

protected: // virtual
	virtual void _execute(Entity* entity);
	virtual void _initBetweenReplications();
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
private: // methods
	//std::string _buildText();
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		WriteToType writeToType = Write::WriteToType::SCREEN;
		std::string filename = "";
	} DEFAULT;
	WriteToType _writeToType = DEFAULT.writeToType;
	std::string _filename = DEFAULT.filename;
private: // attributes 1:n
	List<WriteElement*>* _writeElements = new List<WriteElement*>();
};


#endif /* WRITE_H */

