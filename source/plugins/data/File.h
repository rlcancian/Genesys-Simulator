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


#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/ModelDataManager.h"
//#include "ParserChangesInformation.h"
#include "../../kernel/simulator/PluginInformation.h"

/*!
File module
DESCRIPTION
Use the File module to access external files for the ReadWrite module, Variable
module, and Expression module. The File module identifies the system file name and
defines the access type and operational characteristics of the file.
TYPICAL USES
* File containing predefined airline flight data
* File specifying customer order times and relevant information
* File to write user model configuration data from menu input
PROMPTS 
Prompt Description
Name The name of the file whose characteristics are being defined.
This name must be unique.
Access Type The file type.
Operating System File
Name
Name of the actual file that is being read from or to which it is
being written.
Connecting String Connection string used to open ADO connection to the data
source.
Structure File structure, which can be unformatted, free format, or a
specific C or FORTRAN format.
End of File Action Type of action to occur if an end of file condition is reached.
Initialize Option Action to be taken on file at beginning of each simulation
replication.
Comment Character Character indicating comment record.
Recordset Name Name used to identify the recordset in the Expression,
ReadWrite, and Variable modules. This name must be unique
within the file. This field is available for Microsoft Excel,
Microsoft Excel 2007, Microsoft Access, Microsoft Access
2007, and ActiveX Data Objects files.
CommandText Text of the command that will be used to open the recordset (for
example, SQL statement, procedure name, table name.) This
field is available for ActiveX Data Object files only.
CommandType Type of command entered in the CommandText.
Named Range The named range in the Excel workbook to which the recordset
refers.
Table Name The name of the table in the Access database to which the
recordset refers.
 */
class File : public ModelDataDefinition {
public:
	File(Model* model, std::string name = "");
	virtual ~File() = default;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show();

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // could be overriden 
	virtual bool _check(std::string* errorMessage);
	virtual ParserChangesInformation* _getParserChangesInformation();

};

#endif /* FILE_H */

