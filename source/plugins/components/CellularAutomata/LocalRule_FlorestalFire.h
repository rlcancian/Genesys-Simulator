#pragma once

#include <random>

#include "LocalRule.h"
#include "Cell.h"

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

    virtual void applyRule(Cell* cell) {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_real_distribution<double> uniform(0.0, 1.0);
        StateSet* set = parentCellularAutomata->getStateSet();
        std::vector<unsigned int> sumNeighStates;
        sumNeighStates.resize(set->getStatesSyze());
        for (Cell* neigh : cell->getNeighbors()) {
            sumNeighStates.at(neigh->getCurrentState()->getDoubleValue())++;
        }
        cell->setNextState(cell->getCurrentState());
        switch ((int) cell->getCurrentState()->getDoubleValue()) {
            case 0: // soil
                if (uniform(e1) < pSoil2Grass+sumNeighStates.at(1)*pSoil2Grass)
                    cell->setNextState(set->getState(1));
                    break;
            case 1: // grass
                if (uniform(e1) < pGrass2Tree+sumNeighStates.at(2)*pGrass2Tree)
                    cell->setNextState(set->getState(2));
                else if (uniform(e1) < pGrass2Fire+(sumNeighStates.at(3)>0)*pGrassPropagateFire) 
                    cell->setNextState(set->getState(3));
                    break;
                break;
            case 2: // tree
                if (uniform(e1) < pTree2Fire+(sumNeighStates.at(3)>0)*pTreePropagateFire) 
                    cell->setNextState(set->getState(3));
                else if (uniform(e1) < pTree2Soil+sumNeighStates.at(0)*pTree2Soil)
                    cell->setNextState(set->getState(0));
                break;
            case 3: // fire
                cell->setNextState(set->getState(0));
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
