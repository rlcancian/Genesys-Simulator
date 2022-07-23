/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   FSMState.h
 * Author: Henrique da Cunha Buss
 *
 * Created on 28 de Maio de 2022, 11:13
 */

#ifndef FSMSTATE_H
#define FSMSTATE_H

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../components/FSM.h"

class FSM;

class FSMState : public ModelDataDefinition
{
public:
    FSMState(Model *model, std::string name = "");
    virtual ~FSMState() = default;

public: // static
    static ModelDataDefinition *LoadInstance(Model *model, std::map<std::string, std::string> *fields);
    static PluginInformation *GetPluginInformation();
    static ModelDataDefinition *NewInstance(Model *model, std::string name = "");

public:
    virtual std::string show();

public:
    bool isFinal() const;
    void setIsFinal(bool isFinal);
    void setRefinement(FSM *refinement);
    bool hasRefinement() const;
    FSM *refinement() const;

protected: // must be overriden
    virtual bool _loadInstance(std::map<std::string, std::string> *fields);
    virtual std::map<std::string, std::string> *_saveInstance(bool saveDefaultValues);

protected: // could be overriden
    virtual bool _check(std::string *errorMessage);
    virtual void _initBetweenReplications();

private:
    bool _isFinal = false;
    FSM *_refinement = nullptr;
};

#endif /* FSMSTATE_H */
