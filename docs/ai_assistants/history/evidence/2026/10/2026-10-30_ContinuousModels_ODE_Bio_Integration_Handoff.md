---
kind: handoff
status: partial
created: 2026-10-30
source: Codex local validation
document_type: implementation-handoff
authority: informative
owner: project-maintainer
date: 2026-10-30
tracks: 550
---

# GenESyS Continuous Models — ODE/Bio Integration Handoff

**Evidence date:** 2026-10-30
**Status:** `PARTIALLY_VALIDATED`
**Branch:** `WiP20261016/ContinuousModels`
**PR:** #550 (open, draft; not merged)

## 1. Escopo analisado

Esta rodada fechou uma lacuna no caminho de `ContinuousSystemComponent` e
acrescentou regressões de calendário para ODE e BioNetwork. O escopo não cobriu
todas as verificações F8–F10 do mandato de 30/10; em particular, não realizou
uma matriz completa de falhas, cancelamento, múltiplos modelos contínuos,
validação sanitizer completa (ASan/UBSan focados foram executados, mas LSan
reportou alocações do fixture sem comparação reproduzida na base) ou validação
funcional interativa da GUI.

## 2. Diagnóstico inicial

**Confirmado no código:** `ContinuousSystemComponent` recebia uma entidade,
chamava `ODESolver::integrate(simulatedTime)` de forma síncrona e concluía a
entidade imediatamente. `DiffusionSimulate` e `BioSimulate` já usavam
`ContinuousModelComponent::_requestContinuousExecution()` e o ciclo assíncrono
comum.

**Confirmado no código:** o caminho compartilhado já suporta interrupção
limitada pelo calendário, sincronização, evento convencional de continuação e
reentrada no mesmo componente. A API explícita `ODESolver::integrate()` é
separada desse fluxo.

**Confirmado no código:** as referências citadas nas equações do ODESolver não
eram mantidas como dependências antes de `_clearOrphanedDataDefinitions()`. O
Data Definition de parâmetro era removido antes do solver voltar a avaliar a
expressão durante a simulação.

## 3. Evidências e problemas encontrados

**Confirmado no código:** a implementação síncrona do componente ODE não
participava do contrato de retenção e continuação aplicado aos outros dois
componentes.

**Confirmado por execução:** o novo cenário com `StopTime=10`, `StepSize=1`,
`StepsToSynchronize=10` e evento discreto em 5,5 observou estado ODE confirmado
no tempo local 5, retomou a execução e encaminhou a entidade uma única vez no
tempo global 10.

**Confirmado por execução:** o cenário BioNetwork com duas entidades aguardando
uma execução compartilhada observou estado confirmado em 5,5, concluiu ambas no
tempo 10 e preservou a soma dos montantes de A e B.

**Confirmado por execução:** o cenário híbrido com variável confirmou que o
parâmetro valia 1 antes da simulação, que o parser o resolvia após `Model::check()`,
que o estado confirmado era 5 no evento de 5,5 e que, após alterar a taxa para 2,
o estado final era 15. Isso reproduz e corrige o resultado `-5` da tentativa
anterior.

**Confirmado no código:** não foi necessário manter a alteração experimental do
lexer. A causa confirmada foi a poda da dependência sem associação; o teste do
parâmetro também passou com o lexer original.

## 4. Impacto arquitetural

`ContinuousSystemComponent` passa a coordenar a solicitação usando a mesma
infraestrutura de conclusão e retenção de entidade que `DiffusionSimulate` e
`BioSimulate`. A entidade deixa de ser encaminhada no instante de chegada e
aguarda a conclusão da janela de evolução da Data Definition.

`ODESolver::integrate(targetTime)` permanece disponível para chamadas explícitas
reativas. O componente de fluxo de entidades já não usa esse método diretamente.
Não foram alterados o kernel, o comparador do calendário, `InternalEvent`, a
ordenação de eventos simultâneos ou o plugin `Failure`.

## 5. Solução implementada

`ContinuousSystemComponent::_onDispatchEvent()` agora cria uma
`ExecutionRequest` que usa a janela temporal da definição e chama
`_requestContinuousExecution()`. Se a solicitação for rejeitada, registra erro
fatal e remove a entidade, mantendo o comportamento de falha adotado pelos
componentes contínuos sem porta de erro.

Os testes novos executam o kernel/calendário real. O cenário ODE usa derivada
constante para isolar o ciclo temporal, além de um parâmetro global em equação
ODE que muda de 1 para 2 num evento discreto. `ODESolver::_createEditableDataDefinitions()`
associa as referências de cada expressão usando o mecanismo do modelo, para que
elas sobrevivam à poda de órfãos. O cenário Bio usa a reação A → B e altera a
constante de reação no evento discreto.

## 6. Patches e commits

**Confirmado no código:** arquivos alterados nesta rodada:

- `source/plugins/components/Continuous/ContinuousSystemComponent.cpp`
- `source/plugins/data/Continuous/ODESolver.cpp`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`
- `docs/ai_assistants/reference/CONTINUOUS_MODEL_EXECUTION.md`
- este handoff e o ledger de evidências.

**Confirmado no código:** commits da rodada até este handoff:

- `f94a0201b15dfa3a9ea9207f01b7b509c072a6ec` — integra `ContinuousSystemComponent`
  ao ciclo assíncrono.
- `46410ce7624668bbed356d15d232d1ab46b64777` — documentação e evidências.
- `5f6e059dc` — corrige a retenção de referências ODE e adiciona a regressão de
  parâmetro; contém também uma tentativa de precedência lexical.
- `4c1baab2e` — remove a alteração lexical redundante após a regressão provar que
  a causa era somente a falta de associação de dependências.

## 7. Validação executada

**Confirmado por execução:**

- Build completo `tests-unit`: passou.
- `ctest --preset tests-unit --output-on-failure`: 1.936 passaram; 4
  desabilitados; 0 falhas.
- Build completo `tests-kernel-unit`: passou.
- `ctest --preset tests-kernel-unit --output-on-failure`: 1.936 passaram; 4
  desabilitados; 0 falhas.
- Build `tests-smoke` e CTest: 3/3 passaram.
- Target `genesys_test_plugins_continuous_diffusion`: 36/36 passaram após os
  achados da revisão automatizada e os respectivos testes de regressão.
- Build `gui-app`: passou; isso comprova compilação, não
  interação funcional.
- `python3 scripts/validate-ai-docs.py`: passou (50 Markdown ativos; 25
  documentos históricos retidos).

Os testes unitário e kernel executaram os inventários completos do preset,
incluindo casos de inventário estrutural. Quatro casos históricos de
Search/Remove permaneceram desabilitados.

**Confirmado por execução:** três cenários ODE/Bio de calendário passaram sob
ASan/UBSan com `detect_leaks=0`, sem diagnóstico de acesso inválido ou
comportamento indefinido.

**LSan com detecção habilitada:** cada caso isolado chegou à conclusão
funcional, mas o processo terminou com relatório idêntico de 4.910 bytes em
161 alocações. Os três casos (ODE, ODE com parâmetro e BioNetwork) repetiram a
mesma contagem. Executados juntos, dois casos reportaram exatamente o dobro:
9.820 bytes em 322 alocações. Os stacks apontam para carregamento global de
plugins pelo fixture (`PluginManager`, componentes e controles de `Buffer`). O
total por caso coincide com os 4.910 bytes/161 alocações citados como referência
histórica no mandato; contudo, não foi reconstruída a baseline `WorkInProgress`
com este mesmo binário e fixtures. A repetição idêntica por teste é forte
indício de custo do bootstrap compartilhado, mas não prova ausência de vazamento
novo. LSan segue como ressalva não comparada com baseline equivalente.

**Ainda não executado:** benchmark, teste interativo GUI e CI hospedada para o
SHA final desta rodada. Os 12 workflows do HEAD publicado
`46410ce7624668bbed356d15d232d1ab46b64777` foram aprovados; eles não cobrem os
commits locais `5f6e059dc` e `4c1baab2e` ainda não publicados.

### Addendum — achados da revisão automatizada da PR

**Confirmado no código:** a revisão independente apontou três riscos P2.

- `DiffusionSimulate` já rejeitava explicitamente `StartTime != 0` para janela
  local customizada (commit `8a5a42cc10`); foi acrescentado teste confirmando
  que a configuração falha com diagnóstico claro, sem iniciar/encaminhar como
  sucesso.
- O orçamento `ODESolver::MaxSteps` acumulava passos entre blocos autônomos.
  A base agora invoca um hook no início de cada lote e ODESolver reinicia o
  orçamento nesse ponto. A regressão executa dez passos com
  `MaxSteps=2`, sincronização a cada passo e verifica a conclusão no tempo 1.
- O payload final de BioNetwork preservava `status=Running`. Um hook de
  conclusão atualiza o payload depois da transição do estado para `Completed`;
  testes cobrem execução síncrona e execução de calendário com duas entidades.

**Confirmado por execução no commit local `0e6ac0cd8`:** os 36 casos de
`DiffusionFieldPluginTest`, `ContinuousObservableTest` e
`ContinuousCalendarTest` passaram; unitários e kernel passaram com 1.939
aprovados, 4 desabilitados e nenhuma falha; smoke passou 3/3; GUI Qt6 compilou;
os quatro casos de regressão passaram sob ASan/UBSan com `detect_leaks=0`.
O teste de janela de difusão verifica a restrição de início zero já presente
no código. A execução atual de LSan está detalhada abaixo; a baseline
comparável de `WorkInProgress` permanece pendente.

**Atualização de LSan após os testes acima:** com `detect_leaks=1`, os quatro
casos de regressão executaram suas asserções funcionais. Os testes de difusão e
Bio síncrono terminaram sem leaks; o caso ODE com inicialização de plugins e o
Bio componentizado reportaram cada um 4.910 bytes/161 alocações, com origem no
bootstrap global de plugins/controles de `Buffer`. Isso reproduz a quantidade
registrada antes, mas ainda não a compara com uma fixture equivalente de
`WorkInProgress`; portanto não é evidência suficiente para atribuir ou excluir
regressão de leak.

O novo código ainda precisa de CI no SHA publicado e permanece sujeito às
lacunas da seção 8; este addendum não eleva o status `PARTIALLY_VALIDATED`.

## 8. Riscos remanescentes

- A mudança de `ContinuousSystemComponent` para a janela completa da ODE
  altera seu comportamento de encaminhamento em relação à integração reativa
  anterior; o caso de oscilador harmônico do smoke passou, mas falta uma matriz
  completa de compatibilidade para modelos históricos que dependem de várias
  entidades e integrações reativas.
- Compatibilidade total com modelos históricos que dependam de uma entidade
  solicitar integrações reativas repetidas ainda não foi validada; o componente
  agora espera o fim do intervalo configurado.
- A matriz integral de interrupções continua concentrada em Diffusion e nos
  novos cenários ODE/Bio; combinações de várias Data Definitions, cancelamento
  e eventos gerados por observadores não foram todas repetidas nesta rodada.
- Não foi repetida a matriz completa de eventos fracionários/criados por
  observadores nos três domínios nesta rodada.
- Lifetime sob sanitizers, eventos pendentes durante destruição e coexistência
  de várias Data Definitions continuam dependendo de validação posterior.
- A GUI foi compilada, sem teste interativo funcional.
- `source/plugins/data/DiscreteProcessing/Resource.h` modificado e `teste1.gui`
  não rastreado eram trabalho local preexistente e foram preservados; não
  pertencem a esta implementação.

## 9. Próximos passos

1. Comparar o resultado LSan com baseline equivalente e investigar os
   vazamentos reportados no carregamento do fixture.
2. Ampliar regressões de Diffusion com várias configurações, sem enfraquecer
   tolerâncias científicas.
3. Cobrir múltiplas Data Definitions contínuas, cancelamento e destruição.
4. Revisar persistência e testar a GUI em comportamento pertinente.
5. Atualizar a PR e aguardar CI do SHA publicado; manter a PR em draft até que
   os gates da governança estejam satisfeitos.
6. Rever a PR completa e só então decidir se o fluxo governado permite merge.

## Apêndice técnico

### SHA, branch e PR

- SHA inicial da rodada: `0bb9cbb66bef8650ec68d1a1dbbff292e64f9b6e`.
- SHA final validado do código executável nesta rodada:
  `4c1baab2e21830af0eead9b6e6dff8d6e3d5037e` (inclui a integração ODE,
  dependências das expressões e regressões).
  O SHA final do branch, incluindo a consolidação de evidências, será informado
  na conclusão desta execução.
- Branch: `WiP20261016/ContinuousModels`.
- PR: #550, aberta e draft no checkpoint histórico; estado atual deve ser
  verificado depois do push.
- Merge: não realizado.

### Hierarquia e API

- `ContinuousSystemComponent : ContinuousModelComponent`.
- `DiffusionSimulate : ContinuousModelComponent`.
- `BioSimulate : ContinuousModelComponent`.
- API de solicitação utilizada: `_requestContinuousExecution(definition,
  request, entity, errorMessage)`.
- Nenhuma API pública foi adicionada ou removida nesta rodada.
- Contrato de persistência não alterado.

### Cenários executados

- ODE: origem global zero, janela local 0–10, passo 1, sincronização nominal
  10, evento discreto 5,5, sincronização em 5, retomada na grade seguinte e
  conclusão/encaminhamento em 10.
- ODE com parâmetro: `y'=rate`, `rate=1` até a fronteira confirmada 5; evento em
  5,5 altera `rate` para 2; a retomada conclui em `y=15` no tempo local 10.
- BioNetwork: mesma janela, evento discreto 5,5, duas entidades ativadoras
  compartilhando uma execução e encaminhadas no tempo 10; balanço A+B=10.

### Comandos de validação

```bash
cmake --build --preset tests-unit --parallel 2
ctest --preset tests-unit --output-on-failure
cmake --build --preset tests-kernel-unit --parallel 2
ctest --preset tests-kernel-unit --output-on-failure
cmake --build --preset tests-smoke --parallel 2
ctest --preset tests-smoke --output-on-failure
cmake --build --preset gui-app --parallel 2
cmake --build /tmp/genesys-continuous-asan --target genesys_test_plugins_continuous_diffusion --parallel 2
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/genesys-continuous-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter='ContinuousCalendarTest.ODEComponentWaitsAndResumesAfterDiscreteInterruption:ContinuousCalendarTest.ODEReadsDiscreteParameterChangeAfterCalendarResumption:ContinuousCalendarTest.BioSimulateResumesSharedRunForBothWaitingEntities'
```

Logs locais:

- `/tmp/genesys-unit-build-final-final.log`
- `/tmp/genesys-unit-test-final-final.log`
- `/tmp/genesys-kernel-build-final-final.log`
- `/tmp/genesys-kernel-test-final-final.log`
- `/tmp/genesys-smoke-build-final-final.log`
- `/tmp/genesys-smoke-test-final-final.log`
- `/tmp/genesys-gui-build-final-final.log`
- `/tmp/genesys-asan-config.log`
- `/tmp/genesys-asan-build-final-final.log`
- `/tmp/genesys-asan-test-final-final.log` (ASan/UBSan: 3/3 passaram,
  LSan desligado)
- `/tmp/genesys-asan-final-ode-parameter-lsan.log` (teste passou; LSan: 4.910
  bytes/161 alocações)
- `/tmp/genesys-asan-final-ode-lsan.log` e
  `/tmp/genesys-asan-final-bio-lsan.log` (cada teste passou; LSan: 4.910
  bytes/161 alocações)
- execução do validador de documentação; código de saída 0.

### Decisões preservadas

- O relógio global segue controlado pelo kernel.
- Eventos comuns veem o último estado confirmado.
- O Observable expõe o estado interno por passo.
- Eventos simultâneos não receberam nova prioridade.
- Não foi implementado rollback.
- `StepsToSynchronize` e a política de limite pelo calendário não foram
  alterados nesta rodada.

### Classificação do resultado

**Confirmado por execução:** testes unitários, kernel, smoke, target contínuo e
build GUI citados acima; ASan/UBSan focados passaram com LSan desligado.
**Confirmado no código:** migração ODE ao ciclo assíncrono compartilhado.
**Hipótese a validar:** compatibilidade completa dos modelos históricos que
usam `ContinuousSystemComponent` como atualização reativa em cada chegada.
**Forte indício:** os mesmos 4.910 bytes/161 alocações surgem em cada teste,
com stacks no bootstrap de plugins; permanece pendente comparar com execução
equivalente na baseline.
**Status:** `PARTIALLY_VALIDATED`.
