#ifndef MOVED_GUI_COMPATIBILITY_INCLUDES_H
#define MOVED_GUI_COMPATIBILITY_INCLUDES_H

// Temporary compatibility header for the physical GUI source-tree move.
// Some legacy source files still contain multiple includes on one line after
// the automated path adjustment used during the salvage move. The first include
// on those lines is still seen by the preprocessor, but subsequent includes are
// ignored as extra tokens. This header keeps the moved GUI buildable while those
// source files are repaired incrementally.

#include "../../../kernel/simulator/Simulator.h"
#include "../../../kernel/simulator/Plugin.h"
#include "../../../kernel/simulator/PropertyGenesys.h"
#include "../../../kernel/simulator/SimulationControlAndResponse.h"
#include "../../../kernel/simulator/LicenceManager.h"
#include "../../../kernel/simulator/SimulatorFacade.h"

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelManager.h"
#include "../../../kernel/simulator/model/ModelSimulation.h"
#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"

#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include "../../../kernel/simulator/essentialPlugins/Entity.h"

#include "../../../plugins/data/Logic/Variable.h"
#include "../../../plugins/data/DiscreteProcessing/Queue.h"

#include "../../../tools/Statistics/FitterDefaultImpl.h"
#include "../../../tools/Statistics/HypothesisTesterDefaultImpl1.h"
#include "../../../tools/Optimization/OptimizerDefaultImpl1.h"
#include "../../../tools/Continuous/SolverDefaultImpl1.h"

#endif // MOVED_GUI_COMPATIBILITY_INCLUDES_H
