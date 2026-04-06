# Genesys 2026-1 — Roadmap de estabilização de build e testes

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

## Próximo passo imediato recomendado
Executar **Fase 2 (inventário técnico do kernel)** começando por `source/kernel/simulator`:
1. mapear ownership e tempo de vida em `Model`, `ModelSimulation`, `PluginManager`, `TraceManager`;
2. gerar lista dos 10 riscos mais prováveis de regressão/leak;
3. transformar os 3 maiores riscos em tarefas com teste de regressão.
