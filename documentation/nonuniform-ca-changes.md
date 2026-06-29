# Registro de Implementação: Autômatos Celulares Não Uniformes

Branch: `dcs/nonuniform-ca`

---

## Visão geral

O objetivo do trabalho é estender o framework de autômatos celulares do GenESyS para suportar **não uniformidade**: a capacidade de cada célula ter sua própria regra local, sua própria vizinhança, ou ambas simultaneamente. O framework existente só suportava autômatos uniformes, onde todas as células compartilham a mesma regra e a mesma vizinhança.

Foram implementados três novos tipos de CA, duas novas condições de contorno, uma API de região, duas novas regras locais, um componente de simulação estendido e 24 testes unitários distribuídos em três executáveis.

---

## Arquivos criados

### `CellularAutomata/CellularAutomata_NonUniformRule.h`

Subclasse de `CellularAutomataBase` que permite atribuir uma regra local diferente a cada célula.

Internamente mantém um `std::unordered_map<long, LocalRule*>` mapeando número de célula → regra específica. No método `applyLocalRule()`, para cada célula o CA verifica se existe uma regra específica no mapa; se sim, usa essa regra; se não, usa a `localRule` global como fallback. Isso permite configurar regras apenas para um subconjunto de células, deixando as demais com o comportamento padrão.

**Métodos públicos:**
- `setCellRule(long cellNumber, LocalRule* rule)`
- `setCellRule(std::vector<int> position, LocalRule* rule)`

---

### `CellularAutomata/CellularAutomata_NonUniformNeighborhood.h`

Subclasse de `CellularAutomataBase` que permite atribuir uma vizinhança diferente a cada célula.

O ponto central desta classe é o override de `init()`. O framework armazena os vizinhos de cada célula de forma estática no momento da inicialização (dentro de `Cell::neighbors`). O override funciona em duas fases: primeiro chama `CellularAutomataBase::init()`, que inicializa todas as células com a vizinhança global; depois percorre o mapa de vizinhanças específicas e recalcula `cell->setNeighbors()` para cada célula que tem uma vizinhança própria. Isso permite que durante `applyLocalRule()` o código seja idêntico ao CA clássico — a não uniformidade já está "embutida" nos vizinhos de cada célula.

As vizinhanças por célula devem ser criadas com `registerWithCA=false` para não sobrescrever a vizinhança global do CA (ver modificação em `Neighborhood.h`).

**Métodos públicos:**
- `setCellNeighborhood(long cellNumber, Neighborhood* hood)`
- `setCellNeighborhood(std::vector<int> position, Neighborhood* hood)`

---

### `CellularAutomata/CellularAutomata_NonUniform.h`

Combina regra e vizinhança não uniformes simultaneamente. Herda de `CellularAutomata_NonUniformNeighborhood`, recebendo o override de `init()` gratuitamente, e adiciona seu próprio mapa de regras por célula com override de `applyLocalRule()`.

A escolha de herdar de `NonUniformNeighborhood` (e não de `NonUniformRule`) é deliberada: `NonUniformNeighborhood` tem o override de `init()` que é onde a vizinhança por célula é configurada. `NonUniformRule` só tem override de `applyLocalRule()`. Herdar de `NonUniformNeighborhood` e sobrescrever `applyLocalRule()` combina os dois mecanismos sem duplicação de código.

**Métodos públicos:**
- `setCellRule(long cellNumber, LocalRule* rule)`
- `setCellRule(std::vector<int> position, LocalRule* rule)`
- (herda `setCellNeighborhood` da classe base)

---

### `CellularAutomata/Boundary_Reflexive.h`

Condição de contorno reflexiva: posições fora dos limites são espelhadas de volta ao grid por reflexão iterativa. Para cada dimensão, aplica `if (pos < 0) pos = -pos; if (pos >= D) pos = 2*(D-1)-pos;` em loop até a posição ser válida. A borda age como um espelho — a célula de borda vê o interior do grid refletido.

---

### `CellularAutomata/Boundary_Adiabatic.h`

Condição de contorno adiabática (sem fluxo): posições fora dos limites são travadas na célula de borda mais próxima usando `std::clamp`. Células de borda "veem a si mesmas" quando a vizinhança solicita uma posição fora do grid. Isso garante que não há informação fluindo para dentro a partir de fora do domínio.

---

### `CellularAutomata/LocalRule_Custom.h`

Regra de exemplo definida pelo usuário: **voto por maioria**. Cada célula assume o estado mais frequente entre seus vizinhos. Em caso de empate, o menor valor de estado vence. Demonstra o padrão de extensão do framework via subclasse de `LocalRule`.

---

### `CellularAutomata/LocalRule_PermissiveLife.h`

Regra configurável de sobrevivência/nascimento para estados binários:
- Célula viva sobrevive se o número de vizinhos vivos estiver em `[surviveMin, surviveMax]`
- Célula morta nasce se o número de vizinhos vivos estiver em `[birthMin, birthMax]`

Parâmetros padrão: `surviveMin=1, surviveMax=4, birthMin=2, birthMax=3`. Quando usada com vizinhança VonNeumann, produz dinâmica de crescimento mais permissiva que o Game of Life — estruturas se formam e se estabilizam com mais facilidade. Isso torna visível a diferença de comportamento nas "zonas ecológicas" do Scenario 4.

---

### `smarts/Smart_CellularAutomata_NonUniform.h` / `.cpp`

Quatro cenários de demonstração, cada um rodando de forma independente e imprimindo o estado do grid no terminal a cada passo:

**Scenario 1 — NONUNIFORMRULE**: Grid 10×10, Moore global, Game of Life como regra padrão. Coluna esquerda (x=0) usa `LocalRule_Custom` (maioria). Demonstra que células lado a lado podem evoluir segundo regras completamente diferentes.

**Scenario 2 — NONUNIFORMNEIGHBOOR**: Grid 10×10, Game of Life uniforme. Coluna central (x=5) usa VonNeumann; demais células usam Moore. Demonstra que a mesma regra produz comportamentos distintos conforme a vizinhança atribuída.

**Scenario 3 — NONUNIFORM**: Grid 10×10. Linha superior (y=0) recebe VonNeumann + `LocalRule_Custom`; restante recebe Moore + Game of Life. Demonstra as duas formas de não uniformidade combinadas.

**Scenario 4 — Zonas Ecológicas**: Grid 20×20 dividido em duas zonas pela linha x=10. Zona esquerda usa Moore + Game of Life (comportamento clássico, permite gliders). Zona direita usa VonNeumann + `LocalRule_PermissiveLife` (estruturas crescem e se estabilizam de forma diferente). Um glider é semeado na zona GoL próximo à fronteira, e uma estrutura em L é semeada na zona PermissiveLife. O comportamento na fronteira entre as zonas é o que só um CA não uniforme pode produzir.

---

## Arquivos modificados

### `CellularAutomata/Neighborhood.h`

Adicionado parâmetro `bool registerWithCA = true` ao construtor. Quando `false`, o construtor não chama `ca->setNeighborhood(this)`, permitindo criar vizinhanças por célula sem sobrescrever a vizinhança global do CA. Valor padrão `true` mantém compatibilidade total com todo código existente.

**Por que foi necessário:** o construtor de `Neighborhood` tinha como efeito colateral fixo registrar a vizinhança no CA. Ao criar múltiplas vizinhanças para células específicas, a última criada viraria a vizinhança global — comportamento incorreto. O parâmetro `registerWithCA` resolve isso com a mudança mínima possível.

### `CellularAutomata/Neighborhood_Moore.h`, `Neighborhood_VonNeumann.h`, `Neighborhood_Center.h`

Propagação dos parâmetros `includeCellItself` e `registerWithCA` até o construtor base. Assinatura padronizada:

```cpp
Neighborhood_Moore(CellularAutomataBase* parentCellularAutomata,
                   unsigned short radius = 1,
                   BoundaryCondition* boundary = nullptr,
                   bool includeCellItself = false,
                   bool registerWithCA = true)
```

### `CellularAutomataComp.h`

- Adicionado `NONUNIFORM = 7` ao enum `CellularAutomataType`
- Declarados seis novos métodos públicos para configurar células e regiões através do componente:

```cpp
bool setCellLocalRule(long cellNumber, LocalRule* rule);
bool setCellLocalRule(std::vector<int> position, LocalRule* rule);
bool setCellNeighborhood(long cellNumber, Neighborhood* hood);
bool setCellNeighborhood(std::vector<int> position, Neighborhood* hood);
bool setRegionRule(std::vector<int> posMin, std::vector<int> posMax, LocalRule* rule);
bool setRegionNeighborhood(std::vector<int> posMin, std::vector<int> posMax, Neighborhood* hood);
```

### `CellularAutomataComp.cpp`

- Includes dos novos arquivos adicionados (incluindo `Boundary_Reflexive.h` e `Boundary_Adiabatic.h`)
- `setCellularAutomataType()`: adicionados casos para `NONUNIFORMRULE`, `NONUNIFORMNEIGHBOOR` e `NONUNIFORM`
- `setBoundaryType()`: adicionados casos para `REFLEXIVE` (→ `new Boundary_Reflexive()`) e `ADIABATIC` (→ `new Boundary_Adiabatic()`)
- `_loadInstance()`: implementado — carrega 6 campos de configuração (`caType`, `latticeType`, `neighborhoodType`, `boundaryType`, `stateSetType`, `localRuleType`) e instancia os objetos correspondentes
- `_saveInstance()`: implementado — salva os mesmos 6 campos
- `_check()`: melhorado — verifica ponteiros nulos com mensagens específicas; permite `_localRule == nullptr` para os tipos que operam com regras por célula
- `setCellLocalRule()` / `setCellNeighborhood()`: implementados via `dynamic_cast` para o tipo não-uniforme correto; retornam `false` se o CA ativo não suportar o recurso
- `setRegionRule()` / `setRegionNeighborhood()`: delegam para os métodos equivalentes em `CellularAutomata_NonUniform` via `dynamic_cast`

### Testes — reorganização em 3 arquivos

Os testes foram divididos em três executáveis independentes:

- `test_cellular_automata_neighborhood.cpp` → `genesys_test_cellular_automata_neighborhood` (8 testes — sem alteração de conteúdo)
- `test_cellular_automata_nonuniform.cpp` → `genesys_test_cellular_automata_nonuniform` (12 testes novos)
- `test_cellular_automata_boundary.cpp` → `genesys_test_cellular_automata_boundary` (4 testes novos)

Total: 24 testes (8 pré-existentes + 16 novos). Ver seção de testes abaixo.

---

## Testes unitários

### Testes existentes (8) — `genesys_test_cellular_automata_neighborhood`

| Teste | O que verifica |
|---|---|
| `CellularAutomataLattice/ConvertsLinearCellNumbers...` | Conversão bidirecional entre número linear de célula e posição n-dimensional |
| `CellularAutomataMooreNeighborhood/CountsInternalNeighbors...` | Contagem correta de vizinhos Moore para múltiplas dimensões e raios |
| `CellularAutomataMooreNeighborhood/ReturnsDeterministicLexicographic...` | Vizinhos Moore retornados sempre na mesma ordem lexicográfica |
| `CellularAutomataMooreNeighborhood/KeepsFixedBoundaryPositions...` | Condição de contorno Fixed retorna células fixas para posições fora do grid |
| `CellularAutomataVonNeumannNeighborhood/CountsInternalNeighbors...` | Contagem correta de vizinhos VonNeumann para múltiplas dimensões e raios |
| `CellularAutomataVonNeumannNeighborhood/ReturnsDeterministicLexicographic...` | Vizinhos VonNeumann retornados sempre na mesma ordem lexicográfica |
| `CellularAutomataVonNeumannNeighborhood/KeepsFixedBoundaryPositions...` | Condição de contorno Fixed com VonNeumann |
| `CellularAutomataGameOfLife/KeepsTwoDimensionalMooreBlinker...` | Um blinker horizontal evolui para vertical em 1 passo (regressão GoL 2D) |

---

### Novos testes (12) — `genesys_test_cellular_automata_nonuniform`

#### `CellularAutomataNonUniformRule/AppliesPerCellRuleOverridingGlobalRule`

**O que testa:** a regra por célula tem prioridade sobre a regra global.

**Como:** grid 1D com 5 células, vizinhança Center, regra global AlwaysAlive. Células 1 e 3 recebem AlwaysDead como regra específica. Após um passo, o resultado esperado é `[1, 0, 1, 0, 1]` — as células sem override seguem o global (AlwaysAlive → 1), e as células com override seguem a regra específica (AlwaysDead → 0).

**Por que é importante:** prova que o mapa de regras por célula é consultado corretamente antes do fallback global.

---

#### `CellularAutomataNonUniformRule/FallsBackToGlobalRuleForUnassignedCells`

**O que testa:** células sem regra específica usam a regra global.

**Como:** grid 1D com 3 células, regra global AlwaysDead, sem overrides por célula. Todas as células iniciam com estado 1. Após um passo, todas devem estar com estado 0. Confirma que o fallback funciona mesmo quando o mapa está vazio.

---

#### `CellularAutomataNonUniformNeighborhood/AssignsCorrectNeighborCountPerCell`

**O que testa:** a vizinhança por célula é efetivamente aplicada no momento da inicialização.

**Como:** grid 2D 5×5, vizinhança global Moore r=1 (8 vizinhos para células interiores). A célula `{2,2}` recebe VonNeumann r=1 (4 vizinhos). Após `init()`, verifica que `{2,2}` tem 4 vizinhos e que outras células interiores têm 8.

**Por que é importante:** prova que o override de `init()` em `NonUniformNeighborhood` recalcula os vizinhos corretamente sem afetar as demais células.

---

#### `CellularAutomataNonUniformNeighborhood/EvolvesCorrectlyWithMixedNeighborhoods`

**O que testa:** a simulação com vizinhanças mistas produz evolução correta.

**Como:** grid 2D 5×5, Game of Life, blinker horizontal `{1,2},{2,2},{3,2}` ativo. A célula `{2,2}` usa VonNeumann. Após um passo, o blinker deve ter rotacionado para vertical `{2,1},{2,2},{2,3}`, com `{1,2}` e `{3,2}` mortos. O resultado é o mesmo de um CA uniforme neste caso específico porque os vizinhos ortogonais de `{2,2}` são exatamente os que GoL precisaria para a rotação do blinker — mas o teste demonstra que a simulação roda corretamente com vizinhanças mistas.

---

#### `CellularAutomataNonUniform/AppliesPerCellRuleAndNeighborhoodIndependently`

**O que testa:** regra e vizinhança por célula podem ser atribuídas simultaneamente e de forma independente.

**Como:** grid 2D 5×5, global AlwaysAlive + Moore. A célula `{2,2}` recebe VonNeumann (vizinhança) e AlwaysDead (regra). Após um passo: `{2,2}` deve estar morta (regra AlwaysDead aplicada) e com 4 vizinhos (VonNeumann aplicada); todas as demais devem estar vivas (AlwaysAlive global).

---

#### `CellularAutomataNonUniform/SameGoLRuleDifferentNeighborhoodProducesDifferentOutcome`

**O que testa:** a vizinhança atribuída por célula altera o resultado mesmo quando a regra é idêntica — prova que regra e vizinhança interagem.

**Como:** grid 2D 7×7, Fixed boundary, Game of Life + Moore como global. A célula `{4,4}` recebe VonNeumann; a célula `{1,1}` usa Moore (padrão). São semeadas 3 células vivas diagonais de `{4,4}` e 3 células vivas diagonais de `{1,1}`. Após um passo:
- `{1,1}` nasce (Moore vê as 3 diagonais → 3 vizinhos vivos → GoL: nasce)
- `{4,4}` permanece morta (VonNeumann não vê diagonais → 0 vizinhos vivos → GoL: não nasce)

**Por que é importante:** este é o teste mais crítico para o tipo `NonUniform`. Os anteriores usavam regras determinísticas (AlwaysAlive/AlwaysDead) que ignoram os vizinhos, então não provam que a vizinhança afeta o resultado. Este teste usa GoL — que depende dos vizinhos — e demonstra que a mesma regra produz resultados distintos quando a vizinhança muda.

---

#### `CellularAutomataCompDispatch/ClassHierarchyEnablesPerCellApiForNonUniformTypesOnly`

**O que testa:** a hierarquia de classes permite que `CellularAutomataComp::setCellLocalRule()` e `setCellNeighborhood()` façam dispatch correto via `dynamic_cast`.

**Como:** verifica os invariantes de herança que `CellularAutomataComp` pressupõe internamente:
- `CellularAutomata_NonUniformRule` é castável para `NonUniformRule`
- `CellularAutomata_NonUniform` é castável para `NonUniform` e para `NonUniformNeighborhood` (por herança)
- `CellularAutomata_Classic` não é castável para nenhum tipo não-uniforme

**Por que é importante:** `CellularAutomataComp` mantém o CA como `CellularAutomataBase*` e usa `dynamic_cast` para rotear `setCellLocalRule`/`setCellNeighborhood` ao subtipo correto. Se a hierarquia estivesse errada (ex: `NonUniform` não herdasse de `NonUniformNeighborhood`), o dispatch silenciosamente retornaria `false` e vizinhanças por célula não seriam aplicadas no tipo combinado.

---

#### `CellularAutomataPermissiveLife/DeadCellBornWhenNeighborCountInBirthRange`

**O que testa:** a regra `LocalRule_PermissiveLife` aplica corretamente as condições de nascimento e sobrevivência.

**Como:** grid 2D 5×5, VonNeumann, PermissiveLife com padrões (survives 1–4, born 2–3). Semeada uma forma em L: `{2,2}`, `{3,2}`, `{2,3}`. Após um passo:
- `{2,2}` sobrevive (2 vizinhos vivos, dentro de 1–4)
- `{3,2}` sobrevive (1 vizinho vivo, dentro de 1–4)
- `{2,3}` sobrevive (1 vizinho vivo, dentro de 1–4)
- `{3,3}` nasce (2 vizinhos vivos — `{3,2}` e `{2,3}` — dentro de 2–3)
- `{1,2}` não nasce (1 vizinho vivo, abaixo de birthMin=2)

---

#### `CellularAutomataCustomRule/MajorityVoteConvergesUniformRegion`

**O que testa:** a regra `LocalRule_Custom` (maioria) aplica corretamente o voto entre vizinhos.

**Como:** grid 1D com 3 células, estado inicial `[1, 0, 1]`, vizinhança Center, Boundary Closed. Após um passo:
- Célula 0 (Closed): vizinhos são 2 (estado 1) e 1 (estado 0) → empate → menor valor vence → estado 0
- Célula 1: vizinhos são 0 (estado 1) e 2 (estado 1) → maioria 1 → estado 1
- Célula 2 (Closed): vizinhos são 1 (estado 0) e 0 (estado 1) → empate → menor valor vence → estado 0

Resultado esperado: `[0, 1, 0]`.

---

#### `CellularAutomataNonUniformRegion/SetRegionRuleAssignsRuleToAllCellsInBox`

**O que testa:** `setRegionRule` atribui a regra corretamente a todas as células do bounding box.

**Como:** grid 1D com 5 células, regra global AlwaysAlive. Região `[1,3]` recebe AlwaysDead. Todas as células iniciam com estado 0. Após um passo: células 0 e 4 (fora da região) devem ser 1; células 1, 2 e 3 (dentro da região) devem ser 0.

---

#### `CellularAutomataNonUniformRegion/SetRegionNeighborhoodAssignsNeighborhoodToAllCellsInBox`

**O que testa:** `setRegionNeighborhood` recalcula os vizinhos de todas as células do bounding box.

**Como:** grid 2D 5×5, vizinhança global Moore r=1 (8 vizinhos para células interiores). Região `[1,1]`–`[3,3]` recebe VonNeumann r=1. Após `init()`, verifica que células interiores da região têm 4 vizinhos (VonNeumann) e células fora da região têm 8 vizinhos (Moore).

---

#### `CellularAutomataNonUniformRegion/SetRegionRuleAndNeighborhoodCombined`

**O que testa:** `setRegionRule` e `setRegionNeighborhood` podem ser usados juntos na mesma região.

**Como:** grid 2D 5×5, global GoL+Moore. Região `[2,2]`–`[3,3]` recebe AlwaysDead (regra) e VonNeumann (vizinhança). Após um passo: células da região estão mortas (AlwaysDead); células fora evoluem normalmente com GoL+Moore.

---

### Novos testes (4) — `genesys_test_cellular_automata_boundary`

#### `BoundaryReflexive/CornerCellSeesReflectedNeighbors`

**O que testa:** a condição reflexiva mapeia posições inválidas de volta ao grid.

**Como:** lattice 1D [0..4], raio 1. Verifica que os vizinhos das células 0 e 4 têm números dentro do intervalo [0,4] — nenhum índice inválido é retornado.

---

#### `BoundaryReflexive/ReflectedNeighborHasSameStateAsInnerCell`

**O que testa:** o vizinho refletido aponta para a célula interior correta.

**Como:** lattice 1D [0..4], célula 1 viva. Verifica que os vizinhos da célula 0 incluem a célula 1 (a posição −1 reflete para +1).

---

#### `BoundaryAdiabatic/CornerCellSeesItselfForOutOfBoundsNeighbor`

**O que testa:** a condição adiabática trava posições inválidas na célula de borda.

**Como:** lattice 1D [0..4]. Verifica que os vizinhos da célula 0 incluem ela mesma (posição −1 é clamped para 0), e que os vizinhos da célula 4 incluem ela mesma (posição 5 é clamped para 4).

---

#### `BoundaryAdiabatic/AllDeadGridRemainsDeadAfterStep`

**O que testa:** a condição adiabática não introduz estado artificial nas bordas.

**Como:** grid 1D de 5 células, todas mortas, GoL, Adiabatic. Após um passo, todas as células devem continuar mortas — a auto-referência da borda contribui estado 0, não induz nascimentos.

---

## Decisões de design

**`registerWithCA=false` em vez de uma nova classe de vizinhança**
Alterar apenas o parâmetro do construtor existente é a mudança mínima e retrocompatível. Criar uma classe separada (`Neighborhood_Standalone`) seria desnecessário e adicionaria complexidade ao modelo de herança.

**`CellularAutomata_NonUniform` herda de `NonUniformNeighborhood`**
`NonUniformNeighborhood` sobrecarrega `init()`, que é onde a vizinhança por célula é configurada. Herdar dela e sobrescrever apenas `applyLocalRule()` combina os dois mecanismos sem duplicação de código. A alternativa (herança múltipla ou composição) seria mais complexa.

**`dynamic_cast` em `CellularAutomataComp`**
O componente mantém um ponteiro `CellularAutomataBase*` genérico. O `dynamic_cast` é feito apenas nos métodos `setCellLocalRule`/`setCellNeighborhood`, que são chamados exclusivamente quando o CA já é do tipo correto. Expor a API não-uniforme como método virtual na base tornaria a interface de `CellularAutomataBase` dependente de funcionalidades que só fazem sentido nos subtipos não-uniformes.

**Vizinhos são estáticos após `init()`**
Mantém a semântica existente do simulador. Vizinhanças por célula são configuradas antes de `init()` e calculadas durante `init()`. Mudar vizinhos durante a simulação exigiria redesenho mais profundo do `Lattice`.

**`LocalRule_PermissiveLife` como classe parametrizável, não hardcoded**
Torna a classe reutilizável para outros cenários (ex: GoL pode ser expresso como PermissiveLife com `surviveMin=2, surviveMax=3, birthMin=3, birthMax=3`). Mantém `LocalRule_Custom` como exemplo de extensão livre e `LocalRule_PermissiveLife` como regra de biblioteca configurável — duas formas complementares de customização.

**`Boundary_Reflexive` com loop de reflexão iterativa**
Um único espelho não é suficiente quando `radius > D/2` (a posição refletida pode sair do outro lado). O loop garante que posições muito negativas ou muito grandes são refletidas repetidamente até ficarem dentro do grid. A complexidade extra só ocorre no pré-processamento de `init()`, não durante a simulação.

**`Boundary_Adiabatic` via `std::clamp`**
A implementação mais simples e eficiente para condição de fluxo zero: `std::clamp(pos, 0, D-1)`. Células de borda se auto-referenciam, o que conserva o estado da borda (nenhuma informação externa entra). A alternativa (retornar a própria célula como ponteiro nulo ou célula fantasma) exigiria tratamento especial em `LocalRule`, acoplando a condição de contorno à regra.

**`setRegionRule` / `setRegionNeighborhood` com helper `_iterateRegion`**
A iteração n-dimensional sobre um bounding box [posMin, posMax] é delegada ao método privado `_iterateRegion`, que usa recursão por dimensão (idêntica ao padrão usado em `Neighborhood_Moore` e `Neighborhood_VonNeumann`). Isso mantém os métodos públicos simples e o helper pode ser reutilizado para futuras APIs de região sem duplicação.
