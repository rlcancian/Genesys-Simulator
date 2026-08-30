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
#include "../../../kernel/simulator/model/ModelComponent.h"

/*!
 * \brief Writes one or more literal/expression text lines to the screen
 * (trace output) or to a file.
 *
 * Arena correspondence: the write-only slice of Arena's "ReadWrite module"
 * (Rockwell Automation, *Getting Started with Arena*, "The Advanced
 * Process Panel", p. 58). `WriteToType` (`SCREEN/FILE`) matches Arena's
 * corresponding output destinations.
 *
 * Known difference from Arena: there is no read direction at all (no
 * component reads a value from Screen/File/Attribute/Variable back into
 * the model), and no structured recordset integration with File (§5.10) —
 * see `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.16.
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
	virtual ~Write() override;

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
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

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
