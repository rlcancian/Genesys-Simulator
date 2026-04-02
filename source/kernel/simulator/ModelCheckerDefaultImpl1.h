/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelCheckerDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 8 de Agosto de 2018, 18:44
 */

#ifndef MODELCHECKERDEFAULTIMPL1_H
#define MODELCHECKERDEFAULTIMPL1_H

#include "ModelChecker_if.h"
#include "Model.h"
#include "PluginManager.h"

//namespace GenesysKernel {

class ModelCheckerDefaultImpl1 : public ModelChecker_if {
public:
	ModelCheckerDefaultImpl1(Model* model);
	virtual ~ModelCheckerDefaultImpl1() = default;
public:
	virtual bool checkAll() override;
	virtual bool checkConnected() override;
	virtual bool checkSymbols() override;
	virtual bool checkActivationCode() override;
	virtual bool checkLimits() override;
	virtual bool checkOrphaned() override;
private:
	void _recursiveConnectedTo(PluginManager* pluginManager, ModelComponent* comp, List<ModelComponent*>* visited, List<ModelComponent*>* unconnected, bool* drenoFound);
	void _showResult(bool result, std::string checking);
private:
	Model* _model;
};
//namespace\\}
#endif /* MODELCHECKERDEFAULTIMPL1_H */

