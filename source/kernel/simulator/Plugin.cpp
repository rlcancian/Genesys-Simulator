/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Plugin.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:58
 */

#include "Plugin.h"
#include "Model.h"
#include "SourceModelComponent.h"
#include "SinkModelComponent.h"
//#include "Assign.h"
#include <assert.h>

//using namespace GenesysKernel;

Plugin::Plugin(StaticGetPluginInformation getInformation) {
    this->_StatMethodGetInformation = getInformation;
    try {
        PluginInformation* infos = _StatMethodGetInformation();
        this->_pluginInfo = infos;
        this->_isValidPlugin = true;
    } catch (...) {
        this->_isValidPlugin = false;
    }
}

std::string Plugin::show() {

    std::string message = "<" + _pluginInfo->getCategory() + "> ";
    if (_pluginInfo->isSource()) {
        message += "Source ";
    }
    if (_pluginInfo->isSink()) {
        message += "Sink ";
    }
    if (_pluginInfo->isSendTransfer()) {
        message += "SendTransfer ";
    }
    if (_pluginInfo->isReceiveTransfer()) {
        message += "ReceiveTransfer ";
    }
    if (_pluginInfo->isGenerateReport()) {
        message += "GenerateReport ";
    }
    if (_pluginInfo->isComponent()) {
        message += "Component ";
    } else {
        message += "Element ";
    }
    message += "\"" + _pluginInfo->getPluginTypename() + "\" ";
    if (_pluginInfo->getDynamicLibFilenameDependencies()->size() > 0) {
        message += "depends on=[";
        for (std::string depends : *_pluginInfo->getDynamicLibFilenameDependencies()) {
            message += depends + ",";
        }
        message = message.substr(0, message.length() - 2);
        message += "] ";
    }
    if (_pluginInfo->getDescriptionHelp() != "") {
        message += "description help=\"" + _pluginInfo->getDescriptionHelp() + "\" ";
    }
    //if (_pluginInfo->getObservation() != "") {
    //	message += "observation=\"" + _pluginInfo->getObservation() + "\" ";
    //}

    if (_pluginInfo->getFields()->size() > 0) {
        message += "fields=[";
        for (std::pair<std::string, std::string> field : *_pluginInfo->getFields()) {
            message += field.first + ", ";
        }
        message = message.substr(0, message.length() - 2);
        message += "] ";
    }
    if (_pluginInfo->getLanguageTemplate() != "") {
        message += "template=<" + _pluginInfo->getLanguageTemplate() + "nextId=0 > ";
    }
    message = message.substr(0, message.length() - 1);
    return message;
}

PluginInformation* Plugin::getPluginInfo() const {
    return _pluginInfo;
}

bool Plugin::isIsValidPlugin() const {
    return _isValidPlugin;
}

//Plugin::Plugin(std::string pluginTypename, bool source, bool drain) {
//    //_fullfilename = fullfilename;
//    _source = source;
//    _drain = drain;
//}

ModelDataDefinition* Plugin::loadNew(Model* model, PersistenceRecord *fields) {
    if (this->_pluginInfo->isComponent()) {
        return _loadNewComponent(model, fields);
    } else {
        return _loadNewElement(model, fields);
    }
}

bool Plugin::loadAndInsertNew(Model* model, PersistenceRecord *fields) {
    if (this->_pluginInfo->isComponent()) {
        ModelComponent* newComp = _loadNewComponent(model, fields);
        if (newComp != nullptr) {
            //model->getTraceManager()->trace(newComp->show());
            return true; //model->components()->insert(newComp);
        }
    } else {
        ModelDataDefinition* newElem = _loadNewElement(model, fields);
        if (newElem != nullptr) {
            //model->getTraceManager()->trace(newElem->show());
            return true; //model->elements()->insert(this->_pluginInfo->pluginTypename(), newElem);
        }
    }
    return false;
}

ModelDataDefinition* Plugin::newInstance(Model* model, std::string name) {
    StaticConstructorDataDefinitionInstance constructor = this->getPluginInfo()->getDataDefinitionConstructor();
    ModelDataDefinition* instance = (constructor(model, name));
    return instance;
}

//--------------------------
// Private
//--------------------------

ModelComponent* Plugin::_loadNewComponent(Model* model, PersistenceRecord *fields) {
    StaticLoaderComponentInstance loader = this->_pluginInfo->GetComponentLoader();
    ModelComponent* newElementOrComponent = loader(model, fields);
    return newElementOrComponent;
}

ModelDataDefinition* Plugin::_loadNewElement(Model* model, PersistenceRecord *fields) {
    StaticLoaderDataDefinitionInstance loader = this->_pluginInfo->getDataDefinitionLoader();
    ModelDataDefinition* newElementOrComponent = loader(model, fields);
    return newElementOrComponent;
}
