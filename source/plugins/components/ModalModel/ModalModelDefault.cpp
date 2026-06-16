/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModalModelDefault.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 01 de Julho de 2025, 14:26
 */

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
// #include "kernel/simulator/Simulator.h"
// #include "kernel/simulator/PluginManager.h"

namespace {
bool ContainsNode(List<DefaultNode *> *nodes, DefaultNode *node) {
  return nodes != nullptr && node != nullptr &&
         nodes->find(node) != nodes->list()->end();
}

int NodeIndex(List<DefaultNode *> *nodes, DefaultNode *node) {
  if (nodes == nullptr || node == nullptr) {
    return -1;
  }
  for (unsigned int i = 0; i < nodes->size(); i++) {
    if (nodes->getAtRank(i) == node) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void ExecuteStateAction(Model *model, FSMState *state, bool entryAction) {
  if (model == nullptr || state == nullptr) {
    return;
  }
  const std::string expression = entryAction ? state->getEntryActionExpression()
                                             : state->getExitActionExpression();
  if (expression != "") {
    model->parseExpression(expression);
  }
}
} // namespace

/// Externalize function GetPluginInformation to be accessible through dynamic
/// linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
  return &ModalModelDefault::GetPluginInformation;
}
#endif

//
// public: /// constructors
//

ModalModelDefault::ModalModelDefault(Model *model, std::string name)
    : ModelComponent(model, Util::TypeOf<ModalModelDefault>(), name) {}

//
// public: /// new public user methods for this component
//

void ModalModelDefault::addNode(DefaultNode *node) {
  if (node != nullptr && _nodes->find(node) == _nodes->list()->end()) {
    _nodes->insert(node);
  }
}

void ModalModelDefault::removeNode(DefaultNode *node) { _nodes->remove(node); }

void ModalModelDefault::addTransition(DefaultNodeTransition *transition) {
  if (transition != nullptr &&
      _transitions->find(transition) == _transitions->list()->end()) {
    _transitions->insert(transition);
    if (transition->getSource() != nullptr) {
      transition->getSource()->addTransition(transition);
    }
  }
}

void ModalModelDefault::removeTransition(DefaultNodeTransition *transition) {
  _transitions->remove(transition);
  if (transition != nullptr && transition->getSource() != nullptr) {
    transition->getSource()->removeTransition(transition);
  }
}

List<DefaultNode *> *ModalModelDefault::getNodes() const { return _nodes; }

List<DefaultNodeTransition *> *ModalModelDefault::getTransitions() const {
  return _transitions;
}

void ModalModelDefault::setMaxTransitionsPerDispatch(
    unsigned int maxTransitionsPerDispatch) {
  _maxTransitionsPerDispatch = maxTransitionsPerDispatch;
}

unsigned int ModalModelDefault::getMaxTransitionsPerDispatch() const {
  return _maxTransitionsPerDispatch;
}

//
// public: /// virtual methods
//

std::string ModalModelDefault::show() { return ModelComponent::show() + ""; }

//
// public: /// static methods that must have implementations (Load and New just
// the same. GetInformation must provide specific infos for the new component
//

PluginInformation *ModalModelDefault::GetPluginInformation() {
  PluginInformation *info = new PluginInformation(
      Util::TypeOf<ModalModelDefault>(), &ModalModelDefault::LoadInstance,
      &ModalModelDefault::NewInstance);
  info->setCategory("ModalModel");
  // info->setMinimumInputs(1);
  // info->setMinimumOutputs(1);
  // info->setMaximumInputs(1);
  info->setMaximumOutputs(2); //[0]: normal output after every reaction; [1]:
                              // final output after finishing condition
  // info->setSource(false);
  // info->setSink(false);
  // info->setSendTransfer(false);
  // info->setReceiveTransfer(false);
  // info->insertDynamicLibFileDependence("...");
  info->setDescriptionHelp(
      "Represents an aggregate modal model made of nodes and transitions. It "
      "can be specialized for FSM and Petri-net style execution.");
  // info->setAuthor("...");
  // info->setDate("...");
  // info->setObservation("...");
  return info;
}

ModelComponent *ModalModelDefault::LoadInstance(Model *model,
                                                PersistenceRecord *fields) {
  ModalModelDefault *newComponent = new ModalModelDefault(model);
  try {
    newComponent->_loadInstance(fields);
  } catch (const std::exception &e) {
  }
  return newComponent;
}

ModelDataDefinition *ModalModelDefault::NewInstance(Model *model,
                                                    std::string name) {
  return new ModalModelDefault(model, name);
}

void ModalModelDefault::addOutputExpressionReference(
    ModelDataDefinition *expressionReference) {
  _optionalEditableDataDefinitionInsert(expressionReference->getName(),
                                        expressionReference);
}

void ModalModelDefault::removeOutputExpressionReference(
    DefaultNodeTransition *expressionReference) {
  _optionalEditableDataDefinitionRemove(expressionReference->getName());
}

//
// protected: /// virtual method that must be overridden
//

bool ModalModelDefault::_loadInstance(PersistenceRecord *fields) {
  bool res = ModelComponent::_loadInstance(fields);
  if (res) {
    _maxTransitionsPerDispatch = fields->loadField(
        "maxTransitionsPerDispatch", DEFAULT.maxTransitionsPerDispatch);
    _timeDelayExpressionPerDispatch =
        fields->loadField("timeDelayExpressionPerDispatch",
                          DEFAULT.timeDelayExpressionPerDispatch);
    _timeDelayPerDispatchTimeUnit = fields->loadField(
        "timeDelayPerDispatchTimeUnit", DEFAULT.timeDelayPerDispatchTimeUnit);

    _nodes->clear();
    _transitions->clear();
    _entryNode = nullptr;
    _currentNode = nullptr;

    PluginManager *plugins =
        _parentModel->getParentSimulator()->getPluginManager();
    std::unordered_map<std::string, DefaultNode *> nodesByName;
    unsigned int nodesSize = fields->loadField("nodesSize", 0u);
    for (unsigned int i = 0; i < nodesSize; i++) {
      const std::string prefix = "node" + Util::StrIndex(i) + ".";
      auto nodeFields =
          std::unique_ptr<PersistenceRecord>(fields->newInstance());
      for (auto it = fields->begin(); it != fields->end(); ++it) {
        if (it->first.rfind(prefix, 0) == 0) {
          PersistenceRecord::Entry entry = it->second;
          entry.first = it->first.substr(prefix.size());
          nodeFields->insert(entry);
        }
      }
      if (nodeFields->size() == 0) {
        continue;
      }

      std::string nodeType =
          nodeFields->loadField("typename", Util::TypeOf<DefaultNode>());
      Plugin *nodePlugin = plugins->find(nodeType);
      if (nodePlugin == nullptr) {
        traceError("Could not load node plugin \"" + nodeType +
                   "\" while loading ModalModelDefault \"" + getName() + "\"");
        continue;
      }

      ModelDataDefinition *loaded =
          nodePlugin->loadNew(_parentModel, nodeFields.get());
      DefaultNode *node = dynamic_cast<DefaultNode *>(loaded);
      if (node == nullptr) {
        traceError("Loaded modal node is not a DefaultNode for typename \"" +
                   nodeType + "\"");
        continue;
      }
      node->setModelLevel(_id);
      addNode(node);
      nodesByName[node->getName()] = node;
    }

    unsigned int transitionsSize = fields->loadField("transitionsSize", 0u);
    for (unsigned int i = 0; i < transitionsSize; i++) {
      const std::string suffix = Util::StrIndex(i);
      std::string sourceName =
          fields->loadField("transitionSource" + suffix, "");
      std::string destinationName =
          fields->loadField("transitionDestination" + suffix, "");
      auto sourceIt = nodesByName.find(sourceName);
      auto destinationIt = nodesByName.find(destinationName);
      if (sourceIt == nodesByName.end() || destinationIt == nodesByName.end()) {
        traceError("Skipping modal transition with unknown source/destination "
                   "while loading \"" +
                   getName() + "\"");
        continue;
      }
      std::string transitionType = fields->loadField(
          "transitionTypename" + suffix, Util::TypeOf<DefaultNodeTransition>());
      DefaultNodeTransition *transition = nullptr;
      if (transitionType == Util::TypeOf<EFSMTransition>()) {
        transition = new EFSMTransition(
            sourceIt->second, destinationIt->second,
            fields->loadField("transitionName" + suffix, "T" + suffix));
      } else {
        transition = new DefaultNodeTransition(
            sourceIt->second, destinationIt->second,
            fields->loadField("transitionName" + suffix, "T" + suffix));
      }
      transition->setGuardExpression(
          fields->loadField("transitionGuard" + suffix, ""));
      transition->setOutputExpression(
          fields->loadField("transitionOutput" + suffix, ""));
      transition->setInputEvent(
          fields->loadField("transitionInputEvent" + suffix, ""));
      if (EFSMTransition *efsmTransition =
              dynamic_cast<EFSMTransition *>(transition)) {
        efsmTransition->setTriggerEvent(fields->loadField(
            "transitionTriggerEvent" + suffix, transition->getInputEvent()));
        efsmTransition->setProbabilityExpression(
            fields->loadField("transitionProbabilityExpression" + suffix, ""));
      }
      transition->setPriority(
          fields->loadField("transitionPriority" + suffix, 0u));
      transition->setProbability(
          fields->loadField("transitionProbability" + suffix, 1.0));
      transition->setTransitionKind(
          static_cast<DefaultNodeTransition::TransitionKind>(fields->loadField(
              "transitionKind" + suffix,
              static_cast<int>(
                  DefaultNodeTransition::TransitionKind::DETERMINISTIC))));
      addTransition(transition);
    }

    std::string entryNodeName = fields->loadField("entryNode", "");
    if (entryNodeName != "") {
      auto entryIt = nodesByName.find(entryNodeName);
      if (entryIt != nodesByName.end()) {
        _entryNode = entryIt->second;
      }
    }
    if (_entryNode == nullptr && _nodes->size() > 0) {
      _entryNode = _nodes->front();
    }
    _currentNode = _entryNode;
  }
  return res;
}

void ModalModelDefault::_saveInstance(PersistenceRecord *fields,
                                      bool saveDefaultValues) {
  ModelComponent::_saveInstance(fields, saveDefaultValues);
  fields->saveField("maxTransitionsPerDispatch", _maxTransitionsPerDispatch,
                    DEFAULT.maxTransitionsPerDispatch, saveDefaultValues);
  fields->saveField("timeDelayExpressionPerDispatch",
                    _timeDelayExpressionPerDispatch,
                    DEFAULT.timeDelayExpressionPerDispatch, saveDefaultValues);
  fields->saveField("timeDelayPerDispatchTimeUnit",
                    _timeDelayPerDispatchTimeUnit,
                    DEFAULT.timeDelayPerDispatchTimeUnit, saveDefaultValues);
  if (_entryNode != nullptr) {
    fields->saveField("entryNode", _entryNode->getName(), "",
                      saveDefaultValues);
  }

  fields->saveField("nodesSize", _nodes->size(), 0u, saveDefaultValues);
  unsigned int i = 0;
  for (DefaultNode *node : *_nodes->list()) {
    const std::string prefix = "node" + Util::StrIndex(i) + ".";
    auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
    ModelComponent::SaveInstance(nodeFields.get(), node);
    for (auto it = nodeFields->begin(); it != nodeFields->end(); ++it) {
      PersistenceRecord::Entry entry = it->second;
      entry.first = prefix + it->first;
      fields->insert(entry);
    }
    i++;
  }

  fields->saveField("transitionsSize", _transitions->size(), 0u,
                    saveDefaultValues);
  i = 0;
  for (DefaultNodeTransition *transition : *_transitions->list()) {
    const std::string suffix = Util::StrIndex(i);
    if (transition->getSource() != nullptr) {
      fields->saveField("transitionSource" + suffix,
                        transition->getSource()->getName(), "",
                        saveDefaultValues);
    }
    if (transition->getDestination() != nullptr) {
      fields->saveField("transitionDestination" + suffix,
                        transition->getDestination()->getName(), "",
                        saveDefaultValues);
    }
    fields->saveField("transitionTypename" + suffix,
                      dynamic_cast<EFSMTransition *>(transition) != nullptr
                          ? Util::TypeOf<EFSMTransition>()
                          : Util::TypeOf<DefaultNodeTransition>(),
                      Util::TypeOf<DefaultNodeTransition>(), saveDefaultValues);
    fields->saveField("transitionName" + suffix, transition->getName(), "",
                      saveDefaultValues);
    fields->saveField("transitionGuard" + suffix,
                      transition->getGuardExpression(), "", saveDefaultValues);
    fields->saveField("transitionOutput" + suffix,
                      transition->getOutputExpression(), "", saveDefaultValues);
    fields->saveField("transitionInputEvent" + suffix,
                      transition->getInputEvent(), "", saveDefaultValues);
    if (EFSMTransition *efsmTransition =
            dynamic_cast<EFSMTransition *>(transition)) {
      fields->saveField("transitionTriggerEvent" + suffix,
                        efsmTransition->getTriggerEvent(), "",
                        saveDefaultValues);
      fields->saveField("transitionProbabilityExpression" + suffix,
                        efsmTransition->getProbabilityExpression(), "",
                        saveDefaultValues);
    }
    fields->saveField("transitionPriority" + suffix, transition->getPriority(),
                      0u, saveDefaultValues);
    fields->saveField("transitionProbability" + suffix,
                      transition->getProbability(), 1.0, saveDefaultValues);
    fields->saveField(
        "transitionKind" + suffix,
        static_cast<int>(transition->getTransitionKind()),
        static_cast<int>(DefaultNodeTransition::TransitionKind::DETERMINISTIC),
        saveDefaultValues);
    i++;
  }
}

bool ModalModelDefault::_check(std::string &errorMessage) {
  bool resultAll = true;
  if (_nodes->size() == 0) {
    errorMessage += "ModalModelDefault requires at least one internal node. ";
    resultAll = false;
  }
  if (_maxTransitionsPerDispatch == 0) {
    errorMessage += "maxTransitionsPerDispatch must be greater than zero. ";
    resultAll = false;
  }
  if (_entryNode != nullptr && !ContainsNode(_nodes, _entryNode)) {
    errorMessage += "Entry node does not belong to this modal model. ";
    resultAll = false;
  }
  if (_timeDelayExpressionPerDispatch != "") {
    resultAll &=
        _parentModel->checkExpression(_timeDelayExpressionPerDispatch,
                                      "time delay per dispatch", errorMessage);
  }

  std::unordered_set<DefaultNode *> knownNodes;
  std::unordered_set<std::string> nodeNames;
  for (DefaultNode *node : *_nodes->list()) {
    if (node == nullptr) {
      errorMessage += "ModalModelDefault contains a null internal node. ";
      resultAll = false;
      continue;
    }
    knownNodes.insert(node);
    if (node->getName() != "" &&
        nodeNames.find(node->getName()) != nodeNames.end()) {
      errorMessage +=
          "Duplicated modal node name \"" + node->getName() + "\". ";
      resultAll = false;
    }
    nodeNames.insert(node->getName());

    FSMState *fsmState = dynamic_cast<FSMState *>(node);
    if (fsmState != nullptr) {
      if (fsmState->getEntryActionExpression() != "") {
        resultAll &= _parentModel->checkExpression(
            fsmState->getEntryActionExpression(),
            "entry action[" + fsmState->getName() + "]", errorMessage);
      }
      if (fsmState->getExitActionExpression() != "") {
        resultAll &= _parentModel->checkExpression(
            fsmState->getExitActionExpression(),
            "exit action[" + fsmState->getName() + "]", errorMessage);
      }
    }
  }

  for (auto transition : *_transitions->list()) {
    if (transition == nullptr) {
      errorMessage += "ModalModelDefault contains a null transition. ";
      resultAll = false;
      continue;
    }
    if (knownNodes.find(transition->getSource()) == knownNodes.end()) {
      errorMessage += "Transition \"" + transition->getName() +
                      "\" has a source outside the modal model. ";
      resultAll = false;
    }
    if (knownNodes.find(transition->getDestination()) == knownNodes.end()) {
      errorMessage += "Transition \"" + transition->getName() +
                      "\" has a destination outside the modal model. ";
      resultAll = false;
    }
    if (transition->getProbability() < 0.0) {
      errorMessage += "Transition \"" + transition->getName() +
                      "\" has negative probability. ";
      resultAll = false;
    }
    std::string guard = transition->getGuardExpression();
    if (guard != "") {
      resultAll &= _parentModel->checkExpression(
          guard, "guard expression[" + transition->getName() + "]",
          errorMessage);
    }
    std::string inputEvent = transition->getInputEvent();
    if (inputEvent != "") {
      resultAll &= _parentModel->checkExpression(
          inputEvent, "input event[" + transition->getName() + "]",
          errorMessage);
    }
    std::string output = transition->getOutputExpression();
    if (output != "") {
      resultAll &= _parentModel->checkExpression(
          output, "output expression[" + transition->getName() + "]",
          errorMessage);
    }
    EFSMTransition *efsmTransition = dynamic_cast<EFSMTransition *>(transition);
    if (efsmTransition != nullptr &&
        efsmTransition->getProbabilityExpression() != "") {
      resultAll &= _parentModel->checkExpression(
          efsmTransition->getProbabilityExpression(),
          "probability expression[" + transition->getName() + "]",
          errorMessage);
    }
  }
  return resultAll;
}

void ModalModelDefault::_initBetweenReplications() {}

void ModalModelDefault::_createAttachedAttributes() {
  std::string currentNodeAttribute =
      "Entity.ModalModel." + getName() + ".CurrentNode";
  std::string lastNodeAttribute =
      "Entity.ModalModel." + getName() + ".LastNode";
  _attachedAttributesInsert({currentNodeAttribute, lastNodeAttribute});
}

std::string ModalModelDefault::getTimeDelayExpressionPerDispatch() {
  return _timeDelayExpressionPerDispatch;
}

void ModalModelDefault::setTimeDelayExpressionPerDispatch(
    const std::string time_delay_expression_per_dispatch) {
  _timeDelayExpressionPerDispatch = time_delay_expression_per_dispatch;
}

Util::TimeUnit ModalModelDefault::getTimeDelayPerDispatchTimeUnit() {
  return _timeDelayPerDispatchTimeUnit;
}

void ModalModelDefault::setTimeDelayPerDispatchTimeUnit(
    const Util::TimeUnit time_delay_per_dispatch_time_unit) {
  _timeDelayPerDispatchTimeUnit = time_delay_per_dispatch_time_unit;
}

void ModalModelDefault::_onDispatchEvent(Entity *entity,
                                         unsigned int inputPortNumber) {
  std::string currentNodeAttribute =
      "Entity.ModalModel." + getName() + ".CurrentNode";
  std::string lastNodeAttribute =
      "Entity.ModalModel." + getName() + ".LastNode";
  const std::string dispatchEvent = std::to_string(inputPortNumber);

  DefaultNode *localCurrentNode = nullptr;
  double index = entity->getAttributeValue(currentNodeAttribute);
  double lastNodeId = entity->getAttributeValue(lastNodeAttribute);
  if (index == 0.0 && lastNodeId == 0.0 && _entryNode != nullptr) {
    localCurrentNode = _entryNode;
  } else {
    unsigned int currentIdx = static_cast<unsigned int>(index);
    if (currentIdx < _nodes->size()) {
      localCurrentNode = _nodes->getAtRank(currentIdx);
    }
  }
  if (localCurrentNode == nullptr) {
    localCurrentNode = _entryNode;
    if (localCurrentNode == nullptr) {
      for (DefaultNode *node : *_nodes->list()) {
        if (node->isInitialNode()) {
          localCurrentNode = node;
          break;
        }
      }
    }
  }
  if (localCurrentNode == nullptr && _nodes->size() > 0) {
    localCurrentNode = _nodes->front();
  }
  if (localCurrentNode == nullptr) {
    traceError("Modal model \"" + getName() +
               "\" has no current node to dispatch");
    _parentModel->sendEntityToComponent(
        entity, this->getConnectionManager()->getFrontConnection());
    return;
  }
  int initialNodeIndex = NodeIndex(_nodes, localCurrentNode);
  if (initialNodeIndex >= 0) {
    entity->setAttributeValue(currentNodeAttribute,
                              static_cast<double>(initialNodeIndex), "", true);
  }
  unsigned int transitions = 0;
  while (transitions < _maxTransitionsPerDispatch) {
    traceSimulation(this,
                    "Current node is \"" + localCurrentNode->getName() +
                        "\" and " + std::to_string(transitions) +
                        " transitions fired sor far",
                    TraceManager::Level::L7_internal);

    List<DefaultNodeTransition *> *outgoing =
        localCurrentNode->getTransitions();
    std::vector<DefaultNodeTransition *> enabled;
    for (DefaultNodeTransition *transition : *outgoing->list()) {
      if (transition->canFire(_parentModel, entity, dispatchEvent)) {
        enabled.push_back(transition);
      }
    }
    if (enabled.size() == 0) {
      traceSimulation(this, "No transition is enabled to fire",
                      TraceManager::Level::L7_internal);
      break;
    } else {
      std::stable_sort(enabled.begin(), enabled.end(),
                       [](DefaultNodeTransition *a, DefaultNodeTransition *b) {
                         return a->getPriority() < b->getPriority();
                       });
      const unsigned int bestPriority = enabled.front()->getPriority();
      std::vector<DefaultNodeTransition *> candidates;
      for (DefaultNodeTransition *option : enabled) {
        if (option->getPriority() == bestPriority) {
          candidates.push_back(option);
        }
      }
      DefaultNodeTransition *chosen = candidates.front();
      bool hasProbabilisticCandidate = false;
      for (DefaultNodeTransition *option : candidates) {
        if (option->getTransitionKind() ==
            DefaultNodeTransition::TransitionKind::PROBABILISTIC) {
          hasProbabilisticCandidate = true;
          break;
        }
      }
      if (hasProbabilisticCandidate) {
        double probabilitySum = 0.0;
        for (DefaultNodeTransition *option : candidates) {
          const double p =
              std::max(0.0, option->effectiveProbability(_parentModel, entity));
          probabilitySum += p;
        }
        if (probabilitySum > 0.0) {
          double sample = (static_cast<double>(std::rand()) /
                           static_cast<double>(RAND_MAX)) *
                          probabilitySum;
          double accum = 0.0;
          for (DefaultNodeTransition *option : candidates) {
            accum += std::max(
                0.0, option->effectiveProbability(_parentModel, entity));
            if (sample <= accum) {
              chosen = option;
              break;
            }
          }
        }
      }
      traceSimulation(this,
                      "Transition \"" + chosen->getName() + "\" from \"" +
                          chosen->getSource()->getName() + "\" to \"" +
                          chosen->getDestination()->getName() + "\" fires.",
                      TraceManager::Level::L7_internal);
      ExecuteStateAction(_parentModel,
                         dynamic_cast<FSMState *>(localCurrentNode), false);
      chosen->execute(_parentModel, entity);
      transitions++;
      DefaultNode *nextNode = chosen->getDestination();
      ExecuteStateAction(_parentModel, dynamic_cast<FSMState *>(nextNode),
                         true);
      localCurrentNode = nextNode;
      if (localCurrentNode != nullptr) {
        int nodeIndex = NodeIndex(_nodes, localCurrentNode);
        if (nodeIndex >= 0) {
          entity->setAttributeValue(currentNodeAttribute,
                                    static_cast<double>(nodeIndex), "", true);
        }
        entity->setAttributeValue(
            lastNodeAttribute, static_cast<double>(localCurrentNode->getId()),
            "", true);
      } else {
        traceError("New current node is unknown");
      }
    }
  }
  traceSimulation(this,
                  "Current node is \"" +
                      (localCurrentNode != nullptr ? localCurrentNode->getName()
                                                   : "<none>") +
                      "\" and " + std::to_string(transitions) +
                      " transitions were fired, so moving on...",
                  TraceManager::Level::L7_internal);
  double waitTime =
      _parentModel->parseExpression(_timeDelayExpressionPerDispatch);
  Util::TimeUnit stu =
      _parentModel->getSimulation()
          ->getReplicationBaseTimeUnit(); // getReplicationLengthTimeUnit();
  waitTime *= Util::TimeUnitConvert(_timeDelayPerDispatchTimeUnit, stu);

  _parentModel->sendEntityToComponent(
      entity, this->getConnectionManager()->getFrontConnection(), waitTime);
}

DefaultNode *ModalModelDefault::getCurrentNode() { return _currentNode; }

DefaultNode *ModalModelDefault::getEntryNode() { return _entryNode; }

void ModalModelDefault::setEntryNode(DefaultNode *const entry_node) {
  _entryNode = entry_node;
}

//
// protected: /// virtual methods that could be overriden by derived classes, if
// needed
//

/*
bool ModalModelDefault::_check(std::string& errorMessage) {
        bool resultAll = true;
        resultAll &= _someString != "";
        resultAll &= _someUint > 0;
        return resultAll;
}
*/

/*
ParserChangesInformation* ModalModelDefault::_getParserChangesInformation() {
        ParserChangesInformation* changes = new ParserChangesInformation();
        //@TODO not implemented yet
        changes->getassignments().append("");
        changes->getexpressionProductions().append("");
        changes->getexpressions().append("");
        changes->getfunctionProductions().append("");
        changes->getassignments().append("");
        changes->getincludes().append("");
        changes->gettokens().append("");
        changes->gettypeObjs().append("");
        return changes;
}
*/

/*
void ModalModelDefault::_initBetweenReplications() {
        _someString = "Test";
        _someUint = 1;
}
*/

/*
void ModalModelDefault::_addSimulationControl(SimulationControl* property) {
}
*/

// void ModalModelDefault::_createInternalStatisticReporters() { }

// void ModalModelDefault::_createEditableDataDefinitions() { }
