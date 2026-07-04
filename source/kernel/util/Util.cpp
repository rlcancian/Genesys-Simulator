/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Util.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 13:02
 */

#include "Util.h"
#include <typeinfo>
#include <map>

//using namespace GenesysKernel;

Util::identification Util::_S_lastId = 0;
unsigned int Util::_S_indentation;
std::map<std::string, Util::identification> Util::_S_lastIdOfType = std::map<std::string, Util::identification>();
std::map<std::string, std::string> Util::_S_TypeOf = std::map<std::string, std::string>();

void Util::IncIndent() {
	Util::_S_indentation++;
}

void Util::SetIndent(const unsigned short indent) {
	Util::_S_indentation = indent;
}

void Util::DecIndent() {
	Util::_S_indentation--;
}

void Util::SepKeyVal(std::string str, std::string &key, std::string &value) {
	bool settingKey = true;
	key.clear();
	value.clear();
	key.reserve(256);
	value.reserve(256);
	for (std::string::iterator it = str.begin(); it != str.end(); it++) {
		const char c = (*it);
		if (settingKey) {
			if (c != '=') 
				key+=c;
			else
				settingKey = false;
		} else {
			value+=c;
		}
	}
	key.shrink_to_fit();
	value.shrink_to_fit();
}

std::string Util::Indent() {
	std::string spaces = "";
	for (unsigned int i = 0; i < Util::_S_indentation; i++) {
		spaces += "|  ";
	}
	return spaces;
}

std::string Util::SetW(std::string text, unsigned short width) {
	std::string spaces(width, ' ');
	std::string result = text + spaces;
	return result.substr(0, width);
}

std::string Util::StrTimeUnitShort(Util::TimeUnit timeUnit) {
	switch (static_cast<int> (timeUnit)) {
		case 1: return "ps";
		case 2: return "ns";
		case 3: return "us";
		case 4: return "ms";
		case 5: return "s";
		case 6: return "min";
		case 7: return "h";
		case 8: return "d";
		case 9: return "w";
		default: break;
	}
	return "";
}

std::string Util::StrTimeUnitLong(Util::TimeUnit timeUnit) {
	switch (static_cast<int> (timeUnit)) {
		case 1: return "picosecond";
		case 2: return "nanosecond";
		case 3: return "microsecond";
		case 4: return "milisecond";
		case 5: return "second";
		case 6: return "minute";
		case 7: return "hour";
		case 8: return "day";
		case 9: return "week";
	}
	return "unknown";
}

std::string Util::convertEnumToStr(Util::TimeUnit timeUnit) {
	switch (static_cast<int> (timeUnit)) {
		case 1: return "picosecond";
		case 2: return "nanosecond";
		case 3: return "microsecond";
		case 4: return "milisecond";
		case 5: return "second";
		case 6: return "minute";
		case 7: return "hour";
		case 8: return "day";
		case 9: return "week";
	}
	return "unknown";
}

std::string Util::convertEnumToStr(Util::AllocationType allocation) {
	switch (allocation) {
		case Util::AllocationType::NonValueAdded:
			return "NonValueAdded";
		case Util::AllocationType::Others:
			return "Others";
		case Util::AllocationType::Transfer:
			return "Transfer";
		case Util::AllocationType::ValueAdded:
			return "ValueAdded";
		case Util::AllocationType::Wait:
			return "Wait";
		default:
			return "Unknown";
	}
}

std::string Util::StrAllocation(Util::AllocationType allocation) {
	switch (allocation) {
		case Util::AllocationType::NonValueAdded:
			return "NonValueAdded";
		case Util::AllocationType::Others:
			return "Others";
		case Util::AllocationType::Transfer:
			return "Transfer";
		case Util::AllocationType::ValueAdded:
			return "ValueAdded";
		case Util::AllocationType::Wait:
			return "Wait";
		default:
			return "Unknown";
	}
}

Util::identification Util::GenerateNewId() {
	Util::_S_lastId++;
	return Util::_S_lastId;
}

Util::identification Util::GenerateNewIdOfType(std::string objtype) {
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

Util::identification Util::GetLastIdOfType(std::string objtype) {
	std::map<std::string, Util::identification>::iterator it = Util::_S_lastIdOfType.find(objtype);
	if (it == Util::_S_lastIdOfType.end()) {
		// a new one. create the pair
		Util::_S_lastIdOfType.insert(std::pair<std::string, Util::identification>(objtype, 0));
		it = Util::_S_lastIdOfType.find(objtype);
	}
	//Util::identification next = (*it).second;
	//next++;
	//(*it).second = next;
	return (*it).second;

}

void Util::ResetIdOfType(std::string objtype) {
	std::map<std::string, Util::identification>::iterator it = Util::_S_lastIdOfType.find(objtype);
	if (it == Util::_S_lastIdOfType.end()) {
		// a new one. create the pair
		Util::_S_lastIdOfType.insert(std::pair<std::string, Util::identification>(objtype, 0));
		it = Util::_S_lastIdOfType.find(objtype);
	}
	(*it).second = 0;
}

void Util::ResetAllIds() {
	_S_lastIdOfType = std::map<std::string, Util::identification>();
}

double Util::TimeUnitConvert(Util::TimeUnit timeUnit1, Util::TimeUnit timeUnit2) {
	double scaleValues[] = {1.0, 1000.0, 1000.0, 1000.0, 1000.0, 1000.0, 60.0, 60.0, 24.0, 7.0};
	// picosecond = 1, nanosecond = 2, microsecond = 3, milisecond = 4, second = 5, minute = 6, hour = 7, day = 8, week = 9
	double res = 1.0;
	int intTimeUnit1, intTimeUnit2;
	intTimeUnit1 = static_cast<int> (timeUnit1);
	intTimeUnit2 = static_cast<int> (timeUnit2);
	if (intTimeUnit1 <= intTimeUnit2) {
		for (int i = intTimeUnit1 + 1; i <= intTimeUnit2; i++) {
			res /= scaleValues[i];
		}
	} else {
		for (int i = intTimeUnit2 + 1; i <= intTimeUnit1; i++) {
			res *= scaleValues[i];
		}
	}
	return res;
}

//-------------------

std::string Util::StrTruncIfInt(double value) {
	int intvalue = static_cast<int> (value);
	if (intvalue == value)
		return std::to_string(intvalue) + ".0";
	else
		return std::to_string(value);
}

std::string Util::StrTruncIfInt(std::string strValue) {
	if (strValue.length() > 7 && strValue.substr(strValue.length() - 7, 7) == ".000000")
		return strValue.substr(0, strValue.length() - 7);
	else
		return strValue;
}

std::string Util::Trim(std::string str) {
	const char* typeOfWhitespaces = " \t\n\r\f\v";
	str.erase(str.find_last_not_of(typeOfWhitespaces) + 1);
	str.erase(0, str.find_first_not_of(typeOfWhitespaces));
	return str;
}

std::string Util::StrReplace(std::string text, std::string searchFor, std::string replaceBy) {
	unsigned int pos = text.find(searchFor, 0);
	while (pos < text.length()) {// != std::string::npos) {
		text = text.replace(pos, searchFor.length(), replaceBy);
		pos = text.find(searchFor, 0);
	}
	return text;
}

std::string Util::StrReplaceSpecialChars(std::string text) {
	text = StrReplace(text, " ", "_");
	text = StrReplace(text, "::", "_");
	text = StrReplace(text, "*", "_");
	text = StrReplace(text, "\"", "");
	return text;
}


// returns a string in the form "[<index>] for array indexes"
std::string Util::StrIndex(int index) {
	return "[" + std::to_string(index) + "]";
}

/*
char* Util::Str2CharPtr(std::string str) {
	char * cstr = new char [str.length() + 1];
	std::strcpy(cstr, str.c_str());
	// cstr now contains a c-string copy of str
	char * p = std::strtok(cstr, " ");
	while (p != 0) {
		std::cout << p << '\n';
		p = std::strtok(NULL, " ");
	}
	return cstr;
}
*/

// trim all spaces within the string (in place) -- used to transform general names into valid literals

void Util::Trimwithin(std::string &str) {
	//ltrim(s);
	//rtrim(s);
	//s.erase(std::remove_if(s.begin(), s.end(), std::isspace), s.end());
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
}

std::string Util::Map2str(std::map<std::string, std::string>* mapss) {
	std::string res = "";
	for (std::map<std::string, std::string>::iterator it = mapss->begin(); it != mapss->end(); it++) {
		res += (*it).first + "=" + (*it).second + " ";
	}
	res = res.substr(0, res.length() - 1);
	return res;
}

std::string Util::Map2str(std::map<std::string, double>* mapss) {
	std::string res = "";
	for (std::map<std::string, double>::iterator it = mapss->begin(); it != mapss->end(); it++) {
		res += (*it).first + "=" + StrTruncIfInt(std::to_string((*it).second)) + " ";
	}
	res = res.substr(0, res.length() - 1);
	return res;
}

char Util::DirSeparator() {
#ifdef __linux__
    return '/';
#elif _WIN32
    return '\\';
#else
    return '/';
#endif
}

std::string Util::List2str(std::list<unsigned int>* list) {
	std::string res = "";
	for (unsigned int elem : *list) {
		res += std::to_string(elem) + ", ";
	}
	res = res.substr(0, res.length() - 2);
	return res;
}

std::string Util::FilenameFromFullFilename(const std::string& s) {
	char sep = DirSeparator();
	size_t i = s.rfind(sep, s.length());
	if (i != std::string::npos) {
		return (s.substr(i + 1, s.length() - i));
	}
	return s;
}

void Util::FileDelete(const std::string& filename) {
	char removeFilename[filename.length() + 1];
	strcpy(removeFilename, filename.c_str());
	std::remove(removeFilename);
}

std::string Util::PathFromFullFilename(const std::string& s) {
	char sep = DirSeparator();
	return s.substr(0, s.find_last_of(sep));
}

std::string Util::RunningPath() {
    char result[PATH_MAX];
#ifdef __linux__
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
#elif _WIN32
    // @ToDo: (pequena alteração): Get runningPath For windows
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
#endif
	std::string fullfilename = std::string(result, (count > 0) ? count : 0);
	return PathFromFullFilename(fullfilename);
}

std::vector<std::string> Util::ListFiles(std::string dir, std::string fileFilter, mode_t attribFilter) {
	std::vector<std::string> files;
	DIR *dp;
	struct dirent *dirp;
	struct stat statbuffer;
	if ((dp = opendir(dir.c_str())) == NULL) {
		//cout << "Error(" << errno << ") opening " << dir << endl;
		//return errno;
	}
	int status;
	while ((dirp = readdir(dp)) != NULL) {
		status = stat(dirp->d_name, &statbuffer);
		if (status & attribFilter) { // https://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
			if (fileFilter == "" || std::string(dirp->d_name).find(fileFilter) != std::string::npos) {
				files.push_back(std::string(dirp->d_name));
			}
		}
	}
	closedir(dp);
	return files;
}

bool Util::FileExists(const std::string& name) {
	try {
		if (FILE * file = fopen(name.c_str(), "r")) {
			fclose(file);
			return true;
		} else {
			return false;
		}
	} catch (const std::exception& e) {
		return false;

	}
}


//-------------------

Util::CommandResult Util::ExecuteCommand(std::string command) {
    (void)command;

    CommandResult result;
    result.success = false;
    result.commandErrOutput = "Not implemented";
    return result;
}
