# Revisão de possíveis leaks de memória no pacote `source/kernel` (2026-03-30)

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: revisão do documento de leaks revalidada contra o estado atual do kernel
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`
- Data desta reauditoria documental: `2026-04-10`

## Escopo e método da reauditoria

- Leitura do documento histórico original e reclassificação item a item.
- Revalidação direta dos pontos no código atual (headers + implementações) das classes citadas.
- Foco em ownership real no estado atual: destrutores, ponteiros owner, substituição de ponteiros e limpeza global.
- Esta atualização preserva o valor histórico dos achados, mas distingue explicitamente o que era risco em 2026-03-30 e o que permanece risco **hoje**.

## Achados principais auditados

### 1) `SimulationScenario` acumula alocações sem desalocação

**Registro histórico (2026-03-30)**
- O review anterior apontava destrutor default, alocações com `new` e possível leak incremental em `setSelectedControls()`.

### Audit status
`PARTIAL`

### Evidence
- `SimulationScenario` agora declara destrutor explícito (`virtual ~SimulationScenario();`) e implementa cleanup de `_controlValues`, `_responseValues`, `_selectedControls`, `_selectedResponses`, incluindo deleção dos `pair` internos nas listas de valores.
- `setSelectedControls()` foi atualizado para copiar a lista recebida, liberar `_selectedControls` anterior e substituir o ponteiro, removendo o leak incremental antes descrito.
- Ainda há alocação dinâmica em `setControl()` (`new std::pair<...>`), mas os objetos passam a ser desalocados no destrutor.

### Remaining gaps
- Embora o leak principal tenha sido mitigado, a classe permanece com ownership manual por ponteiros crus e sem RAII por valor/`unique_ptr`.
- Não há limpeza pontual de itens em ciclos longos antes da destruição do objeto (trade-off de memória de runtime, mesmo sem leak no shutdown).

---

### 2) `ModelSimulation` possui objetos owner alocados com `new` sem destruição explícita

**Registro histórico (2026-03-30)**
- O review anterior descrevia destrutor default com múltiplos members owner alocados por `new`.

### Audit status
`PARTIAL`

### Evidence
- `ModelSimulation` hoje declara destrutor explícito e libera `_ownedControls`, `_cstatsAndCountersSimulation` (com deleção dos elementos), `_cstatsAndCountersMapSimulation`, `_breakpointsOnTime`, `_breakpointsOnComponent`, `_breakpointsOnEntity`.
- O reporter (`_simulationReporter`) também é desalocado quando `_ownsSimulationReporter` é verdadeiro.
- Há lógica explícita para remover controles da lista do modelo antes de deletá-los, reduzindo risco de dangling internos.

### Remaining gaps
- Ownership ainda depende de disciplina manual (`new`/`delete` + flags de ownership), mantendo superfície de risco de regressão.
- Migração para RAII continua recomendada para diminuir risco futuro de double free/use-after-free em evolução de código.

---

### 3) `TraceManager` aloca várias listas e `_errorMessages` sem destrutor explícito

**Registro histórico (2026-03-30)**
- O review anterior apontava destrutor default e leak provável em handlers/listas auxiliares.

### Audit status
`DONE`

### Evidence
- `TraceManager` agora declara destrutor virtual explícito.
- Implementação atual destrói explicitamente `_traceHandlers`, `_traceErrorHandlers`, `_traceReportHandlers`, `_traceSimulationHandlers`, handlers-method correspondentes, `_traceSimulationExceptionRule` e `_errorMessages`.

### Remaining gaps
- Nenhum leak direto do achado histórico permaneceu demonstrável neste ponto.
- O backlog arquitetural de RAII ainda pode existir como melhoria, mas não como pendência deste leak específico.

---

### 4) `ModelManager` mantém `_models` com `new` e destrutor default

**Registro histórico (2026-03-30)**
- O review anterior indicava falta de cleanup global, com risco sobre `_models` e modelos remanescentes.

### Audit status
`DONE`

### Evidence
- `ModelManager` agora declara destrutor virtual explícito.
- Destrutor atual percorre `_models` e destrói modelos remanescentes.
- Também trata o caso de `_currentModel` “destacado” (não pertencente a `_models`) e o desaloca.
- Depois libera `_models` e neutraliza ponteiros.

### Remaining gaps
- O risco principal descrito no review antigo foi efetivamente eliminado no estado atual.

---

### 5) `StatisticsDatafileDefaultImpl1` tem owners com `new` e destrutor default

**Registro histórico (2026-03-30)**
- O documento antigo citava `StatisticsDatafileDefaultImpl1` com `_collector`, `_collectorSorted` e `sort` alocados com `new`.

### Audit status
`SUPERSEDED`

### Evidence
- No estado atual auditado, não foi encontrado artefato `StatisticsDatafileDefaultImpl1` no repositório.
- O artefato relacionado existente é `CollectorDatafileDefaultImpl1`.
- Em `CollectorDatafileDefaultImpl1`, os membros são por valor (`_filename`, `_lastValue`, `_numElements`, `_nextReadIndex`) e não reproduzem o padrão de owners com `new` descrito no texto histórico.
- O destrutor default da implementação atual não implica, por si só, o risco específico anteriormente descrito.

### Remaining gaps
- Não há evidência, no snapshot atual, de leak equivalente ao claim histórico dessa seção.
- Sem histórico de rename/refactor no próprio documento, não é possível afirmar com 100% de certeza se houve renomeação direta da classe antiga ou substituição arquitetural completa.

---

### 6) `SamplerDefaultImpl1` aloca `_param` com `new` e destrutor default

**Registro histórico (2026-03-30)**
- O review anterior descrevia `_param` como owner sem destruição explícita.

### Audit status
`PARTIAL`

### Evidence
- `SamplerDefaultImpl1` agora declara destrutor explícito e libera `_param` quando `_ownsParam` é verdadeiro.
- `setRNGparameters()` também libera `_param` previamente owned antes de substituir por parâmetro externo e ajustar `_ownsParam=false`.

### Remaining gaps
- Ownership ainda é manual (ponteiro cru + flag `_ownsParam`), mantendo risco de manutenção futura.
- RAII por `std::unique_ptr<RNG_Parameters>` (ou objeto por valor onde possível) continua sendo recomendação válida de hardening.

## Recomendações de hardening (estado atual)

1. Manter prioridade na migração gradual de owners manuais para RAII (`std::unique_ptr` ou composição por valor).
2. Para classes ainda em `PARTIAL` (`SimulationScenario`, `ModelSimulation`, `SamplerDefaultImpl1`), reduzir ponteiros owner crus como backlog técnico explícito.
3. Preservar regra de projeto: novo `new` deve vir com ownership claro e estratégia de liberação verificável.
4. Continuar uso de sanitizers (ASan/LSan/UBSan) em CI para prevenir regressões de leak/lifetime.

## Resumo executivo reclassificado

- Itens antes tratados como “alto risco atual” em `TraceManager` e `ModelManager` foram **corrigidos** no estado atual (`DONE`).
- `SimulationScenario`, `ModelSimulation` e `SamplerDefaultImpl1` não sustentam mais o leak direto descrito originalmente, mas ainda dependem de ownership manual (`PARTIAL`).
- O item de `StatisticsDatafileDefaultImpl1` foi **superado por mudança posterior/obsolescência de artefato** (`SUPERSEDED`), com equivalente atual (`CollectorDatafileDefaultImpl1`) sem o padrão de leak descrito.
- No recorte auditado deste documento, não houve item classificado como `OPEN`.

## Remaining Work

- `PARTIAL` — `SimulationScenario`: migrar ownership manual de listas/pares para RAII por valor ou smart pointers, reduzindo risco de regressão.
- `PARTIAL` — `ModelSimulation`: reduzir owners manuais (`new`/`delete`) e flags de ownership para modelo RAII.
- `PARTIAL` — `SamplerDefaultImpl1`: trocar ponteiro cru `_param` + `_ownsParam` por contrato RAII explícito.
- `UNCERTAIN` — rastreabilidade histórica entre `StatisticsDatafileDefaultImpl1` (documento antigo) e `CollectorDatafileDefaultImpl1` (estado atual): confirmar por histórico de commits/refactor quando necessário.
