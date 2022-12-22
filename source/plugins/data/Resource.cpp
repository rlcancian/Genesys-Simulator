/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Resource.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:52
 */

#include "Resource.h"
#include "../../kernel/simulator/Counter.h"
#include "../../kernel/simulator/Model.h"
#include "Schedule.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Resource::GetPluginInformation;
}
#endif

ModelDataDefinition* Resource::NewInstance(Model* model, std::string name) {
	return new Resource(model, name);
}

Resource::Resource(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Resource>(), name) {
	_resourceEventHandlers->setSortFunc([](const SortedResourceEventHandler* a, const SortedResourceEventHandler * b) {
		return a->second < b->second; /// Handlers are sorted by priority
	});
	// OLD
	//GetterMemberDouble getter = DefineGetterMember<Resource>(this, &Resource::getCapacity);
	//SetterMemberDouble setter = DefineSetterMember<Resource>(this, &Resource::setCapacity);
	//model->getControls()->insert(new SimulationControl(Util::TypeOf<Resource>(), getName() + ".Capacity", getter, setter));
	//GetterMemberDouble getter2 = DefineGetterMember<Resource>(this, &Resource::getCostPerUse);
	//SetterMemberDouble setter2 = DefineSetterMember<Resource>(this, &Resource::setCostPerUse);
	//model->getControls()->insert(new SimulationControl(Util::TypeOf<Resource>(), getName() + ".CostPerUse", getter2, setter2));
	// ...
	PropertyT<unsigned int>* prop1 = new PropertyT<unsigned int>(Util::TypeOf<Resource>(), "Capacity",
			DefineGetter<Resource, unsigned int>(this, &Resource::getCapacity),
			DefineSetter<Resource, unsigned int>(this, &Resource::setCapacity));
	_parentModel->getControls()->insert(prop1);
	_addProperty(prop1);
}

std::string Resource::show() {
	return ModelDataDefinition::show() +
			",capacity=" + Util::StrTruncIfInt(std::to_string(_capacity)) +
			",costBusyByTimeUnit=" + Util::StrTruncIfInt(std::to_string(_costBusyTimeUnit)) +
			",costIdleByTimeUnit=" + Util::StrTruncIfInt(std::to_string(_costIdleTimeUnit)) +
			",costPerUse=" + Util::StrTruncIfInt(std::to_string(_costPerUse)) +
			",state=" + Util::StrTruncIfInt(std::to_string(static_cast<int> (_resourceState)));
}

bool Resource::seize(unsigned int quantity, double priority) {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	_sumCapacityOverTime += _lastTimeCapacityEvaluated * getCapacity();
	_lastTimeCapacityEvaluated = tnow;
	bool canSeize = (getCapacity() - _numberBusy) >= quantity;
	if (canSeize) {
		_sumNumberBusyOverTime += std::max<double>(_lastTimeReleased, _lastTimeSeized) * _lastTimeAnythingNumberBusy;
		_numberBusy += quantity;
		if (_reportStatistics)
			_counterNumSeizes->incCountValue(quantity);
		_lastTimeSeized = tnow; // instant when seized
		_lastTimeAnythingNumberBusy = _numberBusy;
		_counterTotalCostPerUse->incCountValue(_costPerUse);
		if (_resourceState != Resource::ResourceState::BUSY) {
			_resourceState = Resource::ResourceState::BUSY;
			_counterTotalCostIdle->incCountValue(_costIdleTimeUnit * (tnow - _lastTimeIdle));
			_lastTimeBusy = tnow;
		}
	}
	return canSeize;
}

void Resource::release(unsigned int quantity) {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	_sumNumberBusyOverTime += std::max<double>(_lastTimeReleased, _lastTimeSeized) * _lastTimeAnythingNumberBusy;
	_sumCapacityOverTime += _lastTimeCapacityEvaluated * getCapacity();
	_lastTimeCapacityEvaluated = tnow;
	if (_numberBusy >= quantity) {
		_numberBusy -= quantity;
	} else {
		_numberBusy = 0;
	}
	if (_numberBusy == 0) {
		_resourceState = Resource::ResourceState::IDLE;
		_counterTotalCostBusy->incCountValue(_costBusyTimeUnit * (tnow - _lastTimeBusy));
		_lastTimeIdle = tnow;
	}
	_lastTimeReleased = tnow;
	_lastTimeAnythingNumberBusy = _numberBusy;
	double timeSeized = _lastTimeReleased - _lastTimeSeized;
	if (_reportStatistics) {
		_counterNumReleases->incCountValue(quantity);
		_cstatTimeSeized->getStatistics()->getCollector()->addValue(timeSeized);
		_counterTotalTimeSeized->incCountValue(timeSeized);
	}
	_notifyReleaseEventHandlers();
	_checkFailByCount();
}

void Resource::_fail() {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	_sumCapacityOverTime += _lastTimeCapacityEvaluated * getCapacity();
	_originalCapacity = getCapacity();
	_lastTimeFailed = tnow;
	_lastTimeCapacityEvaluated = tnow;
	_capacity = 0;
	_isActive = false;
	_resourceState = ResourceState::FAILED;
	_parentModel->getTracer()->traceSimulation(this, "Resource \"" + getName() + "\" has failed. Capacity " + std::to_string(_originalCapacity) + " changed to 0");
}

void Resource::_active() {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	_capacity = _originalCapacity;
	_lastTimeCapacityEvaluated = tnow;
	if (_reportStatistics) {
		double failureTime = tnow - _lastTimeFailed;
		_counterTotalTimeFailed->incCountValue(failureTime);
		_cstatTimeFailed->getStatistics()->getCollector()->addValue(failureTime);
	}
	_isActive = true;
	if (_numberBusy == 0)
		_resourceState = ResourceState::IDLE;
	else
		_resourceState = ResourceState::BUSY;
	_parentModel->getTracer()->traceSimulation(this, "Resource \"" + getName() + "\" has been activated. Capacity set back to " + std::to_string(_capacity));
}

//
//
//

void Resource::setResourceState(ResourceState _resourceState) {
	_resourceState = _resourceState;
}

Resource::ResourceState Resource::getResourceState() const {
	return _resourceState;
}

void Resource::setCapacity(unsigned int _capacity) {
	_capacity = _capacity;
}

unsigned int Resource::getCapacity() const {
	if (_capacitySchedule == nullptr || _parentModel == nullptr || !_isActive)
		return _capacity;
	else
		return _parentModel->parseExpression(_capacitySchedule->getExpression());
}

void Resource::setCostBusyTimeUnit(double _costBusyTimeUnit) {
	_costBusyTimeUnit = _costBusyTimeUnit;
}

double Resource::getCostBusyTimeUnit() const {
	return _costBusyTimeUnit;
}

void Resource::setCostIdleTimeUnit(double _costIdleTimeUnit) {
	_costIdleTimeUnit = _costIdleTimeUnit;
}

double Resource::getCostIdleTimeUnit() const {
	return _costIdleTimeUnit;
}

void Resource::setCostPerUse(double _costPerUse) {
	_costPerUse = _costPerUse;
}

double Resource::getCostPerUse() const {
	return _costPerUse;
}

unsigned int Resource::getNumberBusy() const {
	return _numberBusy;
}

void Resource::addReleaseResourceEventHandler(ResourceEventHandler eventHandler, ModelComponent* component, unsigned int priority) {
	ModelComponent* compHandler;
	for (SortedResourceEventHandler* sreh : * _resourceEventHandlers->list()) {
		compHandler = sreh->first.second;
		if (compHandler == component) {
			return; // already exists. Do not insert again
		}
	}
	SortedResourceEventHandler* sortedEventHandler = new SortedResourceEventHandler({
		{eventHandler, component}, priority
	});
	_resourceEventHandlers->insert(sortedEventHandler);
}

double Resource::getLastTimeSeized() const {
	return _lastTimeSeized;
}

double Resource::getInstantCapacityUtilization() const {
	double capacity = getCapacity();
	if (capacity == 0)
		return 0.0;
	else
		return _numberBusy / capacity;
}

double Resource::getCapacityUtilization() const {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	if (_parentModel != nullptr && tnow > 0)
		return _sumNumberBusyOverTime / (_sumCapacityOverTime * tnow);
	else
		return 0.0;
}

double Resource::getSeizedUtilization() const {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	if (_parentModel != nullptr && tnow > 0)
		return _counterTotalTimeSeized->getCountValue() / tnow;
	else
		return 0.0;
}

void Resource::insertFailure(Failure* failure) {
	_failures->insert(failure);
	failure->falingResources()->insert(this);
}

void Resource::removeFailure(Failure* failure) {
	_failures->remove(failure);
	failure->falingResources()->remove(this);
}

void Resource::setCapacitySchedule(Schedule* _capacitySchedule) {
	_capacitySchedule = _capacitySchedule;
}

Schedule* Resource::getCapacitySchedule() const {
	return _capacitySchedule;
}


//
//
//

void Resource::_checkFailByCount() {
	for (Failure* failure : *_failures->list()) {
		if (failure->getFailureType() == Failure::FailureType::COUNT) {
			failure->checkIsGoingToFailByCount(this);
		}
	}
}

// 

void Resource::_initBetweenReplications() {
	ModelDataDefinition::_initBetweenReplications();
	_lastTimeSeized = 0.0;
	_lastTimeReleased = 0.0;
	_lastTimeFailed = 0.0;
	_lastTimeCapacityEvaluated = 0.0;
	_lastTimeAnythingNumberBusy = 0.0;
	_lastTimeIdle = 0.0;
	_lastTimeBusy = 0.0;
	_sumNumberBusyOverTime = 0.0;
	_sumCapacityOverTime = 0.0;
	_numberBusy = 0;
	_isActive = true;
}

void Resource::_notifyReleaseEventHandlers() {
	for (SortedResourceEventHandler* sortedHandler : *_resourceEventHandlers->list()) {
		ResourceEventHandler handler = sortedHandler->first.first;
		handler(this);
	}
}

bool Resource::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_capacity = fields->loadField("Capacity", DEFAULT.capacity);
		_costBusyTimeUnit = fields->loadField("CostBusyTimeUnit", DEFAULT.cost);
		_costIdleTimeUnit = fields->loadField("CostIdleTimeUnit", DEFAULT.cost);
		_costPerUse = fields->loadField("CostPerUse", DEFAULT.cost);
		_resourceState = static_cast<Resource::ResourceState> (fields->loadField("ResourceState", static_cast<int> (DEFAULT.resourceState)));

//		unsigned int nv = fields->loadField("Failures", 0u);
//		for (unsigned short i = 0; i < nv; i++) {
//			Failure* item = new Failure();
//			item->loadInstance(fields, i);
//			this->_failures->insert(item);
//		}
	}
	return res;
}

void Resource::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("Capacity", _capacity, DEFAULT.capacity, saveDefaultValues);
	fields->saveField("CostBusyTimeUnit", _costBusyTimeUnit, DEFAULT.cost, saveDefaultValues);
	fields->saveField("CostIdleTimeUnit", _costIdleTimeUnit, DEFAULT.cost, saveDefaultValues);
	fields->saveField("CostPerUse", _costPerUse, DEFAULT.cost, saveDefaultValues);
	fields->saveField("ResourceState", static_cast<int> (_resourceState), static_cast<int> (DEFAULT.resourceState), saveDefaultValues);
	
//	Failure* let;
//	fields->saveField("Failures", _failures->size(), 0u, saveDefaultValues);
//	unsigned short i = 0;
//	for (std::list<Failure*>::iterator it = _failures->list()->begin(); it != _failures->list()->end(); it++, i++) {
//		let = (*it);
//		let->saveInstance(fields, i, saveDefaultValues);
//		i++;
//	}
}

bool Resource::_check(std::string* errorMessage) {
	//@TODO CHECK!
	*errorMessage += "";
	return true;
}

void Resource::_onReplicationEnd(SimulationEvent* se) {
	double totalTime = se->getSimulatedTime();
	double seizedTime = _counterTotalTimeSeized->getCountValue();
	double finalProportionSeized = seizedTime / totalTime;
	_cstatProportionSeized->getStatistics()->getCollector()->addValue(finalProportionSeized); // final proportionSeized is just one more value to this cstat
}

void Resource::_createInternalAndAttachedData() {
	if (_reportStatistics && _cstatTimeSeized == nullptr) {
		_cstatProportionSeized = new StatisticsCollector(_parentModel, getName() + "." + "ProportionSeized", this);
		_internalDataInsert("ProportionSeized", _cstatProportionSeized);
		_cstatCapacityUtilization = new StatisticsCollector(_parentModel, getName() + "." + "CapacityUtilization", this);
		_internalDataInsert("CapacityUtilization", _cstatCapacityUtilization);
		_cstatTimeSeized = new StatisticsCollector(_parentModel, getName() + "." + "TimeSeized", this);
		_internalDataInsert("TimeSeized", _cstatTimeSeized);
		_cstatTimeFailed = new StatisticsCollector(_parentModel, getName() + "." + "TimeFailed", this);
		_internalDataInsert("TimeFailed", _cstatTimeFailed);
		_counterTotalTimeSeized = new Counter(_parentModel, getName() + "." + "TotalTimeSeized", this);
		_internalDataInsert("TotalTimeSeized", _counterTotalTimeSeized);
		_counterTotalTimeFailed = new Counter(_parentModel, getName() + "." + "TotalTimeFailed", this);
		_internalDataInsert("TotalTimeFailed", _counterTotalTimeFailed);
		_counterNumSeizes = new Counter(_parentModel, getName() + "." + "Seizes", this);
		_internalDataInsert("Seizes", _counterNumSeizes);
		_counterNumReleases = new Counter(_parentModel, getName() + "." + "Releases", this);
		_internalDataInsert("Releases", _counterNumReleases);
		_counterTotalCostBusy = new Counter(_parentModel, getName() + "." + "CostBusy", this);
		_internalDataInsert("CostBusy", _counterTotalCostBusy);
		_counterTotalCostIdle = new Counter(_parentModel, getName() + "." + "CostIdle", this);
		_internalDataInsert("CostIdle", _counterTotalCostIdle);
		_counterTotalCostPerUse = new Counter(_parentModel, getName() + "." + "CostPerUse", this);
		_internalDataInsert("CostPerUse", _counterTotalCostPerUse);
		_parentModel->getOnEvents()->addOnReplicationEndHandler(this, &Resource::_onReplicationEnd);
	} else if (!_reportStatistics && _cstatTimeSeized != nullptr) {
		_internalDataClear();
	}
	for (Failure* failure : *_failures->list()) {
		_attachedDataInsert(getName() + "." + failure->getName(), failure);
	}
}

//
//
//

PluginInformation* Resource::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Resource>(), &Resource::LoadInstance, &Resource::NewInstance);
	info->insertDynamicLibFileDependence("failure.so");
	info->insertDynamicLibFileDependence("schedule.so");
	//info->set...
	return info;
}

ModelDataDefinition* Resource::LoadInstance(Model* model, PersistenceRecord *fields) {
	Resource* newElement = new Resource(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}
