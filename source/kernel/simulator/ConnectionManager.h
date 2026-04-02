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
	virtual ~ConnectionManager() = default;
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
	 * \brief insert
	 * \param component
	 * \param inputPortNumber
	 * \details Creates and inserts a connection to \p component at input port \p inputPortNumber.
	 */
	void insert(ModelComponent* component, unsigned int inputPortNumber = 0); /*!< Insert a new connection by specifying the destnation component and optionally an input port, if not 0*/
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
	 * \brief connections
	 * \return
	 */
	std::map<unsigned int, Connection*>* connections() const;
	//void setCurrentOutputConnections(unsigned int _currentOutputConnections);
	/*!
	 * \brief getCurrentOutputConnectionsSize
	 * \return
	 */
	unsigned int getCurrentOutputConnectionsSize() const;
	/*!
	 * \brief setMaxOutputConnections
	 * \param _maxOutputConnections
	 */
	void setMaxOutputConnections(unsigned int _maxOutputConnections);
	/*!
	 * \brief getMaxOutputConnections
	 * \return
	 */
	unsigned int getMaxOutputConnections() const;
	/*!
	 * \brief setMinOutputConnections
	 * \param _minOutputConnections
	 */
	void setMinOutputConnections(unsigned int _minOutputConnections);
	/*!
	 * \brief getMinOutputConnections
	 * \return
	 */
	unsigned int getMinOutputConnections() const;
	//void setCurrentInputConnections(unsigned int _currentInputConnections);
	/*!
	 * \brief getCurrentInputConnectionsSize
	 * \return
	 */
	unsigned int getCurrentInputConnectionsSize() const;
	/*!
	 * \brief setMaxInputConnections
	 * \param _maxInputConnections
	 */
	void setMaxInputConnections(unsigned int _maxInputConnections);
	/*!
	 * \brief getMaxInputConnections
	 * \return
	 */
	unsigned int getMaxInputConnections() const;
	/*!
	 * \brief setMinInputConnections
	 * \param _minInputConnections
	 */
	void setMinInputConnections(unsigned int _minInputConnections);
	/*!
	 * \brief getMinInputConnections
	 * \return
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
