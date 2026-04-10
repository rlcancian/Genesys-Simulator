/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   CollectorDatafileDefaultImpl11.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 1 de Agosto de 2018, 20:58
 */

#include "CollectorDatafileDefaultImpl1.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

CollectorDatafileDefaultImpl1::CollectorDatafileDefaultImpl1() {
}

void CollectorDatafileDefaultImpl1::clear() {
	// Reset the backing file and in-memory metadata to an empty collector state.
	std::ofstream file;
	try {
		file.open(_filename, std::ofstream::out | std::ofstream::trunc);
		file.close();
	} catch (const std::exception& e) {
		throw "ERROR - can't open the file ";
	}
	_numElements = 0;
	_lastValue = std::numeric_limits<double>::quiet_NaN();
	_nextReadIndex = 0;
}

void CollectorDatafileDefaultImpl1::addValue(double value, double weight) {
	// Append one binary double value and keep cached metadata in sync.
	std::ofstream file;
	try {
		if (this->_numElements > 0) {
			file.open(_filename, std::ofstream::out | std::ofstream::app);
		} else {
			file.open(_filename, std::ofstream::out | std::ofstream::trunc);
		}
		//std::string strValue = std::to_string(value);
		//file.write(reinterpret_cast<char*> (&strValue), strValue.length());
		file.write(reinterpret_cast<char*> (&value), sizeof (double));
		file.close();
	} catch (const std::exception& e) {
		throw "ERROR - can't open the file ";
	}
	_numElements++;
	_lastValue = value;
}

double CollectorDatafileDefaultImpl1::getLastValue() {
	return _lastValue;
}

unsigned long CollectorDatafileDefaultImpl1::numElements() {
	return _numElements;
}

double CollectorDatafileDefaultImpl1::getValue(unsigned int num) {
	// Read a value by zero-based position with strict bounds validation.
	std::ifstream file;
	double value;
	if (num >= _numElements) {
		throw "ERROR - num greater than numElements";
	}
	try {
		file.open(_filename, std::ifstream::binary | std::ifstream::in);
		file.seekg(sizeof (double) * num);
		//valueType d;
		file.read(reinterpret_cast<char*> (&value), sizeof (double));
		file.close();
	} catch (const std::exception& e) {
		throw "ERROR - can't open the file or get the line ";
	}
	return value;
}

double CollectorDatafileDefaultImpl1::getNextValue() {
	// Read the next sequential value and advance the internal read cursor.
	double value = getValue(_nextReadIndex);
	_nextReadIndex++;
	return value;
}

void CollectorDatafileDefaultImpl1::seekFirstValue() {
	// Rewind sequential reads to the first stored element.
	_nextReadIndex = 0;
}

std::string CollectorDatafileDefaultImpl1::getDataFilename() {
	return _filename;
}

void CollectorDatafileDefaultImpl1::setDataFilename(std::string filename) {
	// Bind to a file and rebuild cached metadata from persisted binary doubles.
	_filename = filename;
	_numElements = 0;
	_lastValue = std::numeric_limits<double>::quiet_NaN();
	_nextReadIndex = 0;

	std::ifstream file(_filename, std::ifstream::binary | std::ifstream::ate);
	if (!file.is_open()) {
		return;
	}

	std::streampos size = file.tellg();
	if (size <= 0) {
		return;
	}

	_numElements = static_cast<unsigned int>(size / static_cast<std::streampos>(sizeof (double)));
	if (_numElements > 0) {
		file.seekg(static_cast<std::streamoff>((_numElements - 1) * sizeof (double)), std::ifstream::beg);
		file.read(reinterpret_cast<char*> (&_lastValue), sizeof (double));
	}
}

void CollectorDatafileDefaultImpl1::setAddValueHandler(CollectorAddValueHandler addValueHandler) {
	// @TODO: just to use it
}

void CollectorDatafileDefaultImpl1::setClearHandler(CollectorClearHandler clearHandler) {
	// @TODO: just to use it
}
