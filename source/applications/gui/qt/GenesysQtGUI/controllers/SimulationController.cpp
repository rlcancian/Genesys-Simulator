#include "controllers/SimulationController.h"

#include <QMessageBox>
#include "../../../../../kernel/simulator/Simulator.h"
#include "../../../../../kernel/simulator/ModelSimulation.h"

SimulationController::SimulationController(QWidget* ownerWidget, Simulator* simulator)
    : _ownerWidget(ownerWidget), _simulator(simulator) {
}

bool SimulationController::hasCurrentModelSimulation() const {
    return _simulator != nullptr
            && _simulator->getModelManager() != nullptr
            && _simulator->getModelManager()->current() != nullptr
            && _simulator->getModelManager()->current()->getSimulation() != nullptr;
}

ModelSimulation* SimulationController::currentSimulation() const {
    if (!hasCurrentModelSimulation()) {
        return nullptr;
    }
    return _simulator->getModelManager()->current()->getSimulation();
}

bool SimulationController::ensureReady(bool checkModel,
                                       bool modelChecked,
                                       const std::function<bool()>& runModelCheck,
                                       const std::function<bool()>& syncModelFromText) const {
    if (!hasCurrentModelSimulation()) {
        QMessageBox::warning(_ownerWidget, "Simulation", "No model is loaded to run simulation.");
        return false;
    }

    if (checkModel && !modelChecked) {
        if (!runModelCheck || !runModelCheck()) {
            return false;
        }
    }

    if (!syncModelFromText || !syncModelFromText()) {
        return false;
    }

    return hasCurrentModelSimulation();
}
