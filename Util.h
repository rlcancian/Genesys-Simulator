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

#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <typeinfo>
#include <string>
#include <cctype>
#include <algorithm> 
#include <cctype>
#include <locale>

//namespace GenesysKernel {
// trim from start (in place)

static inline std::string map2str(std::map<std::string, std::string>* mapss) {
	std::string res = "";
	for (std::map<std::string, std::string>::iterator it = mapss->begin(); it != mapss->end(); it++) {
		res += (*it).first + "=" + (*it).second + " ";
	}
	res = res.substr(0, res.length() - 1);
	return res;
}

/*
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)

static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)

static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}
*/

static std::string trim(std::string str) {
	const char* typeOfWhitespaces = " \t\n\r\f\v";
	str.erase(str.find_last_not_of(typeOfWhitespaces) + 1);
	str.erase(0,str.find_first_not_of(typeOfWhitespaces));
	return str;
}

// trim all spaces within the string (in place) -- used to transform general names into valid literals

static inline void trimwithin(std::string &str) {
	//ltrim(s);
	//rtrim(s);
	//s.erase(std::remove_if(s.begin(), s.end(), std::isspace), s.end());
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
}

static inline std::string getFileName(const std::string& s) {
	char sep = '/';
	size_t i = s.rfind(sep, s.length());
	if (i != std::string::npos) {
		return (s.substr(i + 1, s.length() - i));
	}
	return s;
}

class Util {
public:
	typedef unsigned long identification;
	typedef unsigned int rank;

	enum class TimeUnit : int {
		picosecond = 1,
		nanosecond = 2,
		microsecond = 3,
		milisecond = 4,
		second = 5,
		minute = 6,
		hour = 7,
		day = 8,
		week = 9
	};
	//static std::string ToStrTimeUnit(TimeUnit tu);

	enum class TraceLevel : int {
		L0_noTraces = 0,
		L1_errorFatal = 1,
		L2_errorRecover = 2,
		L3_results = 3,
		L4_warning = 4,
		L5_arrival = 5,
		L6_internal = 6,
		L7_detailed = 7,
		L8_mostDetailed = 8
/*				,
		
		simulatorResult = 10,
		toolResult = 11,
		modelResult = 12,
		componentResult = 13,
		elementResult = 14,

		modelSimulationEvent = 15,

		componentArrival = 20,
		simulatorInternal = 21,
		toolInternal = 22,
		modelSimulationInternal = 23,
		modelInternal = 24,
		componentInternal = 25,
		elementInternal = 26,

		simulatorDetailed = 30,
		toolDetailed = 31,
		modelSimulationDetailed = 32,
		modelDetailed = 33,
		componentDetailed = 34,
		elementDetailed = 35,

		everythingMostDetailed = 99
 */ 
	};
private:
	static Util::identification _S_lastId;
	static std::map<std::string, Util::identification> _S_lastIdOfType;
	static std::map<std::string, std::string> _S_TypeOf;

public: // indentation and string
	static unsigned int _S_indentation; // \todo: IS PRIVATE. ITS HERE JUST TO INCLUDE IT AS A WATCH
	static void SetIndent(const unsigned short indent);
	static void IncIndent();
	static void DecIndent();
	static void SepKeyVal(std::string str, std::string *key, std::string *value);
	static std::string Indent();
	static std::string SetW(std::string text, unsigned short width);
	static std::string StrTimeUnitShort(Util::TimeUnit timeUnit);
	static std::string StrTimeUnitLong(Util::TimeUnit timeUnit);
public: // identitification
	static Util::identification GenerateNewId();
	static Util::identification GenerateNewIdOfType(std::string objtype);
	static Util::identification GetLastIdOfType(std::string objtype);
	static void ResetIdOfType(std::string objtype);

public: // simulation support
	static double TimeUnitConvert(Util::TimeUnit timeUnit1, Util::TimeUnit timeUnit2);

public: // template implementations

	/*!
	 * Return the name of the class used as T.
	 */
	template<class T> static std::string TypeOf() {
		std::string name = typeid (T).name();
		std::map<std::string, std::string>::iterator it = _S_TypeOf.find(name);
		if (it != _S_TypeOf.end()) {
			return (*it).second;
		} else {
			std::string newname(name);
			while (std::isdigit(newname[0])) {
				newname.erase(0, 1);
			}
			_S_TypeOf.insert(std::pair<std::string, std::string>(name, newname));
			return newname;
		}
	}

	/*!
	 * Every component or element has a unique ID for its class, but not unique for other classes. IDs are generated sequentially for each class.
	 */
	template<class T> static Util::identification GenerateNewIdOfType() {
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


// \todo Implement it using templates (check impact on calling syntax)

static inline std::string LoadField(std::map<std::string, std::string>* fields, std::string fieldName, std::string defaultValue = "") {
	return fields->find(fieldName) != fields->end() ? ((*(fields->find(fieldName))).second) : defaultValue;
}

static inline double LoadField(std::map<std::string, std::string>* fields, std::string fieldName, double defaultValue) {
	return std::stod(fields->find(fieldName) != fields->end() ? ((*(fields->find(fieldName))).second) : std::to_string(defaultValue));
}

static inline unsigned int LoadField(std::map<std::string, std::string>* fields, std::string fieldName, unsigned int defaultValue) {
	return std::stoi(fields->find(fieldName) != fields->end() ? ((*(fields->find(fieldName))).second) : std::to_string(defaultValue));
}

static inline int LoadField(std::map<std::string, std::string>* fields, std::string fieldName, int defaultValue) {
	return std::stoi(fields->find(fieldName) != fields->end() ? ((*(fields->find(fieldName))).second) : std::to_string(defaultValue));
}

static inline Util::TimeUnit LoadField(std::map<std::string, std::string>* fields, std::string fieldName, Util::TimeUnit defaultValue) {
	return static_cast<Util::TimeUnit>(std::stoi(fields->find(fieldName) != fields->end() ? ((*(fields->find(fieldName))).second) : std::to_string(static_cast<int> (defaultValue))));
}

// \todo Implement it using templates (check impact on calling syntax)

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, std::string fieldValue, const std::string fieldDefaultValue = "") {
	if (fieldValue != fieldDefaultValue)
		fields->emplace(fieldName, "\"" + fieldValue + "\"");
}

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, double fieldValue, const double fieldDefaultValue) {
	if (fieldValue != fieldDefaultValue)
		fields->emplace(fieldName, std::to_string(fieldValue));
}

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, unsigned int fieldValue, const unsigned int fieldDefaultValue) {
	if (fieldValue != fieldDefaultValue)
		fields->emplace(fieldName, std::to_string(fieldValue));
}

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, unsigned int fieldValue) {
	fields->emplace(fieldName, std::to_string(fieldValue));
}

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, int fieldValue, const int fieldDefaultValue) {
	if (fieldValue != fieldDefaultValue)
		fields->emplace(fieldName, std::to_string(fieldValue));
}

static inline void SaveField(std::map<std::string, std::string>* fields, std::string fieldName, Util::TimeUnit fieldValue, const Util::TimeUnit fieldDefaultValue) {
	if (fieldValue != fieldDefaultValue)
		fields->emplace(fieldName, std::to_string(static_cast<int> (fieldValue)));
}

//namespace\\}
#endif /* UTIL_H */

