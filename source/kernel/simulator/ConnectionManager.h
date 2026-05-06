/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ConnectionManager.h
 * Author: rlcancian
 *
 * Created on 1 de Julho de 2019, 18:39
 */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <utility>
#include <map>
#include "../util/List.h"

//namespace GenesysKernel {

class ModelComponent;

//typedef unsigned int ConnectionPort; 
/*!
 * \brief The ConnectionChannel class defines an input or output port that connects a ModelComponent to another
 */
struct ConnectionChannel { /*< actually a port is only a uint, but it could be more complex, including type, presence or absence of data, etc */
	unsigned int portNumber = 0;
	std::string portDescription = "";
};


/*!
 * Defines a connection between two or more ModelComponents. 
 * A (receiver) component's inputPort may receive connection from one or more (sender) components' outputPorts.
 * A (sender) component's outputPort is connected to one and only one (receiver) component's inputPort.
 */
struct Connection {
	ModelComponent* component;
	ConnectionChannel channel;
};

/*!
 * \brief Maintains outgoing connections from a component to downstream components.
 *
 * A connection maps an output rank (port) to a destination component/input port.
 * The manager also stores min/max input/output constraints used during validation.
 */
class ConnectionManager {
public:
	/*! \brief Creates an empty connection manager with zero constraints. */
	ConnectionManager();
	virtual ~ConnectionManager();
public:
	/*!
	 * \brief size
	 * \return Number of configured output connections.
	 */
	unsigned int size();
	/*!
	 * \brief getFrontConnection
	 * \return Connection at the first available output rank.
	 */
	Connection* getFrontConnection();
	/*!
	 * \brief getConnectionAtPort
	 * \param rank
	 * \return Connection bound to output rank \p rank.
	 */
	Connection* getConnectionAtPort(unsigned int rank);
	/*!
	 * \brief Inserts a new connection to a target component.
	 * \details Creates and inserts a connection to \p component at input port
	 * \p inputPortNumber. When the port is not specified, the connection uses
	 * the default input port 0.
	 * \param component Target component.
	 * \param inputPortNumber Target input port number.
	 */
	void insert(ModelComponent* component, unsigned int inputPortNumber = 0);
	/*!
	 * \brief insert
	 * \param connection
	 * \details Inserts a prebuilt connection object.
	 */
	void insert(Connection* connection);
	/*!
	 * \brief insertAtPort
	 * \param port
	 * \param connection
	 * \details Forces insertion/replacement at a specific output port.
	 */
	void insertAtPort(unsigned int port, Connection* connection);
	/*!
	 * \brief remove
	 * \param connection
	 * \details Removes a connection by pointer match.
	 */
	void remove(Connection* connection);
	/*!
	 * \brief removeAtPort
	 * \param port
	 * \details Removes the connection associated with output port \p port.
	 */
	void removeAtPort(unsigned int port);
	/*!
	 * \brief Returns the internal output-port to connection map.
	 * \return Map of output ports to outgoing connections.
	 */
	std::map<unsigned int, Connection*>* connections() const;
	//void setCurrentOutputConnections(unsigned int _currentOutputConnections);
	/*!
	 * \brief Returns the number of currently configured outgoing connections.
	 * \return Current output connection count.
	 */
	unsigned int getCurrentOutputConnectionsSize() const;
	/*!
	 * \brief Sets the maximum number of outgoing connections allowed.
	 * \param _maxOutputConnections Maximum outgoing connection count.
	 */
	void setMaxOutputConnections(unsigned int _maxOutputConnections);
	/*!
	 * \brief Returns the maximum number of outgoing connections allowed.
	 * \return Maximum outgoing connection count.
	 */
	unsigned int getMaxOutputConnections() const;
	/*!
	 * \brief Sets the minimum number of outgoing connections allowed.
	 * \param _minOutputConnections Minimum outgoing connection count.
	 */
	void setMinOutputConnections(unsigned int _minOutputConnections);
	/*!
	 * \brief Returns the minimum number of outgoing connections allowed.
	 * \return Minimum outgoing connection count.
	 */
	unsigned int getMinOutputConnections() const;
	//void setCurrentInputConnections(unsigned int _currentInputConnections);
	/*!
	 * \brief Returns the number of currently configured incoming connections.
	 * \return Current input connection count.
	 */
	unsigned int getCurrentInputConnectionsSize() const;
	/*!
	 * \brief Sets the maximum number of incoming connections allowed.
	 * \param _maxInputConnections Maximum incoming connection count.
	 */
	void setMaxInputConnections(unsigned int _maxInputConnections);
	/*!
	 * \brief Returns the maximum number of incoming connections allowed.
	 * \return Maximum incoming connection count.
	 */
	unsigned int getMaxInputConnections() const;
	/*!
	 * \brief Sets the minimum number of incoming connections allowed.
	 * \param _minInputConnections Minimum incoming connection count.
	 */
	void setMinInputConnections(unsigned int _minInputConnections);
	/*!
	 * \brief Returns the minimum number of incoming connections allowed.
	 * \return Minimum incoming connection count.
	 */
	unsigned int getMinInputConnections() const;
private:
	std::map<unsigned int, Connection*>* _nextConnections = new std::map<unsigned int, Connection*>();
	unsigned int _minInputConnections = 0;
	unsigned int _maxInputConnections = 0;
	unsigned int _currentInputConnections = 0;
	unsigned int _minOutputConnections = 0;
	unsigned int _maxOutputConnections = 0;
};
//namespace\\}
#endif /* CONNECTIONMANAGER_H */
