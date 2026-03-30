# Revisão de possíveis leaks de memória no pacote `source/kernel` (2026-03-30)

## Escopo e método

- Busca textual por alocações manuais (`new`) e liberações (`delete`) em `source/kernel`.
- Inspeção manual de classes com ponteiros proprietários (`owner`) e destrutor `= default`.
- Foco principal: risco de vazamento por ausência de `delete`, substituição de ponteiro sem liberar antigo e falta de RAII.

## Achados principais (alto risco)

### 1) `SimulationScenario` acumula alocações sem desalocação

**Evidências**
- A classe possui múltiplos membros alocados com `new` e destrutor default (`virtual ~SimulationScenario() = default`).
- `setSelectedControls()` cria uma nova lista e sobrescreve `_selectedControls` sem liberar a anterior.
- `setControl()` aloca `pair` com `new` e apenas armazena na lista.

**Risco**
- Vazamento persistente por cenário criado.
- Vazamento incremental a cada chamada de `setSelectedControls()`.

**Solução sugerida**
- Curto prazo: implementar destrutor explícito liberando `_selectedControls`, `_selectedResponses`, `_controlValues`, `_responseValues` e os elementos apontados pelas listas de `pair`.
- Médio prazo: migrar para RAII (`std::list<std::pair<std::string,double>>` por valor, e listas por valor em vez de ponteiros).

---

### 2) `ModelSimulation` possui objetos owner alocados com `new` sem destruição explícita

**Evidências**
- Membros `_simulationReporter`, `_cstatsAndCountersSimulation`, `_cstatsAndCountersMapSimulation`, `_breakpointsOnTime`, `_breakpointsOnComponent`, `_breakpointsOnEntity` são alocados com `new`.
- Destrutor da classe é `= default`.

**Risco**
- Vazamento por instância de `ModelSimulation` (objetos de longa vida do kernel).

**Solução sugerida**
- Curto prazo: destrutor explícito com `delete` de todos os owners.
- Melhor prática: trocar membros para `std::unique_ptr<>` (ou ainda melhor, membros por valor quando possível).

---

### 3) `TraceManager` aloca várias listas e `_errorMessages` sem destrutor explícito

**Evidências**
- Diversos handlers são `new List<...>()` no header.
- `_errorMessages` é alocado no construtor.
- Destrutor é `= default`.

**Risco**
- Vazamento por ciclo de vida do `Simulator`.

**Solução sugerida**
- Curto prazo: adicionar destrutor para liberar todos os members owner.
- Médio prazo: usar composição por valor (`List<T> _traceHandlers;`) ou `std::unique_ptr<List<T>>` quando necessário.

---

### 4) `ModelManager` mantém `_models` com `new` e destrutor default

**Evidências**
- `_models` é alocado com `new` no header.
- Destrutor `= default`.
- A remoção individual (`remove`) faz `delete model`, porém não há cleanup global no encerramento do manager.

**Risco**
- Vazamento da estrutura `_models` e potencialmente de modelos remanescentes não removidos explicitamente.

**Solução sugerida**
- Implementar destrutor que itera e destrói modelos restantes e em seguida destrói `_models`.
- Alternativa robusta: `List<std::unique_ptr<Model>>`.

---

### 5) `StatisticsDatafileDefaultImpl1` tem owners com `new` e destrutor default

**Evidências**
- `_collector`, `_collectorSorted` e `sort` são alocados com `new`.
- Destrutor default.

**Risco**
- Vazamento em cada instância do coletor estatístico.

**Solução sugerida**
- Criar destrutor liberando os três ponteiros owner.
- Evolução: substituir por `std::unique_ptr`.

---

### 6) `SamplerDefaultImpl1` aloca `_param` com `new` e destrutor default

**Evidências**
- `_param = new DefaultImpl1RNG_Parameters();` no header.
- Destrutor default.

**Risco**
- Vazamento de parâmetros de RNG em cada instância do sampler.

**Solução sugerida**
- Liberar `_param` no destrutor.
- Preferível: armazenar `DefaultImpl1RNG_Parameters` por valor ou `std::unique_ptr<RNG_Parameters>` com ownership explícito.

## Recomendações de hardening (priorizadas)

1. **Padronizar ownership**: owner deve ser `std::unique_ptr` (ou valor), observer deve ser ponteiro cru/referência.
2. **Eliminar `new` em inicialização de atributo** quando a classe pode conter o objeto por valor.
3. **Regra de projeto**: toda classe com ponteiro owner precisa de um dos dois:
   - destrutor explícito com liberação, ou
   - smart pointer com deleção automática.
4. **Executar sanitizers no CI** para detectar regressões:
   - `-fsanitize=address,leak,undefined`
   - suíte mínima de smoke tests e unit tests.
5. **Adicionar checklist de PR**: “introduziu `new`? Onde está o owner e a estratégia de liberação?”.

## Resumo executivo

Há sinais concretos de risco de leak no kernel, principalmente em classes com muitos members alocados com `new` e destrutor default. O caminho mais seguro é migrar gradualmente para RAII (`std::unique_ptr`/valor), começando por `SimulationScenario`, `ModelSimulation`, `TraceManager`, `ModelManager`, `StatisticsDatafileDefaultImpl1` e `SamplerDefaultImpl1`.
