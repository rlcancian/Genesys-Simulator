# Auditoria C++23 no diretório `/source/kernel`

## Objetivo
Aplicar a mesma estratégia usada em `ModelSimulation` para o restante de `/kernel`: identificar padrões legados (laços com iterador explícito, casts antigos e ownership manual) e iniciar uma modernização segura e incremental.

## Varredura inicial (automática)
Comandos executados:

- `rg -n "std::list<.*>::iterator" source/kernel --glob '*.{cpp,h}'`
- `rg -n "\\([A-Za-z_][A-Za-z0-9_:<>\\* ]*\\)\\s*[A-Za-z_][A-Za-z0-9_]*" source/kernel --glob '*.{cpp,h}'`
- `rg -n "new [A-Za-z_][A-Za-z0-9_]*\\(" source/kernel --glob '*.{cpp,h}'`

Contagens encontradas:

- **59** ocorrências de laços com `std::list<...>::iterator` (padrão legado de iteração).
- **98** ocorrências candidatas a cast estilo C (busca ampla; inclui falsos positivos).
- **80** ocorrências de `new` (nem todas são problema; muitas possuem ownership implícito do framework).

## Arquivos com maior concentração de iteração legada

Top por ocorrências de `std::list<...>::iterator`:

1. `source/kernel/simulator/TraceManager.cpp` (12)
2. `source/kernel/util/List.h` (9)
3. `source/kernel/util/ListObservable.h` (7)
4. `source/kernel/simulator/SimulationReporterDefaultImpl1.cpp` (5)
5. `source/kernel/simulator/ModelCheckerDefaultImpl1.cpp` (5)
6. `source/kernel/simulator/Model.cpp` (5)
7. `source/kernel/simulator/OnEventManager.cpp` (4)

## Modernizações aplicadas neste lote

### 1) `OnEventManager.cpp`
- Laços com iterador explícito substituídos por `range-based for` nos métodos internos de notificação.
- Mantida semântica de despacho de handlers (funções e métodos).

### 2) `TraceManager.cpp`
- Todas as iterações de listas de handlers (`trace`, `traceError`, `traceSimulation`, `traceReport`) migradas para `range-based for`.
- Sem alteração de contrato funcional.

### 3) `PluginManager.cpp`
- Método `find` modernizado para iteração por elemento (`Plugin* plugin : ...`) com retorno direto.

### 4) `ModelDataManager.cpp`
- `show`, `getDataDefinition` e `getRankOf` modernizados para `range-based for`.
- Iteração no mapa com *structured bindings* (`const auto& [key, value]`).

## Riscos ainda existentes (próximos passos)

1. **Ownership manual em headers com `= new` em membros**  
   Há vários membros inicializados com `new` diretamente em headers (ex.: `OnEventManager.h`, `SimulationScenario.h`, `ConnectionManager.h`), o que merece revisão por RAII/smart pointers.

2. **Containers utilitários legados (`List.h`, `ListObservable.h`)**  
   Esses arquivos concentram parte significativa dos padrões antigos e devem ser tratados em lote próprio para evitar regressão.

3. **Casts estilo C (triagem manual necessária)**  
   A busca textual superestima casos; ideal rodar uma checagem semântica (ex.: clang-tidy) para separar cast real de sintaxe de ponteiro para função/template.

## Conclusão
Sim — o mesmo trabalho foi iniciado para todo `/kernel` com varredura global + primeira onda de modernização em arquivos centrais do simulador. Recomenda-se continuar em lotes pequenos (por subsistema), validando build/testes a cada etapa.
