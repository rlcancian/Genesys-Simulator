#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "plugins/components/ModalModel/DefaultNode.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/Submodel.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &CellularAutomataComp::GetPluginInformation,
        &DefaultNode::GetPluginInformation,
        &FSMState::GetPluginInformation,
        &ModalModelDefault::GetPluginInformation,
        &ModalModelFSM::GetPluginInformation,
        &ModalModelPetriNet::GetPluginInformation,
        &PetriPlace::GetPluginInformation,
        &Submodel::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
