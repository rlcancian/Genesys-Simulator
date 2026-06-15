/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelCheckerDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 8 de Agosto de 2018, 18:44
 */

#include "ModelCheckerDefaultImpl1.h"
#include "../SourceModelComponent.h"
#include "../SinkModelComponent.h"
#include "ModelComponentManager.h"
#include "../Simulator.h"

#include <assert.h>
#include <unordered_set>

//using namespace GenesysKernel;

ModelCheckerDefaultImpl1::ModelCheckerDefaultImpl1(Model* model) {
	_model = model;
}

bool ModelCheckerDefaultImpl1::checkAll() {
	bool res = true;
	res &= checkSymbols();
	if (res)
		res &= checkLimits();
	if (res)
		res &= checkConnected();
	return res;
}

void ModelCheckerDefaultImpl1::_showResult(bool result, std::string checking) {
	std::string msgResult;
	if (result) {
		msgResult = checking + " successfully.";
	} else {
		msgResult = checking + " failed.";
	}
	_model->getTracer()->trace(msgResult);
}

void ModelCheckerDefaultImpl1::_recursiveConnectedTo(PluginManager* pluginManager, ModelComponent* comp, List<ModelComponent*>* visited, List<ModelComponent*>* unconnected, bool* drenoFound) {
	visited->insert(comp);
	_model->getTracer()->trace("Connected to \"" + comp->getName() + "\"");
	Plugin* plugin = pluginManager->find(comp->getClassname());
	if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
		unconnected->insert(comp);
		_model->getTracer()->traceError(
			"Component \"" + comp->getName() + "\" uses unregistered plugin type \"" + comp->getClassname() + "\".");
		*drenoFound = false;
		return;
	}
	if (plugin->getPluginInfo()->isSink() || (plugin->getPluginInfo()->isSendTransfer() && comp->getConnectionManager()->size() == 0)) {//(dynamic_cast<SinkModelComponent*> (comp) != nullptr) {
		// it is a sink OR it can send entities throught a transfer and has no nextConnections
		*drenoFound = true;
	} else { // it is not a sink
		if (comp->getConnectionManager()->size() == 0) {
			unconnected->insert(comp);
			_model->getTracer()->traceError("Component \"" + comp->getName() + "\" is unconnected (not a sink with no next componentes connected to)");
			*drenoFound = false;
		} else {
			ModelComponent* nextComp;
			for (const auto& [port, conn] : *comp->getConnectionManager()->connections()) {
				nextComp = conn->component;
				if (visited->find(nextComp) == visited->list()->end()) { // not visited yet
					*drenoFound = false;
					Util::IncIndent();
					this->_recursiveConnectedTo(pluginManager, nextComp, visited, unconnected, drenoFound);
					Util::DecIndent();
				} else {
					Util::IncIndent();
					_model->getTracer()->trace("Connected to " + nextComp->getName());
					Util::DecIndent();
					*drenoFound = true;
				}
			}
		}
	}
}

bool ModelCheckerDefaultImpl1::checkConnected() {
	_model->getTracer()->trace("Checking connected", TraceManager::Level::L7_internal);
	bool resultAll = true;
	PluginManager* pluginManager = this->_model->getParentSimulator()->getPluginManager();
	Plugin* plugin;
	Util::IncIndent();
	{
		// Use automatic local containers to avoid leaking traversal bookkeeping structures.
		List<ModelComponent*> visited;
		List<ModelComponent*> unconnected;
		for (ModelComponent* comp : *_model->getComponentManager()) {
			plugin = pluginManager->find(comp->getClassname());
			if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
				resultAll = false;
				unconnected.insert(comp);
				_model->getTracer()->traceError(
					"Component \"" + comp->getName() + "\" uses unregistered plugin type \"" + comp->getClassname() + "\".");
				continue;
			}
			if (plugin->getPluginInfo()->isSource() || plugin->getPluginInfo()->isReceiveTransfer()) { //(dynamic_cast<SourceModelComponent*> (comp) != nullptr) {
				// it is a source component OR it can receive enetities from transfer
				bool drenoFound = false;
				// Keep recursive traversal state entirely in stack-owned bookkeeping objects.
				_recursiveConnectedTo(pluginManager, comp, &visited, &unconnected, &drenoFound);
				if (!drenoFound)
					resultAll = false;
			}
		}
		// check if any component remains unconnected
		for (ModelComponent* comp : *_model->getComponentManager()) {
			if (visited.find(comp) == visited.list()->end()) { //not found
				resultAll = false;
				_model->getTracer()->traceError("Component \"" + comp->getName() + "\" is unconnected.");
			}
		}
	}
	_showResult(resultAll, "Checking connected");
	Util::DecIndent();
	return resultAll;
}

bool ModelCheckerDefaultImpl1::checkSymbols() {
	bool res = true;
	_model->getTracer()->trace("Checking symbols", TraceManager::Level::L7_internal);
	Util::IncIndent();
	{
		// check components
		_model->getTracer()->trace("Components:");
		Util::IncIndent();
		{
			//List<ModelComponent*>* components = _model->getComponents();
			for (ModelComponent* comp : *_model->getComponentManager()) {
				res &= comp->Check(comp);
			}
		}
		Util::DecIndent();

		// check elements
		if (res) {
			_model->getTracer()->trace("Elements:");
			Util::IncIndent();
			{
				bool result;
                std::string errorMessage = "";
				// Iterate over a value snapshot of type names while checking all registered data definitions.
				for (const std::string& elementType : _model->getDataManager()->getDataDefinitionClassnames()) {
					List<ModelDataDefinition*>* elements = _model->getDataManager()->getDataDefinitionList(elementType);
					for (ModelDataDefinition* modeldatum : *elements->list()) {
						// copyed from modelCOmponent. It is not inside the ModelDataDefinition::Check because ModelDataDefinition has no access to Model to call Tracer
						_model->getTracer()->trace("Checking " + modeldatum->getClassname() + ": \"" + modeldatum->getName() + "\" (id " + std::to_string(modeldatum->getId()) + ")"); //std::to_string(component->_id));
						Util::IncIndent();
						{
							try {
                                result = modeldatum->Check(modeldatum, errorMessage);
								res &= result;
								if (!result) {
                                    _model->getTracer()->traceError("Error: Checking has failed with message '" + errorMessage + "'");
								}
							} catch (const std::exception& e) {
								_model->getTracer()->traceError("Error verifying component " + modeldatum->show(), e);
							}
						}
						Util::DecIndent();
					}
				}
			}
			Util::DecIndent();
		}
	}
	_showResult(res, "Checking symbols");
	Util::DecIndent();

	return res;
}

bool ModelCheckerDefaultImpl1::checkActivationCode() {
	/* @ToDo: (importante): not implemented yet */
	_model->getTracer()->trace("Checking activation code", TraceManager::Level::L7_internal);
	Util::IncIndent();
	{

	}
	_showResult(true, "Checking activation code");
	Util::DecIndent();
	return true;
}

bool ModelCheckerDefaultImpl1::checkLimits() {
	bool res = true;
	std::string text;
	unsigned int value, limit;
	LicenceManager *licence = _model->getParentSimulator()->getLicenceManager();
	_model->getTracer()->trace("Checking model limits", TraceManager::Level::L7_internal);
	Util::IncIndent();
	{
		value = _model->getComponentManager()->getNumberOfComponents();
		limit = licence->getModelComponentsLimit();
		res &= value <= limit;
		_model->getTracer()->trace("Model has " + std::to_string(value) + "/" + std::to_string(limit) + " components");
		if (!res) {
			text = "Model has " + std::to_string(_model->getComponentManager()->getNumberOfComponents()) + " components, exceding the limit of " + std::to_string(licence->getModelComponentsLimit()) + " components imposed by the current activation code";
			//_model->getTraceManager()->trace(TraceManager::Level::errors, text);
		} else {
			value = _model->getDataManager()->getNumberOfDataDefinitions();
			limit = licence->getModelDatasLimit();
			res &= value <= limit;
			_model->getTracer()->trace("Model has " + std::to_string(value) + "/" + std::to_string(limit) + " elements");
			if (!res) {
				text = "Model has " + std::to_string(_model->getDataManager()->getNumberOfDataDefinitions()) + " elements, exceding the limit of " + std::to_string(licence->getModelDatasLimit()) + " elements imposed by the current activation code";
				//_model->getTraceManager()->trace(TraceManager::Level::errors, text);
			}
		}
		if (!res) {
			_model->getTracer()->traceError("Error: Checking has failed with message '" + text + "'");
		}
	}
	_showResult(res, "Checking limits");
	Util::DecIndent();
	return res;
}
