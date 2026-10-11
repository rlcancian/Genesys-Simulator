---
document_type: reference
authority: task-specific-handoff
owner: project-maintainer
last_reviewed: 2026-10-11
status: active
---

Antes de qualquer análise ou alteração, leia obrigatoriamente /docs/ai_assistants/README.md no branch WorkInProgress e informe-se sobre todas as regras atuais do GenESyS.

# GenESyS — Canal de comunicação ChatGPT ↔ Codex para BacteriaColony

**Protocolo:** `BC-HANDOFF-001` (2026-10-11).
**Repositório:** `rlcancian/Genesys-Simulator`.
**Branch em que este handoff foi publicado:** `WiP20261008/BacteriaColony`.
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
