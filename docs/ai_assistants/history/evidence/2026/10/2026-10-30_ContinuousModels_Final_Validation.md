---
document_type: evidence-report
authority: informative
owner: continuous-models-implementation
date: 2026-10-30
status: partial-validation
tracks: 550
---

# Continuous Models — final validation checkpoint

## 1. Escopo analisado

Esta rodada concluiu a auditoria local de coexistência de definições contínuas
no calendário compartilhado e corrigiu a validação pós-passo que tratava eventos
empatados com o relógio global como eventos passados. O escopo incluiu
`ContinuousModelDataDefinition`, `ODESolver`, `ContinuousModelComponent`, o
calendário real do `ModelSimulation`, testes de regressão, a referência técnica e
o capítulo do Developer Manual.

O checkout iniciou em `2f14420eace7f36f530e5f6b19153772659b86e6`. O commit de
código e teste desta rodada é `419d8287f` (`fix(continuous): ignore calendar
events at current time`). A atualização de documentação e evidências está sendo
registrada em commit separado. Branch: `WiP20261016/ContinuousModels`; PR #550
permanece aberta e em draft, com base `WorkInProgress`.

## 2. Diagnóstico inicial

**Confirmado no código:** `_nextExternalCalendarEventTime()` ignorava eventos
cujo timestamp não era posterior ao relógio global, mas
`_hasExternalCalendarEventBefore()` examinava qualquer evento restante na fila.
Assim, um evento empatado com o relógio global podia ser interpretado como
anterior ao estado de trabalho já calculado.

**Confirmado por execução:** uma fixture com duas `ODESolver` autônomas,
`StepsToSynchronize` diferentes e duas cadeias independentes de entidades
reproduziu falha de execução com a mensagem “A calendar event was scheduled
before the already-evolved continuous state.” O backtrace identificou
`_startAutomaticExecution()` e a verificação pós-passo.

Uma primeira versão do teste criou incorretamente um `Event` sem componente e
causou uma falha dentro de `Event::show()`. O fixture foi corrigido antes de
atribuir a falha ao código de produção. Essa falha de fixture não integra a
correção.

## 3. Evidências e problemas encontrados

- **Confirmado por execução:** antes da correção, as duas execuções independentes
  não concluíam; após a correção, ambas completam no calendário real e mantêm os
  resultados esperados (`y'=1`, duração 4, estado 4; `y'=2`, duração 6, estado
  12).
- **Confirmado por execução:** o mesmo cenário exercita eventos de início
  simultâneos sem depender de ordem especial entre eles. Nenhum comparador,
  prioridade ou regra de desempate foi alterado.
- **Confirmado por execução:** 37 testes focados de Diffusion, Observable e
  calendário híbrido passaram.
- **Confirmado por execução:** 19 testes focados passaram em ASan/UBSan.
- **Confirmado por execução:** LSan, no novo teste isolado, reportou 4.910 bytes
  em 161 alocações. O total coincide com o registro histórico da execução
  anterior de PR #550, mas não foi construída nesta rodada uma baseline
  equivalente em `WorkInProgress`; portanto, não se conclui que o vazamento seja
  preexistente nem causado pela correção.
- **Confirmado por execução:** o build da GUI Qt6 e a geração do Developer
  Manual foram concluídos. Não houve interação manual na GUI.
- **Evidência histórica:** os 15 checks registrados anteriormente estavam
  verdes no SHA `2f14420`. Eles não validam o novo SHA `419d8287f`; os checks do
  HEAD publicado precisam ser consultados após o push.

## 4. Impacto arquitetural

A consulta de eventos passa a aplicar a mesma fronteira temporal usada pela
busca do próximo evento: eventos em `globalTime` ou anteriores a ele não são
limites futuros da evolução. Eventos posteriores continuam limitando o horizonte
antes do próximo passo que os ultrapassaria.

O calendário e seu comportamento para timestamps iguais permanecem inalterados.
Não foram adicionados eventos internos, prioridades, caminhos de retomada ou
mecanismos de rollback.

O Developer Manual também foi corrigido para descrever o ciclo assíncrono
compartilhado de `ContinuousSystemComponent`, `DiffusionSimulate` e `BioSimulate`;
seu texto anterior dizia incorretamente que o componente ODE mantinha apenas um
caminho reativo síncrono.

## 5. Solução implementada

`_hasExternalCalendarEventBefore()` agora captura o relógio global e ignora
eventos cujo tempo seja `<= globalTime`. A condição de comparação do limite
futuro e a tolerância numérica continuam iguais. O novo teste usa
`ModelSimulation::start()` com duas EDOs, duas fontes, dois componentes contínuos,
dois destinos e diferentes durações/frequências de sincronização.

**Confirmado no código:** a solução não altera os métodos numéricos, estado
confirmado, `StepsToSynchronize`, `InternalEvent`, propriedade de entidades ou
comparador do calendário.

## 6. Patches e commits

- `419d8287f` — filtra eventos que já estão no timestamp atual do kernel na
  validação pós-passo e adiciona teste real de coexistência de duas EDOs.
- Commit seguinte — atualização da referência técnica, Developer Manual,
  artefato PDF, ledger e este relatório.

Arquivos de produção/teste do primeiro commit:

- `source/plugins/data/Continuous/ContinuousModelDataDefinition.cpp`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`

Alterações locais preexistentes em `source/plugins/data/DiscreteProcessing/Resource.h`
e o arquivo não rastreado `teste1.gui` foram preservados e não foram incluídos.

## 7. Validação

Ambiente registrado:

- Ubuntu 24.04; GCC 13.3.0; CMake 3.28.3; Ninja 1.11.1; CTest 3.28.3;
  Git 2.43.0; GitHub CLI 2.45.0.
- Os presets declaram C++23 obrigatório e extensões desligadas.

| Verificação | Resultado desta rodada |
|---|---|
| Foco contínuo `DiffusionFieldPluginTest|ContinuousObservableTest|ContinuousCalendarTest` | 37/37 passaram |
| `tests-unit` | 1.940 passaram, 4 desabilitados, 0 falhos (1.944 listados) |
| `tests-kernel-unit` | 1.940 passaram, 4 desabilitados, 0 falhos (1.944 listados) |
| `tests-smoke` | 3/3 passaram |
| ASan/UBSan foco Observable + calendário | 19/19 passaram; `detect_leaks=0` nesta execução |
| LSan novo teste de coexistência | testes passaram, relatório final de 4.910 bytes/161 alocações; baseline comparável pendente |
| GUI | `cmake --build --preset gui-app --parallel 2` passou; sem interação manual |
| Developer Manual | `./make.sh` passou; PDF com 388 páginas e 8.074.782 bytes |
| GitHub Actions no SHA novo | pendente de push/conclusão; os checks verdes no SHA anterior não contam como validação do novo código |

Comandos de validação executados:

```text
cmake --build --preset tests-unit --parallel 2
ctest --preset tests-unit --output-on-failure
cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
cmake --build --preset tests-smoke --parallel 2
ctest --preset tests-smoke --output-on-failure
cmake --build --preset gui-app --parallel 2
cmake --build build/observable-asan --target genesys_test_plugins_continuous_diffusion --parallel 2
ASAN_OPTIONS=detect_leaks=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter='ContinuousCalendarTest.*:ContinuousObservableTest.*'
ASAN_OPTIONS=detect_leaks=1:exitcode=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.IndependentAutonomousOdesShareCalendarWithoutLosingTheirRuns
./make.sh  # from docs/developers/ManualGenESyS_subrepo
```

## 8. Riscos remanescentes

- **Confirmado no backlog humano:** `HUM-SCI-002` ainda registra que eventos
  discretos comuns podem alterar entradas utilizadas por trabalho contínuo já
  calculado; política de invalidação/impacto precisa de tratamento explícito
  antes de esse padrão ser considerado suportado. Esta rodada não implementa
  rollback.
- **Confirmado por execução:** LSan detecta 4.910 bytes em 161 alocações no
  fixture executado. A origem e a relação com o código novo permanecem
  indeterminadas sem baseline equivalente.
- **Confirmado no estado da PR observado antes deste commit:** PR #550 continuava
  draft, com `reviewDecision` vazio. Não há autorização normativa para marcar
  pronta ou integrar sem o gate de revisão aplicável.
- **Confirmado no diff da PR observado:** o escopo histórico da PR contém mudanças
  de GUI/crash e outros arquivos além da integração contínua; a revisão integral
  e a separação/justificativa dessas mudanças continuam necessárias antes da
  integração.
- A matriz solicitada para cancelamento/destruição, replicações, coexistência
  dos três domínios em conjunto, interações cross-model extensas e persistência
  round-trip de todos os componentes não foi integralmente executada nesta
  rodada. O green CI anterior não substitui essas lacunas.

## 9. Próximos passos

1. Publicar os commits no branch existente e verificar os checks do SHA final.
2. Executar revisão independente da PR completa, inclusive mudanças históricas
   fora do escopo contínuo; manter a PR em draft até satisfazer a governança.
3. Executar baseline comparável de LSan em `WorkInProgress` e investigar a
   origem dos 4.910 bytes/161 alocações.
4. Completar a matriz de lifecycle, replicação, persistência e calendário para
   ODE, difusão e BioNetwork, sem alterar a semântica científica pendente.
5. Manter `HUM-SCI-002` sem status `implemented` até concluir sua evidência e
   resolver a condição de invalidação que o backlog explicita.

**Status da missão nesta evidência: `PARTIALLY_VALIDATED`.** A correção do caso
de calendário empatado está demonstrada; a integração total e os gates de revisão
e LSan não estão concluídos.
