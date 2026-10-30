---
document_type: implementation-handoff
authority: informative
owner: project-maintainer
date: 2026-10-30
status: PARTIALLY_VALIDATED
tracks: 550
---

# GenESyS — Calendar-bounded continuous execution handoff

## 1. Escopo analisado

Continuação da PR #550 para limitar o lookahead contínuo pelo calendário do
kernel, retomar a mesma execução depois de uma interrupção, devolver a entidade
ao mesmo componente e manter o estado confirmado separado do estado de trabalho.
Foram preservadas as decisões humanas: sem rollback, sem ordenamento adicional
para timestamps iguais, `ModelSimulation` continua sendo o único proprietário do
relógio global, e `StepsToSynchronize=0` continua síncrono e sem `InternalEvent`.

O escopo também inclui a correção autorizada de mutação parcial de configuração
em requisições de janela temporal inválidas. O plugin `Failure` não foi alterado.

## 2. Diagnóstico inicial

**Confirmado no código:** `_continueEvolution()` agendava a sincronização nominal
e calculava todo o bloco antes do kernel despachar o evento. A lista de eventos
futuros de `Model` é cronológica e usa comparador somente por tempo. A entidade
associada a uma execução assíncrona era retida por ID no `ContinuousModelComponent`,
mas não existia caminho de reentrada para continuar uma execução suspensa.

**Confirmado no código:** a inicialização entre replicações iniciava a trajetória
antes de callbacks de início de replicação adicionarem eventos comuns ao
calendário. Assim, o primeiro bloco não poderia ser limitado por esses eventos.

**Confirmado no código:** `requestSimulation()` alterava `StopTime` e `StepSize`
antes de completar `_check()`, deixando a configuração persistida parcialmente
alterada quando uma validação específica de subclasse falhava.

## 3. Evidências

**Confirmado por execução:** o alvo `genesys_test_plugins_continuous_diffusion`
passa 32/32, incluindo limites de calendário em 0,5, 5,5, 9,9 e 10; origem
global 100 com evento em 105,5; confirmação local em 5; retorno da mesma
entidade em 106 e conclusão em 110. O teste também cobre evento criado pelo
Observable em 3,5, anterior ao horizonte nominal.

**Confirmado por execução:** o teste de regressão de `BioSimulate` permite os
dois resultados compatíveis quando entidade e sincronização possuem o mesmo
timestamp, sem alterar o comparador ou presumir uma ordem.

**Confirmado por execução:** o teste de janela inválida verifica que os valores
originais de `StopTime` e `StepSize` permanecem intactos.

## 4. Impacto arquitetural

`ContinuousModelDataDefinition` agora limita blocos pelo primeiro evento futuro
externo, ignora seu próprio `InternalEvent` ao consultar o calendário, refaz a
consulta após cada passo e suspende o avanço na grade temporal. Um evento entre
pontos da grade causa sincronização no último ponto permitido e continuação no
próximo ponto da grade após o evento discreto.

O estado `WaitingForContinuation` explicita essa suspensão. O componente agenda
um `Event` convencional com a mesma entidade e um input port reservado em tempo
de execução. `ContinuousSystemComponent`, `DiffusionSimulate` e `BioSimulate`
interceptam essa reentrada e retomam a mesma Data Definition. Um único evento
representante retoma uma execução compartilhada; as outras entidades associadas
continuam retidas até a conclusão.

O início automático entre replicações foi adiado a um evento interno em tempo
zero, depois de a lista de eventos ser preenchida pelos callbacks de início de
replicação. Isso permite calcular o primeiro limite com o calendário disponível.

## 5. Solução implementada

- Consulta ao primeiro evento posterior ao relógio global, ignorando eventos
  internos próprios da Data Definition.
- Corte por inteiro de passos completos anteriores ao evento; sem passo parcial
  automático até o timestamp fracionário.
- Evento de sincronização no limite permitido e evento de continuação no próximo
  ponto da grade quando o calendário interrompe o bloco.
- Reconsulta após cada passo. Eventos do Observable antes do estado já calculado
  provocam falha diagnóstica; não há rollback de estado ou efeitos do handler.
- Seleção de uma única entidade representante para a reentrada do componente.
- Remoção dos eventos convencionais de continuação próprios durante conclusão,
  cancelamento ou destruição do componente.
- Validação transacional de overrides de tempo em `requestSimulation()`.
- Nenhum ajuste foi feito ao comparador global de eventos ou ao plugin `Failure`.

## 6. Patches e commits realizados

SHA inicial: `efc3efcbef31ac259d141582fb9d2906b7c8f2ac`.

Commit de implementação e testes: `e3ecf6278` —
`feat(continuous): bound evolution by calendar events`.
Commit de documentação: `ff1751920d058c52cc3f7e518aa02285e3d91244`
(`docs(continuous): document calendar-bounded resumption`), que atualiza a
referência técnica, o ledger e este handoff.

Arquivos de implementação e teste previstos:

- `source/plugins/data/Continuous/ContinuousModelDataDefinition.h/.cpp`
- `source/plugins/components/Continuous/ContinuousModelComponent.h/.cpp`
- `source/plugins/components/Continuous/ContinuousSystemComponent.cpp`
- `source/plugins/components/Continuous/DiffusionSimulate.cpp`
- `source/plugins/components/BiochemicalSimulation/BioSimulate.cpp`
- `source/tests/unit/test_plugins_continuous_diffusion.cpp`
- `source/tests/unit/test_simulator_runtime.cpp`
- `docs/ai_assistants/reference/CONTINUOUS_MODEL_EXECUTION.md`

## 7. Validação executada

Toolchain observada: GCC 13.3.0, CMake/CTest 3.28.3, Ninja 1.11.1, Git 2.43.0,
GitHub CLI 2.45.0. Os presets configuram C++23 (`CMAKE_CXX_STANDARD=23`, requerido).

- `tests-unit`: 1.933 aprovados, 0 falhos, 4 desabilitados.
- `tests-kernel-unit`: 1.933 aprovados, 0 falhos, 4 desabilitados.
- `tests-smoke`: 3/3 aprovados.
- GUI Qt6: configuração e build concluídos.
- Testes focados contínuos: 32/32 aprovados; teste kernel focal de `BioSimulate`:
  1/1 aprovado.
- ASan/UBSan com LeakSanitizer habilitado: 32/32 aprovados, sem diagnóstico de
  memória, undefined behavior ou leak.
- Warnings preexistentes no build instrumentado: `FitterDummyImpl.cpp` contém
  funções não-void sem `return`; não pertencem a esta alteração.
- `scripts/validate-ai-docs.py`: passou.
- GitHub Actions e PR: estado pós-push ainda pendente.

Logs locais em `/tmp/genesys-{unit,kernel,smoke,gui,sanitize}-*.log`.

## 8. Riscos remanescentes

- Um passo na grade que começa antes e termina depois de uma descontinuidade
  fracionária é executado após o evento, sem reiniciar o solver ou criar passo
  fracionário. Essa é a aproximação explicitamente mantida nesta implementação.
- `StepsToSynchronize=0` não cede o controle a eventos discretos intermediários.
- A consulta ao calendário ocorre após todos os passos; a otimização de pular
  consultas quando não há observadores/hooks não foi aplicada.
- O input port de continuação usa `UINT_MAX` como marcador privado de runtime;
  ele não é uma propriedade persistida.
- Interações com parâmetros alterados por handlers que invalidem estado já
  calculado são rejeitadas com erro, sem rollback. Efeitos externos arbitrários
  que o handler já tenha produzido não podem ser desfeitos.
- A validação dos três domínios continua coberta por regressões herdadas, mas a
  nova matriz de interrupção ainda está focada em `DiffusionField`.

## 9. Próximos passos

1. Acompanhar a conclusão dos checks da PR #550; no último levantamento ainda
   estavam pendentes.
2. Se algum check falhar, diagnosticar a causa e corrigir apenas problemas
   pertencentes a este escopo.
3. Manter a PR em draft para revisão independente; não fazer merge em
   `WorkInProgress`.

## Apêndice técnico

### Identificação

- Branch: `WiP20261016/ContinuousModels`
- PR: #550, alvo `WorkInProgress`, estado draft a revalidar depois do push.
- SHA inicial: `efc3efcbef31ac259d141582fb9d2906b7c8f2ac`.
- SHA da implementação validada: `e3ecf6278`.
- SHA final publicado (implementação e documentação):
  `ff1751920d058c52cc3f7e518aa02285e3d91244`.
- Estado Git observado após o push: apenas `Resource.h` modificado e
  `teste1.gui` não rastreado, ambos preexistentes e preservados fora dos commits.

### Interfaces adicionadas

- `ExecutionState::WaitingForContinuation`.
- `addContinuationHandler()`, `removeContinuationHandler()` e `resumeExecution()`.
- `_nextExternalCalendarEventTime()` e `_hasExternalCalendarEventBefore()` para
  limitar a execução e rejeitar eventos retroativos à grade já evoluída.
- `ContinuousModelComponent::CONTINUATION_INPUT_PORT`.
- As três classes concretas interceptam eventos com esse marcador antes de
  tratar uma nova chegada.

### Sequência de tempo

```text
T0=100; step=1; STS nominal=110; evento discreto=105.5
Passos de trabalho: 101, 102, 103, 104, 105
InternalEvent confirma em 105
Evento discreto existente: 105.5
Evento convencional da mesma entidade retorna ao componente: 106
Execução retoma sem reiniciar o estado ou duplicar passos
```

### Testes e comandos

```text
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
```

### Persistência e decisão sobre timestamps iguais

Nenhum campo persistido foi acrescentado ou removido. O input port de continuação
é transitório e interno. Nenhuma mudança ocorreu no comparador de eventos; não
há precedência por tipo, origem, componente ou identificador.

### Classificação das evidências

- **Confirmado no código:** cálculo de limite, evento próprio excluído da consulta,
  estado `WaitingForContinuation`, reentrada sem chamada de inicialização,
  remoção de eventos do componente e validação atômica.
- **Confirmado por execução:** suites unitária, kernel e smoke; build Qt6; testes
  focados com o calendário real.
- **Forte indício:** a seleção de uma entidade representante é suficiente para
  retomar uma execução compartilhada, enquanto outras entidades permanecem
  retidas; o fluxo de integração o demonstra para uma entidade.
- **Hipótese a validar:** desempenho da consulta do calendário por passo em
  modelos com grades muito grandes; interrupções em todos os três domínios sob
  carga; resultado final de sanitizers e CI hospedado.
