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

# 2. Compilar todos os testes sem executar nenhum
cmake --build build/tests-kernel-unit
```

O passo 1 precisa ser feito apenas uma vez. O passo 2 pode ser repetido sempre que o código mudar.

---

### Executar todos os testes de autômatos celulares

O executável `genesys_test_cellular_automata_neighborhood` contém todos os 17 testes de CA (8 existentes + 9 novos de CA não uniforme):

```bash
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood
```

Saída esperada (todos passando):

```
[==========] Running 17 tests from 9 test suites.
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
[==========] 17 tests ran.
[  PASSED  ] 17 tests.
```

---

### Executar apenas os testes de CA não uniforme

Use o filtro `--gtest_filter` para rodar somente os testes adicionados neste trabalho:

```bash
# Todos os testes não uniformes de uma vez
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniform*:CellularAutomataCompDispatch*:CellularAutomataPermissiveLife*:CellularAutomataCustomRule*"

# Apenas testes de regra não uniforme
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniformRule*"

# Apenas testes de vizinhança não uniforme
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniformNeighborhood*"

# Apenas testes combinados (regra + vizinhança simultâneas)
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniform/*"
```

---

### Referência dos 9 novos testes

Todos os testes não uniformes usam `CellularAutomata_NonUniform` — a única classe implementada.

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
| Ambos simultaneamente | Use ambas as APIs na mesma instância |

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
6. [Não uniforme] Atribuir regras/vizinhanças por célula
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

### Exemplo 4 — Zonas ecológicas

**Cenário**: grid 20×20 dividido em `x=10`. Zona esquerda: Moore + GoL. Zona direita: VonNeumann + `PermissiveLife`.

```cpp
const int cols = 20, rows = 20, zoneX = 10;

CellularAutomata_NonUniform automaton;
Boundary_Closed boundary;
Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
LocalRule_PermissiveLife permissiveRule(&automaton);  // registra primeiro
LocalRule_GameOfLife golRule(&automaton);              // registra por último → global = GoL
Lattice lattice(&automaton, nullptr, {cols, rows});

std::vector<Neighborhood_VonNeumann*> rightNeighborhoods;
for (int y = 0; y < rows; ++y) {
    for (int x = zoneX; x < cols; ++x) {
        auto* vn = new Neighborhood_VonNeumann(&automaton, 1, &boundary, false, false);
        rightNeighborhoods.push_back(vn);
        automaton.setCellNeighborhood({x, y}, vn);
        automaton.setCellRule({x, y}, &permissiveRule);
    }
}

automaton.init();

// Glider GoL na zona esquerda
lattice.getCell({6, 7})->setCurrentState(State(1));
// ...

// Forma em L na zona direita (morre sob GoL, cresce sob PermissiveLife)
lattice.getCell({13, 8})->setCurrentState(State(1));
// ...

for (int step = 0; step < 10; ++step)
    automaton.step();

for (auto* vn : rightNeighborhoods)
    delete vn;
```

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
├── CellularAutomata_NonUniform.h             ← única classe (regra + vizinhança por célula)
├── LocalRule_Custom.h                        ← voto por maioria
├── LocalRule_PermissiveLife.h                ← sobrevivência/nascimento configurável
└── Neighborhood.h                            ← parâmetro registerWithCA adicionado aqui

source/applications/terminal/examples/smarts/
├── Smart_CellularAutomata_NonUniform.h
└── Smart_CellularAutomata_NonUniform.cpp     ← 4 cenários de demonstração completos

source/tests/unit/
└── test_cellular_automata_neighborhood.cpp   ← 17 testes (8 existentes + 9 novos)
```
