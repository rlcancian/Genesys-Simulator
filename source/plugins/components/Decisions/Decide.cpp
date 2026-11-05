/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Decide.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 9 de Agosto de 2018, 20:39
 */

#include "plugins/components/Decisions/Decide.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/ConnectionManager.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include <iterator>

namespace {
class DecideConditionsControl final : public SimulationControl {
public:
	DecideConditionsControl(Decide* owner)
		: SimulationControl(Util::TypeOf<Decide>(), owner->getName(), "Conditions", "Ordered conditions; final output is Else", true),
		  _owner(owner) {}
	std::string getValue() const override {
		return std::to_string(_owner->getConditions()->size()) + " Items";
	}
	List<std::string>* getStrValues() override {
		return new List<std::string>(*_owner->getConditions());
	}
	void setValue(std::string value, bool remove = false) override {
		if (remove) {
			for (int i = static_cast<int>(_owner->getConditions()->size()) - 1; i >= 0; --i) {
				if (_owner->getConditions()->getAtRank(i) == value) { removeListValueAt(i); break; }
			}
		} else insertListValueAt(static_cast<int>(_owner->getConditions()->size()), value);
	}
	bool supportsPositionalListEditing() const override { return true; }
	void insertListValueAt(int index, const std::string& value) override {
		if (index < 0) throw std::out_of_range("condition index");
		_owner->editConditionInsertAt(static_cast<unsigned int>(index), value);
	}
	void replaceListValueAt(int index, const std::string& value) override {
		if (index < 0) throw std::out_of_range("condition index");
		_owner->editConditionReplaceAt(static_cast<unsigned int>(index), value);
	}
	void removeListValueAt(int index) override {
		if (index < 0) throw std::out_of_range("condition index");
		_owner->editConditionRemoveAt(static_cast<unsigned int>(index));
	}
private:
	Decide* _owner;
};
}

void Decide::markConditionPortEdit(unsigned int index, bool inserted) {
	_pendingConditionPortEdits.emplace_back(index, inserted);
}

bool Decide::takeConditionPortEdit(unsigned int& index, bool& inserted) {
	if (_pendingConditionPortEdits.empty()) return false;
	index = _pendingConditionPortEdits.front().first;
	inserted = _pendingConditionPortEdits.front().second;
	_pendingConditionPortEdits.pop_front();
	return true;
}

bool Decide::takeConditionEdit(ConditionEdit& edit) {
	if (_pendingConditionEdits.empty()) return false;
	edit = std::move(_pendingConditionEdits.front());
	_pendingConditionEdits.pop_front();
	return true;
}

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Decide::GetPluginInformation;
}
#endif

ModelDataDefinition* Decide::NewInstance(Model* model, std::string name) {
	return new Decide(model, name);
}

Decide::Decide(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Decide>(), name) {
	SimulationControl* propConditions = new DecideConditionsControl(this);

	_parentModel->getControls()->insert(propConditions);

	_addSimulationControl(propConditions);
}

Decide::~Decide() {
	delete _conditions;
	// Counter* elements in _numberOuts are owned by the associations system and
	// deleted by ~ModelDataDefinition(). Only the list container itself is freed here.
	delete _numberOuts;
}

List<std::string>* Decide::getConditions() const {
	return _conditions;
}

void Decide::addConditions(std::string newCondition) {
	insertConditionAt(static_cast<unsigned int>(_conditions->size()), std::move(newCondition));
}

void Decide::removeConditions(std::string condition) {
	for (unsigned int index = 0; index < _conditions->size(); ++index) {
		if (_conditions->getAtRank(index) == condition) {
			removeConditionAt(index);
			return;
		}
	}
}

void Decide::insertConditionAt(unsigned int index, std::string condition) {
	if (index > _conditions->size()) throw std::out_of_range("condition index");
	auto* values = _conditions->list();
	auto it = values->begin();
	std::advance(it, index);
	values->insert(it, std::move(condition));
	auto* connections = _connections->connections();
	std::map<unsigned int, Connection*> shifted;
	for (const auto& [port, connection] : *connections) {
		shifted.emplace(port >= index ? port + 1 : port, connection);
	}
	*connections = std::move(shifted);
	synchronizeOutputStatistics();
}

void Decide::replaceConditionAt(unsigned int index, std::string condition) {
	if (index >= _conditions->size()) throw std::out_of_range("condition index");
	_conditions->setAtRank(index, std::move(condition));
}

void Decide::removeConditionAt(unsigned int index) {
	if (index >= _conditions->size()) throw std::out_of_range("condition index");
	auto* values = _conditions->list();
	auto it = values->begin();
	std::advance(it, index);
	values->erase(it);
	_connections->removeAtPort(index);
	auto* connections = _connections->connections();
	std::map<unsigned int, Connection*> shifted;
	for (const auto& [port, connection] : *connections) {
		shifted.emplace(port > index ? port - 1 : port, connection);
	}
	*connections = std::move(shifted);
	synchronizeOutputStatistics();
}

void Decide::editConditionInsertAt(unsigned int index, std::string condition) {
	if (index > _conditions->size()) throw std::out_of_range("condition index");
	_pendingConditionEdits.push_back(ConditionEdit{ConditionEdit::Kind::Insert, index, {}, condition});
	insertConditionAt(index, std::move(condition));
	markConditionPortEdit(index, true);
}

void Decide::editConditionReplaceAt(unsigned int index, std::string condition) {
	if (index >= _conditions->size()) throw std::out_of_range("condition index");
	_pendingConditionEdits.push_back(ConditionEdit{ConditionEdit::Kind::Replace, index,
		_conditions->getAtRank(index), condition});
	replaceConditionAt(index, std::move(condition));
}

void Decide::editConditionRemoveAt(unsigned int index) {
	if (index >= _conditions->size()) throw std::out_of_range("condition index");
	ConditionEdit edit{ConditionEdit::Kind::Remove, index, _conditions->getAtRank(index), {}};
	if (Connection* connection = getConnectionManager()->getConnectionAtPort(index)) {
		edit.removedTarget = connection->component;
		edit.removedTargetPort = connection->channel.portNumber;
	}
	_pendingConditionEdits.push_back(std::move(edit));
	removeConditionAt(index);
	markConditionPortEdit(index, false);
}

void Decide::synchronizeOutputStatistics() {
	_createInternalStatisticReporters();
}

std::string Decide::show() {
	return ModelComponent::show() + "";
}

void Decide::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double value;
	unsigned short i = 0;
	for (const std::string& condition : *_conditions->list()) {
		value = _parentModel->parseExpression(condition);
		traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, std::to_string(i + 1) + "th condition evaluated to " + Util::StrTruncIfInt(std::to_string(value)) + "  // " + condition);
		if (value) {
			if (_reportStatistics) {
				_numberOuts->getAtRank(i)->incCountValue();
			}
			_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getConnectionAtPort(i));
			return;
		}
		i++;
	}
    traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "No condition has been evaluated true");
	if (_reportStatistics) {
		_numberOuts->getAtRank(i)->incCountValue();
	}
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getConnectionAtPort(i));
}

//void Decide::_initBetweenReplications() { // inherited method takes care of it
//	if (_numberOuts != nullptr) {
//		for (Counter* counter : *_numberOuts->list()) {
//			counter->clear();
//		}
//	}
//}

bool Decide::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		unsigned int nv = fields->loadField("conditions", 0);
		for (unsigned int i = 0; i < nv; i++) {
			this->_conditions->insert(fields->loadField("condition" + Util::StrIndex(i), ""));
		}
	}
	return res;
}

void Decide::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("conditions", _conditions->size(), 0u, saveDefaultValues);
	unsigned short i = 0;
	for (const std::string& condition : *_conditions->list()) {
		fields->saveField("condition" + Util::StrIndex(i), condition, "", saveDefaultValues);
		i++;
	}
}

bool Decide::_check(std::string& errorMessage) {
	bool allResult = true;
	for (const std::string& condition : *_conditions->list()) {
		allResult &= _parentModel->checkExpression(condition, "condition", errorMessage);
		_checkCreateAttachedReferencedDataDefinition(condition);
	}
	return allResult;
}

// void Decide::_createAttachedAttributes() { }

PluginInformation* Decide::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Decide>(), &Decide::LoadInstance, &Decide::NewInstance);
	info->setCategory("Decisions");
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(999);
	std::string help = "This module allows for decision-making processes in the system.";
	help += " It includes options to make decisions based on one or more conditions(for example, if entity type is Gold Card) or based on one or more probabilities(for example, 75 %, true; 25 %, false).";
	help += " Conditions can be based on attribute values(for example, Priority), variable values(for example, Number Denied), the entity type, or an expression(for example, NQ(ProcessA.Queue)).";
	help += " Multiple exit points are shown for each condition or probability and a single Else exit.";
	help += " TYPICAL USES: (1) Dispatching a faulty part for rework; (2) Branching accepted vs. rejected checks; (3) Sending priority customers to a dedicated process.";
	info->setDescriptionHelp(help);
	return info;
}

ModelComponent* Decide::LoadInstance(Model* model, PersistenceRecord *fields) {
	Decide* newComponent = new Decide(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Decide instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Decide::_createInternalStatisticReporters() {
	if (_reportStatistics) {
		if (_numberOuts == nullptr) {
			_numberOuts = new List<Counter*>();
		}
		const unsigned int outputCount = _conditions->size() + 1;
		while (_numberOuts->size() > outputCount) {
			const unsigned int index = static_cast<unsigned int>(_numberOuts->size() - 1);
			_mandatoryNonEditableDataDefinitionRemove("CountNumberOut" + Util::StrIndex(index));
			_numberOuts->list()->pop_back();
		}
		for (unsigned int i = _numberOuts->size(); i < outputCount; i++) {
			Counter* counter = new Counter(_parentModel, getName() + "." + "CountNumberOut" + Util::StrIndex(i), this);
			_numberOuts->insert(counter);
			_mandatoryNonEditableDataDefinitionInsert("CountNumberOut" + Util::StrIndex(i), counter);
		}
		for (unsigned int i = 0; i < _numberOuts->size(); i++) {
			Counter* counter = _numberOuts->getAtRank(i);
			if (counter != nullptr) {
				_mandatoryNonEditableDataDefinitionInsert("CountNumberOut" + Util::StrIndex(i), counter);
			}
		}
	} else  {
		this->_statisticReportersClear();
		if (_numberOuts != nullptr) {
			for (unsigned int i = 0; i < _numberOuts->size(); ++i) {
				_mandatoryNonEditableDataDefinitionRemove("CountNumberOut" + Util::StrIndex(i));
			}
		}
		delete _numberOuts;
		_numberOuts = nullptr;
	}
}

// void Decide::_createEditableDataDefinitions() { }
