---
document_type: evidence-report
authority: informative
owner: continuous-models-implementation
date: 2026-10-30
status: local-validation-complete-hosted-checks-pending
tracks: 550
---

# GenESyS Continuous Models — final closeout checkpoint

## 1. Escopo analisado

Esta rodada fecha defeitos finais da PR #550, revisa seu diff histórico,
reconcilia `HUM-SCI-002`, valida persistência e ciclo de vida e prepara a PR para
integração. Não adiciona nova funcionalidade de simulação.

O branch é `WiP20261016/ContinuousModels`, base `WorkInProgress`. SHA inicial:
`fc9afd79e2923afdbe09f8660e403932b97d97b6`. SHA de código/testes neste
checkpoint: `a28bc2168` (`fix(continuous): preserve ODE precision on
persistence`). A atualização documental e o handoff desta rodada ainda serão
commitados; a CI correspondente ao SHA final publicado está pendente neste
registro.

## 2. Diagnóstico

- **Confirmado no código:** quando uma solicitação contínua sem entidade era
  rejeitada, o componente removia seu completion handler, mas deixava o
  continuation handler, cuja lambda capturava `this`.
- **Confirmado por execução:** a fixture de regressão reproduziu heap-use-after-free
  em ASan quando a definição contínua foi reutilizada após a destruição do
  componente.
- **Confirmado por execução:** a persistência ODE perdeu `precision=1e-8`, pois a
  escrita numérica legada utiliza `std::to_string`, que produz somente seis
  casas decimais. A carga restaurava zero.
- **Confirmado no código:** os três comentários automatizados antigos da PR
  correspondiam a contratos agora explícitos: rejeição de janela ODE/difusão
  não suportada, orçamento `MaxSteps` por lote ODE e payload terminal
  `Completed` do BioNetwork.

## 3. Evidências

- **Confirmado por execução:** antes do patch de lifetime, ASan apontou leitura
  após liberação na lambda de continuação de `ContinuousModelComponent`; após a
  remoção de ambos os handlers, a regressão passa.
- **Confirmado por execução:** o preset unitário passou 1.945 testes, com quatro
  desabilitados e zero falhas.
- **Confirmado por execução:** o preset kernel passou 1.945 testes, com quatro
  desabilitados e zero falhas.
- **Confirmado por execução:** smoke passou 3/3.
- **Confirmado por execução:** o target contínuo passou 44/44.
- **Confirmado por execução:** GUI Qt6 compilou. Não foi feita interação manual
  com animação, pause/resume ou editor nesta rodada.
- **Confirmado por execução:** os testes focados de lifetime, round-trip ODE e
  round-trip BioNetwork passaram sob ASan/UBSan/LSan com leak detection ativo,
  sem diagnóstico.
- **Confirmado por execução:** `python3 scripts/validate-ai-docs.py` passou após
  a reconciliação inicial da documentação. Deve ser executado novamente após
  este relatório e os registros finais.
- **Evidência histórica:** os 16 checks da PR passaram no SHA `fc9afd79e...` antes
  desta correção. Eles não validam `a28bc2168` nem os commits documentais que
  ainda serão publicados; checks finais estão pendentes.

## 4. Impacto

A correção de lifetime remove as duas inscrições criadas para uma solicitação
rejeitada sem entidades pendentes. Ela não muda propriedade de entidades,
callback APIs nem o ciclo normal de execução.

A persistência ODE mantém o campo numérico `precision` legado e acrescenta
`precisionExact` textual opcional para preservar tolerâncias menores que seis
casas decimais. O carregador usa o campo exato quando presente e continua
aceitando arquivos antigos sem ele. O teste verifica configuração submicro e
parâmetros ODE principais.

`HUM-SCI-002` passa a `implemented` para a integração determinística aprovada:
eventos discretos ordinários leem o último estado confirmado, observadores leem
o estado de trabalho por passo, o calendário limita antecipação e a mesma
execução/entidade retoma no componente. A alteração de parâmetro da EDO em 5,5
é coberta por `ContinuousCalendarTest.ODEReadsDiscreteParameterChangeAfterCalendarResumption`.

A política não implementa rollback genérico. Eventos retroativos são rejeitados
com diagnóstico; efeitos colaterais arbitrários de handlers não são desfeitos.
Um evento fora da grade não causa passo fracionário: a execução mantém a grade
e o passo que cruza a descontinuidade é a aproximação numérica existente.
Semântica estocástica CTMC permanece fora do escopo.

## 5. Solução implementada

1. `ContinuousModelComponent::_requestContinuousExecution()` remove completion e
   continuation handlers quando a solicitação é rejeitada antes de qualquer
   entidade ser associada.
2. Um teste cria uma solicitação inválida sem entidade, destrói o componente,
   reutiliza a Data Definition, despacha a sincronização real que antes invocaria
   a lambda pendente e verifica o caminho de cancelamento sem callback inválido.
3. `ODESolver` persiste precisão com `max_digits10` em `precisionExact`, preservando
   o campo numérico legado para leitores anteriores.
4. Testes verificam configuração round-trip de ODE (`precision=1e-8`) e
   BioNetwork (janela, frequência, membership e payload); DiffusionField já
   possuía teste de configuração round-trip.
5. Os arquivos do scanner gerado e do lexer tinham somente deriva de espaços em
   branco no diff histórico; foram normalizados sem alteração semântica.
6. `HUM-SCI-002`, referência contínua, backlog, status e ledger foram
   reconciliados para refletir a política determinística implementada e seus
   limites explícitos.

## 6. Patches e commits

- `77f8c6cdc` — `fix(continuous): unsubscribe rejected continuation callbacks`.
- `d5b23f215` — `chore(parser): remove whitespace-only lexer drift`.
- `a28bc2168` — `fix(continuous): preserve ODE precision on persistence`.
- SHA anterior conhecido: `fc9afd79e2923afdbe09f8660e403932b97d97b6`.
- Os commits documentais e o SHA final publicado serão acrescentados antes do
  merge.

## 7. Validação

| Verificação | Resultado neste checkpoint |
|---|---|
| `tests-unit` | PASS — 1.945 aprovados, 4 desabilitados, 0 falhas |
| `tests-kernel-unit` | PASS — 1.945 aprovados, 4 desabilitados, 0 falhas |
| `tests-smoke` | PASS — 3/3 |
| Target contínuo | PASS — 44/44 |
| ASan/UBSan/LSan focado | PASS — 3/3, leak detection ativo, sem diagnóstico |
| GUI Qt6 | PASS — compilação; sem teste interativo alegado |
| Persistência contínua | PASS — DiffusionField, ODESolver, BioNetwork; testes de configuração e legado |
| AI docs governance | PASS — validação local; revalidar após os commits documentais |
| GitHub Actions no código novo | PENDING — executar na cabeça final publicada |
| PR review/merge | PENDING — PR permanece draft até os gates finais |

Comandos principais executados:

```bash
cmake --preset tests-unit
cmake --build --preset tests-unit --parallel 2
ctest --preset tests-unit --output-on-failure
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel 2
ctest --preset tests-smoke --output-on-failure
cmake --preset gui-app
cmake --build --preset gui-app --parallel 2
python3 scripts/validate-ai-docs.py
```

Sanitizer flags: `-fsanitize=address,undefined -fno-omit-frame-pointer`;
`ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`, `LSAN_OPTIONS=exitcode=23` e
`UBSAN_OPTIONS=halt_on_error=1`.

O histórico de LSan aponta 4.910 bytes/161 alocações tanto no teste ODE do
branch quanto em uma baseline de bootstrap de plugins em `WorkInProgress`, com
stacks normalizados coincidentes. Isso é forte evidência de alocação preexistente,
mas os corpos dos fixtures não são idênticos e não prova igualdade universal.
Os testes focados desta correção passaram sem leak.

## 8. Riscos remanescentes

- **Confirmado no código:** `StepsToSynchronize=0` é síncrono e não intercala
  eventos discretos.
- **Confirmado no código:** descontinuidades fora da grade preservam a grade e
  usam o passo cruzador existente; não há integração fracionária automática.
- **Confirmado no código:** não há rollback genérico dos efeitos de handlers;
  eventos retroativos falham explicitamente.
- **Evidência histórica:** LSan do processo amplo de carregamento de plugins
  mantém a ocorrência 4.910 bytes/161 alocações descrita acima.
- **Confirmado por execução:** GUI compilou; interação manual do cenário de
  animação pause/resume não foi repetida nesta rodada.
- **Pendente:** checks hospedados no SHA final, revisão após marcar a PR pronta e
  verificação do merge/post-merge.
- **Forte indício:** os comentários automatizados antigos estão endereçados no
  código e nos testes; threads remotas ainda precisam ser resolvidas após a
  publicação dos novos commits.

## 9. Próximos passos

1. Commitar documentação/evidências sem incluir `Resource.h` nem `teste1.gui`.
2. Fazer push em `WiP20261016/ContinuousModels` e verificar todos os checks no
   SHA final.
3. Atualizar a descrição da PR, resolver os comentários antigos já endereçados,
   marcá-la pronta e revalidar revisão/checks.
4. Fazer merge commit em `WorkInProgress` somente se todos os gates permanecerem
   aprovados e verificar o SHA integrado e o estado pós-merge.
5. Encerrar o ciclo atual sem iniciar novas funcionalidades.
