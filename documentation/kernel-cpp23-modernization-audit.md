# Auditoria C++23 no diretório `/source/kernel`

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: C++23/kernel modernization audit revalidated against current code in `source/kernel`
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`

## Objetivo
Aplicar a mesma estratégia usada em `ModelSimulation` para o restante de `/kernel`: identificar padrões legados (laços com iterador explícito, casts antigos e ownership manual) e manter uma modernização segura e incremental com revalidação periódica.

## Varredura inicial (automática) — histórico
Comandos registrados na auditoria original:

- `rg -n "std::list<.*>::iterator" source/kernel --glob '*.{cpp,h}'`
- `rg -n "\\([A-Za-z_][A-Za-z0-9_:<>\\* ]*\\)\\s*[A-Za-z_][A-Za-z0-9_]*" source/kernel --glob '*.{cpp,h}'`
- `rg -n "new [A-Za-z_][A-Za-z0-9_]*\\(" source/kernel --glob '*.{cpp,h}'`

Contagens históricas registradas:

- **59** ocorrências de `std::list<...>::iterator`.
- **98** ocorrências candidatas a cast estilo C.
- **80** ocorrências de `new`.

### Audit status
`PARTIAL` — a seção continua útil como baseline histórico, mas não representa o estado atual sem recontagem.

### Evidence
Reexecução local na árvore atual (mesmos padrões):

- `std::list<...>::iterator`: **33** ocorrências.
- candidatos a cast estilo C: **118** ocorrências (busca ampla, com falsos positivos esperados).
- `new`: **80** ocorrências.

### Remaining gaps
- As contagens históricas de iteradores/casts não são mais reproduzidas exatamente; devem ser tratadas como fotografia inicial.
- A busca de cast por regex continua exigindo triagem semântica para separar risco real de ruído.

## Arquivos com maior concentração de iteração legada
Top histórico (auditoria original):

1. `source/kernel/simulator/TraceManager.cpp` (12)
2. `source/kernel/util/List.h` (9)
3. `source/kernel/util/ListObservable.h` (7)
4. `source/kernel/simulator/SimulationReporterDefaultImpl1.cpp` (5)
5. `source/kernel/simulator/ModelCheckerDefaultImpl1.cpp` (5)
6. `source/kernel/simulator/Model.cpp` (5)
7. `source/kernel/simulator/OnEventManager.cpp` (4)

### Audit status
`PARTIAL` — a lista mantém valor histórico, mas não é mais representativa como ranking atual.

### Evidence
Ranking atual por `std::list<...>::iterator`:

1. `source/kernel/util/List.h` (9)
2. `source/kernel/util/ListObservable.h` (7)
3. `source/kernel/simulator/ModelCheckerDefaultImpl1.cpp` (5)
4. `source/kernel/simulator/Model.cpp` (4)
5. `source/kernel/simulator/ModelSimulation.cpp` (3)
6. `source/kernel/simulator/ComponentManager.h` (3)

Reavaliação explícita dos arquivos solicitados:

- `TraceManager.cpp` → `SUPERSEDED` como “concentrador”: não há ocorrência atual de `std::list<...>::iterator` no arquivo.
- `Model.cpp` → `PARTIAL`: ainda relevante (4 ocorrências), porém abaixo da concentração histórica.
- `List.h` → `OPEN`: segue como concentração alta e padrão legado estrutural.
- `ListObservable.h` → `OPEN`: segue como concentração alta e padrão legado estrutural.

### Remaining gaps
- Priorização futura deve usar o ranking atual, mantendo o ranking histórico apenas como contexto.

## Modernizações aplicadas neste lote (revalidadas)

### 1) `OnEventManager.cpp`
- Histórico: laços com iterador explícito substituídos por `range-based for` em notificações.
- Estado atual: confirmado (`_NotifyHandlers` e `_NotifyHandlerMethods` seguem em `range-based for`).

#### Audit status
`DONE`

#### Evidence
- Iteração por elemento em notificações sem `std::list<...>::iterator` no `.cpp`.

#### Remaining gaps
- Nenhum gap direto deste item.

### 2) `TraceManager.cpp`
- Histórico: iterações de handlers migradas para `range-based for`.
- Estado atual: confirmado em `trace`, `traceError`, `traceSimulation` e `traceReport`.

#### Audit status
`DONE`

#### Evidence
- Loops de handlers permanecem em `range-based for`.
- Sem ocorrência de `std::list<...>::iterator` no arquivo.

#### Remaining gaps
- Nenhum gap direto deste item.

### 3) `PluginManager.cpp`
- Histórico: `find` modernizado para iteração por elemento e retorno direto.
- Estado atual: confirmado (`find` usa `for (Plugin* plugin : *this->_plugins->list())`).

#### Audit status
`DONE`

#### Evidence
- Iteração por elemento consolidada em `find` e em outros fluxos de varredura.

#### Remaining gaps
- Nenhum gap direto deste item.

### 4) `ModelDataManager.cpp`
- Histórico: `show`, `getDataDefinition` e `getRankOf` modernizados; structured bindings em iteração de mapa.
- Estado atual: modernização majoritária confirmada, porém há laço legado remanescente em `getNumberOfDataDefinitions()`.

#### Audit status
`PARTIAL`

#### Evidence
- `show()` usa `for (const auto& [typenameKey, definitions] : *_datadefinitions)`.
- `getNumberOfDataDefinitions()` ainda usa `std::map<...>::iterator` explícito.

#### Remaining gaps
- Converter o laço residual para iteração moderna e fechar o arquivo como `DONE`.

## Riscos ainda existentes (reclassificados)

### 1) Ownership manual em headers com `= new` em membros
#### Audit status
`OPEN`

#### Evidence
- Padrão ainda presente em múltiplos headers (ex.: `OnEventManager.h`, `TraceManager.h`, `PluginManager.h`, `ConnectionManager.h`, entre outros).
- Recontagem atual no escopo `source/kernel/*.h`: **82** ocorrências de `= new`.

#### Remaining gaps
- Backlog real de migração gradual para RAII/smart pointers e simplificação de destrutores manuais.

### 2) Containers utilitários legados (`List.h`, `ListObservable.h`)
#### Audit status
`OPEN`

#### Evidence
- Ambos permanecem no topo de concentração de `std::list<...>::iterator`.
- Estrutura ainda depende de alocação manual (`_list = new std::list<T>()`) e cursor interno mutável (`_it`).

#### Remaining gaps
- Lote dedicado para modernização estrutural desses containers (API, ownership e iteração), com mitigação de regressão.

### 3) Casts estilo C / necessidade de triagem manual
#### Audit status
`UNCERTAIN`

#### Evidence
- Busca textual ampla retornou **118** candidatos no estado atual.
- Regex inclui falso positivo; sem triagem semântica não há classificação confiável por risco/arquivo.

#### Remaining gaps
- Executar triagem semântica (ex.: clang-tidy/checks específicos) antes de transformar o número bruto em plano de ação.

## Conclusão
A auditoria original permanece útil como registro da primeira varredura, mas deve ser lida como documento híbrido (histórico + revalidação). As modernizações em `OnEventManager.cpp`, `TraceManager.cpp` e `PluginManager.cpp` estão consolidadas (`DONE`); `ModelDataManager.cpp` permanece `PARTIAL`; e o backlog real concentra-se em ownership manual de headers, containers legados e triagem semântica de casts.

Itens históricos superados por mudanças posteriores:
- `TraceManager.cpp` como “maior concentração” de iteração legada (`SUPERSEDED` no ranking atual).
- Contagem histórica `std::list<...>::iterator = 59` como estado corrente (`SUPERSEDED` por recontagem atual = 33).

## Remaining Work
- `PARTIAL` — `ModelDataManager.cpp`: remover iteração legada residual em `getNumberOfDataDefinitions()`.
- `OPEN` — Ownership manual em headers com `= new` (migração para RAII/smart pointers).
- `OPEN` — Modernização estrutural de `List.h` e `ListObservable.h`.
- `UNCERTAIN` — Triagem semântica de candidatos a cast estilo C (resultado textual isolado não conclusivo).
