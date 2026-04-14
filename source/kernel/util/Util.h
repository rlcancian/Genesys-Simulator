/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Util.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 13:02
 */


//
// VINCULADOR
// SAIDA
// ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/genesysterminalapplication
//


#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <typeinfo>
#include <string>
#include <list>
#include <cctype>
#include <algorithm>
#include <cctype>
#include <locale>
#include <stdio.h>
// dir
#include <limits.h>
#include <unistd.h>
#include <vector>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <sys/stat.h>
#include <cstring>

#include <iostream>
#include <string>
//namespace GenesysKernel {

class Util {
public:
	typedef unsigned long identification;
	typedef unsigned int rank;

	// @ToDo: (pequena alteração): Should be insde ModelSimulation, where time goes on

	/*!
	 * \brief The TimeUnit enum
	 */
	enum class TimeUnit : int {
		unknown = 0,
		picosecond = 1,
		nanosecond = 2,
		microsecond = 3,
		milisecond = 4,
		second = 5,
		minute = 6,
		hour = 7,
		day = 8,
		week = 9,
		num_elements = 10
	};

	/*!
	 * \brief StrTimeUnitShort
	 * \param timeUnit
	 * \return
	 */
	static std::string StrTimeUnitShort(Util::TimeUnit timeUnit);
	/*!
	 * \brief StrTimeUnitLong
	 * \param timeUnit
	 * \return
	 */
	static std::string StrTimeUnitLong(Util::TimeUnit timeUnit);
	static std::string convertEnumToStr(Util::TimeUnit timeUnit);

	// @ToDo: (pequena alteração): check: here? Shouldn´t it be on SimulationReport interface?
	enum class AllocationType : int {
		ValueAdded = 0, NonValueAdded = 1, Transfer = 2, Wait = 3, Others = 4, num_elements = 5
	};

	static std::string StrAllocation(Util::AllocationType allocation);
	static std::string convertEnumToStr(Util::AllocationType allocation);

	enum class TimeFormat : unsigned int {
		twelve = 12,
		twentyFour = 24,
	};

private:
	static unsigned int _S_indentation;
	static Util::identification _S_lastId;
	static std::map<std::string, Util::identification> _S_lastIdOfType;
	static std::map<std::string, std::string> _S_TypeOf;

public: // indentation and string
	static void SetIndent(const unsigned short indent);
	static void IncIndent();
	static void DecIndent();
	static void SepKeyVal(std::string str, std::string& key, std::string& value);
	static std::string Indent();
	static std::string SetW(std::string text, unsigned short width);
	static std::string StrTruncIfInt(double value);
	static std::string StrTruncIfInt(std::string strValue);
	static std::string Trim(std::string str);
	static std::string StrReplace(std::string text, std::string searchFor, std::string replaceBy);
	static std::string StrReplaceSpecialChars(std::string text);
	static std::string StrIndex(int index);
	//static char* Str2CharPtr(std::string str);
	static void Trimwithin(std::string& str);

public: // show strucutres
	static std::string Map2str(std::map<std::string, std::string>* mapss);
	static std::string Map2str(std::map<std::string, double>* mapss);
	static std::string List2str(std::list<unsigned int>* list);

public:
	// identitification // @ToDo: (importante): CHECK ALL, since some should be private and available to FRIEND classes in the kernel
	static Util::identification GenerateNewId();
	static Util::identification GenerateNewIdOfType(std::string objtype);
	static Util::identification GetLastIdOfType(std::string objtype);
	static void ResetIdOfType(std::string objtype);
	static void ResetAllIds();

public: // simulation support
	static double TimeUnitConvert(Util::TimeUnit timeUnit1, Util::TimeUnit timeUnit2);

public: // files
	static char DirSeparator();
	static std::string FilenameFromFullFilename(const std::string& s);
	static void FileDelete(const std::string& filename);
	static std::string PathFromFullFilename(const std::string& s);
	static std::string RunningPath();
	static std::vector<std::string> ListFiles(std::string dir, std::string fileFilter = "",
	                                          mode_t attribFilter = S_IFREG & S_IFDIR);
	static bool FileExists(const std::string& name);

	//public: // operating system specifics
	class CommandResult {
	public:
		CommandResult() {
		}

		bool success = false;
		std::string commandStdOutput = "";
		std::string commandErrOutput = "";
		std::string workingDirectory = "./";
		std::string destinationPath = "./";
	};

	static Util::CommandResult ExecuteCommand(std::string command);

public: // template implementations

	/*!
	 * Return the name of the class used as T.
	 */
	template <typename T>
	static std::string TypeOf() {
		std::string name = typeid(T).name();
		std::map<std::string, std::string>::iterator it = _S_TypeOf.find(name);
		if (it != _S_TypeOf.end()) {
			return (*it).second;
		}
		else {
			std::string newname(name);
			while (std::isdigit(newname[0])) {
				newname.erase(0, 1);
			}
			_S_TypeOf.insert(std::pair<std::string, std::string>(name, newname));
			return newname;
		}
	}

	/*!
	 * Every component or modeldatum has a unique ID for its class, but not unique for other classes. IDs are generated sequentially for each class.
	 */
	template <class T>
	static Util::identification GenerateNewIdOfType() {
		std::string objtype = Util::TypeOf<T>();
		std::map<std::string, Util::identification>::iterator it = Util::_S_lastIdOfType.find(objtype);
		if (it == Util::_S_lastIdOfType.end()) {
			// a new one. create the pair
			Util::_S_lastIdOfType.insert(std::pair<std::string, Util::identification>(objtype, 0));
			it = Util::_S_lastIdOfType.find(objtype);
		}
		Util::identification next = (*it).second;
		next++;
		(*it).second = next;
		return (*it).second;
	}

private:
	Util();
	virtual ~Util() = default;
};

//namespace\\}
#endif /* UTIL_H */
