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
#include <fstream>
#include "kernel/simulator/ModelComponent.h"

/*!
 This component ...
 */
class Write : public ModelComponent {
public:
	enum class WriteToType : int {
		SCREEN = 0, FILE = 1, num_elements = 2
	};
public:
	static std::string convertEnumToStr(WriteToType type);
public: // constructors
	Write(Model* model, std::string name = "");
	virtual ~Write() = default;

public:
	void insertText(std::list<std::string> texts);
	void setFilename(std::string _filename);
	std::string filename() const;
	void setWriteToType(WriteToType _writeToType);
	Write::WriteToType writeToType() const;

public: // virtual
	virtual std::string show() override;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

protected: // virtual
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private: // methods
private: // attributes 1:1
	const struct DEFAULT_VALUES {
		const WriteToType writeToType = Write::WriteToType::SCREEN;
		const std::string filename = "";
	} DEFAULT;
	WriteToType _writeToType = DEFAULT.writeToType;
	std::string _filename = DEFAULT.filename;
	std::ofstream _savefile;

private: // attributes 1:n
	List<std::string>* _writeElements = new List<std::string>();
};


#endif /* WRITE_H */

