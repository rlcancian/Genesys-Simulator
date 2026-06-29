# Guia: Autômatos Celulares Não Uniformes no GenESyS

Branch: `dcs/nonuniform-ca`

---

## Parte 1 — Como executar os testes

### Dependências necessárias

O sistema de build usa **CMake + Ninja**. Antes de compilar pela primeira vez, verifique se as ferramentas estão instaladas:

```bash
which ninja cmake g++
```

Se `ninja` ou `g++` não estiverem presentes (Ubuntu/Debian):

```bash
sudo apt update && sudo apt install -y ninja-build g++
```

O compilador precisa suportar C++23. Verifique a versão:

```bash
g++ --version   # precisa ser 13 ou superior
```

---

### Compilar os testes

```bash
# 1. Configurar o preset (cria o diretório build/tests-kernel-unit)
cmake --preset tests-kernel-unit

# 2. Compilar o executável de testes
cmake --build build/tests-kernel-unit --target genesys_test_cellular_automata_neighborhood -- -k 0
```

O passo 1 precisa ser feito apenas uma vez (ou ao deletar o diretório `build/tests-kernel-unit`). O `-k 0` no passo 2 ignora erros em outros alvos do projeto que não afetam os testes de CA.

---

### Executar todos os testes de autômatos celulares

O executável `genesys_test_cellular_automata_neighborhood` contém todos os 24 testes de CA (8 existentes + 16 novos):

```bash
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood
```

Saída esperada (todos passando):

```
[==========] Running 24 tests from 13 test suites.
[----------] 1 test from CellularAutomataLattice
[----------] 3 tests from CellularAutomataMooreNeighborhood
[----------] 3 tests from CellularAutomataVonNeumannNeighborhood
[----------] 1 test from CellularAutomataGameOfLife
[----------] 2 tests from CellularAutomataNonUniformRule
[----------] 2 tests from CellularAutomataNonUniformNeighborhood
[----------] 2 tests from CellularAutomataNonUniform
[----------] 1 test from CellularAutomataCompDispatch
[----------] 1 test from CellularAutomataPermissiveLife
[----------] 1 test from CellularAutomataCustomRule
[----------] 2 tests from BoundaryReflexive
[----------] 2 tests from BoundaryAdiabatic
[----------] 3 tests from CellularAutomataNonUniformRegion
[==========] 24 tests ran.
[  PASSED  ] 24 tests.
```

---

### Executar subconjuntos de testes

Use o filtro `--gtest_filter` para rodar grupos específicos:

```bash
# Apenas testes de CA não uniforme (regra/vizinhança por célula)
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniform*:CellularAutomataCompDispatch*:CellularAutomataPermissiveLife*:CellularAutomataCustomRule*:CellularAutomataNonUniformRegion*"

# Apenas condições de contorno novas
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="BoundaryReflexive*:BoundaryAdiabatic*"

# Apenas API de região
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniformRegion*"
```

---

### Referência dos 16 novos testes

| Suite | Teste | O que verifica |
|---|---|---|
| `CellularAutomataNonUniformRule` | `AppliesPerCellRuleOverridingGlobalRule` | Regra por célula tem prioridade sobre a regra global |
| `CellularAutomataNonUniformRule` | `FallsBackToGlobalRuleForUnassignedCells` | Células sem override usam a regra global |
| `CellularAutomataNonUniformNeighborhood` | `AssignsCorrectNeighborCountPerCell` | Vizinhança por célula é calculada corretamente no `init()` |
| `CellularAutomataNonUniformNeighborhood` | `EvolvesCorrectlyWithMixedNeighborhoods` | Simulação roda corretamente com vizinhanças mistas |
| `CellularAutomataNonUniform` | `AppliesPerCellRuleAndNeighborhoodIndependently` | Regra e vizinhança por célula podem ser atribuídas simultaneamente |
| `CellularAutomataNonUniform` | `SameGoLRuleDifferentNeighborhoodProducesDifferentOutcome` | Mesma regra + vizinhança diferente → resultado diferente (prova central) |
| `CellularAutomataCompDispatch` | `NonUniformAcceptsPerCellApiClassicDoesNot` | `dynamic_cast` roteia corretamente a API por célula |
| `CellularAutomataPermissiveLife` | `DeadCellBornWhenNeighborCountInBirthRange` | `LocalRule_PermissiveLife` aplica condições de nascimento e sobrevivência |
| `CellularAutomataCustomRule` | `MajorityVoteConvergesUniformRegion` | `LocalRule_Custom` aplica voto por maioria com desempate correto |
| `BoundaryReflexive` | `CornerCellSeesReflectedNeighbors` | Células de canto recebem vizinhos refletidos dentro dos limites |
| `BoundaryReflexive` | `ReflectedNeighborHasSameStateAsInnerCell` | O vizinho refletido em -1 aponta para a célula 1 (reflexão correta) |
| `BoundaryAdiabatic` | `CornerCellSeesItselfForOutOfBoundsNeighbor` | Célula de borda vê a si mesma quando olha para fora |
| `BoundaryAdiabatic` | `AllDeadGridRemainsDeadAfterStep` | Sem fluxo de estado cruzando a fronteira adiabática |
| `CellularAutomataNonUniformRegion` | `SetRegionRuleAssignsRuleToAllCellsInBox` | `setRegionRule` preenche o mapa para todas as 9 células do bounding box |
| `CellularAutomataNonUniformRegion` | `SetRegionNeighborhoodAssignsCorrectNeighborCountAfterInit` | `setRegionNeighborhood` com VonNeumann afeta apenas a célula-alvo após `init()` |
| `CellularAutomataNonUniformRegion` | `CellRuleOverridesRegionRule` | `setCellRule` sobrescreve a entrada de `setRegionRule` para aquela célula |

---

## Parte 2 — Como criar uma modelagem de CA não uniforme

### Classe disponível

Uma única classe implementa todos os modos de CA não uniforme:

```
source/plugins/components/ModalModel/CellularAutomata/
└── CellularAutomata_NonUniform.h
```

| Capacidade | API |
|---|---|
| Regra por célula | `setCellRule(cellNumber, rule)` ou `setCellRule(position, rule)` |
| Vizinhança por célula | `setCellNeighborhood(cellNumber, hood)` ou `setCellNeighborhood(position, hood)` |
| Regra por região | `setRegionRule(posMin, posMax, rule)` |
| Vizinhança por região | `setRegionNeighborhood(posMin, posMax, hood)` |
| Ambos simultaneamente | Use qualquer combinação das APIs acima na mesma instância |

Células sem regra individual usam a regra global. Células sem vizinhança individual usam a vizinhança global.

---

### Padrão de construção

Todo CA no GenESyS segue a mesma sequência de configuração:

```
1. Criar o autômato (CellularAutomata_NonUniform)
2. Criar a condição de contorno (boundary)
3. Criar a vizinhança global (registra no autômato automaticamente)
4. Criar a regra local global (registra no autômato automaticamente)
5. Criar o lattice (registra no autômato automaticamente)
6. [Não uniforme] Atribuir regras/vizinhanças por célula ou por região
7. Chamar automaton.init()
8. Definir estados iniciais das células
9. Chamar automaton.step() para avançar passos
```

> **Atenção sobre efeitos colaterais**: os construtores de `Neighborhood`, `LocalRule` e `Lattice` se auto-registram no CA. Se dois objetos do mesmo tipo são criados com o mesmo CA, o **último sobrescreve o anterior** como global.

---

### Exemplo 1 — Regra não uniforme

**Cenário**: grid 1D com 5 células. Regra global: `AlwaysAlive`. Células 1 e 3: `AlwaysDead`.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"

CellularAutomata_NonUniform automaton;
Boundary_Closed boundary;
Neighborhood_Center neighborhood(&automaton, 1, &boundary);
LocalRule_AlwaysDead  deadRule(&automaton);   // registra primeiro
LocalRule_AlwaysAlive aliveRule(&automaton);  // registra por último → global = aliveRule
Lattice lattice(&automaton, nullptr, {5});

automaton.setCellRule(1, &deadRule);
automaton.setCellRule(3, &deadRule);

automaton.init();
automaton.step();
// Resultado: [1, 0, 1, 0, 1]
```

---

### Exemplo 2 — Vizinhança não uniforme

**Cenário**: grid 2D 5×5, Game of Life global, Moore global. A célula `{2,2}` usa VonNeumann.

```cpp
CellularAutomata_NonUniform automaton;
Boundary_Closed boundary;
Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);  // global: Moore
LocalRule_GameOfLife rule(&automaton);
Lattice lattice(&automaton, nullptr, {5, 5});

// registerWithCA=false evita sobrescrever a vizinhança global
Neighborhood_VonNeumann vonNeumannCenter(&automaton, 1, &boundary, false, /*registerWithCA=*/false);
automaton.setCellNeighborhood({2, 2}, &vonNeumannCenter);

automaton.init();
// lattice.getCell({2,2})->getNeighbors().size() == 4  (VonNeumann)
// lattice.getCell({1,1})->getNeighbors().size() == 8  (Moore)
```

---

### Exemplo 3 — Combinado (regra + vizinhança por célula)

**Cenário**: grid 2D 7×7. Célula `{4,4}` usa VonNeumann (sem `setCellRule` → mantém GoL global). Prova que a mesma regra produz resultados diferentes conforme a vizinhança.

```cpp
CellularAutomata_NonUniform automaton;
Boundary_Fixed boundary;
Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
LocalRule_GameOfLife gol(&automaton);
Lattice lattice(&automaton, nullptr, {7, 7});

Neighborhood_VonNeumann vnForCenter(&automaton, 1, &boundary, false, false);
automaton.setCellNeighborhood({4, 4}, &vnForCenter);

automaton.init();

// Semear três vizinhos diagonais de {4,4} (invisíveis para VonNeumann)
lattice.getCell({3, 3})->setCurrentState(State(1));
lattice.getCell({5, 3})->setCurrentState(State(1));
lattice.getCell({3, 5})->setCurrentState(State(1));

// Semear três vizinhos diagonais de {1,1} (visíveis para Moore)
lattice.getCell({0, 0})->setCurrentState(State(1));
lattice.getCell({2, 0})->setCurrentState(State(1));
lattice.getCell({0, 2})->setCurrentState(State(1));

automaton.step();
// {1,1} = 1 (Moore vê 3 diagonais → nasce)
// {4,4} = 0 (VonNeumann não vê diagonais → não nasce)
```

---

### Exemplo 4 — Zonas ecológicas com API de região

**Cenário**: grid 20×20 dividido em `x=10`. Zona esquerda: Moore + GoL. Zona direita: VonNeumann + `PermissiveLife`. Usa `setRegionRule` e `setRegionNeighborhood` em vez de laços manuais.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Adiabatic.h"

const int cols = 20, rows = 20;

CellularAutomata_NonUniform automaton;
Boundary_Adiabatic boundary;                               // sem fluxo nas bordas externas
Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
LocalRule_PermissiveLife permissiveRule(&automaton);       // registra primeiro
LocalRule_GameOfLife golRule(&automaton);                  // registra por último → global = GoL
Lattice lattice(&automaton, nullptr, {cols, rows});

// Um único objeto VonNeumann compartilhado pela zona direita inteira
Neighborhood_VonNeumann vnRight(&automaton, 1, &boundary, false, /*registerWithCA=*/false);

// Atribuir vizinhança e regra para a zona direita com duas chamadas
automaton.setRegionNeighborhood({10, 0}, {19, 19}, &vnRight);
automaton.setRegionRule({10, 0}, {19, 19}, &permissiveRule);

automaton.init();

// Semear padrões...
for (int step = 0; step < 10; ++step)
    automaton.step();
```

> **Comparação com o exemplo anterior**: a API de região substitui os laços `for (x...) for (y...)` com `new Neighborhood_VonNeumann(...)` individuais. O compartilhamento de um único objeto de vizinhança é seguro aqui porque `setRegionNeighborhood` armazena ponteiros — todos os ponteiros apontam para o mesmo objeto, e o objeto VonNeumann não guarda estado por célula.

---

### Exemplo 5 — Condição de contorno reflexiva

**Cenário**: grade 1D onde as ondas "ricocheteiam" nas bordas em vez de envolver (Closed) ou desaparecer (Fixed).

```cpp
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Reflexive.h"

CellularAutomata_Classic automaton;
Boundary_Reflexive boundary;
Neighborhood_Center neighborhood(&automaton, 1, &boundary);
LocalRule_Elementary elementaryRule(&automaton, 30);
Lattice lattice(&automaton, nullptr, {20});

automaton.init();
lattice.getCell(10)->setCurrentState(State(1));   // semente central

for (int step = 0; step < 15; ++step)
    automaton.step();
// Ao atingir a borda, o padrão é refletido de volta para o interior
```

---

### Condições de contorno disponíveis

| Classe | Comportamento fora dos limites | Uso típico |
|--------|-------------------------------|------------|
| `Boundary_Fixed` | Retorna célula fantasma com estado fixo (padrão: 0) | CA elementar, GoL padrão |
| `Boundary_Closed` | Wrapping periódico (toroidal) | Grade toroidal, testes de blinker |
| `Boundary_Reflexive` | Reflete a coordenada de volta para dentro | Ondas que ricocheteiam nas bordas |
| `Boundary_Adiabatic` | Clamps para a borda mais próxima (célula de borda vê a si mesma) | Zonas com fronteiras isolantes |

Todas implementam `BoundaryCondition` e podem ser usadas com qualquer tipo de CA.

---

### API de região — detalhes

`setRegionRule` e `setRegionNeighborhood` recebem dois vetores `posMin` e `posMax` que definem o bounding box (inclusivo em todas as dimensões) e iteram todas as posições inteiras dentro dele:

```cpp
// Atribuir regra a um retângulo 2D
automaton.setRegionRule({x0, y0}, {x1, y1}, &rule);

// Atribuir vizinhança a um cubo 3D
automaton.setRegionNeighborhood({x0, y0, z0}, {x1, y1, z1}, &hood);
```

**Regras de precedência:**
- `setCellRule` / `setCellNeighborhood` chamados **depois** de `setRegionRule` / `setRegionNeighborhood` sobrescrevem a entrada daquela célula específica.
- A chamada mais recente sempre vence, pois ambas escrevem no mesmo `unordered_map`.

**Requisito:** o lattice precisa estar criado (e associado ao autômato) antes de `setRegionRule` / `setRegionNeighborhood`, pois a conversão de posição para índice linear usa `lattice->cellNDimPosition2Number()`.

---

### Como definir uma regra local customizada

```cpp
class MinhaRegra : public LocalRule {
public:
    MinhaRegra(CellularAutomataBase* ca) : LocalRule(ca) {}
    void applyRule(Cell* cell) override {
        const std::vector<Cell*>& neighbors = cell->getNeighbors();
        long proximoEstado = /* lógica customizada */;
        cell->setNextState(State(proximoEstado));
    }
};
```

> **Regra de ouro**: leia `getCurrentState()`, escreva `setNextState()`. Nunca modifique `currentState` diretamente.

---

### Regras incluídas no trabalho

**`LocalRule_PermissiveLife`** (`LocalRule_PermissiveLife.h`): regra configurável de sobrevivência/nascimento para estados binários.

```cpp
// Parâmetros padrão: sobrevive com 1-4 vizinhos vivos, nasce com 2-3
LocalRule_PermissiveLife regra(&automaton);

// Parâmetros equivalentes ao Game of Life:
LocalRule_PermissiveLife golEquivalente(&automaton, /*surviveMin=*/2, /*surviveMax=*/3,
                                                     /*birthMin=*/3,   /*birthMax=*/3);
```

**`LocalRule_Custom`** (`LocalRule_Custom.h`): voto por maioria. Em empate, o menor valor de estado vence.

```cpp
LocalRule_Custom regra(&automaton);
```

---

### Localização dos arquivos de implementação

```
source/plugins/components/ModalModel/CellularAutomata/
├── CellularAutomata_NonUniform.h             ← única classe (regra + vizinhança por célula e por região)
├── Boundary_Reflexive.h                      ← condição de contorno reflexiva (espelho)
├── Boundary_Adiabatic.h                      ← condição de contorno adiabática (sem fluxo)
├── LocalRule_Custom.h                        ← voto por maioria
├── LocalRule_PermissiveLife.h                ← sobrevivência/nascimento configurável
└── Neighborhood.h                            ← parâmetro registerWithCA adicionado aqui

source/applications/terminal/examples/smarts/
├── Smart_CellularAutomata_NonUniform.h
└── Smart_CellularAutomata_NonUniform.cpp     ← 4 cenários de demonstração completos

source/tests/unit/
└── test_cellular_automata_neighborhood.cpp   ← 24 testes (8 existentes + 16 novos)
```
