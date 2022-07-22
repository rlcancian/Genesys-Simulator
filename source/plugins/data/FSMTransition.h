/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   FSMTransition.h
 * Author: henrique.buss
 *
 * Created on 28 de Maio de 2022, 11:16
 */

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

#include <string>

#include "../../kernel/simulator/ModelDataDefinition.h"

class FSMTransition : public ModelDataDefinition
{
public:
    FSMTransition(Model *model, std::string name = "");
    virtual ~FSMTransition() = default;

public: // static
    static ModelDataDefinition *LoadInstance(Model *model, std::map<std::string, std::string> *fields);
    static PluginInformation *GetPluginInformation();
    static ModelDataDefinition *NewInstance(Model *model, std::string name = "");

public:
    virtual std::string show();

public:
    void setGuardExpression(std::string expression);
    std::string guardExpression();
    void setDelayExpression(std::string expression);
    std::string delayExpression();
    double delay() const;
    void onTransition(std::function<void(Model *)> handler);
    void perform();
    bool canPerform();

public:
    const struct DEFAULT_VALUES
    {
        const std::string guardExpression = "1";
        const std::string delayExpression = "0";
    } DEFAULT;

private:
    std::string _guardExpression = DEFAULT.guardExpression;
    std::string _delayExpression = DEFAULT.delayExpression;
    std::function<void(Model *)> _onTransition = nullptr;

protected: // must be overriden
    virtual bool _loadInstance(std::map<std::string, std::string> *fields);
    virtual std::map<std::string, std::string> *_saveInstance(bool saveDefaultValues);

protected: // could be overriden
    virtual bool _check(std::string *errorMessage);
};

#endif /* FSMTRANSITION_H */
