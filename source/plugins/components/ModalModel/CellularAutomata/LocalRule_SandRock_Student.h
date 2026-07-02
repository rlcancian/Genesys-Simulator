// Aluno: Matheus Antonio de Souza - 21203363
#pragma once

#include <vector>

#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/StateSet_Enumerable.h"

class LocalRule_SandRock_Student : public LocalRule {
public:
    LocalRule_SandRock_Student(CellularAutomataBase* parentCellularAutomata)
        : LocalRule(parentCellularAutomata) {
    }

    LocalRule_SandRock_Student(const LocalRule_SandRock_Student& orig)
        : LocalRule(orig) {
    }
    virtual ~LocalRule_SandRock_Student() = default;
    static constexpr long EMPTY = 0;
    static constexpr long SAND = 1;
    static constexpr long ROCK = 2;
public:

    virtual void applyRule(Cell* cell) override {
        if (cell == nullptr || stateSet == nullptr) {
            return;
        }

        auto* enumerableStateSet = dynamic_cast<StateSet_Enumerable*>(stateSet);
        if (enumerableStateSet == nullptr || enumerableStateSet->getStatesSize() < 3) {
            cell->setNextState(cell->getCurrentState());
            return;
        }

        auto pos = cell->getPosition();
        // Cada célula possui uma preferência entre cair para esquerda ou para direita
        // Neste caso esta sendo utilizado um padrão xadrez que é visualmente agradável
        bool preferLeft = ((pos[0] + pos[1]) & 1) != 0;

        const auto& nbrs = cell->getNeighbors();
        // 0   5   10  14  19
        // 1   6   11  15  20
        // 2   7       16  21
        // 3   8   12  17  22
        // 4   9   13  18  23

        // nw2 nnw n2  nne ne2
        // wnw nw   n  ne  ene
        // w2  w       e   e2
        // wsw sw   s  se  ese
        // sw2 ssw s2  sse se2

        long nw = nbrs[6]->getCurrentState().getValue();
        long w = nbrs[7]->getCurrentState().getValue();
        long sw = nbrs[8]->getCurrentState().getValue();

        long n = nbrs[11]->getCurrentState().getValue();
        long s = nbrs[12]->getCurrentState().getValue();

        long ne = nbrs[15]->getCurrentState().getValue();
        long e = nbrs[16]->getCurrentState().getValue();
        long se = nbrs[17]->getCurrentState().getValue();

        const long wnw = nbrs[1]->getCurrentState().getValue();
        const long w2  = nbrs[2]->getCurrentState().getValue();
        const long wsw = nbrs[3]->getCurrentState().getValue();

        const long ene = nbrs[20]->getCurrentState().getValue();
        const long e2  = nbrs[21]->getCurrentState().getValue();
        const long ese = nbrs[22]->getCurrentState().getValue();

        const long currentState = cell->getCurrentState().getValue();

        // A estratétia funciona da seguinte maneira para uma célula areia:
        // 1) Se a célula de baixo (s) estiver livre, a areia cai verticalmente (vira vazio)
        // 2) Se (s) está bloqueado, e preferLeft
        //     - tenta ir para (sw).
        //     - se falhar, tenta (se).
        // 3) Analogamente, em caso de !preferLeft
        //     - tenta ir para (se).
        //     - se falhar, tenta (sw).
        // 4) Não há movimentos vaĺidos, permanece onde está
        //
        // Do ponto de vista de uma célula vazia, ela segue a mesma lógica observando os grãos de areia de cima
        // que tentariam escorregar para sua posição, e se o grão cederia ou não a preferencia.
        // Isto garante a conservação de matéria. Pelo menos nos casos que testei.
        // Um comportamento que não consegui reproduzir é o de preencher completamente um recipiente dependendo de
        // seu formato.
        switch (currentState) {
        case SAND: {
            // 1) Queda vertical tem prioridade máxima.
            if (s == EMPTY) {
                cell->setNextState(*enumerableStateSet->getState(EMPTY));
                return;
            }

            // 2) Tentativa de escorregamento diagonal.
            bool canSW = (sw == EMPTY) && (w != SAND && w != ROCK);
            bool canSE = (se == EMPTY) && (e != SAND && e != ROCK);

            if (preferLeft) {
                if (canSW) {
                    cell->setNextState(*enumerableStateSet->getState(EMPTY));
                    return;
                }
                // Fallback para SE: só escorrega para direita (se) se não ha uma areia a direita(e2) com
                // preferencia para cair em SE. Por ser preferLeft ser definido em xadrez, sei que ela
                // também é preferLeft e cederá passagem para ela.
                if (canSE && !(e2 == SAND && ese != EMPTY)) {
                    cell->setNextState(*enumerableStateSet->getState(EMPTY));
                    return;
                }
            } else {
                if (canSE) {
                    cell->setNextState(*enumerableStateSet->getState(EMPTY));
                    return;
                }
                // Fallback para SW: análogo, cedo se w2 quer meu sw.
                if (canSW && !(w2 == SAND && wsw != EMPTY)) {
                    cell->setNextState(*enumerableStateSet->getState(EMPTY));
                    return;
                }
            }

            // Não consigo me mover: permaneço SAND.
            cell->setNextState(*enumerableStateSet->getState(SAND));
            break;
        }
        case EMPTY: {
            // 1) não cai verticalmente.
            if (n == SAND) {
                cell->setNextState(*enumerableStateSet->getState(SAND));
                return;
            }

            // 2) se há rock em cima não tem como areia escorregar pra aqui
            if (n == ROCK) {
                cell->setNextState(*enumerableStateSet->getState(EMPTY));
                return;
            }

            // 3) escorregamento diagonal.
            // (Sabemos n != SAND, então diagonais não competem com queda vertical)
            bool nw_fills = false;
            if (nw == SAND && w != EMPTY) {
                if (preferLeft) {
                    // por xadrez, nw prefere seu sw(w2)
                    const bool canSW_of_nw = (w2 == EMPTY) && (wnw == EMPTY);
                    //
                    const bool fallback_safe = !(ne == SAND && e != EMPTY);

                    if (!canSW_of_nw && fallback_safe) {
                        nw_fills = true;
                    }
                } else {
                    // nw prefere seu se, direção preferida.
                    nw_fills = true;
                }
            }

            // simétrico
            bool ne_fills = false;
            if (ne == SAND && e != EMPTY) {
                if (preferLeft) {
                    ne_fills = true;
                } else {
                    const bool canSE_of_ne = (e2 == EMPTY) && (ene == EMPTY);
                    const bool fallback_safe = !(nw == SAND && w != EMPTY);
                    if (!canSE_of_ne && fallback_safe) {
                        ne_fills = true;
                    }
                }
            }

            if (nw_fills || ne_fills) {
                cell->setNextState(*enumerableStateSet->getState(SAND));
            } else {
                cell->setNextState(*enumerableStateSet->getState(EMPTY));
            }
            break;
        }
        case ROCK: {
            // objeto imóvel, sempre é pedra
            cell->setNextState(*enumerableStateSet->getState(ROCK));
            break;
        }
        default:
            cell->setNextState(cell->getCurrentState());
            break;
        }
    }
protected:
private:
};
