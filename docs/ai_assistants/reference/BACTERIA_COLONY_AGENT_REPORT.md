---
document_type: reference
authority: task-specific-execution-report
owner: project-maintainer
last_reviewed: 2026-10-11
status: active
---

# GenESyS — Relatório de execução Codex: Gro/BacteriaColony

**Protocolo recebido:** [BACTERIA_COLONY_AGENT_HANDOFF.md](BACTERIA_COLONY_AGENT_HANDOFF.md)
**Handoff executado:** `BC-HANDOFF-001` (execução única).
**Relatório:** resultado local publicado na branch candidata descrita abaixo.

## 1. Identificação

- **Status:** `PARTIAL` / `PARTIAL_NEEDS_REVIEW`.
- **Branch original:** `WiP20261008/BacteriaColony`, preservada para auditoria.
- **HEAD original observado:** `dcc8bb89910e3b0f8d8c15f29f89dcb36846f230`.
- **Branch candidata:** `WiP20261009/BacteriaColony-clean-candidate`.
- **Base de integração:** `origin/WorkInProgress` = `50a340d460876c657d8a3be66554bb29cfe8519a`.
- **HEAD inicial da candidata:** `847e6605882a8886dd1ff7991a63d718d2168f5e`.
- **Commit funcional desta execução:** `4d281158` (`fix(gui): map centered bacteria coordinates in viewer`).
- **HEAD final:** commit de documentação que adiciona este relatório e a evidência de `BC-HANDOFF-001`; consulte o HEAD do branch publicado.
- **Ambiente:** Linux x86_64, CMake 3.28.3, Ninja 1.11.1, GCC 13.3, Qt 6.4.2, C++23.
- **Builds:** presets CMake do repositório; `gui-app` configurado como Debug, C++23 e Qt6.
- **Worktree:** a candidata tinha alterações locais de coordenadas ainda não commitadas, produzidas nesta execução, e o arquivo preexistente não rastreado `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`. O código necessário foi commitado seletivamente; o inventário gerado ficou fora do staging e continua não rastreado. A branch original permaneceu limpa.

## 2. Resumo executivo

A branch candidata foi preservada sem a ancestralidade de `0f64fa8e07` e continuou baseada em `WorkInProgress`. A análise de proveniência registrada em §22.8 do plano se confirmou: os 19 exemplos Gro de terceiros e o inventário gerado ficaram fora do histórico da candidata; somente as verificações legítimas de coordenadas foram reaplicadas seletivamente antes desta execução. Nenhum arquivo local não rastreado foi removido.

O teste e correção de `tumble()` e o cenário demonstrativo com divisão real já estavam no HEAD inicial da candidata. Os testes integrais repetidos nesta execução confirmaram esses contratos, incluindo crescimento, movimento, duas concentrações independentes e persistência do demo. A inspeção geométrica encontrou uma divergência visual concreta para coordenadas centradas; foi corrigida com um helper puro e dois testes novos.

A aplicação Qt6 compilou, iniciou e apresentou o diálogo de abrir modelo. Não consegui validar nesta sessão a interação completa com o viewer: não consegui identificar/acionar sua entrada no menu por automação, o AT-SPI Python não está instalado e captura X11 da raiz retornou `BadMatch`. Portanto seleção de canais, comparação de mapas, reset e disputa com o calendário permanecem pendentes de verificação. Não afirmo a GUI interativa como aprovada. Há uma inconsistência documental entre HANDOFF §2.4 e plano §22.8 quanto à validação visual anterior; os dois registros foram preservados para revisão independente.

## 3. Ciclos A–F

| Ciclo | Status | Evidência/commit | Pendência |
|---|---|---|---|
| A — Auditoria de arquivos/histórico | `CONCLUÍDO` | `0f64fa8e07` não é ancestral da candidata; proveniência e disposição estão em plano §22.8 e nesta seção | Nenhuma decisão de licença foi tomada |
| B — Semântica de `tumble()` | `CONCLUÍDO` (código já presente na candidata; revalidado) | `BacteriaColonyTumbleUpdatesDirectionBeforeFollowingGroCommand`, RNG kernel e testes `run`/`tumble` passaram nos presets | Nenhuma |
| C — Demo com divisão efetiva | `CONCLUÍDO` (código/modelo já presentes; revalidado) | `BacteriaColonyTwoSignalDemoLoadsRunsAndPersists` passou; evidência funcional em plano §22.8 | GUI viva deste run não foi exercitada integralmente |
| D — Viewer Qt6 interativo | `PARCIAL` | Build e processo Qt6; arquivo escolhido pelo diálogo; falha de automação relatada em §4.4 | Seleção de sinal, heatmap e reset requerem validação interativa |
| E — Branch candidata limpa | `CONCLUÍDO` | Candidata baseada em `50a340d`; commit `4d281158`; sem blobs contaminantes na ancestralidade/diff | Não foi aberto PR nem feito merge |
| F — Regressões finais | `PARCIAL` | Unit, kernel-unit, smoke e GUI build passaram localmente | GUI interativa e CI hospedada não demonstradas |

## 4. Diagnóstico técnico

### 4.1 Proveniência e exclusão dos exemplos/inventário

O commit `0f64fa8e07` foi auditado na branch original, que permanece disponível em `WiP20261008/BacteriaColony` no HEAD observado `dcc8bb8`. Seus conteúdos extrínsecos documentados no plano §22.8 são:

- `models/gro_examples/bandpass.gro`
- `models/gro_examples/chemotaxis.gro`
- `models/gro_examples/coupled_oscillator.gro`
- `models/gro_examples/dilution.gro`
- `models/gro_examples/edge.gro`
- `models/gro_examples/foreach.gro`
- `models/gro_examples/game.gro`
- `models/gro_examples/gfp.gro`
- `models/gro_examples/growth.gro`
- `models/gro_examples/inducer.gro`
- `models/gro_examples/maptocells.gro`
- `models/gro_examples/morphogenesis.gro`
- `models/gro_examples/signal_demo.gro`
- `models/gro_examples/signal_grid.gro`
- `models/gro_examples/skin.gro`
- `models/gro_examples/spatial_oscillations.gro`
- `models/gro_examples/spots.gro`
- `models/gro_examples/symbiosis.gro`
- `models/gro_examples/wave.gro`
- `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp` (4.907 linhas na alteração contaminada; inventário gerado com casos `SUCCEED()`).

Os cabeçalhos dos exemplos citam Eric Klavins/University of Washington e avisos de licença/redistribuição. Não os repliquei nem concluo se podem ser redistribuídos. A branch original preserva os arquivos para auditoria. O inventário gerado também não integra o histórico da candidata. Na worktree candidata existia uma cópia local não rastreada (178.031 bytes); hash SHA-256 antes/depois: `19aa0a2a965c127c2489de99a2e32c9ce8b2b29631aa7777d8fc390010c6cf37`. Ela foi preservada e não staged.

O teste `source/tests/unit/test_runtime_pluginmanager.cpp` continha hunks úteis de coordenadas contínuas/centradas junto com a contaminação. Conforme o registro §22.8, apenas esses hunks foram reaplicados seletivamente em `ff5aa935`; os outros conteúdos daquele commit não foram carregados. A candidata foi verificada sem `models/gro_examples/`, sem o inventário gerado no tree rastreado e sem ancestralidade de `0f64fa8e07`.

### 4.2 `tumble()` e orientação no mesmo passo

O teste de reprodução existente `BacteriaColonyTumbleUpdatesDirectionBeforeFollowingGroCommand` compara o estado lido logo após `tumble()` com a orientação final em execuções com seed fixa. O RED histórico registrado em §22.8 encontrou 18 divergências em 32 bactérias: o runtime amostrava uma direção provisória, mas `_applyKernelRandomTumbles` a sobrescrevia depois da sequência Gro.

A correção presente no candidato amostra pelo callback fornecido por `BacteriaColony` no ponto de execução do comando, usando o gerador `unif(0,1)` já existente no kernel. Assim comandos subsequentes observam a orientação efetiva e a ordem sequencial é mantida; não foi adicionado RNG. Nesta execução, testes de `tumble`, reprodutibilidade do RNG, programas nomeados e o teste `leader/follower` passaram nos presets integrais. Não alterei nem repliquei novamente esse código.

### 4.3 Demo, divisão, movimento e canais

O modelo original suportado `models/BacteriaColony_TwoSignalDemo.gen` foi preservado; seu conteúdo não foi editado nesta execução. O teste `BacteriaColonyTwoSignalDemoLoadsRunsAndPersists` passou em `tests-unit` e `tests-kernel-unit`. O teste existente executa a fixture ligada, exige população maior com descendente relacionado ao pai, compara posições iniciais/finais, verifica aumento de tamanho, valores positivos e distintos nos dois canais, leitura/emissão e maior máximo no campo legado que no canal adicional com degradação.

O registro de §22.8 documenta RED/GREEN da conexão Create → BacteriaColony → Dispose e a escolha de limiar 1.2 para divisão automática efetiva. A execução final dos dois presets revalidou as assertions. Persistência round-trip do demo e modelos suportados também foi incluída na cobertura unitária executada. O formato serializer e `BacteriaSignalGrid` não foram alterados.

### 4.4 GUI Qt6

`cmake --preset gui-app` e `cmake --build --preset gui-app --parallel 8` passaram nesta execução. Iniciei `genesys-gui` com `QT_QPA_PLATFORM=xcb` e abri o diálogo de modelo por Ctrl+O. A captura de janela raiz com `xwd` retornou erro X `BadMatch`; `gnome-screenshot` não estava instalado. AT-SPI Python (`pyatspi`) não está disponível. A automação abriu o menu superior, mas não conseguiu acionar a contribuição do viewer.

Não afirmo, nesta execução, que os seguintes itens foram verificados ao vivo: identificação da colônia dentro do viewer; lista e seleção de canais 1/2; mudança de heatmap; orientação e crescimento desenhados; divisão visível; seleção após reset; ou bloqueio de controles manuais durante calendário ativo. O processo GUI foi encerrado após tentativa de interação. Evidência de sessão anterior está em plano §22.8, que permanece histórica; porém o HANDOFF §2.4 afirma que apenas abertura de janela fora confirmada. Esta divergência precisa ser reconciliada pela revisão independente. Não houve evidência CI hospedada.

### 4.5 Coerência geométrica e correção

O backend do demo usa coordenadas centradas no domínio `[-3.5, 3.5]` para 8×8 células. O viewer calculava o deslocamento como `coordinate + 0.5`, que mapearia `-3.5 → -3`, `0 → 0.5`, `3.5 → 4`; os dois extremos escapam da grade e a origem é desenhada fora do centro. O contrato centrado para converter coordenada em espaço de células é `offset = coordinate + gridSize/2`, dando `0.5`, `4`, `7.5` para a grade 8×8.

Adicionei `BacteriaColonyViewerCoordinates::centeredCoordinateToCellOffset`, usado nos pontos de desenho das bactérias e barreiras, mais testes para os três valores 8×8 e origem 3×3. O teste ficou RED antes da fórmula corrigida e GREEN depois. Commit `4d281158`. A alteração afeta somente a conversão visual; armazenamento, amostragem dos sinais, limites de domínio e geometria da simulação permanecem iguais. Não surgiu evidência que justificasse alterar a extensão de separação, comprimento/largura, tolerância de sobreposição ou regra de borda.

## 5. Matriz de validação da HEAD candidata

| Verificação | Comando/contexto | Resultado | Evidência e limite |
|---|---|---|---|
| Inspeção focal de Gro/sinais/divisão/movimento/persistência | Coberta pelos CTest unitários/kernels e teste de demo | `PASS` | Testes relevantes listados nas seções 4.2–4.3; não é prova científica preditiva |
| `tests-unit` | `ctest --preset tests-unit --output-on-failure` | `PASS` | 1.902 passaram, 4 disabled, zero falhas; CTest registrou 1.906 casos incluindo 980 testes de inventário gerados a partir do arquivo local não rastreado |
| `tests-kernel-unit` | `ctest --preset tests-kernel-unit --output-on-failure` | `PASS` | 1.902 passaram, 4 disabled, zero falhas; mesma ressalva sobre inventário local |
| `PropertyEditorDoubleCommit` | três testes dentro dos presets acima | `PASS` | 3/3 passaram em ambos os presets; falha histórica não se reproduziu |
| `tests-smoke` | `ctest --preset tests-smoke --output-on-failure` | `PASS` | 3/3 passaram |
| `gui-app` | `cmake --preset gui-app`; `cmake --build --preset gui-app --parallel 8` | `PASS` | Configuração e build Qt6 passaram; build sem trabalho incremental pendente no fechamento |
| GUI com interação completa do demo/viewer | tentativa XCB nesta sessão | `NOT_VERIFIED` | diálogo de modelo exercitado; viewer não ativado; X capture `BadMatch`, pyatspi ausente |
| Persistência/modelos `.gen` | testes completos, incluindo demo round-trip | `PASS` (nos testes) | Nenhum `.gen` regravado nesta execução; branch diff contra `WorkInProgress` acrescenta apenas o demo autoral `models/BacteriaColony_TwoSignalDemo.gen` |
| Sanitizers | não executados | `NOT_RUN` | Nenhum defeito novo de ownership/lifetime/UB apareceu que exigisse esta execução |
| Validação de documentação | `python3 scripts/validate-ai-docs.py` | `PASS` | 6 Markdown canônicos de topo, 42 ativos e 25 oldies verificados; nenhuma exclusão liberada |
| CI hospedada | sem workflow associado verificado | `NOT_RUN` | Evidência estritamente local; nenhum resultado remoto é alegado |

Quatro testes históricos Search/Remove estavam desabilitados. Os 980 casos `KernelSimulatorMethodInventory` vêm do arquivo local não rastreado na configuração local do build; por serem `SUCCEED()` gerados, não são considerados prova comportamental e não foram commitados.

## 6. Diff, commits e publicação

- **Comparação:** candidata em relação a `origin/WorkInProgress` (`50a340d460876c657d8a3be66554bb29cfe8519a`).
- **Code commit desta execução:** `4d281158` — correção de mapeamento centrado no viewer e testes do helper; 4 arquivos, 43 inserções e 6 remoções.
- **Commits de publicação/documentação:** registrados no log da branch candidata, incluindo handoff verbatim, este relatório, atualização do plano/índice e ledger mensal.
- **Alterações funcionais anteriores da candidata:** documentadas no histórico da branch e plano §22.8; sem cherry-pick do commit contaminado.
- **Arquivos Gro de terceiros:** omitidos do histórico candidato; preservados na branch original de auditoria.
- **Inventário gerado:** omitido do histórico candidato; cópia local preservada, não rastreada e não staged.
- **Modelo `.gen` deste follow-up:** nenhum foi alterado; o diff da feature contra `WorkInProgress` contém apenas o modelo autoral `BacteriaColony_TwoSignalDemo.gen` entre arquivos `.gen`.
- **Push:** o relatório e os resultados devem estar publicados em `origin/WiP20261009/BacteriaColony-clean-candidate`; confirme após o push.
- **PR/merge:** nenhum PR aberto; nenhuma alteração enviada a `WorkInProgress` por merge.

## 7. Riscos e pendências

- **P1 — GUI interativa:** canais, heatmaps, reset e conflito com calendário não foram verificados nesta sessão e a documentação anterior é inconsistente quanto ao alcance da sessão de viewer precedente.
- **P1 — CI:** não há resultado CI hospedado vinculado a esta candidata; só resultados locais.
- **P2 — arquivo local gerado:** o inventário permanece no worktree do candidato e pode alterar a contagem local de testes; está preservado mas fora do Git. Não removê-lo sem autorização do mantenedor.
- **P2 — licença:** exemplos Gro de terceiros não estão na candidata; decisão/licença de redistribuição não foi tomada.
- **Limite científico aceito:** o campo usa relaxação fenomenológica por passo, não uma alegação de PDE conservativa ou predição biológica.

## 8. Veredito e próxima ação

- **Veredito:** `PARTIAL_NEEDS_REVIEW` (`BC-HANDOFF-001-REPORT-PUBLISHED`).
- O código candidato está limpo em relação à contaminação descrita, os testes/builds locais exigidos passaram e a correção visual tem cobertura unitária. A GUI interativa completa e CI hospedada continuam gates pendentes; por isso `AUTO-BACTERIA-001` permanece `blocked-review` e não é marcado `done_confirmed`.
- **Revisão necessária:** reconciliar HANDOFF §2.4 com plano §22.8; executar o viewer do demo e registrar seleção de ambos os canais, reset e interação durante calendário ativo. Depois, revalidar a candidata e decidir sobre PR.
- **Nenhum PR/merge foi criado.**

`BC-HANDOFF-001-REPORT-PUBLISHED`

## 9. Execução do BC-HANDOFF-002 — polling horário e gates de revisão

**Data:** 2026-10-11. **Status:** `PARTIAL_NEEDS_REVIEW`; a rodada foi executada e
o marcador final registra o resultado parcial. O trabalho continua na candidata
limpa, sem repetir BC-HANDOFF-001.

### 9.1 Checkout e governança

- Branch: `WiP20261009/BacteriaColony-clean-candidate`.
- HEAD local da candidata antes da sincronização: `9345124f647cb4a7f910b5f2ff7577a38b253db9`;
  remoto consultado: `bafa57b80d158253e283037a0df9e642b1f54a96`. Após fetch e
  fast-forward sem conflito, o HEAD local passou a coincidir com o remoto.
- HEAD inicial e remoto coincidiam. A branch original
  `WiP20261008/BacteriaColony` permaneceu em `dcc8bb89910e3b0f8d8c15f29f89dcb36846f230`.
- A referência local `WorkInProgress` estava desatualizada; foram lidas as regras
  canônicas atuais em `origin/WorkInProgress` (`50a340d460876c657d8a3be66554bb29cfe8519a`),
  incluindo `/README.md`, governança, arquitetura, status, backlog e runbooks.
  Nenhuma regra autorizou PR, merge ou promoção.
- O arquivo local preexistente não rastreado
  `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp`
  foi preservado. A validação ocorreu em worktree destacado limpo
  `/tmp/Genesys-bc002-validation`, sem mover ou regravar a cópia original.

### 9.2 Cron local instalado

- **Instalação:** concluída no crontab do usuário, sem `sudo` ou cron privilegiado.
- **Frequência e relógio:** `20 * * * *`, no relógio local
  `America/Sao_Paulo` (UTC−03:00). `cron.service` estava `active` e o relógio
  sincronizado (`NTPSynchronized=yes`).
- **Entrada identificada:**
  `# GENESYS-BC-HANDOFF-002`
  `20 * * * * /home/rafaelcancian/.local/bin/genesys-bacteria-colony-handoff-poll --cron`
- As duas entradas preexistentes do Back In Time foram preservadas. A instalação
  é idempotente: remove apenas a entrada correspondente ao marcador e adiciona
  uma única cópia, sem substituir o restante do crontab.
- Wrapper local (fora do Git):
  `/home/rafaelcancian/.local/bin/genesys-bacteria-colony-handoff-poll`, modo
  `0700`; estado e log sob
  `/home/rafaelcancian/.local/state/genesys-bacteria-colony-handoff-poll`, diretório
  `0700` e arquivos privados `0600`. Não há dependência de `build/`.
- Dependências efetivamente disponíveis: `crontab`, daemon `cron`, `flock`,
  `timeout`, Git, `sha256sum` e Codex CLI. CLI verificada: `codex-cli 0.162.1`;
  `codex login status` retornou sucesso. O teste isolado não interativo respondeu
  `CODEX_NONINTERACTIVE_OK` com `codex exec --approve-for-me --ephemeral --cd …`.
- Foram rejeitadas duas combinações de flags que o parser real não aceita
  (`--no-alt-screen` em `exec` e `--sandbox` junto de `--approve-for-me`); o
  wrapper final usa apenas a forma validada. `--approve-for-me` aplica revisão
  automática dentro do sandbox `workspace-write`; não foi usado bypass nem
  permissão elevada.
- Além do smoke de resposta, uma execução descartável adicional executou `pwd`
  e `git status --short --branch` com a aprovação automática/sandbox, retornando
  `CODEX_WORKSPACE_TOOL_PASS` (exit 0), sem mutar arquivos. A saída detalhada foi
  mantida apenas em `/tmp/bc002-codex-tool-check.log` e não foi versionada.
- O wrapper obtém lock não bloqueante por `flock -n` antes de qualquer polling;
  outra instância sai com sucesso e registra `lock_busy`, sem iniciar Codex. Um
  teste concorrente real produziu esse evento. O tempo máximo do Codex é 50 min,
  seguido de TERM e tolerância de 30 s para encerramento do processo próprio.
- O log registra apenas timestamp, ID, evento e detalhes curtos; não contém
  prompt completo, saída do modelo, credenciais ou dumps. Ao ultrapassar 256 KiB,
  gira quatro backups limitados. Diretório `0700`, log/lock `0600`.
- O polling lê HANDOFF e REPORT do mesmo HEAD obtido por fetch. Em ausência de
  ID novo, encerra antes de autenticar/invocar Codex, sem alteração no checkout.
  O teste `--simulate-trigger` encontrou `BC-HANDOFF-002` com `001` concluído;
  este teste não invoca o modelo. O parser foi ajustado para não confundir o ID
  ativo com a menção histórica ao handoff encerrado.
- Para impedir que o cron inicie esta execução antes de seu marcador remoto, foi
  criado um arquivo local temporário `state/hold`; a primeira invocação cron
  agendada durante a publicação deverá registrar `held`. Ele será removido após
  confirmar o marcador remoto; então a chamada manual do wrapper verificará o
  caminho idempotente `no_new_handoff` sem executar Codex. O resultado da
  invocação agendada e dessa chamada será acrescentado por adendo após ocorrerem.
- **Desativação segura:** executar `crontab -e`, remover somente as duas linhas
  contendo `GENESYS-BC-HANDOFF-002` e seu comando `genesys-bacteria-colony-handoff-poll --cron`,
  salvando o crontab. Confirmar que as duas entradas Back In Time continuam
  presentes. O wrapper, estado e logs ficam locais para auditoria; removê-los é
  opcional e requer decisão separada do mantenedor.

### 9.3 Regressões em worktree limpa

Foi configurado o preset `tests-unit` e compilados 558 alvos no GCC 13.3.0,
CMake 3.28.3, Ninja 1.11.1, C++23 e Qt 6.4.2. O build gerou um inventário de 980
testes stubs `SUCCEED()` na worktree temporária. O primeiro uso de `-LE autogen`
não os excluiu porque `gtest_discover_tests` os registrou somente com label
`unit`; esse resultado não foi usado para a contagem comportamental. A execução
válida excluiu pelo nome real `KernelSimulatorMethodInventory`:

- `ctest --preset tests-unit --output-on-failure -E '^KernelSimulatorMethodInventory\.'`:
  **922 passaram**, quatro históricos Search/Remove desabilitados, zero falhas;
  926 testes comportamentais descobertos.
- `cmake --build build/tests-unit --target genesys_kernel_unit_tests_run --parallel 8`:
  alvo kernel concluiu com exit 0; executou seus binários listados pelo CMake,
  inclusive a geração artificial de 980 stubs, contados separadamente acima.
- Preset `tests-smoke`, configurado na mesma worktree/build isolada para reutilizar
  objetos compatíveis: build do alvo smoke e
  `ctest --test-dir build/tests-unit -L smoke --output-on-failure`: **3/3 passaram**.
- Preset `gui-app`, também configurado nessa build isolada após os testes:
  `genesys_gui` compilou e linkou (**PASS**). O build apresentou um aviso de
  depreciação Qt em `qteditorfactory.cpp`; não houve erro.
- No CTest comportamental, `BacteriaColonyTwoSignalDemoLoadsRunsAndPersists`,
  os testes de dois canais, `tumble`, divisão, movimento e os três
  `PropertyEditorDoubleCommit` passaram. Nenhum arquivo `.gen` foi regravado.
- Não houve execução de CI hospedada nem sanitizers nesta rodada.

### 9.4 Viewer Qt6 e modelo de dois sinais

O `genesys-gui` construído da worktree limpa foi aberto em XCB. Pelo diálogo
`Model > Open`, foi carregado `models/BacteriaColony_TwoSignalDemo.gen`; a aba
mostrou `Bacteria Colony Two Signal Demo`. O viewer foi aberto por
`Tools > Extensions > Biological > Visualize Bacteria Colony...`. Isso prova
interação real com a aplicação Qt6, além do build.

No viewer:

- A colônia correta foi selecionada; a grade exibiu bactérias com corpo orientado
  e o selector listou `Signal 1` e `Signal 2` após a primeira execução.
- Um `Step colony` manual mostrou 32 comandos e seis mutações de sinal, sem
  avançar o relógio do modelo, conforme indicado pelo viewer.
- Iniciar a simulação pelo calendário levou o modelo ao término da replicação;
  ao reabrir o viewer para o modelo, o estado mostrado era `time=1`, população 6
  e seis bactérias, partindo de três. A saída observada mostra divisão ocorrida,
  mobilidade/posições atualizadas e campos não nulos. O teste do demo confirma
  crescimento e volume; esta captura, por si só, não mede crescimento físico.
- Com o mesmo estado da colônia, `Signal 1` mostrou faixa `[0, 88]` e `Signal 2`
  `[0, 7.996]`. Capturas da região da grade tiveram diferença absoluta de 3.870
  pixels, demonstrando heatmaps diferentes. Os PNGs estão no ledger de evidência
  e seus SHA-256 são registrados abaixo.
- A execução principal retornou à janela do modelo enquanto a simulação rodava;
  o viewer não estava visível na árvore de janelas após o término e foi aberto
  novamente, quando refletiu o estado final. Não atribuo causa a esse
  comportamento.
- **Ainda não demonstrado nesta rodada:** reset de replicação com seleção
  preservada/invalidada; acionamento de `Step colony` exatamente enquanto o
  calendário estava em estado running/paused. A inspeção de
  `_executeSelectedColonyStep()` confirmou guarda para `isRunning()` e
  `isPaused()`, que interrompe o timer e impede execução manual nesses estados,
  mas isso é evidência de código, não prova interativa do estado concorrente.
  Assim, o gate de viewer é `PARCIAL`, sem alegar cobertura visual completa.

### 9.5 Evidência, publicação e limites

- Captura Signal 1:
  `history/evidence/2026/10/bacteria-colony-viewer-signal-1-2026-10-11.png`,
  SHA-256 `f09190b028108d1b5b2f8c97e42e87705113a48e7d51822f8ab7e45b5aff5d84`.
- Captura Signal 2:
  `history/evidence/2026/10/bacteria-colony-viewer-signal-2-2026-10-11.png`,
  SHA-256 `147244a5a58f14957523a91b7718641db8dc105c9e85b95ce89f65173a3e8e6c`.
- Ambiente local: Linux x86_64, X11/XCB em sessão Wayland, Qt 6.4.2; screenshot
  capturado por janela X11 do viewer. Os PNGs são evidência desta sessão e não
  alegação de CI.
- A base atual ainda é `origin/WorkInProgress`
  `50a340d460876c657d8a3be66554bb29cfe8519a`; não foram encontrados commits
  posteriores nesta atualização. A candidata continua sem PR ou merge.
- Estado final, commits, resultado do primeiro disparo real do cron e chamada
  idempotente sem hold serão registrados no adendo final, depois do disparo.

## 10. Encerramento do BC-HANDOFF-002

- Handoff executado: BC-HANDOFF-002.
- Status: PARTIAL_NEEDS_REVIEW. Instalação do cron, caminho de execução Codex,
  bloqueio concorrente, idempotência no wrapper, regressões limpas e interação
  principal com o viewer foram verificados. Permanecem pendentes a interação de
  reset/invalidade de seleção e a tentativa manual durante running/paused; o
  código contém a guarda, mas falta evidência interativa.
- Commits desta rodada até a preparação deste encerramento: 2c3c2eac (relatório,
  ledger e capturas) e dea38aba (teste não interativo da ferramenta Codex). O
  commit de encerramento será identificado no próximo adendo após publicação.
- Arquivos .gen: nenhum foi alterado ou regravado nesta rodada. O modelo
  demonstrativo existente foi carregado na GUI; alterações temporárias ocorreram
  apenas na memória do processo e não foram salvas.
- Cron: a entrada permanece em hold até a confirmação remota deste marcador.
  Depois, o hold será removido e o wrapper será chamado uma vez para verificar
  o caminho no_new_handoff sem executar Codex. O tick horário real será
  registrado por adendo quando ocorrer.
- CI: nenhum workflow foi disparado. Não há evidência de CI hospedada.
- Nenhum PR foi aberto; não houve merge em WorkInProgress.

BC-HANDOFF-002-REPORT-PUBLISHED
