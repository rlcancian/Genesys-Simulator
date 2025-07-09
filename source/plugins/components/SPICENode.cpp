/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SPICENode.h
 * Author: bnmfw & joaomai
 *
 * Created on 10 de outubro de 2023, 23:00
 */

#include "SPICENode.h"
#include "../../kernel/simulator/Model.h"



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &SPICENode::GetPluginInformation;
}
#endif

int SPICENode::std_node_count = 0;
std::set<std::string> SPICENode::used_names;

//
// public: /// constructors
//

SPICENode::SPICENode(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<SPICENode>(), name) {
    if (name == "") node_name = "net_" + std::to_string(std_node_count++);
    else {
        if (used_names.find(name) != used_names.end())
            throw std::invalid_argument(name + " already in use!\n");
        node_name = name;
    }
}

SPICENode::operator std::string() const {
    return node_name;
}

//
// public: /// new public user methods for this component
//

std::string SPICENode::getNodeName() {
    return node_name;
}

//
// public: /// virtual methods
//

std::string SPICENode::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* SPICENode::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<SPICENode>(), &SPICENode::LoadInstance, &SPICENode::NewInstance);
	info->setCategory("Electronics simulation");
	return info;
}

ModelComponent* SPICENode::LoadInstance(Model* model, PersistenceRecord *fields) {
	SPICENode* newComponent = new SPICENode(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

ModelDataDefinition* SPICENode::NewInstance(Model* model, std::string name) {
	return new SPICENode(model, name);
}

//
// protected: /// virtual method that must be overriden
//

bool SPICENode::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void SPICENode::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

void SPICENode::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->traceSimulation(this, "I'm just an electric Node model and I'm not event oriented.");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}
