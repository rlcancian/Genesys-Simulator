# Phase 2 — Inventário técnico do kernel/simulator (2026-1)

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
**Confirmado no código**
- `Model` aloca em heap e mantém ponteiros crus para quase todos os subsistemas (`_eventManager`, `_modeldataManager`, `_componentManager`, `_simulation`, `_parser`, `_modelChecker`, `_modelPersistence`, `_futureEvents`, `_controls`, `_responses`, `_modelInfo`).
- O destrutor está como `= default` no header, sem lógica de liberação explícita.

**Implicação**
- A semântica de ownership é “Model possui tudo”, mas a destruição atual não deixa isso explícito.
- Forte indício de vazamento por ciclo de vida longo do processo (ou limpezas incompletas em cenários de criação/destruição de múltiplos modelos).

### 1.2 ModelSimulation
**Confirmado no código**
- `ModelSimulation` cria controles com `new` e insere em `Model::getControls()`.
- Também mantém membros alocados dinamicamente no próprio header (`_cstatsAndCountersSimulation`, `_cstatsAndCountersMapSimulation`, `_breakpointsOnTime`, `_breakpointsOnComponent`, `_breakpointsOnEntity`).
- Método `_createSimulationEvent()` cria `SimulationEvent*` com `new` e retorna ponteiro cru sem ownership explícito.

**Implicação**
- Ownership de `SimulationEvent` é ambíguo (forte indício de leak em callbacks/event handlers).
- Alocações no header e ausência de destrutor customizado aumentam risco de leak.

### 1.3 PluginManager
**Confirmado no código**
- `_plugins` e `_pluginConnector` são criados com `new` e não há destrutor explícito para liberar recursos.
- `insert()` retorna `plugin` mesmo quando `_insert(plugin)` falha; o próprio código já comenta risco de uso após liberação.

**Implicação**
- Bug potencial de ponteiro inválido (use-after-free) para chamadores de `insert()`.
- Lifecycle de plugin connector e lista de plugins depende de side effects externos.

### 1.4 TraceManager
**Confirmado no código**
- Diversas listas de handlers e `_errorMessages` são alocadas com `new`.
- `traceError(...)` não adiciona mensagens em `_errorMessages`, apesar de expor `errorMessages()`.

**Implicação**
- API sugere buffer de erros, mas não o alimenta (inconsistência funcional).
- Forte indício de leak por ausência de destrutor explicitando limpeza.

---

## 2) Top 10 riscos priorizados (Fase 2)

### P0 (alto impacto / provável)
1. **`PluginManager::insert` pode retornar ponteiro inválido** quando `_insert(plugin)` falha e destrói o plugin.
2. **`ModelSimulation::_createSimulationEvent` com ownership indefinido** (alocação heap sem contrato de destruição).
3. **`Model::hasChanged` usa `&=` em cascata**, podendo mascarar mudanças (lógica aparentemente invertida para agregação de flags).
4. **`Model::createEntity` ignora parâmetro `insertIntoModel`** e sempre cria com `true`.

### P1 (médio)
5. **`Model` com destrutor default + múltiplas alocações** (ownership implícito e difícil de auditar).
6. **`ModelSimulation` com alocações em membros sem estratégia explícita de teardown**.
7. **`PluginManager` sem teardown explícito de `_pluginConnector` e `_plugins`**.
8. **`TraceManager::errorMessages()` expõe estrutura não populada por `traceError`**.

### P2 (baixo/moderado)
9. **`PluginManager::autoInsertPlugins` trata BOM/caracteres de controle manualmente** sem normalização robusta da linha inteira.
10. **TODOs críticos no fluxo de simulação/report** que podem ocultar comportamento incompleto em cenários avançados.

---

## 3) Backlog imediato (3 primeiras entregas com teste)

### Task A (P0) — Corrigir retorno de `PluginManager::insert`
- Ajustar `insert()` para retornar `nullptr` quando `_insert(plugin)` falhar.
- Garantir contrato claro: retorno não nulo implica plugin válido/registrado.
- **Teste de regressão sugerido**: simular plugin inválido e verificar retorno `nullptr` + ausência na lista.

### Task B (P0) — Tornar ownership de `SimulationEvent` explícito
- Opção mínima segura: converter `_createSimulationEvent` para retorno por valor (se viável sem quebra) ou adotar destruição explícita no mesmo frame de notificação.
- **Teste de regressão sugerido**: validar que notificações continuam com estado correto (paused/running/time) em start/pause/step.

### Task C (P0/P1) — Corrigir lógica de `Model::hasChanged`
- Substituir cadeia com `&=` por agregação OR (`||`) com semântica “qualquer subsistema mudou”.
- **Teste de regressão sugerido**: marcar mudança em apenas 1 subsistema e esperar `true`.

---

## 4) Estratégia de execução recomendada
1. **Fixes de contrato (Task A e C) primeiro**: menor impacto arquitetural e alto ganho de segurança.
2. **Depois ownership/eventos (Task B)**: requer mapeamento de listeners e validação em testes de suporte/runtime.
3. **Por fim hardening de teardown (Model/ModelSimulation/PluginManager/TraceManager)** com migração incremental para RAII/regra do zero.

## 5) Checkpoints de validação
- Build:
  - `cmake --preset debug`
  - `cmake --build --preset debug --target genesys_kernel genesys_tests -j4`
- Testes:
  - `ctest --preset debug`
  - novos testes de regressão para A/B/C
- Sanitizers:
  - `cmake --preset asan && cmake --build --preset asan --target genesys_tests -j4`
  - `ctest --preset asan`

## 6) Classificação de confiabilidade
- **Confirmado no código**: alocações em heap com ponteiros crus nas classes foco; retorno potencialmente inválido em `PluginManager::insert`; inconsistência funcional de `TraceManager::errorMessages`.
- **Forte indício**: leaks associados a eventos/listas sem teardown explícito.
- **Hipótese a validar**: impacto completo de mudar ownership de `SimulationEvent` sobre todos os listeners existentes.
