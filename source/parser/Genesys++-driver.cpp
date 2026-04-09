#include <string>
#include <exception>
#include "Genesys++-driver.h"
//#include "../Traits.h"

//using namespace GenesysKernel;

genesyspp_driver::genesyspp_driver() {
	_model = nullptr;
	_sampler = nullptr;
}

genesyspp_driver::genesyspp_driver(/*GenesysKernel::*/Model* model, Sampler_if* sampler, bool throws) {
	_model = model;
	_sampler = sampler;
	throwsException = throws;
}

genesyspp_driver::genesyspp_driver(const genesyspp_driver& other) {
	_model = other._model;
	_sampler = other._sampler;
	_isRegisterReferedDataElements = other._isRegisterReferedDataElements;
	result = other.result;
	file = other.file;
	str_to_parse = other.str_to_parse;
	throwsException = other.throwsException;
	errorMessage = other.errorMessage;
	_copyReferedDataElementsFrom(other);
}

genesyspp_driver& genesyspp_driver::operator=(const genesyspp_driver& other) {
	if (this != &other) {
		_destroyReferedDataElements();
		_model = other._model;
		_sampler = other._sampler;
		_isRegisterReferedDataElements = other._isRegisterReferedDataElements;
		result = other.result;
		file = other.file;
		str_to_parse = other.str_to_parse;
		throwsException = other.throwsException;
		errorMessage = other.errorMessage;
		_copyReferedDataElementsFrom(other);
	}
	return *this;
}

genesyspp_driver::~genesyspp_driver() {
	_destroyReferedDataElements();
}

/*
void genesyspp_driver::scan_begin_file() {
	// IMPLEMENTED IN LEXPARSER.LL
}

void genesyspp_driver::scan_end_file() {
	// IMPLEMENTED IN LEXPARSER.LL
}
 

void genesyspp_driver::scan_begin_str() {
	// // IMPLEMENTED IN LEXPARSER.LL
}

void genesyspp_driver::scan_end_str() {
	// IMPLEMENTED IN LEXPARSER.LL
}
 */
int genesyspp_driver::parse_file(const std::string &f) {
	result = 0;
	file = f;
	setErrorMessage("");
	scan_begin_file();
	yy::genesyspp_parser parser(*this);
	int res = -1;
	try {
		res = parser.parse();
	} catch (const std::string& e) {
		setErrorMessage(e);
		setResult(-1);
		scan_end_file();
		if (throwsException) {
			throw std::string(e);
		}
		return res;
	} catch (const std::exception& e) {
		const std::string message = e.what();
		setErrorMessage(message);
		setResult(-1);
		scan_end_file();
		if (throwsException) {
			throw std::string(message);
		}
		return res;
	} catch (...) {
		const std::string message = "Unknown parser error";
		setErrorMessage(message);
		setResult(-1);
		scan_end_file();
		if (throwsException) {
			throw std::string(message);
		}
		return res;
	}
	scan_end_file();
	return res;
}

int genesyspp_driver::parse_str(const std::string &str) {
	result = 0;
	str_to_parse = str;
	setErrorMessage("");
	scan_begin_str();
	yy::genesyspp_parser parser(*this);
	int res = -1;
	try {
		res = parser.parse();
	} catch (const std::string& e) {
		setErrorMessage(e);
		setResult(-1);
		scan_end_str();
		if (throwsException) {
			throw std::string(e);
		}
		return res;
	} catch (const std::exception& e) {
		const std::string message = e.what();
		setErrorMessage(message);
		setResult(-1);
		scan_end_str();
		if (throwsException) {
			throw std::string(message);
		}
		return res;
	} catch (...) {
		const std::string message = "Unknown parser error";
		setErrorMessage(message);
		setResult(-1);
		scan_end_str();
		if (throwsException) {
			throw std::string(message);
		}
		return res;
	}
	scan_end_str();
	return res;
}

void genesyspp_driver::setResult(double value) {
	result = value;
}

double genesyspp_driver::getResult() {
	return result;
}

bool genesyspp_driver::getThrowsException() {
	return throwsException;
}

void genesyspp_driver::setThrowsException(bool throws) {
	throwsException = throws;
}

void genesyspp_driver::setErrorMessage(std::string message) {
	errorMessage = message;
}

std::string genesyspp_driver::getErrorMessage() {
	return errorMessage;
}

/*GenesysKernel::*/Model* genesyspp_driver::getModel() {
	return _model;
}

std::string genesyspp_driver::getFile() {
	return file;
}

void genesyspp_driver::setFile(std::string f) {
	file = f;
}

std::string genesyspp_driver::getStrToParse() {
	return str_to_parse;
}

void genesyspp_driver::setStrToParse(std::string str) {
	str_to_parse = str;
}

void genesyspp_driver::setSampler(Sampler_if* _sampler) {
	this->_sampler = _sampler;
}

Sampler_if* genesyspp_driver::getSampler() const {
	return _sampler;
}

void genesyspp_driver::setRegisterReferedDataElements(bool value) {
	_isRegisterReferedDataElements = value;
}

bool genesyspp_driver::getRegisterReferedDataElements() {
	return _isRegisterReferedDataElements;
}

std::map<std::string, std::list<std::string>*>* genesyspp_driver::getReferedDataElements() {
	return _referedDataElements;
}
void genesyspp_driver::clearReferedDataElements() {
	for (auto& dataElementEntry : *_referedDataElements) {
		delete dataElementEntry.second;
	}
	_referedDataElements->clear();
}

void genesyspp_driver::addRefered(std::pair<std::string,std::string> referedElement) { // pair<DataElementTypename, name>
	if (_isRegisterReferedDataElements) {
		std::list<std::string> *listNamesRefered;
		if (_referedDataElements->find(referedElement.first) == _referedDataElements->end()) { // dataElemTyper never refered before
			_referedDataElements->insert({referedElement.first, new std::list<std::string>()});
		}
		auto it = _referedDataElements->find(referedElement.first);
		listNamesRefered = (*it).second;
		if (listNamesRefered==nullptr) {
			listNamesRefered = new std::list<std::string>();
			(*it).second = listNamesRefered;
		}
		for(std::string name: *listNamesRefered) {
			if (name==referedElement.second)
				return; // already exists. return and do not insert again
		}
		// not found. insert
		listNamesRefered->insert(listNamesRefered->end(), referedElement.second);
	}
	//_referedDataElements->insert(_referedDataElements->end(), referedElement);
}

void genesyspp_driver::error(const yy::location& l, const std::string& m) {
	//std::cout << "Location: "<<l.begin<<" - "<<l.end<<std::endl;
	std::string erro(m);
	erro.append(" from line "+ std::to_string(l.begin.line) + " column "+std::to_string(l.begin.column));
	erro.append(" to line "+std::to_string(l.end.line)+" column "+std::to_string(l.end.column));
	erro.append("\n");
	setErrorMessage(m);
	setResult(-1);
}

void
genesyspp_driver::error(const std::string& m) {
	setErrorMessage(m);
	setResult(-1);
}

void genesyspp_driver::_destroyReferedDataElements() {
	if (_referedDataElements != nullptr) {
		clearReferedDataElements();
		delete _referedDataElements;
		_referedDataElements = nullptr;
	}
}

void genesyspp_driver::_copyReferedDataElementsFrom(const genesyspp_driver& other) {
	_referedDataElements = new std::map<std::string, std::list<std::string>*>();
	for (const auto& dataElementEntry : *other._referedDataElements) {
		if (dataElementEntry.second != nullptr) {
			(*_referedDataElements)[dataElementEntry.first] = new std::list<std::string>(*dataElementEntry.second);
		} else {
			(*_referedDataElements)[dataElementEntry.first] = nullptr;
		}
	}
}
