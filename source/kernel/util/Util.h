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
	 * \brief Returns the short textual symbol for a time unit.
	 * \param timeUnit Time unit to convert.
	 * \return Short symbol such as \c ms or \c h.
	 */
	static std::string StrTimeUnitShort(Util::TimeUnit timeUnit);
	/*!
	 * \brief Returns the long textual label for a time unit.
	 * \param timeUnit Time unit to convert.
	 * \return Long label such as \c millisecond or \c hour.
	 */
	static std::string StrTimeUnitLong(Util::TimeUnit timeUnit);
	/*!
	 * \brief Converts a time unit enum value to its canonical string representation.
	 * \param timeUnit Time unit to convert.
	 * \return Canonical string representation for the enum value.
	 */
	static std::string convertEnumToStr(Util::TimeUnit timeUnit);

	// @ToDo: (pequena alteração): check: here? Shouldn´t it be on SimulationReport interface?
	enum class AllocationType : int {
		ValueAdded = 0, NonValueAdded = 1, Transfer = 2, Wait = 3, Others = 4, num_elements = 5
	};

	/*!
	 * \brief Returns the string representation for an allocation category.
	 * \param allocation Allocation category to convert.
	 * \return Allocation label used in reports and logs.
	 */
	static std::string StrAllocation(Util::AllocationType allocation);
	/*!
	 * \brief Converts an allocation category enum value to a string.
	 * \param allocation Allocation category to convert.
	 * \return Canonical string representation for the enum value.
	 */
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
	/*!
	 * \brief Sets the indentation level used by \c Indent().
	 * \param indent New indentation depth.
	 */
	static void SetIndent(const unsigned short indent);
	/*!
	 * \brief Increments the current indentation level.
	 */
	static void IncIndent();
	/*!
	 * \brief Decrements the current indentation level.
	 */
	static void DecIndent();
	/*!
	 * \brief Splits a key-value string around the first \c = character.
	 * \param str Source text in the form \c key=value.
	 * \param key Output key portion.
	 * \param value Output value portion.
	 */
	static void SepKeyVal(std::string str, std::string& key, std::string& value);
	/*!
	 * \brief Returns the current indentation prefix.
	 * \return A string made of repeated indentation markers.
	 */
	static std::string Indent();
	/*!
	 * \brief Pads or truncates text to a fixed width.
	 * \param text Text to adjust.
	 * \param width Target width.
	 * \return Text resized to the requested width.
	 */
	static std::string SetW(std::string text, unsigned short width);
	/*!
	 * \brief Formats a numeric value without losing integer semantics.
	 * \param value Value to format.
	 * \return A string that preserves the decimal point for exact integers.
	 */
	static std::string StrTruncIfInt(double value);
	/*!
	 * \brief Removes a trailing \c .000000 suffix when present.
	 * \param strValue String representation to normalize.
	 * \return Normalized string value.
	 */
	static std::string StrTruncIfInt(std::string strValue);
	/*!
	 * \brief Trims leading and trailing whitespace.
	 * \param str Input string.
	 * \return Trimmed string.
	 */
	static std::string Trim(std::string str);
	/*!
	 * \brief Replaces all occurrences of one substring with another.
	 * \param text Input text.
	 * \param searchFor Substring to replace.
	 * \param replaceBy Replacement text.
	 * \return Updated text.
	 */
	static std::string StrReplace(std::string text, std::string searchFor, std::string replaceBy);
	/*!
	 * \brief Sanitizes a text for identifier-like usage.
	 * \param text Input text.
	 * \return Text with spaces and selected punctuation normalized.
	 */
	static std::string StrReplaceSpecialChars(std::string text);
	/*!
	 * \brief Formats an integer index in bracket notation.
	 * \param index Index to format.
	 * \return Text such as \c [4].
	 */
	static std::string StrIndex(int index);
	//static char* Str2CharPtr(std::string str);
	/*!
	 * \brief Removes all spaces from a string in place.
	 * \param str String to normalize.
	 */
	static void Trimwithin(std::string& str);

public: // show strucutres
	/*!
	 * \brief Serializes a string-to-string map to a compact text form.
	 * \param mapss Map to serialize.
	 * \return One-line representation of the map contents.
	 */
	static std::string Map2str(std::map<std::string, std::string>* mapss);
	/*!
	 * \brief Serializes a string-to-double map to a compact text form.
	 * \param mapss Map to serialize.
	 * \return One-line representation of the map contents.
	 */
	static std::string Map2str(std::map<std::string, double>* mapss);
	/*!
	 * \brief Serializes a list of unsigned integers to a compact text form.
	 * \param list List to serialize.
	 * \return One-line representation of the list contents.
	 */
	static std::string List2str(std::list<unsigned int>* list);

public:
	// identitification // @ToDo: (importante): CHECK ALL, since some should be private and available to FRIEND classes in the kernel
	/*!
	 * \brief Generates a globally unique identifier.
	 * \return Next process-wide identifier value.
	 */
	static Util::identification GenerateNewId();
	/*!
	 * \brief Generates the next identifier for a given runtime type name.
	 * \param objtype Type name used as identifier namespace.
	 * \return Next identifier within the requested type namespace.
	 */
	static Util::identification GenerateNewIdOfType(std::string objtype);
	/*!
	 * \brief Returns the most recently generated identifier for a type.
	 * \param objtype Type name used as identifier namespace.
	 * \return Last identifier value for the requested type.
	 */
	static Util::identification GetLastIdOfType(std::string objtype);
	/*!
	 * \brief Resets the identifier counter for a specific type.
	 * \param objtype Type name to reset.
	 */
	static void ResetIdOfType(std::string objtype);
	/*!
	 * \brief Resets all identifier counters maintained by the utility class.
	 */
	static void ResetAllIds();

public: // simulation support
	/*!
	 * \brief Computes the multiplicative factor between two time units.
	 * \param timeUnit1 Source unit.
	 * \param timeUnit2 Target unit.
	 * \return Conversion factor from \c timeUnit1 to \c timeUnit2.
	 */
	static double TimeUnitConvert(Util::TimeUnit timeUnit1, Util::TimeUnit timeUnit2);

public: // files
	/*!
	 * \brief Returns the platform directory separator character.
	 * \return Directory separator for the current platform.
	 */
	static char DirSeparator();
	/*!
	 * \brief Extracts the file name from a full path.
	 * \param s Full file path.
	 * \return Trailing file name portion.
	 */
	static std::string FilenameFromFullFilename(const std::string& s);
	/*!
	 * \brief Deletes a file if it exists.
	 * \param filename Path to the file to delete.
	 */
	static void FileDelete(const std::string& filename);
	/*!
	 * \brief Extracts the directory path from a full filename.
	 * \param s Full file path.
	 * \return Directory portion of the path.
	 */
	static std::string PathFromFullFilename(const std::string& s);
	/*!
	 * \brief Returns the directory that contains the running binary.
	 * \return Current executable directory.
	 */
	static std::string RunningPath();
	/*!
	 * \brief Lists files in a directory using an optional substring filter.
	 * \param dir Directory to scan.
	 * \param fileFilter Optional substring filter for file names.
	 * \param attribFilter Attribute mask applied to \c stat results.
	 * \return Matching file names found in the directory.
	 */
	static std::vector<std::string> ListFiles(std::string dir, std::string fileFilter = "",
	                                          mode_t attribFilter = S_IFREG & S_IFDIR);
	/*!
	 * \brief Checks whether a file can be opened for reading.
	 * \param name File path to test.
	 * \return \c true if the file exists and can be opened.
	 */
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

	/*!
	 * \brief Executes a shell command and captures the result.
	 * \param command Command line to run.
	 * \return Command execution outcome and captured output buffers.
	 */
	static Util::CommandResult ExecuteCommand(std::string command);

public: // template implementations

	/*!
	 * \brief Returns the compiler type name used for \c T.
	 * \tparam T Type to inspect.
	 * \return Compiler-provided type name, normalized when possible.
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
	 * \brief Generates the next identifier in the namespace of \c T.
	 * \tparam T Type whose namespace should receive a new identifier.
	 * \return Next sequential identifier for the type.
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
