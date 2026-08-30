/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   File.h
 * Author: rlcancian
 *
 * Created on 20 de Fileembro de 2019, 20:07
 */

#ifndef FILE_H
#define FILE_H


#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
//#include "ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"

/*!
 * \brief Data definition identifying an external file and its access mode
 * (read/write/append/read-write), used by file-driven components and data
 * definitions.
 *
 * Arena correspondence: the "File module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", pp. 72-73), which
 * backs the ReadWrite, Variable and Expression modules with a much richer
 * set of structured data sources (typed file access, ADO connection
 * strings, fixed/free/Fortran record structure, recordsets, SQL command
 * text, named Excel ranges, Access table names, etc.).
 *
 * Known difference from Arena: this class only models a plain
 * operating-system file name and an \c AccessMode (Read/Write/Append/
 * ReadWrite); none of Arena's structured recordset/ADO/spreadsheet/database
 * concepts have a GenESyS equivalent, so this correspondence is partial and
 * covers only unformatted file access.
 */
class File : public ModelDataDefinition {
public:
	enum class AccessMode : unsigned int {
		Read = 0,
		Write = 1,
		Append = 2,
		ReadWrite = 3
	};

	File(Model* model, std::string name = "");
	virtual ~File() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setSystemFilename(std::string systemFilename);
	std::string getSystemFilename() const;
	std::string getFilenameOnly() const;
	std::string getPathOnly() const;

	void setAccessMode(AccessMode accessMode);
	AccessMode getAccessMode() const;
	void setAccessModeAsString(std::string accessMode);
	std::string getAccessModeAsString() const;

	virtual std::string show() override;

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden 
	virtual bool _check(std::string& errorMessage) override;
	virtual ParserChangesInformation* _getParserChangesInformation() override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	static std::string _accessModeToString(AccessMode accessMode);
	static bool _stringToAccessMode(const std::string& accessMode, AccessMode* parsedAccessMode);
	static std::string _normalizePathSeparators(const std::string& filename);

private:
	const struct DEFAULT_VALUES {
		std::string systemFilename = "";
		AccessMode accessMode = AccessMode::Read;
	} DEFAULT;
	std::string _systemFilename = DEFAULT.systemFilename;
	AccessMode _accessMode = DEFAULT.accessMode;
	bool _accessModeWasInvalid = false;
};

#endif /* FILE_H */
