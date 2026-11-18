/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Release.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:17
 */

#include "plugins/components/DiscreteProcessing/Release.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
//#include "kernel/simulator/Attribute.h"
#include <stdexcept>
#include <cmath>
#include "auxiliar/ResourceSetSelection.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Release::GetPluginInformation;
}
#endif

ModelDataDefinition* Release::NewInstance(Model* model, std::string name) {
	return new Release(model, name);
}

Release::Release(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Release>(), name) {
	SimulationControlGeneric<unsigned short>* propPriority = new SimulationControlGeneric<unsigned short>(
									std::bind(&Release::priority, this), std::bind(&Release::setPriority, this, std::placeholders::_1),
									Util::TypeOf<Release>(), getName(), "Priority", "");
	// This block enables explicit typed creation for Release request list elements.
	SimulationControlGenericListPointer<SeizableItem*, Model*, SeizableItem>* propReleaseRequests = new SimulationControlGenericListPointer<SeizableItem*, Model*, SeizableItem> (
									_parentModel,
                                    std::bind(&Release::getReleaseRequests, this), std::bind(&Release::addReleaseRequests, this, std::placeholders::_1), std::bind(&Release::removeReleaseRequests, this, std::placeholders::_1),
									Util::TypeOf<Release>(), getName(), "ReleaseRequests", "", true, true, false,
                                    [](Model* model, const std::string& name) { return new SeizableItem(model, name, "1", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY); },
                                    [](Model* model) { return new SeizableItem(model, "", "1", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY); });	

	_parentModel->getControls()->insert(propPriority);
	_parentModel->getControls()->insert(propReleaseRequests);
	
	// setting properties
	_addSimulationControl(propPriority);
	_addSimulationControl(propReleaseRequests);
}

Release::~Release() {
	for (SeizableItem* item : *_releaseRequests->list()) {
		delete item;
	}
	delete _releaseRequests;
}

std::string Release::show() {
	std::string txt = ModelComponent::show() +
			"priority=" + std::to_string(_priority) +
			"releaseRequests={";
	unsigned short i = 0;
	for (SeizableItem* item : *_releaseRequests->list()) {
		txt += "request" + std::to_string(i++) + "=[" + item->show() + "],";
	}
	txt = txt.substr(0, txt.length() - 1) + "}";
	return txt;
}

void Release::setPriority(unsigned short _priority) {
	this->_priority = _priority;
}

unsigned short Release::priority() const {
	return _priority;
}

List<SeizableItem*>* Release::getReleaseRequests() const {
	return _releaseRequests;
}

void Release::addReleaseRequests(SeizableItem* newRequest) {
	_releaseRequests->insert(newRequest);
}

void Release::removeReleaseRequests(SeizableItem* request) {
	_releaseRequests->remove(request);
}

//void Release::setResource(Resource* _resource) {
//	this->_resource = _resource;
//}

//Resource* Release::resource() const {
//	return _resource;
//}

Resource* Release::_getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity) {
    (void) entity;
    return selectRequestResource(_parentModel, seizable, true, getName()).resource;
}


void Release::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	for (SeizableItem* seizable : *_releaseRequests->list()) {
		Resource* resource = _getResourceFromSeizableItem(seizable, entity);
        if (resource == nullptr) {
            throw std::runtime_error("Release \"" + getName() + "\": request has no resolved Resource");
        }
		unsigned int quantity = resourceRequestQuantity(_parentModel, seizable->getQuantityExpression(), getName());
        if (resource->getNumberBusy() < quantity) {
            throw std::runtime_error("Release \"" + getName() + "\": cannot release "
                + std::to_string(quantity) + " units of Resource \"" + resource->getName()
                + "\"; only " + std::to_string(resource->getNumberBusy()) + " units are busy");
        }
		traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, entity->getName() + " releases " + std::to_string(quantity) + " units of resource \"" + resource->getName() + "\"", TraceManager::Level::L7_internal);
		traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "Resource busy units before release=" + std::to_string(resource->getNumberBusy()), TraceManager::Level::L9_mostDetailed);
		resource->release(quantity); //{releases and sets the 'LastTimeSeized'property}
        if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
            unsigned int memberIndex = 0;
            for (auto* member : *seizable->getSet()->getElementSet()->list()) {
                if (member == resource) { seizable->setLastMemberSeized(memberIndex); break; }
                ++memberIndex;
            }
        }
		if (_reportStatistics) {
			double timeSeized = resource->getLastTimeSeized();
			double allocationEntityResource = entity->getAttributeValue("Entity.Allocation."+resource->getName()); // set by Seize::_onDispatchEvent() when the resource was seized
			std::string allocationCategory = Util::StrAllocation(static_cast<Util::AllocationType>((int) allocationEntityResource));
			std::string attribIndex="";
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + "."+allocationCategory+"Time")->getStatistics()->getCollector()->addValue(timeSeized);
			entity->setAttributeValue("Entity.Total"+allocationCategory+"Time", entity->getAttributeValue("Entity.Total"+allocationCategory+"Time") + timeSeized, attribIndex, true);			
		}
	}
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

void Release::_initBetweenReplications() {
	//for (SeizableItem* seizable : *_releaseRequests->list()) {
	//	if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE)
	//		seizable->getResource()->initBetweenReplications();
	//	else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET)
	//		seizable->getSet()->initBetweenReplications();
	//}
}

bool Release::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_priority = fields->loadField("priority", DEFAULT.priority);
		unsigned short numRequests = fields->loadField("resquests", DEFAULT.releaseRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItem* item = new SeizableItem(nullptr, "", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY);
			item->setElementManager(_parentModel->getDataManager());
			res &= item->loadInstance(fields, i);
			this->_releaseRequests->insert(item);
		}
	}
	return res;
}

void Release::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("priority", _priority, DEFAULT.priority, saveDefaultValues);
	fields->saveField("resquests", _releaseRequests->size(), DEFAULT.releaseRequestSize, saveDefaultValues);
	unsigned short i = 0;
	for (SeizableItem* request : *_releaseRequests->list()) {
		request->saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

void Release::_createAttachedAttributes() {
    if (_reportStatistics) {
        // The allocation category is supplied by the upstream Seize and may
        // differ from Delay's category. Materialize every category we may read.
        for (auto category : {Util::AllocationType::ValueAdded, Util::AllocationType::NonValueAdded,
                              Util::AllocationType::Transfer, Util::AllocationType::Wait,
                              Util::AllocationType::Others}) {
            _attachedAttributesInsert({"Entity.Total" + Util::StrAllocation(category) + "Time"});
        }
    }
}

bool Release::_check(std::string& errorMessage) {
	bool resultAll = true;
	int i = 0;
	for (SeizableItem* seizable : * _releaseRequests->list()) {
		resultAll &= _parentModel->checkExpression(seizable->getQuantityExpression(), "quantity", errorMessage);
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			// Resource* resource = seizable->getResource();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Resource>(), seizable->getResource(), "Resource", errorMessage);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			// Set* set = seizable->getSet();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Set>(), seizable->getSet(), "Set", errorMessage);
		}
		i++;
	}
	return resultAll;
}

PluginInformation* Release::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Release>(), &Release::LoadInstance, &Release::NewInstance);
	info->insertDynamicLibFileDependence("resource.so");
	std::string help = "The Release module is used to release units of a resource that an entity previously has seized.";
	help += " This module may be used to release individual resources or may be used to release resources within a set.";
	help += " For each resource to be released, the name and quantity to release are specified.";
	help += " When the entity enters the Release module, it gives up control of the specified resource(s).";
	help += " Any entities waiting in queues for those resources will gain control of the resources immediately.";
	help += " TYPICAL USES: (1) Finishing a customer order (release the operator); (2) Completing a tax return (release the accountant);";
	help += " (3) Leaving the hospital (release the doctor, nurse, hospital room)";
	info->setDescriptionHelp(help);
	info->setCategory("DiscreteProcessing");
	return info;
}

ModelComponent* Release::LoadInstance(Model* model, PersistenceRecord *fields) {
	Release* newComponent = new Release(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Release instance: " + std::string(e.what()));
	}
	return newComponent;

}

// void Release::_createInternalStatisticReporters() { }

void Release::_createEditableDataDefinitions() {
	int i = 0;
	for (SeizableItem* seizable : * _releaseRequests->list()) {
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			Resource* resource = seizable->getResource();
			if (resource == nullptr) {
				_optionalEditableDataDefinitionRemove("SeizableItem" + Util::StrIndex(i));
				i++;
				continue;
			}
			_optionalEditableDataDefinitionInsert("SeizableItem" + Util::StrIndex(i), resource);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			Set* set = seizable->getSet();
			if (set == nullptr) {
				_optionalEditableDataDefinitionRemove("SeizableItem" + Util::StrIndex(i));
				i++;
				continue;
			}
			_optionalEditableDataDefinitionInsert("SeizableItem" + Util::StrIndex(i), set);
		}
		i++;
	}
}
