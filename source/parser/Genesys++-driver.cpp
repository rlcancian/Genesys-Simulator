#include <string>
#include <exception>
#include "Genesys++-driver.h"
//#include "../Traits.h"

//using namespace GenesysKernel;

namespace {
// Deep-copy the referred-data registry so copied drivers do not share list ownership.
std::map<std::string, std::list<std::string>*>* cloneReferedDataElements(const std::map<std::string, std::list<std::string>*>* source) {
	auto* cloned = new std::map<std::string, std::list<std::string>*>();
	if (source == nullptr) {
		return cloned;
	}
	for (const auto& entry : *source) {
		cloned->insert({entry.first, entry.second != nullptr ? new std::list<std::string>(*entry.second) : nullptr});
	}
	return cloned;
}
}

genesyspp_driver::genesyspp_driver() {}

genesyspp_driver::genesyspp_driver(/*GenesysKernel::*/Model* model, Sampler_if* sampler, bool throws) {
	_model = model;
	_sampler = sampler;
	throwsException = throws;
}

// Release all dynamically allocated lists and the registry map owned by this driver.
genesyspp_driver::~genesyspp_driver() {
	clearReferedDataElements();
	delete _referedDataElements;
	_referedDataElements = nullptr;
}

// Copy constructor performs a deep copy of referred-data lists to avoid shared ownership.
genesyspp_driver::genesyspp_driver(const genesyspp_driver& other)
	: _model(other._model),
	  _sampler(other._sampler),
	  _referedDataElements(cloneReferedDataElements(other._referedDataElements)),
	  _isRegisterReferedDataElements(other._isRegisterReferedDataElements),
	  result(other.result),
	  file(other.file),
	  str_to_parse(other.str_to_parse),
	  throwsException(other.throwsException),
	  errorMessage(other.errorMessage) {}

// Copy assignment replaces local registry with a deep-copied registry from the source driver.
genesyspp_driver& genesyspp_driver::operator=(const genesyspp_driver& other) {
	if (this == &other) {
		return *this;
	}
	auto* newReferedDataElements = cloneReferedDataElements(other._referedDataElements);
	clearReferedDataElements();
	delete _referedDataElements;
	_model = other._model;
	_sampler = other._sampler;
	_referedDataElements = newReferedDataElements;
	_isRegisterReferedDataElements = other._isRegisterReferedDataElements;
	result = other.result;
	file = other.file;
	str_to_parse = other.str_to_parse;
	throwsException = other.throwsException;
	errorMessage = other.errorMessage;
	return *this;
}

// Move constructor transfers the registry pointer and leaves source in a valid empty state.
genesyspp_driver::genesyspp_driver(genesyspp_driver&& other) noexcept
	: _model(other._model),
	  _sampler(other._sampler),
	  _referedDataElements(other._referedDataElements),
	  _isRegisterReferedDataElements(other._isRegisterReferedDataElements),
	  result(other.result),
	  file(std::move(other.file)),
	  str_to_parse(std::move(other.str_to_parse)),
	  throwsException(other.throwsException),
	  errorMessage(std::move(other.errorMessage)) {
	other._model = nullptr;
	other._sampler = nullptr;
	other._referedDataElements = nullptr;
	other._isRegisterReferedDataElements = false;
	other.result = 0.0;
}

// Move assignment safely releases current resources before taking over source ownership.
genesyspp_driver& genesyspp_driver::operator=(genesyspp_driver&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	clearReferedDataElements();
	delete _referedDataElements;
	_model = other._model;
	_sampler = other._sampler;
	_referedDataElements = other._referedDataElements;
	_isRegisterReferedDataElements = other._isRegisterReferedDataElements;
	result = other.result;
	file = std::move(other.file);
	str_to_parse = std::move(other.str_to_parse);
	throwsException = other.throwsException;
	errorMessage = std::move(other.errorMessage);
	other._model = nullptr;
	other._sampler = nullptr;
	other._referedDataElements = nullptr;
	other._isRegisterReferedDataElements = false;
	other.result = 0.0;
	return *this;
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
	// Delete each allocated list before clearing the map to avoid leaking referred-name lists.
	if (_referedDataElements == nullptr) {
		return;
	}
	for (auto& entry : *_referedDataElements) {
		delete entry.second;
		entry.second = nullptr;
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
