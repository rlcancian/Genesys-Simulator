/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ParserDefaultImpl2.cpp
 * Author: Joao Ortigara rafael.luiz.cancian (20181208-...(
 *
 *
 */

#include "ParserDefaultImpl2.h"
#include <exception>

//using namespace GenesysKernel;

// Construct parser wrapper in-place to avoid copying partially initialized driver state.
ParserDefaultImpl2::ParserDefaultImpl2(Model* model, Sampler_if* sampler, bool throws)
	: _model(model), _wrapper(model, sampler, throws) {}

// Delete the sampler owned by this parser wrapper to avoid leaking per-model parser state.
ParserDefaultImpl2::~ParserDefaultImpl2() {
	delete _wrapper.getSampler();
	_wrapper.setSampler(nullptr);
}

double ParserDefaultImpl2::parse(const std::string expression) { // may throw exception
	_wrapper.setThrowsException(true);
	try {
		int res = _wrapper.parse_str(expression);
		if (res == 0) {
			return _wrapper.getResult();
		} else {
			std::string msg = _wrapper.getErrorMessage();
			if (msg.empty()) {
				msg = "Error parsing expression \"" + expression + "\"";
			}
			throw std::string(msg);
		}
	} catch (const std::string& e) {
		_model->getTracer()->traceError(e);
		return _wrapper.getResult();
	} catch (const std::exception& e) {
		std::string msg = _wrapper.getErrorMessage();
		if (msg.empty()) {
			msg = e.what();
		}
		_model->getTracer()->traceError(msg);
		return _wrapper.getResult();
	} catch (...) {
		std::string msg = _wrapper.getErrorMessage();
		if (msg.empty()) {
			msg = "Unknown parser error";
		}
		_model->getTracer()->traceError(msg);
		return _wrapper.getResult();
	}
}

std::string ParserDefaultImpl2::getErrorMessage() {
    std::string errorMessage = _wrapper.getErrorMessage();
	return errorMessage;
}

double ParserDefaultImpl2::parse(const std::string expression, bool& success, std::string& errorMessage) {
	_wrapper.setThrowsException(true); //false);
	int res = -1;
	try {
		res = _wrapper.parse_str(expression);
	} catch (const std::string& e) {
		errorMessage = !_wrapper.getErrorMessage().empty() ? _wrapper.getErrorMessage() : e;
		res = -1;
	} catch (const std::exception& e) {
		errorMessage = !_wrapper.getErrorMessage().empty() ? _wrapper.getErrorMessage() : std::string(e.what());
		res = -1;
	} catch (...) {
		errorMessage = !_wrapper.getErrorMessage().empty() ? _wrapper.getErrorMessage() : "Unknown parser error";
		res = -1;
	}
	if (res == 0) {
        success = true;
		return _wrapper.getResult();
	} else {
        success = false;
        if (errorMessage.empty()) {
        	errorMessage = _wrapper.getErrorMessage();
        }
        if (errorMessage.empty()) {
        	errorMessage = "Error parsing expression \"" + expression + "\"";
        }
		return _wrapper.getResult();
	}
}

void ParserDefaultImpl2::setSampler(Sampler_if* _sampler) {
	_wrapper.setSampler(_sampler);
}

Sampler_if* ParserDefaultImpl2::getSampler() const {
	return _wrapper.getSampler();
}

genesyspp_driver ParserDefaultImpl2::getParser() const {
	return _wrapper;
}
