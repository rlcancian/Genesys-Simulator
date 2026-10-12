---
document_type: reference
authority: task-specific-handoff
owner: project-maintainer
last_reviewed: 2026-10-11
status: active
---

Antes de qualquer análise ou alteração, leia obrigatoriamente /docs/ai_assistants/README.md no branch WorkInProgress e informe-se sobre todas as regras atuais do GenESyS.

# GenESyS — Canal de comunicação ChatGPT ↔ Codex para BacteriaColony

**Protocolo ativo:** `BC-HANDOFF-003` (2026-10-11); `BC-HANDOFF-001` e `BC-HANDOFF-002` encerrados e documentados no REPORT.
**Repositório:** `rlcancian/Genesys-Simulator`.
**Branch operacional atual:** `WiP20261009/BacteriaColony-clean-candidate` (a antiga `WiP20261008/BacteriaColony` permanece como auditoria).
**HEAD de implementação revisado:** `a4277d094320b3a89485b9eba460af9304718aed` (o HEAD da branch avança ao publicar documentação).
**Plano aprovado:** [BACTERIA_COLONY_GRO_INTEGRATION_PLAN.md](BACTERIA_COLONY_GRO_INTEGRATION_PLAN.md).
**Relatório corrente do Codex:** [BACTERIA_COLONY_AGENT_REPORT.md](BACTERIA_COLONY_AGENT_REPORT.md).

## 1. Contrato permanente de comunicação

1. ChatGPT escreve **instruções, diagnóstico e decisões** somente neste arquivo HANDOFF. Codex **não o modifica**, salvo erro factual comprovado: nesse caso, registra a divergência no REPORT e aguarda revisão.
2. Codex escreve seu **relatório completo e atualizado** somente em `docs/ai_assistants/reference/BACTERIA_COLONY_AGENT_REPORT.md` e publica o commit. ChatGPT **não edita esse relatório**. O documento é um handoff operacional mutável, não uma evidência histórica imutável ou substituto de `STATUS.md`.
3. Para cada execução concluída, Codex também cria, quando houver validação material, evidência datada conforme `docs/ai_assistants/history/evidence/README.md`, com referência no ledger mensal, respeitando a governança. Logs volumosos, dumps e artefatos gerados ficam fora do Git salvo política explícita.
4. Depois de publicar relatório e código, Codex informa ao usuário **apenas**: status, branch, HEAD, link do REPORT e bloqueio crítico se houver. Não precisa copiar o relatório na conversa.
5. O usuário poderá pedir ao ChatGPT: "Revise o último relatório do Codex sobre BacteriaColony." ChatGPT consultará remotamente o REPORT, o HEAD e os diffs, e colocará o feedback e novas instruções no HANDOFF, em outro commit, sem depender de copiar/colar conversas.
6. Codex deve, no início de cada sessão, `git fetch origin`, identificar a branch de trabalho, integrar com segurança o HANDOFF publicado remotamente (preferir `git pull --ff-only` quando for possível), lê-lo e **registrar no REPORT o identificador do handoff efetivamente executado**. Não descartar mudanças locais para fazer pull.
7. Cada novo handoff recebe um ID incremental `BC-HANDOFF-NNN`; um mesmo ID deve ser executado **uma única vez**. Se o REPORT já contém execução concluída desse ID, não repetir trabalho: apenas informar situação e aguardar revisão.
8. As instruções deste arquivo não se sobrepõem a `GOVERNANCE.md`, `ARCHITECTURE.md`, `STATUS.md`, backlogs ou runbooks. Se houver conflito real, documente e pare a parte dependente.
9. Não editar simultaneamente os dois documentos pelo mesmo agente. Nunca force-push/rebase destrutivo, nunca apagar arquivos não rastreados do mantenedor. Não abrir PR nem fazer merge em `WorkInProgress` sem autorização posterior.

## 2. Base técnica e estado da revisão independente

O último trabalho local reportou nove ciclos executados: sete completos e dois parciais. Codex informou `tests-unit` e `tests-kernel-unit` com 1.899 testes executados/passando cada, quatro desabilitados; smoke 3/3; build `gui-app` concluído. **São relatos locais do Codex, não testes executados pelo ChatGPT.** A interação GUI com o modelo de dois canais não foi demonstrada; tampouco há CI hospedada vinculada à branch.

A revisão independente examinou o código e os commits até `a4277d09` e confirmou implementação de canais independentes, coordenadas contínuas, divisão híbrida com configuração persistente, movimento cinemático, seletor de canal no viewer e suporte de leitura de `sourceCode` Gro multilinha legado no `GenSerializer`.

### 2.1 Problema prioritário de composição do histórico (confirmado no Git)

O commit `0f64fa8e07` (`chore: synchronize local changes`) incorporou **19 exemplos .gro** sob `models/gro_examples/` com avisos de copyright de terceiros, além de `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp` com cerca de 4.900 linhas de inventário e testes `SUCCEED()`, e mudanças no teste de runtime.

Esses conteúdos **não foram autorizados como parte da entrega BacteriaColony**. A licença e os direitos de redistribuição dos exemplos devem ser examinados; não se conclui automaticamente incompatibilidade, mas eles não devem ser incluídos na candidata de integração. O inventário gerado não verifica comportamento e também é estranho ao escopo.

Apenas deletar os arquivos no commit seguinte **não retira esses blobs do histórico** a ser integrado. Construa uma **branch candidata limpa baseada no estado atual de `origin/WorkInProgress`**, reaplicando seletivamente apenas as mudanças pertinentes e verificadas (sem cherry-pick integral do commit `0f64fa8e07`). Preserve a branch original para auditoria. Não altere o histórico original por force-push. Não transporte os 19 exemplos nem o inventário gerado. Preserve qualquer alteração legítima misturada naquele commit após inspeção específica.

É aceitável concluir correções funcionais na feature branch original antes de materializar a candidata limpa. Ao mudar para a candidata, inclua ambos os documentos de handoff, mantenha a branch e o HEAD atualizados no REPORT e informe o nome exato para as revisões seguintes. **Não faça merge da candidata em `WorkInProgress`.**

### 2.2 Divergência potencial na semântica de `tumble()` (forte indício)

`GroProgramRuntime` atribui uma orientação após `tumble()`; posteriormente `BacteriaColony::_applyKernelRandomTumbles` faz a amostragem efetiva pelo RNG do kernel e sobrescreve a orientação. Um comando Gro posterior no **mesmo passo** que leia `direction`/`theta` poderá observar um valor diferente do aplicado.

Construa teste focalizado `tumble(); observed := direction;` incluindo a ordem de execução e repetições com seed fixa. Caso reproduzido, corrija com o menor impacto, preservando o RNG canônico e a semântica sequencial dos comandos; não crie um segundo gerador/scheduler. Não marque como defeito confirmado sem RED real.

### 2.3 Demonstração incompleta da divisão visual (evidência limitada)

`models/BacteriaColony_TwoSignalDemo.gen` tem divisão automática habilitada, mas o teste executado relatado verifica três bactérias após um passo. **Isso não demonstra divisão automática ocorrendo no cenário**. Acrescente ao demonstrativo e ao teste uma execução limitada e reproduzível que de fato cause ao menos uma divisão e comprove geração, volumes, estado e comunicação multicanal. Preserve os testes unitários de divisão já existentes. Não modifique o motor apenas para satisfazer o exemplo.

### 2.4 GUI multicanal não demonstrada

O viewer Qt6 contém seletor e consulta por canal, mas apenas foi confirmada a abertura de janela, não o cenário de dois canais dentro do viewer. Execute uma validação funcional do modelo aberto: canais 1/2, heatmaps diferentes, posição/orientação e crescimento/divisão, reinicialização e seleção inválida, interação entre passo manual e calendário. Se automação gráfica não estiver disponível, registre exatamente o que foi e não foi observado. Não trate startup como teste de interação.

### 2.5 Consistência geométrica a verificar

O runtime aplica separação posicional de retângulos orientados aproximados, enquanto o viewer usa eixos derivados de `size` e largura visual própria. Verifique diferença entre comprimento/largura usados na separação, desenho, bordas e localização após divisão. Corrija apenas falhas demonstráveis e mantenha sobreposições parciais permitidas, sem física externa.

## 3. Missão autônoma autorizada para BC-HANDOFF-001

Execute em ciclos pequenos e revisáveis, sem retornar ao usuário após cada subtarefa:

**Ciclo A — Auditoria e saneamento:** identifique todo conteúdo extrínseco no histórico, verifique se o código de testes úteis no commit misturado deve ser preservado e planeje a candidata limpa. Documente a proveniência e as exclusões. Não remova arquivos locais do mantenedor.

**Ciclo B — `tumble()`:** reproduza a possível divergência entre orientação observada dentro de um passo e orientação final; corrija apenas caso confirmada; teste seed/reset/reprodutibilidade.

**Ciclo C — Cenário demonstrativo:** faça o demo exercitar divisão real, crescimento, movimento, emissão, leitura e dois campos com degradações distintas; afirme resultados concretos, não somente propriedades de configuração.

**Ciclo D — GUI:** abra o demo no viewer real, troque canais e valide o mapa de calor e a geometria, com evidência visual/interativa se possível; investigue somente inconsistências identificadas.

**Ciclo E — Integração limpa:** materialize a branch candidata sem conteúdos extrínsecos e com histórico revisável, incluindo os documentos de handoff. Execute regressões no HEAD da candidata; revise o diff em relação a `origin/WorkInProgress`, inclusive os três commits pelos quais a branch original estava atrás na revisão anterior. Se houver conflito com mudanças integradas recentes, investigue e corrija sem perda de conteúdo.

**Ciclo F — Fechamento:** rode presets reais `tests-unit`, `tests-kernel-unit`, `tests-smoke`, build `gui-app` e checks focados. Use diagnóstico de memória onde tecnicamente útil. Faça os commits e push na candidata, sem PR/merge, e preencha o REPORT completo.

### Critérios de conclusão

- Candidata parte de `WorkInProgress` atualizado, sem exemplos originais Gro nem inventário de testes `SUCCEED()` no diff/histórico introduzido.
- `tumble()` testado para consistência temporal; correção quando RED reproduzido.
- Demo mostra **divisão ocorrida**, movimento e dois canais independentes.
- Viewer foi exercitado com o demo aberto, ou lacuna documentada objetivamente.
- Persistência dos modelos suportados validada.
- Regressões obrigatórias executadas, com contagens e logs verificáveis.
- Nenhuma mudança de arquitetura, formato incompatível ou escopo científico não autorizada.
- Relatório Codex foi publicado, com ID `BC-HANDOFF-001`, branch, HEAD, links de commits/evidências, resultados e limites.

## 4. Gatilhos de parada e autoridade

Pare o trabalho dependente diante de incompatibilidade comprovada de formato persistido, conflito de branch sem resolução segura, falha bloqueante de regressão, decisão arquitetural/científica/licenciamento não autorizada ou necessidade de alterar o kernel genericamente. Prossiga somente nas subtarefas independentes.

O usuário autorizou autonomia **neste escopo específico**, não publicação em produção, limpeza destrutiva nem promoção para branches estáveis.

## 5. Publicação e resposta

O relatório deve seguir o contrato em `BACTERIA_COLONY_AGENT_REPORT.md`. Após cada checkpoint substancial, atualize e publique o REPORT no GitHub; mantenha detalhes de execução relevantes, inclusive comandos e resultados. Não reescreva fatos anteriores de modo a apagar falhas.

A resposta final local deve ser curta:

`BC-HANDOFF-001 | STATUS | branch | HEAD | docs/ai_assistants/reference/BACTERIA_COLONY_AGENT_REPORT.md`

A partir daí, o usuário pede à instância ChatGPT uma revisão do relatório remoto e esta publica `BC-HANDOFF-002`, se necessário.

---

## 6. BC-HANDOFF-002 — Ciclo horário ChatGPT ↔ Codex (AUTORIZADO, 2026-10-11)

**Status da instrução:** `READY`. **Branch operacional atual:** `WiP20261009/BacteriaColony-clean-candidate`. **HEAD anterior ao handoff:** `9345124f647cb4a7f910b5f2ff7577a38b253db9`.

**Autorização explícita do mantenedor:** instalar e ativar um cron no computador local, executado **uma vez por hora**, que consulte este HANDOFF e acione o Codex local apenas quando houver novo ID de instruções. O ChatGPT terá uma tarefa agendada separada, também horária, para ler REPORT e publicar novo HANDOFF quando houver progresso novo. Esta autorização limita-se a BacteriaColony e não libera merge, PR, push em outras branches, promoção, deleção, instalação privilegiada ou decisões arquiteturais não autorizadas.

### 6.1 Primeira obrigação: estabelecer o cron local durável

1. Confirme o repositório, branch, HEAD, worktree(s), Git/gh, autenticação Git, comando real e versão do `codex` disponível no computador. Inspecione `codex exec --help` e os comandos de login/status suportados pela versão instalada. **Não invente flags CLI.**
2. Crie, fora do repositório (por exemplo, em `~/.local/bin/` e `~/.local/state/`, se adequados ao ambiente real), um wrapper idempotente e auditável para o cron. Não versionar segredos nem dependências de uma pasta `build/`. Descubra o caminho absoluto do checkout/worktree da branch candidata, não presuma `~/Genesys-Simulator`.
3. Preserve as entradas existentes do crontab. Instale **uma única entrada identificada por marcador inequívoco**, sem duplicação, para executar o wrapper em `20 * * * *` no relógio local. Não substitua entradas alheias. Não use `sudo` nem instale cron do sistema: use o crontab do usuário; se indisponível, registre bloqueio e não afirme que o cron está funcionando.
4. Proteja a execução por lock de processo (por exemplo `flock -n` com arquivo sob estado local), evitando duas execuções de Codex simultâneas. Use logs locais rotativos/limitados e identificados por horário, sem tokens, segredos, dumps extensos ou dados privados.
5. A cada disparo, o wrapper deve fazer *polling leve*: confirmar branch, fazer `git fetch`, consultar HEAD remoto dos dois documentos e comparar o **ID de instrução ativa** com o **último ID concluído no REPORT**. **Se não houver instrução nova, encerrar com sucesso imediatamente, sem invocar o modelo, modificar arquivos ou fazer commit/push.**
6. Havendo novo handoff, sincronizar com segurança os commits documentais da branch (`git pull --ff-only` somente quando possível), preservar arquivos sujos/não rastreados e acionar **uma única execução não interativa** do Codex com instrução curta para ler o HANDOFF, executar o ID novo, atualizar REPORT, registrar evidências pertinentes, commit e push. Não usar modos de bypass de sandbox/aprovação ou permissões elevadas. Caso a CLI/credenciais/política não permitam execução não supervisionada, registrar impedimento no REPORT e notificar o mantenedor; não fingir sucesso.
7. Não usar um relógio/cron para atualizar os componentes do simulador. O agendamento é exclusivamente do *agente de desenvolvimento* e não altera a simulação nem eventos GenESyS.
8. Defina uma política local explícita para encerramento de jobs abandonados e logs e verifique com um teste de execução manual e um disparo simulado (sem codificação repetida). Não mate testes ou agentes em funcionamento apenas porque chegou a próxima hora; uma chamada concorrente deve pular por lock.
9. **Término e idempotência:** marque o ID como concluído somente após efetiva publicação do REPORT no remoto. Em caso de erro de Git, indisponibilidade, ou perda de acesso ao modelo, registrar erro local e tentar novamente na próxima hora de forma segura, sem produzir commits duplicados nem reiniciar modificações já aplicadas.
10. A instalação do cron deve sobreviver ao encerramento da sessão atual do Codex e deve ser removível por procedimento documentado. Registre linha `crontab`, wrapper, comandos verificados, teste, comportamento no cron real, forma de parar e possíveis dependências no REPORT, **sem registrar segredos**.

### 6.2 Trabalho de engenharia autorizado no ciclo BC-HANDOFF-002

Além da configuração do cron, concentre-se nos gates ainda pendentes **do relatório BC-HANDOFF-001**, presente no arquivo REPORT desta candidata:

- Verificar o viewer Qt6 realmente com `models/BacteriaColony_TwoSignalDemo.gen` carregado. Demonstrar seleção Signal 1/Signal 2 e diferença do heatmap, visualização das bactérias, crescimento/divisão, reset e bloqueio de passo manual em simulação ativa; anexar evidência técnica reprodutível. Consulte o ponto de extensão/ação Qt real para abrir o viewer: não instalar `pyatspi` às cegas.
- Se a interação física de GUI não for automatizável, criar teste de integração GUI apropriado (Qt Test ou harness existente, se houver), isolando o menor contrato verificável. Registrar a parte que permanece sem prova visual; não afirmar que um teste unitário equivale ao uso real.
- Executar testes de regressão relevantes em *build limpo e isolado* (ou purgado somente na pasta de build gerada, sem tocar arquivos do mantenedor), para que o inventário local não rastreado `source/tests/unit/generated/test_kernel_simulator_method_inventory.generated.cpp` não contamine o inventário CTest. Verificar na configuração real a inclusão de `*.generated.cpp`: não presumir que basta uma pasta build separada; preservar o inventário local e, se necessário, isolar os fontes por worktree limpo próprio. Separar número de testes comportamentais do inventário artificial.
- Conferir base atual `WorkInProgress` e eventuais novos commits que afetem integração. Não fazer merge. Não disparar workflows CI pagos/desnecessários automaticamente; registrar a ausência de CI hospedada como pendência de PR.
- Não rever o desenvolvimento dos nove ciclos nem abrir outras frentes; corrigir apenas defeitos reproduzidos dentro do escopo aprovado.

### 6.3 Protocolo do laço horário e prevenção de ping-pong

O ChatGPT verifica remotamente a branch horariamente, em torno do minuto 05, e só publica `BC-HANDOFF-003` **se** o relatório publicado contiver `BC-HANDOFF-002-REPORT-PUBLISHED` (ou outro marcador explícito de finalização equivalente para o mesmo ID), indicando um resultado material que foi de fato revisado. Se não houver novidade, não altera HANDOFF, não cria commits e não notifica o usuário. Se há bloqueio humano, registra parecer e não repete a mesma instrução automaticamente.

Codex deve ter **um único escritor do REPORT** e não alterar o HANDOFF. ChatGPT é o único escritor do HANDOFF, não altera REPORT. Quando o ChatGPT gerar uma nova versão, atualizará cabeçalho do protocolo e ID de missão, preservando os trechos históricos.

Não execute `BC-HANDOFF-001` novamente: está fechado e registrado no REPORT. Execute `BC-HANDOFF-002` **uma única vez**, materialize o cron e siga as verificações correspondentes; nas horas futuras, o cron somente reage a IDs novos.

### 6.4 Relatório exigido

Publique no `BACTERIA_COLONY_AGENT_REPORT.md`:

- `Handoff executado: BC-HANDOFF-002`, status claro e **marcador final `BC-HANDOFF-002-REPORT-PUBLISHED`** quando a rodada estiver encerrada (mesmo que parcialmente, com os bloqueios declarados);
- instalação do cron (sim/não), horário real, comando validado, lock/idempotência, logs, testes, procedimento de desativação;
- arquivos/commits/HEAD, ambiente, resultados de GUI e testes, logs/evidência rastreáveis;
- pendências e riscos, sem afirmações de execução que não ocorreu.

Ao concluir a rodada, faça commit e push na candidata. **Não abra PR nem faça merge**. O crontab é local: publicar o HANDOFF no GitHub **não instala o cron**; somente sua execução local fará isso.


---

## 7. BC-HANDOFF-003 — Fechar validação interativa de GUI e entregar parecer técnico (2026-10-11)

**Status da instrução:** `READY`. **Handoff anterior:** `BC-HANDOFF-002`, concluído com marcador `BC-HANDOFF-002-REPORT-PUBLISHED` no REPORT desta branch. **Branch exclusiva de desenvolvimento:** `WiP20261009/BacteriaColony-clean-candidate`. **HEAD revisado:** `478b9784c0cce5a9a8c9ff6eb9eb588f1142e284`.

### 7.1 Contexto e prioridade

A rodada anterior confirmou instalação do cron local (primeiro disparo real em 2026-10-11, 14:20 -03), execução Codex não interativa, proteção `flock`, build Qt6, testes comportamentais 922/922 executados com quatro desabilitados fora da contagem de sucesso, e interação real do viewer com demo carregado, duas seleções e heatmaps diferentes. Foi observada população final de 6 a partir de 3. Há capturas e hashes no REPORT.

**Não repita os ciclos anteriores.** Completar exclusivamente os gates remanescentes e preparar uma recomendação de revisão independente. Os testes `SUCCEED()` gerados no build não constituem cobertura comportamental. Mantenha o arquivo inventário local do mantenedor intocado.

### 7.2 Trabalho autorizado (uma rodada curta e auditável)

1. **Verifique o reset/seleção do viewer:** com `models/BacteriaColony_TwoSignalDemo.gen` aberto, experimente reset/início de nova replicação após selecionar `Signal 2`, verifique se canais reconstituídos são apresentados corretamente, se seleção antiga inválida é reajustada e se não ocorre leitura fora dos limites. Diferencie o teste interativo do teste de estado Qt sem renderização, quando necessário.
2. **Verifique o bloqueio de passo manual durante simulação running/paused:** reproduza preferencialmente em Qt6 real; se o modelo demonstrativo terminar rápido demais, use configuração de desenvolvimento/teste controlada em um modelo temporário ou um harness Qt Test existente, sem ampliar o contrato persistido. Registre tentativa, estado da simulação observado, bloqueio de execução manual e ausência de avanço duplicado. Não declare PASS apenas por existir uma guarda `isRunning()/isPaused()` no código.
3. **Confirme o estado da interface do viewer após terminar o calendário:** na rodada anterior o diálogo não permaneceu visível na árvore de janelas durante a execução; investigue se é comportamento esperado do plugin, fechamento programático, pai Qt ou limitação da automação. Só corrija se conseguir reproduzir defeito dentro do escopo. Evite remodelar a arquitetura Qt ou ciclo de vida do simulador.
4. **Confira regressões proporcionais aos arquivos efetivamente alterados.** Se não houver patch funcional, reutilize apenas evidências do HEAD exato validado na rodada 002, complementando os gates novos; se houver patch, execute testes focados e os presets obrigatórios conforme GOVERNANCE. Faça inventário CTest sem contar `KernelSimulatorMethodInventory` artificial como teste comportamental.
5. **Cheque se há mudanças concorrentes em `origin/WorkInProgress`** e se a candidata continua isenta dos blobs extrínsecos. Não abra PR nem faça merge. CI hospedada continua uma condição de futura revisão de PR, não justifica ficar gerando commits de heartbeat.

### 7.3 Critérios de aceitação e parada

O relatório deve apresentar, com fatos e evidências, status de cada gate: `GUI_RESET`, `GUI_RUNNING_PAUSED_MANUAL_GUARD`, `VIEWER_LIFECYCLE`, `REGRESSIONS`, `CLEAN_BRANCH`, `HOSTED_CI`.

Marque cada gate `PASS`, `FAIL`, `PARTIAL`, ou `NOT_RUN` e justifique o alcance; verificação de GUI somente por inspeção de código não basta para `PASS` de interação.

Se a GUI continuar não automatizável mesmo após tentativa razoável, documente o impedimento, a menor ação manual necessária e pare; não insista em repetidas rodadas sem nova evidência.

**Não altere o arquivo HANDOFF**, que pertence ao ChatGPT. Atualize o REPORT com `BC-HANDOFF-003`, incluindo commits, HEAD, comandos/testes, eventuais imagens e o marcador terminal **`BC-HANDOFF-003-REPORT-PUBLISHED`**. Publique na branch candidata. Não abra PR, não faça merge e não altere cron, cadence ou permissões sem necessidade técnica demonstrada.

### 7.4 Prevenção de loops horários

O cron local deve agir somente quando o ID ativo do HANDOFF for maior que o último ID publicado no REPORT. A mesma instrução não deve ser executada duas vezes. Se já executada e publicada, registre somente `no_new_handoff` no log local, sem invocar o modelo ou publicar commits.

Quando `BC-HANDOFF-003` for concluído, ChatGPT revisará o resultado na próxima verificação horária. Se todos os gates da feature estiverem satisfatórios, a próxima resposta deverá recomendar preparação de PR e CI para aprovação humana, não iniciar trabalho novo artificialmente.
