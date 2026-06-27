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
[==========] Running 17 tests from 8 test suites.
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

# Um teste específico pelo nome completo
./build/tests-kernel-unit/source/tests/unit/genesys_test_cellular_automata_neighborhood \
  --gtest_filter="CellularAutomataNonUniform/SameGoLRuleDifferentNeighborhoodProducesDifferentOutcome"
```

O padrão do filtro é `NomeDaSuite/NomeDoTeste`. O `*` é curinga. O `:` separa múltiplos filtros.

---

### Referência dos 9 novos testes

| Suite | Teste | O que verifica |
|---|---|---|
| `CellularAutomataNonUniformRule` | `AppliesPerCellRuleOverridingGlobalRule` | Regra por célula tem prioridade sobre a regra global |
| `CellularAutomataNonUniformRule` | `FallsBackToGlobalRuleForUnassignedCells` | Células sem override usam a regra global |
| `CellularAutomataNonUniformNeighborhood` | `AssignsCorrectNeighborCountPerCell` | Vizinhança por célula é calculada corretamente no `init()` |
| `CellularAutomataNonUniformNeighborhood` | `EvolvesCorrectlyWithMixedNeighborhoods` | Simulação roda corretamente com vizinhanças mistas |
| `CellularAutomataNonUniform` | `AppliesPerCellRuleAndNeighborhoodIndependently` | Regra e vizinhança por célula podem ser atribuídas simultaneamente |
| `CellularAutomataNonUniform` | `SameGoLRuleDifferentNeighborhoodProducesDifferentOutcome` | Mesma regra + vizinhança diferente → resultado diferente (prova central) |
| `CellularAutomataCompDispatch` | `ClassHierarchyEnablesPerCellApiForNonUniformTypesOnly` | `dynamic_cast` roteia corretamente a API por célula |
| `CellularAutomataPermissiveLife` | `DeadCellBornWhenNeighborCountInBirthRange` | `LocalRule_PermissiveLife` aplica condições de nascimento e sobrevivência |
| `CellularAutomataCustomRule` | `MajorityVoteConvergesUniformRegion` | `LocalRule_Custom` aplica voto por maioria com desempate correto |

---

---

## Parte 2 — Como criar uma modelagem de CA não uniforme

### Classes disponíveis

Três novas classes permitem modelar CA não uniformes, localizadas em:

```
source/plugins/components/ModalModel/CellularAutomata/
```

| Classe | Arquivo | Quando usar |
|---|---|---|
| `CellularAutomata_NonUniformRule` | `CellularAutomata_NonUniformRule.h` | Células com regras locais diferentes, mesma vizinhança |
| `CellularAutomata_NonUniformNeighborhood` | `CellularAutomata_NonUniformNeighborhood.h` | Células com vizinhanças diferentes, mesma regra |
| `CellularAutomata_NonUniform` | `CellularAutomata_NonUniform.h` | Células com regra **e** vizinhança diferentes simultaneamente |

---

### Padrão de construção

Todo CA no GenESyS segue a mesma sequência de configuração:

```
1. Criar o autômato
2. Criar a condição de contorno (boundary)
3. Criar a vizinhança global (registra no autômato automaticamente)
4. Criar a regra local global (registra no autômato automaticamente)
5. Criar o lattice (registra no autômato automaticamente)
6. [Não uniforme] Atribuir regras/vizinhanças por célula
7. Chamar automaton.init()
8. Definir estados iniciais das células
9. Chamar automaton.step() para avançar passos
```

> **Atenção sobre efeitos colaterais**: os construtores de `Neighborhood`, `LocalRule` e `Lattice` se auto-registram no CA ao serem criados com um ponteiro para ele. Se dois objetos do mesmo tipo são criados com o mesmo CA, o **último sobrescreve o anterior** como global. Isso é usado intencionalmente para definir a regra/vizinhança global: crie primeiro os auxiliares (que serão sobrescritos) e por último o que deve ser o global.

---

### Exemplo 1 — Regra não uniforme (`CellularAutomata_NonUniformRule`)

**Cenário**: grid 1D com 5 células. Regra global: `LocalRule_AlwaysAlive`. Células 1 e 3: `LocalRule_AlwaysDead`.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Center.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"

// Regras auxiliares (definidas inline para o exemplo)
class LocalRule_AlwaysDead : public LocalRule {
public:
    LocalRule_AlwaysDead(CellularAutomataBase* ca) : LocalRule(ca) {}
    void applyRule(Cell* cell) override { cell->setNextState(State(0)); }
};

class LocalRule_AlwaysAlive : public LocalRule {
public:
    LocalRule_AlwaysAlive(CellularAutomataBase* ca) : LocalRule(ca) {}
    void applyRule(Cell* cell) override { cell->setNextState(State(1)); }
};

// --- Configuração ---
CellularAutomata_NonUniformRule automaton;
Boundary_Closed boundary;
Neighborhood_Center neighborhood(&automaton, 1, &boundary);

// Ordem importa: deadRule registra primeiro, aliveRule registra por último → global = aliveRule
LocalRule_AlwaysDead  deadRule(&automaton);
LocalRule_AlwaysAlive aliveRule(&automaton);

Lattice lattice(&automaton, nullptr, {5});  // grid 1D com 5 células

// Atribuir regra específica por número de célula
automaton.setCellRule(1, &deadRule);
automaton.setCellRule(3, &deadRule);

// Ou por posição n-dimensional (equivalente para 1D)
// automaton.setCellRule({1}, &deadRule);
// automaton.setCellRule({3}, &deadRule);

automaton.init();  // inicializa o lattice e pré-calcula vizinhanças

// Estados iniciais (padrão = 0)
// automaton.step() aplica as regras
automaton.step();

// Resultado esperado: [1, 0, 1, 0, 1]
// Células 0, 2, 4 → aliveRule (global) → estado 1
// Células 1, 3   → deadRule (específica) → estado 0
long c0 = lattice.getCell(0)->getCurrentState().getValue();  // 1
long c1 = lattice.getCell(1)->getCurrentState().getValue();  // 0
```

---

### Exemplo 2 — Vizinhança não uniforme (`CellularAutomata_NonUniformNeighborhood`)

**Cenário**: grid 2D 5×5, Game of Life, vizinhança global Moore. A célula `{2,2}` usa VonNeumann.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniformNeighborhood.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"

CellularAutomata_NonUniformNeighborhood automaton;
Boundary_Closed boundary;
Neighborhood_Moore globalNeighborhood(&automaton, 1, &boundary);  // vizinhança global: Moore
LocalRule_GameOfLife rule(&automaton);
Lattice lattice(&automaton, nullptr, {5, 5});

// Vizinhança por célula: registerWithCA=false evita sobrescrever a vizinhança global
Neighborhood_VonNeumann vonNeumannCenter(&automaton, 1, &boundary, false, /*registerWithCA=*/false);
automaton.setCellNeighborhood({2, 2}, &vonNeumannCenter);

// Ou por número de célula (linha 2, coluna 2 → número 12 em um grid 5×5)
// automaton.setCellNeighborhood(12, &vonNeumannCenter);

automaton.init();
// Após init(), a célula {2,2} tem 4 vizinhos (VonNeumann)
// Demais células interiores têm 8 vizinhos (Moore)

// Verificação:
// lattice.getCell({2,2})->getNeighbors().size() == 4
// lattice.getCell({1,1})->getNeighbors().size() == 8
```

> **Importante**: vizinhanças por célula devem ser criadas com `registerWithCA=false` (último parâmetro do construtor). Sem isso, a última vizinhança criada sobrescreveria a vizinhança global do CA.

---

### Exemplo 3 — Combinado (`CellularAutomata_NonUniform`)

**Cenário**: grid 2D 7×7, Game of Life + Moore como padrão. A célula `{4,4}` usa VonNeumann e mantém GoL. Demonstra que a mesma regra produz resultados diferentes conforme a vizinhança.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Fixed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"

CellularAutomata_NonUniform automaton;
Boundary_Fixed boundary;
Neighborhood_Moore globalMoore(&automaton, 1, &boundary);  // global: Moore
LocalRule_GameOfLife gol(&automaton);                       // global: GoL
Lattice lattice(&automaton, nullptr, {7, 7});

// Célula {4,4} recebe VonNeumann (não registra no CA global)
Neighborhood_VonNeumann vnForCenter(&automaton, 1, &boundary, false, /*registerWithCA=*/false);
automaton.setCellNeighborhood({4, 4}, &vnForCenter);

// Regra para {4,4} continua sendo GoL (nenhum setCellRule chamado → usa global)
// Para atribuir regra diferente:
// LocalRule_Custom custom(&automaton);
// automaton.setCellRule({4, 4}, &custom);

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

// {1,1} nasce: Moore vê 3 vizinhos diagonais vivos → GoL: nasce
// {4,4} fica morta: VonNeumann não vê diagonais → 0 vizinhos vivos → GoL: não nasce
long cell11 = lattice.getCell({1, 1})->getCurrentState().getValue();  // 1
long cell44 = lattice.getCell({4, 4})->getCurrentState().getValue();  // 0
```

---

### Exemplo 4 — Zonas ecológicas (cenário completo)

**Cenário**: grid 20×20 dividido em duas zonas pela linha `x=10`. Zona esquerda usa Moore + Game of Life. Zona direita usa VonNeumann + `LocalRule_PermissiveLife`. Demonstra o comportamento emergente na fronteira.

```cpp
#include "plugins/components/ModalModel/CellularAutomata/CellularAutomata_NonUniform.h"
#include "plugins/components/ModalModel/CellularAutomata/Lattice.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_Moore.h"
#include "plugins/components/ModalModel/CellularAutomata/Neighborhood_VonNeumann.h"
#include "plugins/components/ModalModel/CellularAutomata/Boundary_Closed.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_GameOfLife.h"
#include "plugins/components/ModalModel/CellularAutomata/LocalRule_PermissiveLife.h"
#include <vector>

const int cols = 20, rows = 20, zoneX = 10;

CellularAutomata_NonUniform automaton;
Boundary_Closed boundary;
Neighborhood_Moore globalMoore(&automaton, 1, &boundary);
// Ordem: PermissiveLife registra primeiro, GoL registra por último → global = GoL (fallback da zona esquerda)
LocalRule_PermissiveLife permissiveRule(&automaton);
LocalRule_GameOfLife golRule(&automaton);
Lattice lattice(&automaton, nullptr, {cols, rows});

// Zona direita: cada célula recebe VonNeumann + PermissiveLife
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

// Zona esquerda: glider GoL próximo à fronteira
lattice.getCell({6, 7})->setCurrentState(State(1));
lattice.getCell({7, 8})->setCurrentState(State(1));
lattice.getCell({5, 9})->setCurrentState(State(1));
lattice.getCell({6, 9})->setCurrentState(State(1));
lattice.getCell({7, 9})->setCurrentState(State(1));

// Zona direita: forma em L que cresce sob PermissiveLife
lattice.getCell({13, 8})->setCurrentState(State(1));
lattice.getCell({14, 8})->setCurrentState(State(1));
lattice.getCell({13, 9})->setCurrentState(State(1));

for (int step = 0; step < 10; ++step)
    automaton.step();

// Liberar memória das vizinhanças alocadas dinamicamente
for (auto* vn : rightNeighborhoods)
    delete vn;
```

> **Gerenciamento de memória**: vizinhanças criadas com `new` para células individuais precisam ser liberadas manualmente. Vizinhanças criadas como variáveis locais (na pilha) não precisam.

---

### Como definir uma regra local customizada

Para criar uma regra própria, herde de `LocalRule` e implemente `applyRule(Cell*)`:

```cpp
#include "plugins/components/ModalModel/CellularAutomata/LocalRule.h"
#include "plugins/components/ModalModel/CellularAutomata/Cell.h"
#include "plugins/components/ModalModel/CellularAutomata/State.h"

class MinhaRegra : public LocalRule {
public:
    MinhaRegra(CellularAutomataBase* ca) : LocalRule(ca) {}

    void applyRule(Cell* cell) override {
        // Leitura dos vizinhos (pré-computados no init)
        const std::vector<Cell*>& neighbors = cell->getNeighbors();

        // Leitura do estado atual da própria célula
        long estadoAtual = cell->getCurrentState().getValue();

        // Cálculo do próximo estado (qualquer lógica)
        long proximoEstado = /* ... */;

        // OBRIGATÓRIO: definir o próximo estado (não currentState diretamente)
        cell->setNextState(State(proximoEstado));
    }
};
```

> **Regra de ouro**: dentro de `applyRule`, leia sempre `getCurrentState()` e escreva sempre `setNextState()`. Nunca modifique `currentState` diretamente — a atualização sincronizada (`updateState()`) é feita pelo CA após todas as células serem processadas.

---

### Regras incluídas no trabalho

Além das regras existentes no framework, duas novas regras foram adicionadas:

**`LocalRule_PermissiveLife`** (`LocalRule_PermissiveLife.h`): regra configurável de sobrevivência/nascimento para estados binários.

```cpp
// Parâmetros padrão: sobrevive com 1-4 vizinhos vivos, nasce com 2-3
LocalRule_PermissiveLife regra(&automaton);

// Parâmetros customizados (equivale ao Game of Life):
LocalRule_PermissiveLife golEquivalente(&automaton, /*surviveMin=*/2, /*surviveMax=*/3,
                                                     /*birthMin=*/3,   /*birthMax=*/3);
```

**`LocalRule_Custom`** (`LocalRule_Custom.h`): voto por maioria entre os vizinhos. Cada célula assume o estado mais frequente entre seus vizinhos. Em empate, o menor valor de estado vence.

```cpp
LocalRule_Custom regra(&automaton);
```

---

### Localização dos arquivos de implementação

```
source/plugins/components/ModalModel/CellularAutomata/
├── CellularAutomata_NonUniform.h             ← regra + vizinhança por célula
├── CellularAutomata_NonUniformNeighborhood.h ← vizinhança por célula
├── CellularAutomata_NonUniformRule.h         ← regra por célula
├── LocalRule_Custom.h                        ← voto por maioria (exemplo de regra customizada)
├── LocalRule_PermissiveLife.h                ← sobrevivência/nascimento configurável
└── Neighborhood.h                            ← parâmetro registerWithCA adicionado aqui

source/applications/terminal/examples/smarts/
├── Smart_CellularAutomata_NonUniform.h
└── Smart_CellularAutomata_NonUniform.cpp     ← 4 cenários de demonstração completos

source/tests/unit/
└── test_cellular_automata_neighborhood.cpp   ← 17 testes (8 existentes + 9 novos)
```
