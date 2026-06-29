# Arquitetura Existente: Autômatos Celulares no GenESyS

## 1. Visão Geral

O suporte a autômatos celulares no GenESyS está organizado em dois níveis:

- **Nível de componente de simulação**: classe `CellularAutomataComp`, em `source/plugins/components/ModalModel/`, que integra o autômato ao kernel do GenESyS como um `ModelComponent`.
- **Nível de framework interno**: hierarquia de classes em `source/plugins/components/ModalModel/CellularAutomata/`, que implementa a lógica de simulação do autômato em si.

---

## 2. Componente de Simulação: `CellularAutomataComp`

### Localização
```
source/plugins/components/ModalModel/CellularAutomataComp.h
source/plugins/components/ModalModel/CellularAutomataComp.cpp
```

### Papel
É o ponto de integração entre o kernel do GenESyS e o framework de autômatos celulares. Herda de `ModelComponent` e define enums que descrevem as dimensões configuráveis do autômato.

### Enums de configuração

| Enum | Valores |
|---|---|
| `CellularAutomataType` | CLASSIC, TIMED_1D, ASYNCHRONOUS, NONUNIFORMRULE, NONUNIFORMNEIGHBOOR, NONUNIFORM, USERDEFINED |
| `LatticeType` | RETICULAR, TRIANGULAR, HEXAGONAL, NETWORK, USERDEFINED |
| `NeighboorhoodType` | CENTERED, BACKWARD, FORWARD, VONNEUMANN, MOORE, USERDEFINED |
| `BoundaryType` | FIXED, CLOSED, REFLEXIVE, ADIABATIC, USERDEFINED |
| `StateSetType` | ENUMERATED, INTEGERBASED, BITBASED, DOUBLEBASED, USERDEFINED |
| `LocalRuleType` | ELEMENTAR_CA, GAME_OF_LIFE, BIASED_COMPETITION, HPP, USERDEFINED |

### Atributos privados (persistentes)
```cpp
CellularAutomataType _cellularAutomataType  // padrão: CLASSIC
LatticeType          _latticeType           // padrão: RETICULAR
NeighboorhoodType    _neighboorhoodType     // padrão: VONNEUMANN
BoundaryType         _boundaryType          // padrão: FIXED
StateSetType         _stateSetType          // padrão: ENUMERATED
LocalRuleType        _localRuleType         // padrão: GAME_OF_LIFE
```

### Atributos privados (não persistentes — objetos internos)
```cpp
CellularAutomataBase* _cellularAutomata   // instância do autômato
Lattice*              _lattice            // malha de células
Neighborhood*         _neighboorhood      // vizinhança global
BoundaryCondition*    _boundary           // condição de contorno
StateSet*             _stateSet           // conjunto de estados
LocalRule*            _localRule          // regra local global
```

### Padrão fábrica (factory) nos setters
Cada `setXxxType()` instancia o objeto concreto correspondente:
```
setCellularAutomataType(CLASSIC)   → new CellularAutomata_Classic()
setNeighboorhoodType(MOORE)        → new Neighborhood_Moore(_cellularAutomata)
setBoundaryType(FIXED)             → new Boundary_Fixed()
setLocalRuleType(GAME_OF_LIFE)     → new LocalRule_GameOfLife(_cellularAutomata)
```

### Métodos de ciclo de vida
- `_check()`: conecta os objetos internos entre si (setLattice, setLocalRule, setBoundary etc.) — **sem validação real ainda**.
- `_initBetweenReplications()`: chama `_cellularAutomata->init()`, que inicializa o lattice e pré-calcula vizinhanças.
- `_onDispatchEvent()`: chama `_cellularAutomata->step()` a cada entidade recebida.
- `_loadInstance()` / `_saveInstance()`: **não implementados** (marcados como @TODO).

---

## 3. Framework Interno: `CellularAutomata/`

### 3.1 Hierarquia de classes

```
CellularAutomataBase (abstrata)
├── CellularAutomata_Classic                   ← CA uniforme síncrono
├── CellularAutomata_1DTimed                   ← CA 1D onde 2ª dimensão é o tempo
└── CellularAutomata_NonUniformNeighborhood    ← vizinhança por célula (sobrecarrega init)
    ├── CellularAutomata_NonUniform            ← regra + vizinhança por célula combinadas
    └── (CellularAutomata_NonUniformRule herda de CellularAutomata_Classic
        com apenas regra por célula)

Lattice                                ← malha n-dimensional
Cell                                   ← célula individual
State                                  ← estado (baseado em long)
StateSet                               ← conjunto de estados (base)
└── StateSet_Enumerable

Neighborhood (abstrata)
├── Neighborhood_Center                ← 1D centrada
├── Neighborhood_Moore                 ← n-D, distância Chebyshev ≤ raio
└── Neighborhood_VonNeumann            ← n-D, distância Manhattan ≤ raio

BoundaryCondition (abstrata)
├── Boundary_Fixed                     ← célula fantasma fixa fora dos limites
├── Boundary_Closed                    ← envoltório toroidal (módulo)
├── Boundary_Reflexive                 ← reflexão especular: posição inválida é espelhada de volta
└── Boundary_Adiabatic                 ← sem fluxo: clamp na célula de borda (célula vê a si mesma)

LocalRule (abstrata, pure virtual)
├── LocalRule_Elementary               ← CA elementar de Wolfram (número de regra 0–255)
├── LocalRule_GameOfLife               ← Jogo da Vida de Conway
├── LocalRule_Growty                   ← competição tendenciosa
├── LocalRule_FlorestalFire            ← incêndio florestal
├── LocalRule_HppLatticeGas            ← gás HPP
├── LocalRule_ShowNeighbordhood        ← debug: exibe vizinhos
├── LocalRule_PermissiveLife           ← sobrevivência/nascimento com intervalos configuráveis
└── LocalRule_Custom                   ← exemplo de extensão: voto por maioria
```

### 3.2 Fluxo de dados entre classes

```
CellularAutomataBase
  └─ Lattice            (contém vetor de Cell*)
  └─ Neighborhood       (computa vizinhos no init)
       └─ BoundaryCondition  (resolve vizinhos fora dos limites)
            └─ Lattice  (para mapear posição → célula)
  └─ LocalRule          (aplica regra durante step)
       └─ StateSet       (conjunto de estados válidos)
```

**Importante**: há efeitos colaterais nos construtores. Quando um objeto é criado com `parentCellularAutomata`:
- `Neighborhood(ca, ...)` → chama `ca->setNeighborhood(this)`
- `LocalRule(ca, ...)` → chama `ca->setLocalRule(this)`
- `Lattice(ca, ...)` → chama `ca->setLattice(this)`

Ou seja, apenas criar o objeto já o registra no CA. **Se múltiplos objetos do mesmo tipo são criados com o mesmo CA, o último sobrescreve o anterior.**

### 3.3 Classe `Lattice`

- Malha n-dimensional (dimensões armazenadas em `std::vector<unsigned short>`).
- Mapeamento bidirecional: `cellNDimPosition2Number()` e `cellNumber2NDimPosition()` usando produto de dimensões.
- `init()`:
  1. Cria todas as células (copia do `progenitorCell` se definido).
  2. Define a posição n-dim de cada célula.
  3. Para cada célula, chama `neighborhood->getNeighbors(cell)` e armazena o resultado em `cell->setNeighbors(...)`.
  4. **Os vizinhos são pré-computados e fixos após o init.**

### 3.4 Classe `Cell`

Armazena:
- `cellNum`: posição linear no vetor do lattice.
- `position`: coordenadas n-dimensionais.
- `previousState`, `currentState`, `nextState`: estados `long`-based.
- `neighbors`: vetor de `Cell*` pré-computado no init.
- `updatePending`: flag de atualização pendente.

### 3.5 Classe `CellularAutomataBase`

- Método `init()`: delega para `lattice->init()`.
- Método `step()`: chama o método abstrato `applyLocalRule()`.
- Método abstrato `applyLocalRule()`: implementado pelas subclasses.

### 3.6 `CellularAutomata_Classic`

Implementação padrão (síncrona uniforme):
```
applyLocalRule():
  1. Para cada célula: localRule->applyRule(cell)  ← calcula nextState
  2. Para cada célula: cell->updateState()          ← currentState = nextState
```

A segunda fase garante que todas as células vejam o estado anterior durante o cálculo.

### 3.7 Classes `Neighborhood`

Vizinhanças são implementadas com recursão n-dimensional:
- `Neighborhood_Moore`: offset em cada dimensão varia de −raio a +raio (exceto offset zero).
- `Neighborhood_VonNeumann`: igual à Moore, mas filtra offsets com distância Manhattan > raio.

Ambas foram **generalizadas para n dimensões e raio r** no branch atual (`dcs/nonuniform-ca`).

### 3.8 `LocalRule::applyRule(Cell* cell)`

Recebe uma célula, lê `cell->getNeighbors()` (já pré-computados), e define `cell->setNextState(...)`.

### 3.9 Classe `BoundaryCondition`

- `Boundary_Fixed`: retorna uma célula fantasma com número −99 e estado fixo (padrão 0) para posições fora dos limites.
- `Boundary_Closed`: faz o envoltório toroidal usando módulo por dimensão.
- `Boundary_Reflexive`: reflete posições inválidas de volta ao grid por espelhamento iterativo. Para cada dimensão: `if (pos < 0) pos = -pos; if (pos >= D) pos = 2*(D-1)-pos;` repetido até a posição ser válida. A borda é um "espelho".
- `Boundary_Adiabatic`: sem fluxo de informação para fora. Usa `std::clamp` para travar coordenadas inválidas na borda mais próxima. Células de borda "veem a si mesmas" quando olham para fora do grid.

---

## 4. Fluxo de execução completo

```
1. Usuário chama setCellularAutomataType/setLatticeType/etc.
   → Objetos concretos são instanciados (factory).

2. ModelChecker chama _check()
   → Objetos são ligados uns aos outros.

3. ModelSimulation chama _initBetweenReplications()
   → _cellularAutomata->init()
     → lattice->init()
       → cria células
       → para cada célula: neighborhood->getNeighbors(cell) → cell->setNeighbors(...)

4. Por cada entidade recebida: _onDispatchEvent()
   → _cellularAutomata->step()
     → applyLocalRule()
       → para cada célula: localRule->applyRule(cell)   (calcula nextState)
       → para cada célula: cell->updateState()           (confirma nextState)
```

---

## 5. Limitações atuais

| Problema | Onde |
|---|---|
| `_loadInstance` e `_saveInstance` implementados apenas para os 6 campos enum | `CellularAutomataComp.cpp` |
| `_check()` valida ponteiros nulos mas não verifica coerência semântica entre tipos | `CellularAutomataComp.cpp` |
| `~CellularAutomataBase()` chamado diretamente em vez de `delete` | `CellularAutomataComp.cpp` |
| Vizinhanças por célula são estáticas após `init()` — mudar vizinhança durante a simulação requer redesenho do `Lattice` | arquitetura atual |
| `Neighborhood` se auto-registra no CA no construtor — vizinhanças por célula devem ser criadas com `registerWithCA=false` | `Neighborhood.h` |
| API de região (`setRegionRule`, `setRegionNeighborhood`) disponível apenas em `CellularAutomata_NonUniform` | `CellularAutomata_NonUniform.h` |
