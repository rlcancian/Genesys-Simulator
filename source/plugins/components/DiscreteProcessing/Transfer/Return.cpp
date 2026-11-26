#include "plugins/components/DiscreteProcessing/Transfer/Return.h"
#include "../../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() { return &Return::GetPluginInformation; }
#endif
Return::Return(Model* model,std::string name):ModelComponent(model,Util::TypeOf<Return>(),name){}
PluginInformation* Return::GetPluginInformation(){ auto* i=new PluginInformation(Util::TypeOf<Return>(),&Return::LoadInstance,&Return::NewInstance); i->setCategory("Arena/Transfer"); i->setSendTransfer(true); i->setMinimumInputs(1); i->setMaximumInputs(1); i->setMinimumOutputs(0); i->setMaximumOutputs(0); i->setDescriptionHelp("Returns from a subroutine by transferring the entity to the component identified by the top of its subroutine stack."); return i; }
ModelComponent* Return::LoadInstance(Model* m,PersistenceRecord* f){ auto* c=new Return(m); try{c->_loadInstance(f);}catch(const std::exception& e){c->traceError("Failed to load Return: "+std::string(e.what()));} return c; }
ModelDataDefinition* Return::NewInstance(Model* m,std::string n){return new Return(m,n);}
void Return::_createAttachedAttributes(){ _attachedAttributesInsert({"Entity.Subroutine"}); auto* a=dynamic_cast<Attribute*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(),"Entity.Subroutine")); if(a&&(!a->getDimensionSizes()||a->getDimensionSizes()->empty())) a->insertDimentionSize(1); }
bool Return::_check(std::string& e){ bool ok=true; if(getConnectionManager()->size()!=0){e+="Return '"+getName()+"': conventional outputs are not allowed. ";ok=false;} if(!_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(),"Entity.Subroutine")){e+="Return: Entity.Subroutine Attribute is missing. ";ok=false;} return ok; }
void Return::_onDispatchEvent(Entity* entity,unsigned int){ if(!entity){traceError("Return: null entity.");return;} unsigned int i=0; while(entity->getAttributeValue("Entity.Subroutine",std::to_string(i))!=0.0) ++i; if(i==0){traceError("Return '"+getName()+"': subroutine stack is empty.");return;} --i; auto id=static_cast<Util::identification>(entity->getAttributeValue("Entity.Subroutine",std::to_string(i))); auto* dest=_parentModel->getComponentManager()->find(id); if(!dest){traceError("Return '"+getName()+"': return component does not exist.");return;} entity->setAttributeValue("Entity.Subroutine",0.0,std::to_string(i)); traceSimulation(this,_parentModel->getSimulation()->getSimulatedTime(),entity,this,"Returning from subroutine to "+dest->getName(),TraceManager::Level::L7_internal); _parentModel->sendEntityToComponent(entity,dest,0.0,0); }
