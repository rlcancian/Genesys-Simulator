#pragma once

#include <random>
#include <vector>

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

class LocalRule_FlorestalFire : public LocalRule {
public:

    LocalRule_FlorestalFire(CellularAutomataBase* parentCellularAutomata)
    : LocalRule(parentCellularAutomata) {
    }

    LocalRule_FlorestalFire(const LocalRule_FlorestalFire& orig)
    : LocalRule(orig) {
    }
    virtual ~LocalRule_FlorestalFire() = default;
public:
    virtual std::string getRuleType() const override { return "ForestFire"; }

    virtual void applyRule(Cell* cell) override {
        if (cell == nullptr || stateSet == nullptr) {
            return;
        }

        auto* enumerableStateSet = dynamic_cast<StateSet_Enumerable*>(stateSet);
        if (enumerableStateSet == nullptr || enumerableStateSet->getStatesSize() < 4) {
            cell->setNextState(cell->getCurrentState());
            return;
        }

        std::random_device randomDevice;
        std::default_random_engine engine(randomDevice());
        std::uniform_real_distribution<double> uniform(0.0, 1.0);
        std::vector<unsigned int> sumNeighStates(enumerableStateSet->getStatesSize(), 0);

        for (Cell* neigh : cell->getNeighbors()) {
            const long stateValue = neigh->getCurrentState().getValue();
            if (stateValue >= 0 && static_cast<size_t>(stateValue) < sumNeighStates.size()) {
                sumNeighStates.at(static_cast<size_t>(stateValue))++;
            }
        }

        const long currentState = cell->getCurrentState().getValue();
        cell->setNextState(cell->getCurrentState());

        switch (currentState) {
            case 0: // soil
                if (sumNeighStates.size() > 1 && uniform(engine) < pSoil2Grass + sumNeighStates.at(1) * pSoil2Grass) {
                    cell->setNextState(*enumerableStateSet->getState(1));
                }
                break;
            case 1: // grass
                if (sumNeighStates.size() > 2 && uniform(engine) < pGrass2Tree + sumNeighStates.at(2) * pGrass2Tree) {
                    cell->setNextState(*enumerableStateSet->getState(2));
                } else if (sumNeighStates.size() > 3 &&
                           uniform(engine) < pGrass2Fire + (sumNeighStates.at(3) > 0 ? pGrassPropagateFire : 0.0)) {
                    cell->setNextState(*enumerableStateSet->getState(3));
                }
                break;
            case 2: // tree
                if (sumNeighStates.size() > 3 &&
                    uniform(engine) < pTree2Fire + (sumNeighStates.at(3) > 0 ? pTreePropagateFire : 0.0)) {
                    cell->setNextState(*enumerableStateSet->getState(3));
                } else if (sumNeighStates.size() > 0 && uniform(engine) < pTree2Soil + sumNeighStates.at(0) * pTree2Soil) {
                    cell->setNextState(*enumerableStateSet->getState(0));
                }
                break;
            case 3: // fire
                cell->setNextState(*enumerableStateSet->getState(0));
                break;
            default:
                break;
        }
    }
protected:
private:
    double pSoil2Grass = 0.001;
    double pGrass2Tree = 0.0001;
    double pGrass2Fire = 0.00001;
    double pTree2Fire =  0.00005;
    double pGrassPropagateFire = 0.5;
    double pTreePropagateFire = 0.85;
    double pTree2Soil = 0.0005;
};
