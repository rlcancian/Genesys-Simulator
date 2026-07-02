# Plano de Implementação: Autômatos Celulares Não Uniformes

## Contexto

O tema foca em **autômatos celulares não uniformes**, onde células diferentes podem ter:
- **Regra local diferente** (`NONUNIFORMRULE`): a mesma vizinhança, mas cada célula aplica sua própria regra.
- **Vizinhança diferente** (`NONUNIFORMNEIGHBOOR`): a mesma regra, mas cada célula tem seu próprio conjunto de vizinhos.

Ambos os tipos já estão declarados no enum `CellularAutomataType`, mas **não têm implementação**. O objetivo é implementá-los de forma coerente com a arquitetura existente.

---

## 1. Análise do problema

### 1.1 Não uniformidade de regra

**Situação atual**: `CellularAutomata_Classic::applyLocalRule()` aplica o mesmo objeto `localRule` para todas as células.

**Solução**: criar `CellularAutomata_NonUniformRule`, que mantém um `std::unordered_map<long, LocalRule*>` mapeando número de célula → regra específica. Durante `applyLocalRule()`, para cada célula, usa a regra específica se existir, ou cai de volta na regra global do CA.

Não requer alteração nas classes existentes — é uma nova subclasse de `CellularAutomataBase`.

### 1.2 Não uniformidade de vizinhança

**Situação atual**: `Lattice::init()` chama `parentCellularAutomata->getNeighborhood()` (a vizinhança global) para calcular os vizinhos de **todas** as células. Os vizinhos ficam armazenados em cada `Cell*`.

**Solução**: criar `CellularAutomata_NonUniformNeighborhood`, que após chamar `lattice->init()` (que configura vizinhos padrão), re-calcula os vizinhos das células que têm vizinhança específica.

**Problema arquitetural**: O construtor de `Neighborhood` tem um efeito colateral: chama `ca->setNeighborhood(this)`, sobrescrevendo a vizinhança global do CA. Ao criar múltiplas vizinhanças para células específicas, a última criada viraria a vizinhança global do CA — comportamento indesejado.

**Solução mínima**: adicionar parâmetro `bool registerWithCA = true` ao construtor de `Neighborhood` e propagá-lo para as subclasses. Isso permite criar vizinhanças standalone (por célula) sem alterar o estado global do CA.

---

## 2. Arquivos a modificar/criar

### 2.1 Modificar: `Neighborhood.h`

**Mudança**: adicionar parâmetro `bool registerWithCA = true` ao construtor.

```cpp
Neighborhood(CellularAutomataBase* parentCellularAutomata,
             unsigned short radius = 1,
             BoundaryCondition* boundary = nullptr,
             bool includeCellItself = false,
             bool registerWithCA = true) {
    this->parentCellularAutomata = parentCellularAutomata;
    if (registerWithCA)
        parentCellularAutomata->setNeighborhood(this);
    ...
}
```

**Compatibilidade**: parâmetro com valor padrão `true` — todo código existente continua funcionando sem alteração.

### 2.2 Modificar: `Neighborhood_Moore.h`, `Neighborhood_VonNeumann.h`, `Neighborhood_Center.h`

Propagar o parâmetro `registerWithCA` até o construtor base.

```cpp
Neighborhood_Moore(CellularAutomataBase* parentCellularAutomata,
                   unsigned short radius = 1,
                   BoundaryCondition* boundary = nullptr,
                   bool includeCellItself = false,
                   bool registerWithCA = true)
    : Neighborhood(parentCellularAutomata, radius, boundary, includeCellItself, registerWithCA)
```

### 2.3 Criar: `CellularAutomata/CellularAutomata_NonUniformRule.h`

Nova subclasse de `CellularAutomataBase`.

**Responsabilidade**: permitir que cada célula tenha sua própria regra local.

**Atributos**:
- `std::unordered_map<long, LocalRule*> _cellLocalRules` — mapa de número de célula → regra específica.

**Métodos públicos**:
- `setCellRule(long cellNumber, LocalRule* rule)` — atribui regra a uma célula.
- `setCellRule(std::vector<int> position, LocalRule* rule)` — converte posição para número e atribui.
- `getCellRules() const` — acesso de leitura ao mapa.

**Comportamento de `applyLocalRule()`**:
```
Para cada célula:
  se tem regra específica → aplica essa regra
  senão → aplica localRule (regra global do CA, fallback)
Em seguida, para cada célula: updateState()
```

**Semantics do fallback**: se uma célula não tem regra atribuída, o CA usa a `localRule` global (o último objeto `LocalRule` registrado via construtor ou `setLocalRule()`). Se `localRule == nullptr` e a célula não tem regra própria, a célula não evolui naquele passo.

### 2.4 Criar: `CellularAutomata/CellularAutomata_NonUniformNeighborhood.h`

Nova subclasse de `CellularAutomataBase`.

**Responsabilidade**: permitir que cada célula tenha sua própria vizinhança.

**Atributos**:
- `std::unordered_map<long, Neighborhood*> _cellNeighborhoods` — mapa de número de célula → vizinhança específica.

**Métodos públicos**:
- `setCellNeighborhood(long cellNumber, Neighborhood* hood)` — atribui vizinhança a uma célula.
- `setCellNeighborhood(std::vector<int> position, Neighborhood* hood)` — converte posição e atribui.
- `getCellNeighborhoods() const` — acesso de leitura ao mapa.

**Override de `init()`**:
```
1. Chamar CellularAutomataBase::init()  ← cria células, calcula vizinhos com vizinhança global
2. Para cada entrada em _cellNeighborhoods:
   a. Garantir que a condição de contorno da vizinhança conhece o lattice:
      hood->getBoundary()->setLattice(lattice)
   b. Recalcular vizinhos: hood->getNeighbors(cell)
   c. Sobrescrever: cell->setNeighbors(neighbors)
```

**Comportamento de `applyLocalRule()`**: igual ao Classic — aplica a mesma `localRule` global a todas as células. A não uniformidade já está embutida nos vizinhos de cada célula (definidos no `init()`).

**Pré-condição para vizinhanças por célula**: devem ser criadas com `registerWithCA = false` para não sobrescrever a vizinhança global do CA.

### 2.5 Modificar: `CellularAutomataComp.h`

Adicionar:
- Includes das novas classes.
- Métodos `setCellLocalRule()` e `setCellNeighborhood()` que internamente fazem `dynamic_cast` para o tipo não-uniforme correto.

```cpp
bool setCellLocalRule(long cellNumber, LocalRule* rule);
bool setCellLocalRule(std::vector<int> position, LocalRule* rule);
bool setCellNeighborhood(long cellNumber, Neighborhood* hood);
bool setCellNeighborhood(std::vector<int> position, Neighborhood* hood);
```

### 2.6 Modificar: `CellularAutomataComp.cpp`

#### `setCellularAutomataType()`
Adicionar os casos faltantes:
```cpp
} else if (_cellularAutomataType == CellularAutomataType::NONUNIFORMRULE) {
    _cellularAutomata = new CellularAutomata_NonUniformRule();
} else if (_cellularAutomataType == CellularAutomataType::NONUNIFORMNEIGHBOOR) {
    _cellularAutomata = new CellularAutomata_NonUniformNeighborhood();
```

#### `_loadInstance()`
Carregar os 6 campos de configuração usando `fields->loadField()`:
```cpp
_cellularAutomataType = static_cast<CellularAutomataType>(
    fields->loadField("caType", static_cast<int>(DEFAULT.cellularAutomataType)));
// ... e chamar os setters para instanciar os objetos
setCellularAutomataType(_cellularAutomataType);
setLatticeType(_latticeType);
...
```

#### `_saveInstance()`
Salvar os 6 campos usando `fields->saveField()`:
```cpp
fields->saveField("caType", static_cast<int>(_cellularAutomataType),
                  static_cast<int>(DEFAULT.cellularAutomataType), saveDefaultValues);
// ... demais campos
```

#### `_check()`
Adicionar validação real:
- Verificar que nenhum ponteiro essencial é `nullptr`.
- Verificar coerência: NONUNIFORMRULE requer que `_cellularAutomata` seja do tipo certo.
- Só conectar os objetos se todos os ponteiros forem válidos.
- Retornar `false` com mensagem de erro se alguma condição não for satisfeita.

---

## 3. Novos testes

### 3.1 Teste de regra não uniforme

**Cenário**: lattice 1D com 5 células, vizinhança Center (raio 1), Boundary Closed.
- Estado inicial: `[1, 1, 1, 1, 1]`
- Regra padrão (global): `AlwaysAlive` — células sempre ficam no estado 1.
- Regra específica para células 1 e 3: `AlwaysDead` — células sempre ficam no estado 0.

**Após um passo esperado**: `[1, 0, 1, 0, 1]`

Demonstra que células 0, 2, 4 seguem a regra global e células 1, 3 seguem suas regras específicas.

### 3.2 Teste de vizinhança não uniforme — estrutura

**Cenário**: lattice 2D 5×5, vizinhança global Moore (raio 1), Boundary Closed.
- Célula `{2,2}` recebe vizinhança VonNeumann (raio 1).

**Verificação após init()**:
- Célula `{2,2}` tem 4 vizinhos (VonNeumann).
- Células internas com Moore têm 8 vizinhos.

Demonstra que a vizinhança é realmente aplicada por célula.

### 3.3 Teste de vizinhança não uniforme — comportamento

**Cenário**: lattice 2D 5×5, vizinhança global Moore (raio 1), regra Game of Life.
- Padrão inicial: blinker horizontal `{1,2}, {2,2}, {3,2}`.
- Célula `{2,2}` recebe vizinhança VonNeumann: enxerga menos vizinhos.

**Verificação**: o comportamento de `{2,2}` difere do que seria com Moore, confirmando que a vizinhança por célula altera a simulação.

---

## 4. Diagrama da solução

```
CellularAutomataBase (existente)
├── CellularAutomata_Classic           (existente — uniforme síncrono)
├── CellularAutomata_1DTimed           (existente — 1D com tempo como 2ª dim)
├── CellularAutomata_NonUniformRule    (NOVO — regra por célula)
│     _cellLocalRules: map<long, LocalRule*>
│     applyLocalRule(): usa regra específica ou fallback global
└── CellularAutomata_NonUniformNeighborhood (NOVO — vizinhança por célula)
      _cellNeighborhoods: map<long, Neighborhood*>
      init(): após init padrão, sobrescreve vizinhos por célula
      applyLocalRule(): igual ao Classic (não-uniformidade já está nos vizinhos)
```

```
Neighborhood (modificada)
  construtor agora aceita registerWithCA=true (compatível com código existente)
  com registerWithCA=false → não chama ca->setNeighborhood(this)
  → permite criar vizinhanças standalone para uso por célula
```

---

## 5. Justificativa das escolhas

| Decisão | Justificativa |
|---|---|
| `registerWithCA` no Neighborhood | Mudança mínima; compatível retroativamente; sem alteração nas classes de regra |
| `unordered_map` para regras/vizinhanças | O(1) lookup por número de célula; adequado para lattices grandes |
| Vizinhas recalculadas no `init()` | Mantém a semântica atual: vizinhos são estáticos durante a simulação |
| Fallback para regra global | Permite configurar regras apenas para subconjuntos de células |
| Dois tipos separados (regra / vizinhança) | Fiel à arquitetura existente (enums separados); composição por herança é possível se necessário |

---

## 6. Combinação: regra E vizinhança não uniformes simultaneamente

Além dos dois tipos separados, será implementada uma terceira classe que combina ambos os mecanismos: `CellularAutomata_NonUniform`. Ela representa o caso mais geral de autômato celular não uniforme, onde cada célula pode ter simultaneamente sua própria regra local e sua própria vizinhança.

### 2.7 Criar: `CellularAutomata/CellularAutomata_NonUniform.h`

**Estratégia de implementação**: herdar de `CellularAutomata_NonUniformNeighborhood` (que já cuida da vizinhança por célula no `init()`) e adicionar o mapa de regras por célula.

```cpp
class CellularAutomata_NonUniform : public CellularAutomata_NonUniformNeighborhood {
    std::unordered_map<long, LocalRule*> _cellLocalRules;

    void setCellRule(long cellNumber, LocalRule* rule);
    void setCellRule(std::vector<int> position, LocalRule* rule);

protected:
    virtual void applyLocalRule() override {
        // vizinhos já foram configurados por célula no init() da classe base
        for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
            Cell* cell = lattice->getCell(cellNumber);
            auto it = _cellLocalRules.find(cellNumber);
            LocalRule* rule = (it != _cellLocalRules.end()) ? it->second : localRule;
            if (rule != nullptr)
                rule->applyRule(cell);
        }
        for (long cellNumber = 0; cellNumber < lattice->getCellsSize(); cellNumber++) {
            lattice->getCell(cellNumber)->updateState();
        }
    }
};
```

**Por que herdar de `NonUniformNeighborhood` e não de `NonUniformRule`**: `NonUniformNeighborhood` sobrecarrega `init()`, que é onde a vizinhança por célula é configurada. `NonUniformRule` só sobrecarrega `applyLocalRule()`. Ao herdar de `NonUniformNeighborhood` e sobrescrever `applyLocalRule()`, os dois comportamentos são combinados com mínima duplicação de código.

### Enum para o tipo combinado

Adicionar ao `CellularAutomataType` em `CellularAutomataComp.h`:
```cpp
NONUNIFORM = 7  // regra E vizinhança não uniformes por célula
```

E no factory de `setCellularAutomataType()`:
```cpp
} else if (_cellularAutomataType == CellularAutomataType::NONUNIFORM) {
    _cellularAutomata = new CellularAutomata_NonUniform();
```

E em `setCellLocalRule()` / `setCellNeighborhood()` em `CellularAutomataComp`: o `dynamic_cast` para `CellularAutomata_NonUniform` também funcionará porque ela herda dos dois tipos.

### 3.4 Teste do tipo combinado

**Cenário**: lattice 2D 5×5, vizinhança global Moore (raio 1), regra global `AlwaysAlive`.
- Célula `{2,2}` recebe vizinhança VonNeumann **e** regra `AlwaysDead`.
- Todas as outras células: Moore + `AlwaysAlive`.

**Após um passo**:
- `{2,2}` deve estar morta (regra AlwaysDead).
- Todos os vizinhos de `{2,2}` que usam Moore têm como vizinho a célula `{2,2}` (enxergam-na como estava antes do passo).
- `{2,2}` com VonNeumann enxerga 4 vizinhos (não 8).

Demonstra que regra e vizinhança por célula funcionam simultaneamente e de forma independente.

---

## 7. Regra local definida pelo usuário: `LocalRule_Custom.h`

O guia exige explicitamente "uma solução clara para regras locais definidas pelo usuário" (seção 11) e a demonstração de "ao menos uma regra local definida pelo usuário" nos testes (seção 12).

O mecanismo de regra por célula já satisfaz esse requisito arquiteturalmente — o usuário cria uma subclasse de `LocalRule`, implementa `applyRule()`, e atribui por célula via `setCellLocalRule()`. Para tornar esse padrão explícito e demonstrável, será criada uma classe de exemplo concreta.

### 2.8 Criar: `CellularAutomata/LocalRule_Custom.h`

Uma regra local de exemplo que demonstra o padrão de extensão pelo usuário. A regra implementa uma lógica simples mas não trivial: a célula assume o estado mais frequente entre seus vizinhos (regra de maioria).

```cpp
class LocalRule_Custom : public LocalRule {
public:
    LocalRule_Custom(CellularAutomataBase* ca) : LocalRule(ca) {}

    virtual void applyRule(Cell* cell) override {
        // majority vote: cell takes the most common state among neighbors
        std::map<long, int> counts;
        for (Cell* neighbor : cell->getNeighbors()) {
            counts[neighbor->getCurrentState().getValue()]++;
        }
        if (counts.empty()) {
            cell->setNextState(cell->getCurrentState());
            return;
        }
        long majority = std::max_element(counts.begin(), counts.end(),
            [](const auto& a, const auto& b){ return a.second < b.second; })->first;
        cell->setNextState(State(majority));
    }
};
```

Esta classe serve tanto como documentação do padrão de extensão quanto como terceira regra nos testes não uniformes.

---

## 8. Exemplo de simulação: `Smart_CellularAutomata_NonUniform`

Para satisfazer o requisito de que "o código deve compilar e integrar-se ao restante do GenESyS" (seção 11) e demonstrar o CA não uniforme rodando numa simulação real, serão criados dois arquivos:

```
source/applications/terminal/examples/smarts/Smart_CellularAutomata_NonUniform.h
source/applications/terminal/examples/smarts/Smart_CellularAutomata_NonUniform.cpp
```

O exemplo demonstrará os três tipos não uniformes via `CellularAutomataComp`:

**Cenário 1 — `NONUNIFORMRULE`**: lattice 10×10, vizinhança Moore, regra padrão Game of Life. Células da borda usam `LocalRule_Custom` (maioria). Após alguns passos, a borda evolui diferente do interior.

**Cenário 2 — `NONUNIFORMNEIGHBOOR`**: lattice 10×10, regra Game of Life uniforme. Células da coluna central usam VonNeumann, resto usa Moore. Demonstra que a mesma regra produz comportamentos distintos conforme a vizinhança.

**Cenário 3 — `NONUNIFORM` (combinado)**: lattice 10×10. Células da borda superior têm Moore + `LocalRule_Custom`. Células do interior têm VonNeumann + Game of Life.

O arquivo `.cpp` seguirá o mesmo padrão de `Smart_CellularAutomata.cpp`: instancia o componente via `plugins->newInstance<CellularAutomataComp>()`, configura via getters e novos métodos, e executa a simulação.

---

## 9. O que não será implementado nesta fase

- **Persistência de mapeamentos por célula** (`_loadInstance`/`_saveInstance` dos mapas) — por célula é tipicamente configurado por código no modelo; persistência do tipo de CA e parâmetros globais é suficiente para `_loadInstance`.
- **GUI para configuração por célula** — fora do escopo do framework de autômatos.
