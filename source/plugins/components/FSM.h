/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   FSM.h
 * Author: henrique.buss
 *
 * Created on 27 de Maio de 2022, 16:32
 */

#ifndef FSM_H
#define FSM_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../../plugins/data/FSMState.h"
#include "../../plugins/data/FSMTransition.h"

/*!
 This component ...
 */
class FSM : public ModelComponent
{
public: // constructors
    FSM(Model *model, std::string name = "");
    virtual ~FSM() = default;

public: // virtual
    virtual std::string show();

public: // static
    static PluginInformation *GetPluginInformation();
    static ModelComponent *LoadInstance(Model *model, std::map<std::string, std::string> *fields);
    static ModelDataDefinition *NewInstance(Model *model, std::string name = "");

public:
    List<FSMState *> *states() const;
    List<FSMTransition *> *transitions() const;
    void insertTransition(FSMTransition *transition, FSMState *from, FSMState *to);
    void insertState(FSMState *state);
    void setInitialState(FSMState *state);
    struct TransitionData
    {
        FSMTransition *transition;
        FSMState *to;
    };

protected: // must be overriden
    virtual void _onDispatchEvent(Entity *entity, unsigned int inputPortNumber);
    virtual bool _loadInstance(std::map<std::string, std::string> *fields);
    virtual std::map<std::string, std::string> *_saveInstance(bool saveDefaultValues);

protected: // could be overriden .
    virtual bool _check(std::string *errorMessage);
    virtual void _createInternalAndAttachedData();

private: // methods
    void _transition(Entity *entity);

private: // attributes 1:1
    FSMState *_initialState = nullptr;
    TransitionData *_findTransition(List<TransitionData> *transitions);

private: // attributes 1:n
    List<FSMState *> *_states = new List<FSMState *>();
    List<FSMTransition *> *_transitions;
    std::map<Entity *, FSMState *> _entityStates;

    std::map<FSMState *, List<TransitionData> *> _transitionMap;
};

#endif /* FSM_H */
