# Auditoria C++23 no diretório `/source/kernel`

## Audit Status (WiP20261)
- Branch audited: `WiP20261` (requested scope; local branch name may differ)
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

### Evidence (revalidação atual)
Reexecução local com os mesmos padrões (na árvore atual):

- `std::list<...>::iterator`: **33** ocorrências.
- candidatos a cast estilo C: **118** ocorrências (busca ampla, com falsos positivos esperados).
- `new`: **80** ocorrências.

### Remaining gaps
- A contagem histórica não é mais reproduzida exatamente para iteradores/casts; deve ser tratada como fotografia antiga.
- A busca de casts continua exigindo triagem semântica (regex isolada não separa casos reais de ruído).

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
`PARTIAL` — a lista preserva contexto histórico, mas não é mais representativa como “top atual”.

### Evidence (revalidação atual)
Top atual por `std::list<...>::iterator` (mesmo comando de busca):

1. `source/kernel/util/List.h` (9)
2. `source/kernel/util/ListObservable.h` (7)
3. `source/kernel/simulator/ModelCheckerDefaultImpl1.cpp` (5)
4. `source/kernel/simulator/Model.cpp` (4)
5. `source/kernel/simulator/ModelSimulation.cpp` (3)
6. `source/kernel/simulator/ComponentManager.h` (3)

Reavaliação explícita dos arquivos pedidos:

- `TraceManager.cpp`: `SUPERSEDED` como concentrador de iteração legada (não aparece mais entre ocorrências atuais de `std::list<...>::iterator`).
- `Model.cpp`: `PARTIAL` (continua relevante como alvo de modernização, mas com menor concentração que a histórica).
- `List.h`: `OPEN` (permanece com concentração alta e padrão legado estrutural).
- `ListObservable.h`: `OPEN` (permanece com concentração alta e padrão legado estrutural).

### Remaining gaps
- A priorização de backlog deve usar o ranking atual e não apenas o histórico.

## Modernizações aplicadas neste lote (revalidadas)

### 1) `OnEventManager.cpp`
- Histórico: laços com iterador explícito substituídos por `range-based for` em notificações.
- Estado atual: confirmação positiva dos laços em `_NotifyHandlers`/`_NotifyHandlerMethods` com `range-based for`.

#### Audit status
`DONE`

#### Evidence
- Laços internos de notificação seguem em `for (auto ...)` / `for (auto& ...)`.

#### Remaining gaps
- Sem gap direto no item de iteração deste arquivo.

### 2) `TraceManager.cpp`
- Histórico: iterações de handlers migradas para `range-based for`.
- Estado atual: confirmação positiva em `trace`, `traceError`, `traceSimulation`, `traceReport`.

#### Audit status
`DONE`

#### Evidence
- Iterações de handlers permanecem em `range-based for`; não há ocorrência de `std::list<...>::iterator` no arquivo.

#### Remaining gaps
- Sem gap direto no item de iteração deste arquivo.

### 3) `PluginManager.cpp`
- Histórico: `find` modernizado para iteração por elemento com retorno direto.
- Estado atual: confirmação positiva; iteração observada em `range-based for`.

#### Audit status
`DONE`

#### Evidence
- `PluginManager::find` e outros pontos de varredura da lista de plugins usam iteração por elemento.

#### Remaining gaps
- Sem gap direto no item de iteração deste arquivo.

### 4) `ModelDataManager.cpp`
- Histórico: `show`, `getDataDefinition` e `getRankOf` modernizados para `range-based for`; iteração em mapa com structured bindings.
- Estado atual: modernização majoritária confirmada, mas ainda existe iteração explícita legada no arquivo (`getNumberOfDataDefinitions()` com iterador de `std::map`).

#### Audit status
`PARTIAL`

#### Evidence
- Há uso atual de `const auto& [typenameKey, definitions]` em `show()`.
- Persistência de laço legado com `std::map<...>::iterator` em `getNumberOfDataDefinitions()`.

#### Remaining gaps
- Converter o laço residual de `std::map<...>::iterator` para iteração moderna para fechar o arquivo como `DONE`.

## Riscos ainda existentes (reclassificados)

### 1) Ownership manual em headers com `= new` em membros
#### Audit status
`OPEN`

#### Evidence
- Revalidação local encontrou múltiplos casos ainda ativos em headers do kernel (ex.: `OnEventManager.h`, `TraceManager.h`, `PluginManager.h`, `ConnectionManager.h`, `SimulationScenario.h`, `ModelSimulation.h`, entre outros).
- Quantidade aproximada revalidada no escopo `source/kernel/*.h`: **82** ocorrências de `= new`.

#### Remaining gaps
- Backlog real para migração gradual de ownership para RAII/smart pointers e simplificação de destrutores manuais.

### 2) Containers utilitários legados (`List.h`, `ListObservable.h`)
#### Audit status
`OPEN`

#### Evidence
- Ambos continuam entre os arquivos com maior concentração de `std::list<...>::iterator`.
- Arquitetura ainda depende de alocação manual (`_list = new std::list<T>()`) e cursor interno mutável (`_it`).

#### Remaining gaps
- Lote dedicado para modernização estrutural desses containers (API, ownership e padrões de iteração), com mitigação de regressão.

### 3) Casts estilo C / necessidade de triagem manual
#### Audit status
`UNCERTAIN`

#### Evidence
- Busca textual ampla retornou **118** candidatos no estado atual.
- O padrão de regex é conhecido por capturar falsos positivos; sem análise semântica, não é possível classificar precisamente risco real por arquivo.

#### Remaining gaps
- Rodar triagem semântica (ex.: clang-tidy com checks de modernização/casts) antes de transformar o número bruto em plano de execução.

## Conclusão
A auditoria original permanece útil como registro de primeira onda, mas agora precisa ser lida como documento híbrido (histórico + revalidação). As modernizações de `OnEventManager.cpp`, `TraceManager.cpp` e `PluginManager.cpp` estão consolidadas (`DONE`), `ModelDataManager.cpp` permanece `PARTIAL`, e o backlog real concentra-se principalmente em ownership manual de headers e nos containers legados.

Itens históricos que ficaram superados por mudanças posteriores:
- `TraceManager.cpp` como “maior concentração” de iteração legada (`SUPERSEDED` no ranking atual).
- contagem histórica de `std::list<...>::iterator` (=59) como estado corrente (`SUPERSEDED` por recontagem atual=33).

## Remaining Work
- `PARTIAL` — `ModelDataManager.cpp`: remover iteração legada residual em `getNumberOfDataDefinitions()`.
- `OPEN` — Ownership manual em headers com `= new` (backlog de RAII/smart pointers).
- `OPEN` — Modernização estrutural de `List.h` e `ListObservable.h`.
- `UNCERTAIN` — Triagem semântica de candidatos a cast estilo C (resultado textual não conclusivo).
