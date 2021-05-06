/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OnEventManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 12:28
 */

#ifndef ONEVENTMANAGER_H
#define ONEVENTMANAGER_H

#include "List.h"
#include "Event.h"

//namespace GenesysKernel {
	/* \todo: To implement as item (1) for DS3
	 * used to get and set values no matter the class (for process analyser)
	 * should be a wait to invoke a getter or setter no matter the class (a pointer to a member function without specifying the class 
	 */
	//typedef double (*memberFunctionGetDoubleVarHandler)(); //template<> ... typedef double (T::*getDoubleVarHandler)() or something like that
	//typedef void (*memberFunctionSetDoubleVarHandler)(double);

	class SimulationEvent {
	public:

		SimulationEvent(unsigned int replicationNumber, Event* event) {
			_replicationNumber = replicationNumber;
			_event = event;
		}
	public:

		unsigned int getReplicationNumber() const {
			return _replicationNumber;
		}

		Event* getEventProcessed() const {
			return _event;
		}
	private:
		unsigned int _replicationNumber;
		Event* _event;
	};

	typedef void (*simulationEventHandler)(SimulationEvent*);
	// for handlers that are class members (methods)
	typedef std::function<void(SimulationEvent*) > simulationEventHandlerMethod;

	/*!
	 * OnEventManager allows external methods to hook interval simulation events as listeners (or observers) of pecific events.
	 * All methods added as listeners of an event will be invovked when that event is triggered.
	 */
	class OnEventManager {
	public:
		OnEventManager();
		virtual ~OnEventManager() = default;
	public: // event listeners (handlers)
		void addOnReplicationStartHandler(simulationEventHandler EventHandler);
		void addOnReplicationStepHandler(simulationEventHandler EventHandler);
		void addOnReplicationEndHandler(simulationEventHandler EventHandler);
		void addOnProcessEventHandler(simulationEventHandler EventHandler);
		void addOnEntityMoveHandler(simulationEventHandler EventHandler);
		void addOnSimulationStartHandler(simulationEventHandler EventHandler);
		void addOnSimulationPausedStartHandler(simulationEventHandler EventHandler);
		void addOnSimulationEndHandler(simulationEventHandler EventHandler);
		void addOnEntityRemoveHandler(simulationEventHandler EventHandler);
		void addOnBreakpointHandler(simulationEventHandler EventHandler);
		// for handlers that are class members (methods)
		template<typename Class> void addOnProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*));
		//  \todo: ...
	public:
		void NotifyReplicationStartHandlers(SimulationEvent* se);
		void NotifyReplicationStepHandlers(SimulationEvent* se);
		void NotifyReplicationEndHandlers(SimulationEvent* se);
		void NotifyProcessEventHandlers(SimulationEvent* se);
		void NotifyEntityMoveHandlers(SimulationEvent* se);
		void NotifySimulationStartHandlers(SimulationEvent* se);
		void NotifySimulationPausedStartHandlers(SimulationEvent* se);
		void NotifySimulationEndHandlers(SimulationEvent* se);
		void NotifyBreakpointHandlers(SimulationEvent* se);
	private:
		void _NotifyHandlers(List<simulationEventHandler>* list, SimulationEvent* se);
		void _NotifyHandlerMethods(List<simulationEventHandlerMethod>* list, SimulationEvent* se);
		void _addOnHandler(List<simulationEventHandler>* list, simulationEventHandler EventHandler);
	private: // events listener
		List<simulationEventHandler>* _onReplicationStartHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onReplicationStepHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onReplicationEndHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onProcessEventHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onEntityMoveHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onSimulationStartHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onSimulationPausedStartHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onSimulationEndHandlers = new List<simulationEventHandler>();
		List<simulationEventHandler>* _onBreakpointHandlers = new List<simulationEventHandler>();
		// for handlers that are class members (methods)
		List<simulationEventHandlerMethod>* _onProcessEventHandlerMethods = new List<simulationEventHandlerMethod>();
		//  \todo: ...
	};

	// implementation for template methods

	template<typename Class> void OnEventManager::addOnProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
		simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
		// \todo: if handlerMethod already insert, should not insert it again. Problem to solve <...> for function
		//if (_onProcessEventHandlerMethods->find(handlerMethod) == _onProcessEventHandlerMethods->list()->end())
		this->_onProcessEventHandlerMethods->insert(handlerMethod);
		// trying unique to solve the issue
		//this->_onProcessEventHandlerMethods->list()->unique(); // does not work
		//  \todo: probabily to override == operator for type simulationEventHandlerMethod
// ... 
	}
//namespace\\}
#endif /* ONEVENTMANAGER_H */

