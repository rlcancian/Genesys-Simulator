# Auditoria C++23 no diretório `/source/kernel`

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: C++23/kernel modernization audit revalidated against current code in `source/kernel` (iteração legada, ownership manual, casts candidatos)
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`
- Data desta reauditoria documental: `2026-04-10`

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
`PARTIAL` — a seção continua útil como baseline histórico, mas não representa o estado atual sem recontagem exata equivalente.

### Evidence
Reexecução local na árvore atual (mesmos padrões aproximados, com possível variação por ruído de regex):

- `std::list<...>::iterator`: **33** ocorrências.
- candidatos a cast estilo C: **118** ocorrências (busca ampla, com falsos positivos esperados).
- `= new` em headers `source/kernel`: concentração ainda alta, incluindo `OnEventManager.h`, `TraceManager.h`, `PluginManager.h`, `ConnectionManager.h` e outros.

### Remaining gaps
- As contagens históricas devem ser tratadas como fotografia antiga (não revalidada numericamente com exatidão absoluta).
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
`PARTIAL` — o ranking histórico segue valioso para rastreabilidade, mas não é mais representativo como ranking atual.

### Evidence
Ranking atual aproximado por `std::list<...>::iterator` indica maior concentração em:

1. `source/kernel/util/List.h`
2. `source/kernel/util/ListObservable.h`
3. `source/kernel/simulator/ModelCheckerDefaultImpl1.cpp`
4. `source/kernel/simulator/Model.cpp`

Reavaliação explícita dos itens históricos solicitados:

- `OnEventManager.cpp` → `DONE`.
- `TraceManager.cpp` → `DONE` (e `SUPERSEDED` como “arquivo mais concentrado em iteração legada”).
- `PluginManager.cpp` → `DONE`.
- `ModelDataManager.cpp` → `PARTIAL`.

### Remaining gaps
- Priorização futura deve usar ranking atual.
- Ranking histórico deve permanecer apenas como contexto documental.

## Modernizações aplicadas neste lote (revalidadas)

### 1) `OnEventManager.cpp`
- Histórico: laços com iterador explícito substituídos por `range-based for` em notificações.
- Estado atual: confirmado (`_NotifyHandlers` e `_NotifyHandlerMethods` seguem em `range-based for`).

#### Audit status
`DONE`

#### Evidence
- Loops de notificação usam `for (auto ...)`/`for (auto& ...)` sobre `*list->list()`.

#### Remaining gaps
- Nenhum gap direto do item.

### 2) `TraceManager.cpp`
- Histórico: iterações de handlers migradas para `range-based for`.
- Estado atual: confirmado em `trace`, `traceError`, `traceSimulation` e `traceReport`.

#### Audit status
`DONE`

#### Evidence
- Loops de handlers usam `for (auto handler : ...)` e `for (auto& handlerMethod : ...)`.
- Sem ocorrência de `std::list<...>::iterator` no arquivo.

#### Remaining gaps
- Nenhum gap direto do item.

### 3) `PluginManager.cpp`
- Histórico: `find` modernizado para iteração por elemento e retorno direto.
- Estado atual: confirmado (`find` usa `for (Plugin* plugin : *this->_plugins->list())`).

#### Audit status
`DONE`

#### Evidence
- Iteração em `find`, `_autoFindPlugins`, `show` e destrutor com range-based for.

#### Remaining gaps
- Nenhum gap direto do item.

### 4) `ModelDataManager.cpp`
- Histórico: `show`, `getDataDefinition` e `getRankOf` modernizados; structured bindings em iteração de mapa.
- Estado atual: modernização majoritária confirmada, porém há laço legado remanescente.

#### Audit status
`PARTIAL`

#### Evidence
- `show()` usa `for (const auto& [typenameKey, definitions] : *_datadefinitions)`.
- `getNumberOfDataDefinitions()` ainda usa `std::map<...>::iterator` explícito.

#### Remaining gaps
- Converter laço residual para iteração moderna e fechar o arquivo como `DONE`.

## Riscos ainda existentes (reclassificados)

### 1) Ownership manual em headers com `= new` em membros
#### Audit status
`OPEN`

#### Evidence
- Padrão ainda recorrente em múltiplos headers do kernel.
- Continua exigindo destrutores manuais e disciplina de lifecycle.

#### Remaining gaps
- Backlog real de migração gradual para RAII/smart pointers.

### 2) Containers utilitários legados (`List.h`, `ListObservable.h`)
#### Audit status
`OPEN`

#### Evidence
- Ambos seguem entre os arquivos com maior concentração de iteração legada.
- Estrutura ainda depende de ownership manual (`_list = new std::list<T>()`) e cursor interno mutável (`_it`).

#### Remaining gaps
- Lote dedicado para modernização estrutural desses containers (API + ownership + iteração).

### 3) Casts estilo C / necessidade de triagem manual
#### Audit status
`UNCERTAIN`

#### Evidence
- Busca textual ampla retornou volume relevante de candidatos.
- A regex de triagem inclui falsos positivos.

#### Remaining gaps
- Executar triagem semântica (ex.: clang-tidy/checks específicos) antes de priorizar correções.

## Conclusão
A auditoria original permanece útil como registro da primeira varredura, mas deve ser lida como documento híbrido (histórico + revalidação). As modernizações em `OnEventManager.cpp`, `TraceManager.cpp` e `PluginManager.cpp` estão consolidadas (`DONE`); `ModelDataManager.cpp` permanece `PARTIAL`; e o backlog real concentra-se em ownership manual de headers, containers legados e triagem semântica de casts.

Itens históricos superados por mudanças posteriores:
- `TraceManager.cpp` como “maior concentração” de iteração legada (`SUPERSEDED` no ranking atual).
- Contagem histórica `std::list<...>::iterator = 59` como estado corrente (`SUPERSEDED` por recontagem atual aproximada).

## Remaining Work
- `PARTIAL` — `ModelDataManager.cpp`: remover iteração legada residual em `getNumberOfDataDefinitions()`.
- `OPEN` — Ownership manual em headers com `= new` (migração para RAII/smart pointers).
- `OPEN` — Modernização estrutural de `List.h` e `ListObservable.h`.
- `UNCERTAIN` — Triagem semântica de candidatos a cast estilo C (resultado textual isolado não conclusivo).
