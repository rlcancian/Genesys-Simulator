# Genesys 2026-1 — Roadmap de estabilização de build e testes

## Audit Status (WiP20261)
- Branch auditada: `WiP20261`
- Escopo da auditoria: roadmap de build/testes revalidado contra o estado atual do repositório
- Legenda de status: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`

## Objetivo
Estabilizar o entrypoint CMake na raiz, garantir build incremental do kernel e expandir cobertura de testes unitários (kernel primeiro; depois parser/plugins/tools), sem quebrar comportamento existente.

## Estado atual (confirmado)
- Build raiz com CMake/Ninja está funcional em modo Debug.
- Alvo agregado `genesys_kernel` existe para compilar o núcleo atual.
- Testes unitários e smoke compilam/rodam via CTest.
- Presets básicos existiam para `debug`, `release` e `asan`.

## Fase 1 — Base de build reproduzível (passo atual)
1. Definir matriz explícita de presets:
   - `debug-kernel` (somente kernel, sem testes)
   - `debug` (kernel + testes)
   - `release` (kernel + testes)
   - `asan` e `ubsan` (kernel + testes)
2. Tornar fluxo operacional simples:
   - configurar
   - build kernel
   - build testes
   - ctest

### Critério de pronto (Fase 1)
- Todos os presets configuram com sucesso.
- `cmake --build --preset debug-kernel` compila o kernel.
- `ctest --preset debug` roda suite atual sem regressão.

### Audit status
`DONE`

### Evidence
- `CMakePresets.json` já contém presets de configure para `debug`, `debug-kernel`, `release`, `asan`, `ubsan`, `tests-unit`, `tests-kernel-unit`, `tests-smoke` e `terminal-app`.
- O preset de build `debug-kernel` está apontado para o target `genesys_kernel`.
- Existem test presets para `debug`, `asan`, `tests-unit`, `tests-kernel-unit`, `tests-smoke` e `ubsan`.
- `CMakeLists.txt` raiz inclui `CTest`, possui fallback para GoogleTest embutido e inclui `source/tests` quando `GENESYS_BUILD_TESTS=ON`.

### Remaining gaps
- Nenhum gap estrutural relevante da fase foi encontrado na auditoria documental.

## Fase 2 — Inventário técnico do kernel (alta prioridade)
1. Criar inventário por pacote (util/statistics/simulator):
   - classes
   - dependências
   - ownership/lifetime
   - hotspots de ponteiro cru e TODO/FIXME
2. Classificar cada item: bug confirmado, forte indício, dívida técnica.

### Critério de pronto (Fase 2)
- Relatório por arquivo com evidências e severidade.
- Backlog priorizado (P0/P1/P2).

### Audit status
`PARTIAL`

### Evidence
- Existe inventário técnico formal em `documentation/phase2-kernel-simulator-inventory.md` com classificação de riscos (P0/P1/P2), evidências e backlog inicial.
- O inventário existente cobre explicitamente o pacote `source/kernel/simulator` e classes-chave (`Model`, `ModelSimulation`, `PluginManager`, `TraceManager`).

### Remaining gaps
- O escopo originalmente declarado para Fase 2 inclui `util/statistics/simulator`; o inventário formal encontrado está focado em `simulator`.
- Falta evidência consolidada, no mesmo nível de detalhe, para `source/kernel/util` e `source/kernel/statistics`.

## Fase 3 — Cobertura de testes unitários do kernel
1. Definir meta mínima por classe crítica:
   - construtor/invariantes
   - erro/borda
   - contratos de ownership
   - serialização/persistência quando aplicável
2. Adicionar testes faltantes por subsistema:
   - Model/ModelSimulation/ModelDataManager
   - PluginManager/Plugin/PluginInformation
   - ParserManager/Persistence/TraceManager

### Critério de pronto (Fase 3)
- Cobertura dos cenários críticos acordados.
- Testes de regressão para bugs corrigidos.

### Audit status
`PARTIAL`

### Evidence
- `source/tests/CMakeLists.txt` agrega suíte unitária e smoke no target `genesys_tests`.
- `source/tests/unit/CMakeLists.txt` define diversos executáveis de teste com `gtest_discover_tests(... LABELS "unit")`, incluindo áreas de simulator support/runtime, parser expressions, plugin manager e statistics.
- Há teste autogerado de inventário de métodos do kernel simulator com label `unit;autogen`.
- `source/tests/smoke/CMakeLists.txt` define teste smoke com label `smoke`.

### Remaining gaps
- Não há evidência, nesta auditoria documental, de meta quantitativa de cobertura formalmente atingida por classe crítica.
- Não há rastreabilidade explícita no roadmap entre bugs corrigidos e testes de regressão específicos já incorporados.

## Fase 4 — Parser e plugins (incremental)
1. Definir interface estável mínima entre kernel e plugins.
2. Separar build de plugins em etapas:
   - plugin estático (transição)
   - plugin dinâmico por biblioteca (meta)
3. Testes:
   - parser unit tests
   - plugin contract tests
   - smoke de carregamento

### Critério de pronto (Fase 4)
- Pelo menos 1 plugin real carregado via contrato definido.
- Testes de compatibilidade/versão do contrato.

### Audit status
`PARTIAL`

### Evidence
- `source/parser/CMakeLists.txt` define `genesys_parser` como biblioteca estática integrada ao build principal.
- `source/plugins/CMakeLists.txt` define `genesys_plugins_components_minimal` como biblioteca estática (estado compatível com a etapa de transição estática).
- Existe cobertura de testes relacionada a parser/plugin no conjunto unitário atual (ex.: parser expressions e runtime plugin manager).

### Remaining gaps
- Não foi encontrada evidência de pipeline de plugin dinâmico por biblioteca como meta final da fase.
- Não foi encontrada evidência explícita de suíte dedicada de `plugin contract tests` com verificação de compatibilidade/versão de contrato.
- O critério de “plugin real carregado via contrato definido” permanece parcialmente atendido e precisa de revalidação orientada a contrato/versionamento.

## Fase 5 — Qualidade e CI
1. GitHub Actions com matriz mínima:
   - debug + ctest
   - asan + ctest
   - ubsan + ctest
2. Guardrails:
   - warnings elevados no kernel
   - checagens de regressão de testes

### Critério de pronto (Fase 5)
- Pipeline obrigatório para merge no branch 2026-1.

### Audit status
`OPEN`

### Evidence
- Não foi encontrado diretório `.github/workflows` no estado auditado, portanto não há evidência local de GitHub Actions com a matriz mínima da fase.

### Remaining gaps
- Implementar workflows CI para `debug`, `asan` e `ubsan` com execução de `ctest`.
- Definir e aplicar requisito de pipeline obrigatório para merge no branch alvo.
- `warnings elevados no kernel` e política de guardrails de regressão permanecem `UNCERTAIN` nesta auditoria (sem evidência documental/cmake explícita suficiente).

## Próximo passo imediato recomendado
Executar **Fase 2 (inventário técnico do kernel)** começando por `source/kernel/simulator`:
1. mapear ownership e tempo de vida em `Model`, `ModelSimulation`, `PluginManager`, `TraceManager`;
2. gerar lista dos 10 riscos mais prováveis de regressão/leak;
3. transformar os 3 maiores riscos em tarefas com teste de regressão.

## Remaining Work
- **Fase 2 (`PARTIAL`)**: expandir o inventário técnico com o mesmo rigor para `source/kernel/util` e `source/kernel/statistics`.
- **Fase 3 (`PARTIAL`)**: explicitar meta de cobertura por classe crítica e vincular correções de bug aos testes de regressão correspondentes.
- **Fase 4 (`PARTIAL`)**: formalizar contrato estável kernel/plugins, validar carregamento via contrato e evoluir para estratégia de plugin dinâmico quando aplicável.
- **Fase 5 (`OPEN`)**: criar CI (debug/asan/ubsan + ctest), definir gate obrigatório de merge e registrar guardrails de warnings/regressão.
- **Itens `UNCERTAIN`**: revalidar política atual de warnings elevados e critérios de compatibilidade/versionamento de contrato de plugin com evidência automatizada.
