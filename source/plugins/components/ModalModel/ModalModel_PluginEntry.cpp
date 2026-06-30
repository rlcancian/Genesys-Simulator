#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "plugins/components/ModalModel/DefaultNode.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/Submodel.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &CellularAutomataComp::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &DefaultNode::GetPluginInformation; }
// extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &DefaultTransitionExtensions::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &FSMState::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_3() { return &ModalModelDefault::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &ModalModelFSM::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_5() { return &ModalModelPetriNet::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_6() { return &PetriPlace::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_7() { return &Submodel::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
