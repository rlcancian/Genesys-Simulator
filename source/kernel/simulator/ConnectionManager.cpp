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
	_nextConnections->insert({port, connection});
}

void ConnectionManager::remove(Connection* connection) {
	for (std::map<unsigned int, Connection*>::iterator it = _nextConnections->begin(); it != _nextConnections->end(); it++) {
		if ((*it).second == connection) {
			// TODO(genesys|connection-manager|ownership): Confirmar a politica de ownership de Connection*. Se ConnectionManager for o owner, esta remocao deveria destruir o objeto para evitar leak.
			_nextConnections->erase(it);
			return;
		}
	}
}

void ConnectionManager::removeAtPort(unsigned int port) {
	// TODO(genesys|connection-manager|ownership): Investigar se removeAtPort tambem deve destruir a Connection associada ao porto removido.
	_nextConnections->erase(port);
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
