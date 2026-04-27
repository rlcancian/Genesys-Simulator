/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   TraceManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 11:59
 */

#ifndef TRACEMANAGER_H
#define TRACEMANAGER_H

#include "../util/List.h"
#include "../util/Util.h"
#include <functional>
//namespace GenesysKernel {
class Model;
class Entity;
class ModelComponent;
class Simulator;

class TraceEvent;
class TraceErrorEvent;
class TraceSimulationEvent;
class TraceSimulationProcess;

// for handlers that are simple functions
typedef void (*traceListener)(TraceEvent);
typedef void (*traceErrorListener)(TraceErrorEvent);
typedef void (*traceSimulationListener)(TraceSimulationEvent);
typedef void (*traceSimulationProcessListener)(TraceSimulationProcess);
// for handlers that are class members (methods)
typedef std::function<void(TraceEvent) > traceListenerMethod;
typedef std::function<void(TraceErrorEvent) > traceErrorListenerMethod;
typedef std::function<void(TraceSimulationEvent) > traceSimulationListenerMethod;
typedef std::function<void(TraceSimulationProcess) > traceSimulationProcessListenerMethod;

/*!
 * The TraceManager is used to trace back model simulation information and track/debug the simulation.
 * It works as the model simulation output (cout) and allows external methods to hook up such output as listeners.
 */
class TraceManager {
public:

	enum class Level : int {
		L0_noTraces = 0,
		L1_errorFatal = 1,
		L2_results = 2,
		L3_errorRecover = 3, // or progress
		L4_warning = 4,
		L5_event = 5,
		L6_arrival = 6,
		L7_internal = 7,
		L8_detailed = 8,
		L9_mostDetailed = 9,
		num_elements = 10
	};
public:
	static std::string convertEnumToStr(Level level);
	TraceManager(Simulator* simulator); //(Model* model);
	virtual ~TraceManager();
public: // add trace handlers
	// for handlers that are simple functions
	/*!
	 * \brief addTraceHandler
	 * \param traceListener
	 */
	void addTraceHandler(traceListener traceListener);  //!< Add a function to by notified/invoked then a trace is called
	/*!
	 * \brief addTraceReportHandler
	 * \param traceReportListener
	 */
	void addTraceReportHandler(traceListener traceReportListener);  //!< Add a function to by notified/invoked then a traceReport is called
	/*!
	 * \brief addTraceSimulationHandler
	 * \param traceSimulationListener
	 */
	void addTraceSimulationHandler(traceSimulationListener traceSimulationListener);  //!< Add a function to by notified/invoked then a traceSimulation is called
	/*!
	 * \brief addTraceErrorHandler
	 * \param traceErrorListener
	 */
	void addTraceErrorHandler(traceErrorListener traceErrorListener);  //!< Add a function to by notified/invoked then a traceError is called
	// for handlers that are class members (methods)
	/*!
	 * \brief addTraceHandler
	 * \param object
	 */
	template<typename Class> void addTraceHandler(Class * object, void (Class::*function)(TraceEvent)); //!< Add a method to by notified/invoked then a trace is called
	/*!
	 * \brief addTraceErrorHandler
	 * \param object
	 */
	template<typename Class> void addTraceErrorHandler(Class * object, void (Class::*function)(TraceErrorEvent));   //!< Add a method to by notified/invoked then a traceError is called
	/*!
	 * \brief addTraceReportHandler
	 * \param object
	 */
	template<typename Class> void addTraceReportHandler(Class * object, void (Class::*function)(TraceEvent));  //!< Add a method to by notified/invoked then a traceReport is called
	/*!
	 * \brief addTraceSimulationHandler
	 * \param object
	 */
	template<typename Class> void addTraceSimulationHandler(Class * object, void (Class::*function)(TraceSimulationEvent));  //!< Add a method to by notified/invoked then a traceSimulation is called

public: // add ModelDataDefinition (or subclasses) allowed (or restrcted) to traceSimulation
	void addTraceSimulationExceptionRuleModelData(void* thisobject);
public: // traces (invoke trace handlers)
	void trace(TraceManager::Level level, const std::string& text); //<! Deprected
	void traceSimulation(void* thisobject, TraceManager::Level level, double time, Entity* entity, ModelComponent* component, const std::string& text); //<! Deprected
	void traceSimulation(void* thisobject, TraceManager::Level level, const std::string& text); //<! Deprected
public: // traces
	// Disable callbacks and clear handler lists before simulator teardown destroys GUI objects.
	void beginShutdown();
	void trace(const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed); //!< Trace to a general output, used when not simulating (eg: adding plugins, checking the model, etc
	void traceError(const std::string& text, const std::exception& e); //!< Trace to the error output, used in every situation an error happens (simulating, report, general)
	void traceError(const std::string& text, TraceManager::Level level = TraceManager::Level::L1_errorFatal); //!< Trace to the error output, used in every situation an error happens (simulating, report, general)
	void traceReport(const std::string& text, TraceManager::Level level = TraceManager::Level::L2_results); //!< Trace to the report output, used only when generating the simulation report
    void traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component, const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed,bool showAnyway = false); //!< Trace to the simulation output, used only when simulation is running (eg: compponents or dataElements inform something)
    void traceSimulation(void* thisobject, const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed, bool showAnyway = false); //!< Trace to the simulation output, used only when simulation is running (eg: compponents or dataElements inform something)

public:
	/*!
	 * \brief errorMessages
	 * \return
	 */
	List<std::string>* errorMessages() const;
	/*!
	 * \brief setTraceLevel
	 * \param _traceLevel
	 */
	void setTraceLevel(TraceManager::Level _traceLevel);
	/*!
	 * \brief getTraceLevel
	 * \return
	 */
	TraceManager::Level getTraceLevel() const;
	/*!
	 * \brief getParentSimulator
	 * \return
	 */
	Simulator* getParentSimulator() const;
	/*!
	 * \brief setTraceSimulationRuleAllAllowed
	 * \param _traceSimulationRuleAllAllowed
	 */
	void setTraceSimulationRuleAllAllowed(bool _traceSimulationRuleAllAllowed);
	/*!
	 * \brief isTraceSimulationRuleAllAllowed
	 * \return
	 */
	bool isTraceSimulationRuleAllAllowed() const;

private:
	//void _addHandler(List<traceListener>* list, )
	bool _traceConditionPassed(TraceManager::Level level);
    bool _traceSimulationConditionPassed(TraceManager::Level level, void* thisobject, bool showAnyway = false);
private: // trace listener
	// for handlers that are simple functions
	List<traceListener>* _traceHandlers = new List<traceListener>();
	List<traceErrorListener>* _traceErrorHandlers = new List<traceErrorListener>();
	List<traceListener>* _traceReportHandlers = new List<traceListener>();
	List<traceSimulationListener>* _traceSimulationHandlers = new List<traceSimulationListener>();
	// for handlers that are class members (methods)
	List<traceListenerMethod>* _traceHandlersMethod = new List<traceListenerMethod>();
	List<traceErrorListenerMethod>* _traceErrorHandlersMethod = new List<traceErrorListenerMethod>();
	List<traceListenerMethod>* _traceReportHandlersMethod = new List<traceListenerMethod>();
	List<traceSimulationListenerMethod>* _traceSimulationHandlersMethod = new List<traceSimulationListenerMethod>();
private:
	List<void*>* _traceSimulationExceptionRule = new List<void*>();
private:
	Simulator* _simulator;
private:
	TraceManager::Level _traceLevel; // = TraceManager::Level::L9_mostDetailed;
	bool _shuttingDown = false;
    double _lastTimeTraceSimulation = -1.0; // an invalid time
	Util::identification _lastEntityTraceSimulation = 0;
	Util::identification _lastModuleTraceSimulation = 0;
	bool _traceSimulationRuleAllAllowed = true;
    List<std::string>* _errorMessages = new List<std::string>(); /* @ToDo: (importante): 18/08/24 this is a new one. several methods should use it */

};

// implementation for template methods

template<typename Class> void TraceManager::addTraceHandler(Class * object, void (Class::*function)(TraceEvent)) {
	this->_traceHandlersMethod->insert(std::bind(function, object, std::placeholders::_1));
}

template<typename Class> void TraceManager::addTraceErrorHandler(Class * object, void (Class::*function)(TraceErrorEvent)) {
	this->_traceErrorHandlersMethod->insert(std::bind(function, object, std::placeholders::_1));
}

template<typename Class> void TraceManager::addTraceReportHandler(Class * object, void (Class::*function)(TraceEvent)) {
	this->_traceReportHandlersMethod->insert(std::bind(function, object, std::placeholders::_1));
}

template<typename Class> void TraceManager::addTraceSimulationHandler(Class * object, void (Class::*function)(TraceSimulationEvent)) {
	this->_traceSimulationHandlersMethod->insert(std::bind(function, object, std::placeholders::_1));
}

class TraceEvent {
public:

	TraceEvent(TraceManager::Level level, const std::string& text) {
		_tracelevel = level;
		_text = text;
	}

	TraceEvent(const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed) {
		_tracelevel = level;
		_text = text;
	}

	TraceManager::Level getTracelevel() const {
		return _tracelevel;
	}

	const std::string& getText() const {
		return _text;
	}
private:
	TraceManager::Level _tracelevel;
	std::string _text;
};

class TraceErrorEvent : public TraceEvent {
public:

	TraceErrorEvent(const std::string& text, const std::exception& e) : TraceEvent(text, TraceManager::Level::L3_errorRecover) {
		_e = e;
	}

	TraceErrorEvent(const std::string& text, TraceManager::Level level) : TraceEvent(text, level) {
	}

	const std::exception& getException() const {
		return _e;
	}
private:
	std::exception _e;
};

class TraceSimulationEvent : public TraceEvent {
public:

	TraceSimulationEvent(TraceManager::Level level, double time, Entity* entity, ModelComponent* component, const std::string& text) : TraceEvent(text, level) {
		_time = time;
		_entity = entity;
		_component = component;
	}

	ModelComponent* getComponent() const {
		return _component;
	}

	Entity* getEntity() const {
		return _entity;
	}

	double getTime() const {
		return _time;
	}

private:
	double _time;
	Entity* _entity;
	ModelComponent* _component;
};

/*!
 * Events related to simulation "process" (usually process analyser), associated to entire replication or simulation events (begin/end/pause of replication/simulation)
 * @ToDo: (importante): CLASS NOT FULLY IMPLEMENTED (to be implemented for process analyser)
 */
class TraceSimulationProcess : public TraceEvent {
public:

	TraceSimulationProcess(const std::string& text, TraceManager::Level level = TraceManager::Level::L8_detailed) : TraceEvent(text, level) {
	}
};


//namespace\\}
#endif /* TRACEMANAGER_H */
