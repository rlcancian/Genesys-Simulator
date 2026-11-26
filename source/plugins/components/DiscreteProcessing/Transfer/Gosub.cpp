#include "plugins/components/DiscreteProcessing/Transfer/Gosub.h"
#include "../../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../../../kernel/simulator/model/Model.h"
#include <algorithm>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() { return &Gosub::GetPluginInformation; }
#endif

namespace {
bool registered(Model* model, const LabelDefinition* candidate) {
    if (!model || !candidate) return false;
    const auto* items = model->getDataManager()->getDataDefinitionList(Util::TypeOf<LabelDefinition>())->list();
    return items && std::find(items->begin(), items->end(), candidate) != items->end();
}
}

Gosub::Gosub(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Gosub>(), name) {
    auto* property = new SimulationControlGenericClass<LabelDefinition*, Model*, LabelDefinition>(
        _parentModel, std::bind(&Gosub::getLabelDefinition, this),
        std::bind(&Gosub::setLabelDefinition, this, std::placeholders::_1),
        Util::TypeOf<Gosub>(), getName(), "LabelDefinition", "", false, true, false,
        nullptr, false, false, nullptr, nullptr, nullptr, true);
    _parentModel->getControls()->insert(property);
    _addSimulationControl(property);
}

PluginInformation* Gosub::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<Gosub>(), &Gosub::LoadInstance, &Gosub::NewInstance);
    info->setCategory("Arena/Transfer"); info->setSendTransfer(true);
    info->setMinimumInputs(1); info->setMaximumInputs(1);
    info->setMinimumOutputs(1); info->setMaximumOutputs(1);
    info->insertDynamicLibFileDependence("labeldefinition.so");
    info->setDescriptionHelp("Calls a subroutine through a LabelDefinition and stores the continuation component ID in the entity's subroutine stack.");
    return info;
}
ModelComponent* Gosub::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* c = new Gosub(model); try { c->_loadInstance(fields); } catch (const std::exception& e) { c->traceError("Failed to load Gosub: " + std::string(e.what())); } return c;
}
ModelDataDefinition* Gosub::NewInstance(Model* model, std::string name) { return new Gosub(model, name); }
std::string Gosub::show() { auto* d=getLabelDefinition(); return ModelComponent::show()+",labelDefinition="+(d?d->getName():_destinationName); }
LabelDefinition* Gosub::getLabelDefinition() const { return registered(_parentModel,_destination)?_destination:nullptr; }
void Gosub::setLabelDefinition(LabelDefinition* d) {
    if (d && !registered(_parentModel,d)) { traceError("Gosub: LabelDefinition does not belong to this model."); return; }
    _destination=d; _destinationName=d?d->getName():std::string{};
    if(d) _attachedDataInsert("LabelDefinition",d); else _attachedDataRemove("LabelDefinition");
}
void Gosub::_createAttachedAttributes() {
    _attachedAttributesInsert({"Entity.Subroutine"});
    auto* a=dynamic_cast<Attribute*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(),"Entity.Subroutine"));
    if(a && (!a->getDimensionSizes() || a->getDimensionSizes()->empty())) a->insertDimentionSize(1);
}
void Gosub::_createEditableDataDefinitions() {
    if(!_destinationName.empty() && !getLabelDefinition()) setLabelDefinition(dynamic_cast<LabelDefinition*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<LabelDefinition>(),_destinationName)));
    if(auto* d=getLabelDefinition()) _attachedDataInsert("LabelDefinition",d); else _attachedDataRemove("LabelDefinition");
}
bool Gosub::_loadInstance(PersistenceRecord* f) { if(!ModelComponent::_loadInstance(f)) return false; _destinationName=f->loadField("labelDefinition",""); _destination=nullptr; _createEditableDataDefinitions(); return true; }
void Gosub::_saveInstance(PersistenceRecord* f,bool defaults) { ModelComponent::_saveInstance(f,defaults); auto* d=getLabelDefinition(); f->saveField("labelDefinition",d?d->getName():_destinationName); }
bool Gosub::_check(std::string& e) {
    _createEditableDataDefinitions(); bool ok=true; auto* d=getLabelDefinition();
    if(!d || !d->getEnterIntoLabelComponent()) { e+="Gosub '"+getName()+"': valid LabelDefinition/Label receiver required. "; ok=false; }
    auto* cm=getConnectionManager(); auto* c=cm?cm->getFrontConnection():nullptr;
    if(!c || !c->component || cm->size()!=1 || c->channel.portNumber!=0) { e+="Gosub '"+getName()+"': exactly one continuation connection to input port zero is required. "; ok=false; }
    if(!_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(),"Entity.Subroutine")) { e+="Gosub: Entity.Subroutine Attribute is missing. "; ok=false; }
    return ok;
}
void Gosub::_onDispatchEvent(Entity* entity,unsigned int) {
    auto* d=getLabelDefinition(); auto* c=getConnectionManager()->getFrontConnection();
    if(!entity || !d || !d->getEnterIntoLabelComponent() || !c || !c->component || c->channel.portNumber!=0) { traceError("Gosub '"+getName()+"': invalid Label or continuation."); return; }
    unsigned int i=0; while(entity->getAttributeValue("Entity.Subroutine",std::to_string(i))!=0.0) ++i;
    entity->setAttributeValue("Entity.Subroutine",static_cast<double>(c->component->getId()),std::to_string(i));
    traceSimulation(this,_parentModel->getSimulation()->getSimulatedTime(),entity,this,"Gosub calling subroutine "+d->getName(),TraceManager::Level::L7_internal);
    traceSimulation(this,_parentModel->getSimulation()->getSimulatedTime(),entity,this,"Gosub pushed continuation ID="+std::to_string(c->component->getId())+" at index="+std::to_string(i),TraceManager::Level::L9_mostDetailed);
    d->sendEntityToLabelComponent(entity,0.0);
}
