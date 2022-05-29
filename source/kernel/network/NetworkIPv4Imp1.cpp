/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkIPv4Imp1.cpp
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#include "NetworkIPv4Imp1.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>

// CollectorDatafileDefaultImpl1::CollectorDatafileDefaultImpl1() {
// }

// void CollectorDatafileDefaultImpl1::clear() {
// 	std::ofstream file;
// 	try {
// 		file.open(_filename, std::ofstream::out | std::ofstream::trunc);
// 		file.close();
// 	} catch (const std::exception& e) {
// 		throw "ERROR - can't open the file ";
// 	}
// 	_numElements = 0;
// 	_lastValue = NULL; //@TODO: Change it to NaN or other default/null value
// }

// void CollectorDatafileDefaultImpl1::addValue(double value) {
// 	std::ofstream file;
// 	try {
// 		if (this->_numElements > 0) {
// 			file.open(_filename, std::ofstream::out | std::ofstream::app);
// 		} else {
// 			file.open(_filename, std::ofstream::out | std::ofstream::trunc);
// 		}
// 		file.write(reinterpret_cast<char*> (&value), sizeof (double));
// 		file.close();
// 	} catch (const std::exception& e) {
// 		throw "ERROR - can't open the file ";
// 	}
// 	_numElements++;
// 	_lastValue = value;
// }

// double CollectorDatafileDefaultImpl1::getLastValue() {
// 	return _lastValue;
// }

// unsigned long CollectorDatafileDefaultImpl1::numElements() {
// 	return _numElements;
// }

// double CollectorDatafileDefaultImpl1::getValue(unsigned int num) {
// 	std::ifstream file;
// 	double value;
// 	if (num > _numElements) {
// 		throw "ERROR - num greater than numElements";
// 	}
// 	try {
// 		file.open(_filename, std::ifstream::binary | std::ifstream::in);
// 		file.seekg(sizeof (double) * num);
// 		//valueType d;
// 		file.read(reinterpret_cast<char*> (&value), sizeof (double));
// 		file.close();
// 	} catch (const std::exception& e) {
// 		throw "ERROR - can't open the file or get the line ";
// 	}
// 	return value;
// }

// double CollectorDatafileDefaultImpl1::getNextValue() {
// 	return 0.0; // @TODO:
// }

// void CollectorDatafileDefaultImpl1::seekFirstValue() {
// }

// std::string CollectorDatafileDefaultImpl1::getDataFilename() {
// 	return _filename;
// }

// void CollectorDatafileDefaultImpl1::setDataFilename(std::string filename) {
// 	_filename = filename;
// }

// void CollectorDatafileDefaultImpl1::setAddValueHandler(CollectorAddValueHandler addValueHandler) {
// 	// @TODO: just to use it
// }

// void CollectorDatafileDefaultImpl1::setClearHandler(CollectorClearHandler clearHandler) {
// 	// @TODO: just to use it
// }