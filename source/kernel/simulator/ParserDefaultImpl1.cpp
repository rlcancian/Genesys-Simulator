/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ParserDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 2 de Agosto de 2018, 01:25
 */

#include "ParserDefaultImpl1.h"

//namespace GenesysKernel {

ParserDefaultImpl1::ParserDefaultImpl1(Model* model) {
	_model = model;
}

double ParserDefaultImpl1::parse(const std::string expression) { // may throw exception
	double result = std::atof(expression.c_str()); // change by a real parser
	return result;
}

std::string ParserDefaultImpl1::getErrorMessage() {
    std::string errorMsg = "ParserDefaultImpl1 error message unavailable";
	return errorMsg;
}

double ParserDefaultImpl1::parse(const std::string expression, bool& success, std::string& errorMessage) {
    try {
        double result = this->parse(expression);
        errorMessage.clear();
        success = true;
        return result;
    } catch (...) {
        errorMessage = "Error parsing...";
        success = false;
        return 0.0;
    }
}

//namespace\\}
