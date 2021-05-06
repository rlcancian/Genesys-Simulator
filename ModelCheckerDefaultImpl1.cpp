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
#include "SourceModelComponent.h"
#include "SinkModelComponent.h"
#include "ComponentManager.h"
#include "Simulator.h"

#include <assert.h>

//using namespace GenesysKernel;

ModelCheckerDefaultImpl1::ModelCheckerDefaultImpl1(Model* model) {
	_model = model;
}

bool ModelCheckerDefaultImpl1::checkAll() {
	bool res = true;
	res &= checkSymbols();
	//res &= checkAndAddInternalLiterals();
	//res &= checkActivationCode();
	res &= checkLimits();
	res &= checkConnected();
	return res;
}

//bool ModelCheckerDefaultImpl1::checkAndAddInternalLiterals() {
//    /*  \todo: +-: not implemented yet */
//    return true;
//}

void ModelCheckerDefaultImpl1::_recursiveConnectedTo(PluginManager* pluginManager, ModelComponent* comp, List<ModelComponent*>* visited, List<ModelComponent*>* unconnected, bool* drenoFound) {
	visited->insert(comp);
	_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Connected to component \"" + comp->getName() + "\"");
	Plugin* plugin = pluginManager->find(comp->getClassname());
	assert(plugin != nullptr);
	if (plugin->getPluginInfo()->isSink() || (plugin->getPluginInfo()->isSendTransfer() && comp->getNextComponents()->size() == 0)) {//(dynamic_cast<SinkModelComponent*> (comp) != nullptr) {
		// it is a sink OR it can send entities throught a transfer and has no nextConnections
		*drenoFound = true;
	} else { // it is not a sink
		if (comp->getNextComponents()->size() == 0) {
			unconnected->insert(comp);
			_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Component \"" + comp->getName() + "\" is unconnected (not a sink with no next componentes connected to)");
			*drenoFound = false;
		} else {
			ModelComponent* nextComp;
			for (std::list<Connection*>::iterator it = comp->getNextComponents()->list()->begin(); it != comp->getNextComponents()->list()->end(); it++) {
				nextComp = (*it)->first;
				if (visited->find(nextComp) == visited->list()->end()) { // not visited yet
					*drenoFound = false;
					Util::IncIndent();
					this->_recursiveConnectedTo(pluginManager, nextComp, visited, unconnected, drenoFound);
					Util::DecIndent();
				} else {
					Util::IncIndent();
					_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Connected to " + nextComp->getName());
					Util::DecIndent();
					*drenoFound = true;
				}
			}
		}
	}
}

bool ModelCheckerDefaultImpl1::checkConnected() {
	/*  \todo: +-: not implemented yet */
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Checking connected");
	bool resultAll = true;
	PluginManager* pluginManager = this->_model->getParentSimulator()->getPlugins();
	Plugin* plugin;
	Util::IncIndent();
	{
		List<ModelComponent*>* visited = new List<ModelComponent*>();
		List<ModelComponent*>* unconnected = new List<ModelComponent*>();
		ModelComponent* comp;
		for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
			comp = (*it);
			plugin = pluginManager->find(comp->getClassname());
			assert(plugin != nullptr);
			if (plugin->getPluginInfo()->isSource() || plugin->getPluginInfo()->isReceiveTransfer()) { //(dynamic_cast<SourceModelComponent*> (comp) != nullptr) {
				// it is a source component OR it can receive enetities from transfer
				bool drenoFound = false;
				_recursiveConnectedTo(pluginManager, comp, visited, unconnected, &drenoFound);
			}
		}
		// check if any component remais unconnected
		for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
			comp = (*it);
			if (visited->find(comp) == visited->list()->end()) { //not found
				resultAll = false;
				_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Component \"" + comp->getName() + "\" is unconnected.");
			}
		}

	}
	Util::DecIndent();
	return resultAll;
}

bool ModelCheckerDefaultImpl1::checkSymbols() {
	bool res = true;
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Checking symbols");
	Util::IncIndent();
	{
		// check components
		_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Components:");
		Util::IncIndent();
		{
			//List<ModelComponent*>* components = _model->getComponents();
			for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
				res &= (*it)->Check((*it));
			}
		}
		Util::DecIndent();

		// check elements
		_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Elements:");
		Util::IncIndent();
		{
			std::string elementType;
			bool result;
			ModelElement* element;
			std::string* errorMessage = new std::string();
			std::list<std::string>* elementTypes = _model->getElements()->getElementClassnames();
			for (std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
				elementType = (*typeIt);
				List<ModelElement*>* elements = _model->getElements()->getElementList(elementType);
				for (std::list<ModelElement*>::iterator it = elements->list()->begin(); it != elements->list()->end(); it++) {
					element = (*it);
					// copyed from modelCOmponent. It is not inside the ModelElement::Check because ModelElement has no access to Model to call Tracer
					_model->getTracer()->trace(Util::TraceLevel::L7_detailed, "Checking " + element->getClassname() + ": \"" + element->getName() + "\" (id " + std::to_string(element->getId()) + ")"); //std::to_string(component->_id));
					Util::IncIndent();
					{
						try {
							result = element->Check((*it), errorMessage);
							res &= result;
							if (!result) {
								_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error: Checking has failed with message '" + *errorMessage + "'");
							}
						} catch (const std::exception& e) {
							_model->getTracer()->traceError(e, "Error verifying component " + element->show());
						}
					}
					Util::DecIndent();
				}
			}
		}
		Util::DecIndent();
	}
	Util::DecIndent();

	return res;
}

bool ModelCheckerDefaultImpl1::checkActivationCode() {
	/*  \todo: +-: not implemented yet */
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Checking activation code");
	Util::IncIndent();
	{

	}
	Util::DecIndent();
	return true;
}

bool ModelCheckerDefaultImpl1::checkLimits() {
	bool res = true;
	std::string text;
	unsigned int value, limit;
	LicenceManager *licence = _model->getParentSimulator()->getLicenceManager();
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Checking model limits");
	Util::IncIndent();
	{
		value = _model->getComponents()->getNumberOfComponents();
		limit = licence->getModelComponentsLimit();
		res &= value <= limit;
		_model->getTracer()->trace("Model has " + std::to_string(value) + "/" + std::to_string(limit) + " components");
		if (!res) {
			text = "Model has " + std::to_string(_model->getComponents()->getNumberOfComponents()) + " components, exceding the limit of " + std::to_string(licence->getModelComponentsLimit()) + " components imposed by the current activation code";
			//_model->getTraceManager()->trace(Util::TraceLevel::errors, text);
		} else {
			value = _model->getElements()->getNumberOfElements();
			limit = licence->getModelElementsLimit();
			res &= value <= limit;
			_model->getTracer()->trace("Model has " + std::to_string(value) + "/" + std::to_string(limit) + " elements");
			if (!res) {
				text = "Model has " + std::to_string(_model->getElements()->getNumberOfElements()) + " elements, exceding the limit of " + std::to_string(licence->getModelElementsLimit()) + " elements imposed by the current activation code";
				//_model->getTraceManager()->trace(Util::TraceLevel::errors, text);
			}
		}
		if (!res) {
			_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error: Checking has failed with message '" + text + "'");
		}
	}
	Util::DecIndent();
	return res;
}
