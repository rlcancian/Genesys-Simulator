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

#include "../util/List.h"
#include "Event.h"

//namespace GenesysKernel {
/* 
 * used to get and set values no matter the class (for process analyser)
 * should be a wait to invoke a getter or setter no matter the class (a pointer to a member function without specifying the class 
 */
//typedef double (*memberFunctionGetDoubleVarHandler)(); //template<> ... typedef double (T::*getDoubleVarHandler)() or something like that
//typedef void (*memberFunctionSetDoubleVarHandler)(double);


/*! Stores an event that happened on a specific model
 */
class ModelEvent {
public:
	ModelEvent(Model* model){
		_model = model;
	};
public:
	const Model* getModel() {
		return _model;
	}
private:
	Model* _model;
};
typedef void (*modelEventHandler)(ModelEvent*);
// for handlers that are class members (methods)
typedef std::function<void(ModelEvent*) > modelEventHandlerMethod;


/*! Stores an event that happened on a specific replication
 */
class SimulationEvent {
public:

	void setSimulatedTime(double simulatedTime) {
		this->simulatedTime = simulatedTime;
	}

	double getSimulatedTime() const {
		return simulatedTime;
	}

	void setStopRequested(bool stopRequested) {
		this->stopRequested = stopRequested;
	}

	bool isStopRequested() const {
		return stopRequested;
	}

	void setPauseRequested(bool pauseRequested) {
		this->pauseRequested = pauseRequested;
	}

	bool isPauseRequested() const {
		return pauseRequested;
	}

	void setPaused(bool Paused) {
		_isPaused = Paused;
	}

	bool isPaused() const {
		return _isPaused;
	}

	void setRunning(bool Running) {
		_isRunning = Running;
	}

	bool isRunning() const {
		return _isRunning;
	}

	void setCustomObject(void* customObject) {
		this->customObject = customObject;
	}

	void* getCustomObject() const {
		return customObject;
	}

	void setCurrentReplicationNumber(unsigned int currentReplicationNumber) {
		this->currentReplicationNumber = currentReplicationNumber;
	}

	unsigned int getCurrentReplicationNumber() const {
		return currentReplicationNumber;
	}

	void setCurrentEvent(Event* currentEvent) {
		this->currentEvent = currentEvent;
	}

	Event* getCurrentEvent() const {
		return currentEvent;
	}
	//void setCurrentinputPortNumber(unsigned int currentinputPortNumber) {
	//	this->currentinputPortNumber = currentinputPortNumber;
	//}
	//unsigned int getCurrentinputPortNumber() const {
	//	return currentinputPortNumber;
	//}
	//void setCurrentComponent(ModelComponent* currentComponent) {
	//	this->currentComponent = currentComponent;
	//}
	//ModelComponent* getCurrentComponent() const {
	//	return currentComponent;
	//}
	//void setCurrentEntity(Entity* currentEntity) {
	//	this->currentEntity = currentEntity;
	//}
	//Entity* getCurrentEntity() const {
	//	return currentEntity;
	//}

	void setEntityCreated(Entity* entityCreated) {
		this->entityCreated = entityCreated;
	}

	Entity* getEntityCreated() const {
		return entityCreated;
	}

	void setDestinationComponent(ModelComponent* destinationComponent) {
		this->destinationComponent = destinationComponent;
	}

	ModelComponent* getDestinationComponent() const {
		return destinationComponent;
	}

	void setEntityMoveTimeDelay(double entityMoveTimeDelay) {
		this->entityMoveTimeDelay = entityMoveTimeDelay;
	}

	double getEntityMoveTimeDelay() const {
		return entityMoveTimeDelay;
	}
private:

	SimulationEvent() {
	}
	friend class ModelSimulation;
private:
	Entity* entityCreated = nullptr;
	Event* currentEvent = nullptr;
	//Entity* currentEntity = nullptr;
	//unsigned int currentinputPortNumber = 0;
	//ModelComponent* currentComponent = nullptr;
	ModelComponent* destinationComponent = nullptr;
	void* customObject = nullptr;
	unsigned int currentReplicationNumber = 0;
	double simulatedTime = 0.0;
	double entityMoveTimeDelay = 0.0;
	bool _isRunning = false;
	bool _isPaused = false;
	bool pauseRequested = false;
	bool stopRequested = false;
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
	virtual ~OnEventManager();
public: // event listeners (handlers)
	void addOnModelCheckSucessHandler(modelEventHandler EventHandler);
	void addOnModelLoadHandler(modelEventHandler EventHandler);
	void addOnModelSaveHandler(modelEventHandler EventHandler);
	void addOnReplicationStartHandler(simulationEventHandler EventHandler);
	void addOnReplicationStepHandler(simulationEventHandler EventHandler);
	void addOnReplicationEndHandler(simulationEventHandler EventHandler);
	void addOnProcessEventHandler(simulationEventHandler EventHandler);
	void addOnAfterProcessEventHandler(simulationEventHandler EventHandler);
	void addOnEntityCreateHandler(simulationEventHandler EventHandler);
	void addOnEntityMoveHandler(simulationEventHandler EventHandler);
	void addOnEntityRemoveHandler(simulationEventHandler EventHandler);
	void addOnSimulationStartHandler(simulationEventHandler EventHandler);
	void addOnSimulationPausedHandler(simulationEventHandler EventHandler);
	void addOnSimulationResumeHandler(simulationEventHandler EventHandler);
	void addOnSimulationEndHandler(simulationEventHandler EventHandler);
	void addOnBreakpointHandler(simulationEventHandler EventHandler);
public: // event listeners (method handlers)
	// for handlers that are class members (methods)
	template<typename Class> void addOnModelCheckSuccessHandler(Class * object, void (Class::*function)(ModelEvent*));
	template<typename Class> void addOnModelLoadHandler(Class * object, void (Class::*function)(ModelEvent*));
	template<typename Class> void addOnModelSaveHandler(Class * object, void (Class::*function)(ModelEvent*));
	template<typename Class> void addOnReplicationStartHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnReplicationStepHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnReplicationEndHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnAfterProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnEntityCreateHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnEntityMoveHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnEntityRemoveHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnSimulationStartHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnSimulationPausedHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnSimulationResumeHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnSimulationEndHandler(Class * object, void (Class::*function)(SimulationEvent*));
	template<typename Class> void addOnBreakpointHandler(Class * object, void (Class::*function)(SimulationEvent*));

public:
	void NotifyModelCheckSuccessHandlers(ModelEvent* se);
	void NotifyModelLoadHandlers(ModelEvent* se);
	void NotifyModelSaveHandlers(ModelEvent* se);
	void NotifyReplicationStartHandlers(SimulationEvent* se);
	void NotifyReplicationStepHandlers(SimulationEvent* se);
	void NotifyReplicationEndHandlers(SimulationEvent* se);
	void NotifyProcessEventHandlers(SimulationEvent* se);
	void NotifyAfterProcessEventHandlers(SimulationEvent* se);
	void NotifyEntityCreateHandlers(SimulationEvent* se);
	void NotifyEntityMoveHandlers(SimulationEvent* se);
	void NotifyEntityRemoveHandlers(SimulationEvent* se);
	void NotifySimulationStartHandlers(SimulationEvent* se);
	void NotifySimulationPausedHandlers(SimulationEvent* se);
	void NotifySimulationResumeHandlers(SimulationEvent* se);
	void NotifySimulationEndHandlers(SimulationEvent* se);
	void NotifyBreakpointHandlers(SimulationEvent* se);

private:
	void _NotifyHandlers(List<modelEventHandler>* list, ModelEvent* se);
	void _NotifyHandlers(List<simulationEventHandler>* list, SimulationEvent* se);
	void _NotifyHandlerMethods(List<modelEventHandlerMethod>* list, ModelEvent* se);
	void _NotifyHandlerMethods(List<simulationEventHandlerMethod>* list, SimulationEvent* se);
	void _addOnHandler(List<modelEventHandler>* list, modelEventHandler EventHandler);
	void _addOnHandler(List<simulationEventHandler>* list, simulationEventHandler EventHandler);
private: // events listener
	List<modelEventHandler>* _onModelCheckSuccessHandlers = new List<modelEventHandler>();
	List<modelEventHandler>* _onModelLoadHandlers = new List<modelEventHandler>();
	List<modelEventHandler>* _onModelSaveHandlers = new List<modelEventHandler>();
	List<simulationEventHandler>* _onReplicationStartHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onReplicationStepHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onReplicationEndHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onProcessEventHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onAfterProcessEventHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onEntityCreateHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onEntityMoveHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onEntityRemoveHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onSimulationStartHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onSimulationPausedHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onSimulationResumeHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onSimulationEndHandlers = new List<simulationEventHandler>();
	List<simulationEventHandler>* _onBreakpointHandlers = new List<simulationEventHandler>();
private: // events listener for handlers that are class members (methods)
	List<modelEventHandlerMethod>* _onModelCheckSuccessHandlerMethods = new List<modelEventHandlerMethod>();
	List<modelEventHandlerMethod>* _onModelLoadHandlerMethods = new List<modelEventHandlerMethod>();
	List<modelEventHandlerMethod>* _onModelSaveHandlerMethods = new List<modelEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onReplicationStartHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onReplicationStepHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onReplicationEndHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onProcessEventHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onAfterProcessEventHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onEntityCreateHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onEntityMoveHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onEntityRemoveHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onSimulationStartHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onSimulationPausedHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onSimulationResumeHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onSimulationEndHandlerMethods = new List<simulationEventHandlerMethod>();
	List<simulationEventHandlerMethod>* _onBreakpointHandlerMethods = new List<simulationEventHandlerMethod>();
};

//
// implementation for template methods
//
// 	// @TODO: (!!) Complicated: if handlerMethod already insert, should not insert it again. Problem to solve <...> for function
//if (_onProcessEventHandlerMethods->find(handlerMethod) == _onProcessEventHandlerMethods->list()->end())
// trying unique to solve the issue
//this->_onProcessEventHandlerMethods->list()->unique(); // does not work
//  @TODO: probabily to override == operator for type simulationEventHandlerMethod
// ...

template<typename Class> void OnEventManager::addOnModelCheckSuccessHandler(Class * object, void (Class::*function)(ModelEvent*)) {
	modelEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onModelCheckSuccessHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnModelLoadHandler(Class * object, void (Class::*function)(ModelEvent*)) {
	modelEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onModelLoadHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnModelSaveHandler(Class * object, void (Class::*function)(ModelEvent*)) {
	modelEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onModelSaveHandlerMethods->insert(handlerMethod);
}
template<typename Class> void OnEventManager::addOnReplicationStartHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onReplicationStartHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnReplicationStepHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onReplicationStepHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnReplicationEndHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onReplicationEndHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onProcessEventHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnAfterProcessEventHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onAfterProcessEventHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnEntityCreateHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onEntityCreateHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnEntityMoveHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onEntityMoveHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnEntityRemoveHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onEntityRemoveHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnSimulationStartHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onSimulationStartHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnSimulationPausedHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onSimulationPausedHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnSimulationResumeHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onSimulationResumeHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnSimulationEndHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onSimulationEndHandlerMethods->insert(handlerMethod);
}

template<typename Class> void OnEventManager::addOnBreakpointHandler(Class * object, void (Class::*function)(SimulationEvent*)) {
	simulationEventHandlerMethod handlerMethod = std::bind(function, object, std::placeholders::_1);
	this->_onBreakpointHandlerMethods->insert(handlerMethod);
}


/*
Private Sub ModelLogic_DocumentOpen()
Private Sub ModelLogic_DocumentSave()
Private Sub ModelLogic_OnClearStatistics()
Private Sub ModelLogic_OnFileClose(ByVal filesElementNumber As Long, ByVal fileName As String)
Private Sub ModelLogic_OnFileRead(ByVal filesElementNumber As Long, ByVal fileName As String, ByVal numberOfItems As Long, readBuffer() As Double)
Private Sub ModelLogic_OnFileReadAsVariant(ByVal filesElementNumber As Long, ByVal fileName As String, ByVal numberOfItems As Long, readBuffer() As Variant)
Private Sub ModelLogic_OnFileWrite(ByVal filesElementNumber As Long, ByVal fileName As String, ByVal numberOfItems As Long, writeBuffer() As Double)
Private Sub ModelLogic_OnFileWriteAsVariant(ByVal filesElementNumber As Long, ByVal fileName As String, ByVal numberOfItems As Long, writeBuffer() As Variant)
Private Sub ModelLogic_OnKeystroke(ByVal keyCode As Long)
Private Function ModelLogic_RealTimeGetMaxTimeAdvance() As Long
Private Function ModelLogic_RealTimeInitialize(ByVal processName As String, ByVal remoteProcessName As String) As Long
Private Sub ModelLogic_RealTimeInitializeMaxTimeAdvance()
Private Function ModelLogic_RealTimeReceive(message As String) As Long
Private Function ModelLogic_RealTimeSend(ByVal message As String) As Long
Private Function ModelLogic_RealTimeTerminate() As Long
Private Sub ModelLogic_RunBegin()
Private Sub ModelLogic_RunBeginReplication()
Private Sub ModelLogic_RunBeginSimulation()
Private Sub ModelLogic_RunBreak()
Private Sub ModelLogic_RunEnd()
Private Sub ModelLogic_RunEndReplication()
Private Sub ModelLogic_RunEndSimulation()
Private Sub ModelLogic_RunFastForward()
Private Sub ModelLogic_RunPause()
Private Sub ModelLogic_RunRestart()
Private Sub ModelLogic_RunResume()
Private Sub ModelLogic_RunStep()
Private Sub ModelLogic_SimanError(ByVal routineName As String, msgType As Long, msgNum As Long)
Private Sub ModelLogic_UserContinuousEquations()
Private Function ModelLogic_UserFunction(ByVal entityID As Long, ByVal functionID As Long) As Double
Private Function ModelLogic_UserRule(ByVal entityID As Long, ByVal ruleID As Long) As Double
Private Sub VBA_Block_1_Fire()
 */

//namespace\\}
#endif /* ONEVENTMANAGER_H */
