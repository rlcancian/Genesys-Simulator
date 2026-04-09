/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ConnectionManager.cpp
 * Author: rlcancian
 *
 * Created on 1 de Julho de 2019, 18:39
 */

#include "ConnectionManager.h"

//using namespace GenesysKernel;

ConnectionManager::ConnectionManager() {
}

ConnectionManager::~ConnectionManager() {
	// Destroy every owned connection before deleting the backing port map.
	for (std::pair<const unsigned int, Connection*>& connectionPair : *_nextConnections) {
		delete connectionPair.second;
	}
	delete _nextConnections;
	_nextConnections = nullptr;
}

unsigned int ConnectionManager::size() {
	return _nextConnections->size();
}

Connection* ConnectionManager::getFrontConnection() {
	return getConnectionAtPort(0);
}

Connection* ConnectionManager::getConnectionAtPort(unsigned int rank) {
	std::map<unsigned int, Connection*>::iterator it = _nextConnections->find(rank);
	if (it == _nextConnections->end()) {
		return nullptr;
	}
	return (*it).second;
}

void ConnectionManager::insert(ModelComponent* component, unsigned int inputPortNumber) {
    Connection* connection = new Connection({component, {inputPortNumber}});
	insert(connection);
}

void ConnectionManager::insert(Connection* connection) {
	unsigned int rank = _nextConnections->size();
	insertAtPort(rank, connection);
}

void ConnectionManager::insertAtPort(unsigned int port, Connection* connection) {
	// Replace an existing port connection and release the previous owned object.
	std::map<unsigned int, Connection*>::iterator it = _nextConnections->find(port);
	if (it != _nextConnections->end()) {
		if (it->second != connection) {
			delete it->second;
		}
		it->second = connection;
		return;
	}
	_nextConnections->insert({port, connection});
}

void ConnectionManager::remove(Connection* connection) {
	// Remove and destroy the matched owned connection to keep teardown complete.
	for (std::map<unsigned int, Connection*>::iterator it = _nextConnections->begin(); it != _nextConnections->end(); it++) {
		if ((*it).second == connection) {
			delete (*it).second;
			_nextConnections->erase(it);
			return;
		}
	}
}

void ConnectionManager::removeAtPort(unsigned int port) {
	// Remove and destroy the owned connection bound to the informed output port.
	std::map<unsigned int, Connection*>::iterator it = _nextConnections->find(port);
	if (it != _nextConnections->end()) {
		delete it->second;
		_nextConnections->erase(it);
	}
}

//------------------

std::map<unsigned int, Connection*>* ConnectionManager::connections() const {
	return _nextConnections;
}

//void ConnectionManager::setCurrentOutputConnections(unsigned int _currentOutputConnections) {
//    this->_currentOutputConnections = _currentOutputConnections;
//}

unsigned int ConnectionManager::getCurrentOutputConnectionsSize() const {
	return _nextConnections->size();
}

void ConnectionManager::setMaxOutputConnections(unsigned int _maxOutputConnections) {
	this->_maxOutputConnections = _maxOutputConnections;
}

unsigned int ConnectionManager::getMaxOutputConnections() const {
	return _maxOutputConnections;
}

void ConnectionManager::setMinOutputConnections(unsigned int _minOutputConnections) {
	this->_minOutputConnections = _minOutputConnections;
}

unsigned int ConnectionManager::getMinOutputConnections() const {
	return _minOutputConnections;
}

//void ConnectionManager::setCurrentInputConnections(unsigned int _currentInputConnections) {
//    this->_currentInputConnections = _currentInputConnections;
//}

unsigned int ConnectionManager::getCurrentInputConnectionsSize() const {
	return _currentInputConnections;
}

void ConnectionManager::setMaxInputConnections(unsigned int _maxInputConnections) {
	this->_maxInputConnections = _maxInputConnections;
}

unsigned int ConnectionManager::getMaxInputConnections() const {
	return _maxInputConnections;
}

void ConnectionManager::setMinInputConnections(unsigned int _minInputConnections) {
	this->_minInputConnections = _minInputConnections;
}

unsigned int ConnectionManager::getMinInputConnections() const {
	return _minInputConnections;
}
