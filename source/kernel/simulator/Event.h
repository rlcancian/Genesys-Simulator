/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Event.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:41
 */

#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <functional>

#include "ModelDataDefinition.h"
#include "Entity.h"
#include "ModelComponent.h"

//namespace GenesysKernel {

/*!
 * An an instantaneaous event, triggered at a certain moment by an entity upon reaching a component. The simulated time advances in discrete points in time and that are the instants that an event is triggered.
 */

class Event {//: public ModelDataDefinition {
public:
	Event(double time, Entity* entity, ModelComponent* component, unsigned int componentinputPortNumber = 0);
	Event(double time, Entity* entity, Connection* connection);
	virtual ~Event() = default;
public:
	/*!
	 * \brief getTime
	 * \return
	 */
	double getTime() const;
	/*!
	 * \brief getComponent
	 * \return
	 */
	ModelComponent* getComponent() const;
	/*!
	 * \brief getEntity
	 * \return
	 */
	Entity* getEntity() const;
	/*!
	 * \brief getComponentinputPortNumber
	 * \return
	 */
	unsigned int getComponentinputPortNumber() const;
	/*!
	 * \brief setEntity
	 * \param _entity
	 */
	void setEntity(Entity* _entity);
public: // virtual
	virtual std::string show();
protected:
	double _time;
private:
	Entity* _entity;
	ModelComponent* _component;
	unsigned int _componentinputPortNumber;
};

//***************************************************** 2022/07/05

typedef std::function<void(void*) > InternalEventHandler;

class InternalEvent: public Event { // @ToDo: (importante): BAD SW ENGINEERING Fix it
public:
	InternalEvent(double time, std::string description): Event(time, nullptr, nullptr, 0){
		_description = description;
		_time = time;
	}
	virtual ~InternalEvent() = default;

	template<typename Class>
	void setEventHandler(Class* object, void (Class::*function)(void*), void* parameter) {
		_object = object;
		_handler= std::bind(function, object, std::placeholders::_1);
		_parameter = parameter;
	}

	virtual std::string show() override {
		std::string dataDefinitionName = static_cast<ModelDataDefinition*>(_object)->getName();
		std::string message = "time=" + std::to_string(_time) + //Util::StrTimeUnit(???)+
				",description=\"" + _description+"\""+
				",block=\"" + dataDefinitionName+"\"";
		return message;
	}

	const InternalEventHandler& eventHandler() const {
		return _handler;
	}

	const std::string& description() const {
		return _description;
	}

	void dispatchEvent() const {
		_handler(_parameter);
	}

	void* object() const {
		return _object;
	}

	void* parameter() const {
		return _parameter;
	}


private:
	InternalEventHandler _handler;
	std::string _description;
	void* _parameter;
	void* _object;
};

//namespace\\}
#endif /* EVENT_H */
