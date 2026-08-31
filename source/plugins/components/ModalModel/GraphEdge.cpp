#include "plugins/data/ModalModel/GraphEdge.h"

#include "plugins/data/ModalModel/GraphNode.h"

#include <cmath>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GraphEdge::GetPluginInformation;
}
#endif

GraphEdge::GraphEdge(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<GraphEdge>(), name) {
}

GraphEdge::GraphEdge(Model* model, GraphNode* source, GraphNode* destination, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<GraphEdge>(), name), _source(source), _destination(destination) {
	_syncEndpointNames();
}

PluginInformation* GraphEdge::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GraphEdge>(), &GraphEdge::LoadInstance, &GraphEdge::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Mathematical graph edge; not a GenESyS process Connection.");
	return info;
}

ModelDataDefinition* GraphEdge::LoadInstance(Model* model, PersistenceRecord* fields) {
	GraphEdge* edge = new GraphEdge(model);
	try {
		edge->_loadInstance(fields);
	} catch (const std::exception& e) {
		edge->traceError("Failed to load GraphEdge instance: " + std::string(e.what()));
	}
	return edge;
}

ModelDataDefinition* GraphEdge::NewInstance(Model* model, std::string name) {
	return new GraphEdge(model, name);
}

void GraphEdge::setSource(GraphNode* source) {
	_source = source;
	_syncEndpointNames();
}

GraphNode* GraphEdge::getSource() const {
	return _source;
}

void GraphEdge::setDestination(GraphNode* destination) {
	_destination = destination;
	_syncEndpointNames();
}

GraphNode* GraphEdge::getDestination() const {
	return _destination;
}

void GraphEdge::setDirected(bool directed) {
	_directed = directed;
}

bool GraphEdge::isDirected() const {
	return _directed;
}

void GraphEdge::setWeight(double weight) {
	_hasWeight = true;
	_weight = weight;
}

void GraphEdge::clearWeight() {
	_hasWeight = false;
	_weight = DEFAULT.weight;
}

bool GraphEdge::hasWeight() const {
	return _hasWeight;
}

double GraphEdge::getWeight(double defaultWeight) const {
	return _hasWeight ? _weight : defaultWeight;
}

std::string GraphEdge::getSourceNodeName() const {
	return _sourceNodeName;
}

std::string GraphEdge::getDestinationNodeName() const {
	return _destinationNodeName;
}

std::string GraphEdge::show() {
	return ModelDataDefinition::show() +
	       ", source=\"" + _sourceNodeName + "\"" +
	       ", destination=\"" + _destinationNodeName + "\"" +
	       ", directed=" + std::string(_directed ? "true" : "false") +
	       ", hasWeight=" + std::string(_hasWeight ? "true" : "false") +
	       ", weight=" + std::to_string(_weight);
}

bool GraphEdge::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_source = nullptr;
		_destination = nullptr;
		_sourceNodeName = fields->loadField("sourceNode", std::string(""));
		_destinationNodeName = fields->loadField("destinationNode", std::string(""));
		_directed = fields->loadField("directed", DEFAULT.directed);
		_hasWeight = fields->loadField("hasWeight", DEFAULT.hasWeight);
		_weight = fields->loadField("weight", DEFAULT.weight);
	}
	return res;
}

void GraphEdge::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	_syncEndpointNames();
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("sourceNode", _sourceNodeName, std::string(""), saveDefaultValues);
	fields->saveField("destinationNode", _destinationNodeName, std::string(""), saveDefaultValues);
	fields->saveField("directed", _directed ? 1u : 0u, 0u, saveDefaultValues);
	fields->saveField("hasWeight", _hasWeight ? 1u : 0u, 0u, saveDefaultValues);
	fields->saveField("weight", _weight, DEFAULT.weight, saveDefaultValues);
}

bool GraphEdge::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_source == nullptr) {
		errorMessage += "GraphEdge \"" + getName() + "\" has null source. ";
		resultAll = false;
	}
	if (_destination == nullptr) {
		errorMessage += "GraphEdge \"" + getName() + "\" has null destination. ";
		resultAll = false;
	}
	if (_hasWeight && !std::isfinite(_weight)) {
		errorMessage += "GraphEdge \"" + getName() + "\" has non-finite weight. ";
		resultAll = false;
	}
	return resultAll;
}

void GraphEdge::_syncEndpointNames() {
	if (_source != nullptr) {
		_sourceNodeName = _source->getName();
	}
	if (_destination != nullptr) {
		_destinationNodeName = _destination->getName();
	}
}
