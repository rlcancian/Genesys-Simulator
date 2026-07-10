# Phase 2 — Inventário técnico do kernel/simulator (2026-1)

## Audit Status (WiP20261)
- Branch audited: `WiP20261` (requested scope).
- Working branch in local checkout during this audit: `work`.
- Audit scope: `source/kernel/simulator` phase-2 inventory revalidated against current code.
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`.
- Audit intent: preserve historical context from the original inventory while reclassifying each major finding based on current code evidence.

## Escopo analisado
- Pacote: `source/kernel/simulator`
- Classes foco desta fase:
  - `Model`
  - `ModelSimulation`
  - `PluginManager`
  - `TraceManager`
- Objetivo: mapear ownership/lifetime, identificar riscos de regressão/leak/UB e priorizar ações com testes.

## 1) Mapa de ownership/lifetime (estado atual)

### 1.1 Model
**Texto histórico (inventário original)**
- `Model` aloca em heap e mantém ponteiros crus para quase todos os subsistemas (`_eventManager`, `_modeldataManager`, `_componentManager`, `_simulation`, `_parser`, `_modelChecker`, `_modelPersistence`, `_futureEvents`, `_controls`, `_responses`, `_modelInfo`).
- O destrutor estava descrito como `= default` no header, sem lógica de liberação explícita.

### Audit status
- Classificação do achado “`Model` com destrutor default + múltiplas alocações”: **SUPERSEDED**.
- Classificação do risco de teardown hoje: **PARTIAL**.

### Evidence
- `Model.h` declara destrutor virtual explícito (`virtual ~Model();`).
- `Model.cpp` implementa destrutor explícito com teardown estruturado: `_destroyFutureEvents`, `_destroyTransientEntities`, `_destroyComponents`, `_destroyModelDataDefinitions`, seguido da destruição dos serviços/containers owned.
- `Model::clear()` também aplica rotina explícita de limpeza.

### Remaining gaps
- Ainda há uso intenso de ponteiros crus e ownership implícito em várias associações; embora o teardown exista, não há migração ampla para RAII/smart pointers.

### 1.2 ModelSimulation
**Texto histórico (inventário original)**
- `ModelSimulation` cria controles com `new` e insere em `Model::getControls()`.
- Mantém membros alocados dinamicamente no header (`_cstatsAndCountersSimulation`, `_cstatsAndCountersMapSimulation`, `_breakpointsOnTime`, `_breakpointsOnComponent`, `_breakpointsOnEntity`).
- O inventário original apontava `_createSimulationEvent()` retornando `SimulationEvent*` cru sem ownership explícito.

### Audit status
- Classificação do achado “`ModelSimulation::_createSimulationEvent` com ownership indefinido”: **DONE**.
- Classificação do achado “`ModelSimulation` sem estratégia explícita de teardown”: **SUPERSEDED**.
- Classificação de risco residual de lifetime em `ModelSimulation`: **PARTIAL**.

### Evidence
- Em `ModelSimulation.h`, assinatura atual de `_createSimulationEvent` é `std::unique_ptr<SimulationEvent>`.
- Em `ModelSimulation.cpp`, implementação cria e retorna `std::unique_ptr<SimulationEvent>` e chamadas usam `.get()` apenas para notificação, preservando ownership local.
- `ModelSimulation` possui destrutor explícito removendo/deletando controles owned, estruturas auxiliares, breakpoints e reporter owned.

### Remaining gaps
- Parte da classe ainda depende de alocações manuais e flags de ownership (`_ownsSimulationReporter`), sem unificação completa de política de ownership.

### 1.3 PluginManager
**Texto histórico (inventário original)**
- `_plugins` e `_pluginConnector` são criados com `new`.
- O inventário antigo apontava ausência de destrutor explícito e retorno potencialmente inválido em `insert()` quando `_insert` falha.

### Audit status
- Classificação do achado “`PluginManager::insert` pode retornar ponteiro inválido”: **DONE**.
- Classificação do achado “`PluginManager` sem teardown explícito”: **SUPERSEDED**.
- Classificação do risco de robustez em auto-carga/parsing de lista de plugins: **OPEN**.

### Evidence
- `PluginManager::insert` conecta plugin e, se `_insert(plugin)` falhar, força `plugin = nullptr` antes do retorno; em exceção retorna `nullptr`.
- `PluginManager` possui destrutor explícito liberando plugins e `_pluginConnector`.
- `autoInsertPlugins` ainda faz tratamento manual de caracteres de controle no início da linha (`while (line[0] > 126 || line[0] < 32)`) sem normalização completa da linha.

### Remaining gaps
- O hardening de parsing/normalização no carregamento de arquivo de plugins permanece candidato real de backlog.

### 1.4 TraceManager
**Texto histórico (inventário original)**
- Estruturas de handlers e `_errorMessages` alocadas com `new`.
- O inventário antigo dizia que `traceError(...)` não alimentava `_errorMessages`.
- Também sugeria ausência de teardown efetivo.

### Audit status
- Classificação do achado “`TraceManager::errorMessages()` não populada por `traceError`”: **DONE**.
- Classificação do achado “`TraceManager` sem teardown efetivo”: **SUPERSEDED**.
- Classificação de risco residual de ownership: **PARTIAL**.

### Evidence
- Ambos overloads de `traceError` inserem texto em `_errorMessages`.
- `TraceManager` possui destrutor explícito liberando listas de handlers, regras de exceção e `_errorMessages`.
- Há cobertura de teste unitário para armazenamento de mensagens de erro em `source/tests/unit/test_support_tracemanager.cpp`.

### Remaining gaps
- Estrutura ainda centrada em ponteiros crus e listas alocadas manualmente; teardown existe, mas RAII integral ainda não foi adotado.

---

## 2) Top 10 riscos priorizados (Fase 2) — Reaudit

### Reclassificação dos itens históricos
1. **`PluginManager::insert` pode retornar ponteiro inválido** → **DONE**.
2. **`ModelSimulation::_createSimulationEvent` com ownership indefinido** → **DONE**.
3. **`Model::hasChanged` usa `&=` em cascata** → **DONE** (agregação atual com `||`).
4. **`Model::createEntity` ignora parâmetro `insertIntoModel`** → **OPEN** (implementação ainda cria `new Entity(this, name, true)`).
5. **`Model` com destrutor default + múltiplas alocações** → **SUPERSEDED** (destrutor explícito implementado).
6. **`ModelSimulation` com alocações sem estratégia explícita de teardown** → **SUPERSEDED** (destrutor explícito implementado).
7. **`PluginManager` sem teardown explícito** → **SUPERSEDED** (destrutor explícito implementado).
8. **`TraceManager::errorMessages()` não populada por `traceError`** → **DONE**.
9. **`PluginManager::autoInsertPlugins` com normalização frágil de linha/BOM** → **OPEN**.
10. **TODOs críticos no fluxo de simulação/report** → **UNCERTAIN** (há TODOs, mas impacto operacional atual requer validação direcionada por teste comportamental).

### Validade atual do “Top 10” original
- O top 10 histórico **não permanece válido como lista de prioridade atual**: a maior parte dos itens P0/P1 foi corrigida ou superada por mudanças posteriores.
- Itens realmente vivos após reauditoria concentram-se em:
  - `Model::createEntity` e contrato de `insertIntoModel`.
  - Robustez de parsing em `autoInsertPlugins`.
  - TODOs com possível impacto em cenários avançados (ainda sem confirmação de severidade).

---

## 3) Backlog imediato (Task A/B/C) — Reaudit

### Situação do backlog histórico
- **Task A (corrigir retorno de `PluginManager::insert`)** → **DONE**.
- **Task B (ownership explícito de `SimulationEvent`)** → **DONE**.
- **Task C (corrigir `Model::hasChanged`)** → **DONE**.

### Novo foco recomendado para backlog imediato
- **Task A2 (OPEN)** — Corrigir/confirmar contrato de `Model::createEntity(name, insertIntoModel)` para respeitar o parâmetro recebido.
- **Task B2 (OPEN)** — Hardening de `PluginManager::autoInsertPlugins` (normalização robusta de linha, BOM e espaços de controle).
- **Task C2 (UNCERTAIN)** — Converter TODOs de simulação/report em casos de teste focados para medir risco real antes de classificar severidade.

---

## 4) Estratégia de execução recomendada (revisada)
1. **Contrato funcional primeiro (OPEN):** fechar `createEntity(insertIntoModel)` com teste de regressão específico.
2. **Robustez de entrada (OPEN):** endurecer parsing de `autoInsertPlugins` e cobrir com testes de arquivo contendo BOM/linhas anômalas.
3. **Risco exploratório orientado por teste (UNCERTAIN):** transformar TODOs críticos em hipóteses testáveis antes de qualquer refatoração maior.
4. **Evolução técnica incremental (PARTIAL):** migrar gradualmente para RAII onde ainda existe teardown manual em classes core.

## 5) Checkpoints de validação
- Build:
  - `cmake --preset debug`
  - `cmake --build --preset debug --target genesys_kernel genesys_tests -j4`
- Testes:
  - `ctest --preset debug`
  - incluir regressões para `createEntity(insertIntoModel)` e `autoInsertPlugins`
- Sanitizers:
  - `cmake --preset asan && cmake --build --preset asan --target genesys_tests -j4`
  - `ctest --preset asan`

## 6) Classificação de confiabilidade (revisada)
- **Confirmado no código (DONE/SUPERSEDED):**
  - `PluginManager::insert` já retorna `nullptr` em falha de `_insert`.
  - `_createSimulationEvent` usa `std::unique_ptr<SimulationEvent>`.
  - `Model::hasChanged` agrega por `||`.
  - `Model`, `ModelSimulation`, `PluginManager` e `TraceManager` possuem destrutores explícitos com teardown relevante.
  - `TraceManager::traceError` alimenta `_errorMessages` em ambos overloads.
- **Confirmado no código (OPEN):**
  - `Model::createEntity` ainda ignora o parâmetro `insertIntoModel` na chamada efetiva do construtor de `Entity`.
  - Normalização em `PluginManager::autoInsertPlugins` permanece limitada a tratamento manual de primeiro caractere.
- **Cobertura de regressão já existente (parcial):**
  - `TraceManager` já possui testes unitários cobrindo armazenamento em `errorMessages`.
  - `Model::hasChanged` possui teste de comportamento no runtime unitário.
- **Hipótese a validar (UNCERTAIN):**
  - Severidade prática dos TODOs remanescentes no fluxo de simulação/report em cenários avançados.

## Remaining Work
- **OPEN**
  - `Model::createEntity(name, insertIntoModel)` respeitar o parâmetro recebido e não forçar `true`.
  - Hardening de `PluginManager::autoInsertPlugins` para normalização robusta de entrada.
- **PARTIAL**
  - Evolução incremental para RAII/smart pointers em áreas ainda dependentes de teardown manual.
- **UNCERTAIN**
  - Impacto real dos TODOs do fluxo de simulação/report (necessita validação orientada por teste).
