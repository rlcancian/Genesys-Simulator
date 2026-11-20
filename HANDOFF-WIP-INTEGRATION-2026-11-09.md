# Handoff — integração das branches SignalData e Save Diagnostics

Atualizado em 2026-11-09. O mantenedor pediu integração imediata dos branches mencionados e, depois, informou que precisa desligar o computador. Retomar a partir deste estado, sem reiniciar o trabalho.

## Últimas instruções do mantenedor

- “Integre tudo, agora!!!” — integrar em `WorkInProgress` os branches `WiP202611/wait-signal-data-property`, `WiP202611/property-audit-closeout`, `WiP202611/datadef-component-properties` e `WiP202611/save-failure-diagnostics`.
- “preciso desligar o computador. Registre o que você está fazendo e meus últimos envios, para integrar em WorkInProgress os branches/PRs que citei, para poder retomar quando eu reiniciar o computador”.

## Estado confirmado

- Repositório: `/home/rafaelcancian/Repositories/Genesys-Simulator`.
- Branch local: `WorkInProgress`.
- `HEAD` local e `origin/WorkInProgress`: `7af21877210f007bf900f269ff9de54aab03d009`.
- O checkout está sincronizado; existe o arquivo não rastreado `genesys-2026.11.08.2646-linux-x86_64-portable.tar.gz`. Preserve-o; não o adicione, remova ou altere.
- PR #570 (`wait-signal-data-property`) foi integrada por merge commit `7af21877210f007bf900f269ff9de54aab03d009` em 2026-11-09 12:27 UTC. Checks finais observados: unit, kernel, smoke, GUI diagnostics, aplicações auxiliares, Debian build/lifecycle, plugin checks e launcher passaram.
- PR #571 (`datadef-component-properties`) já estava integrada por merge commit `3eafcc14544c4b91c6b17d35b3827581d9dbcb76`.
- PR #572 (`property-audit-closeout`) já estava integrada por merge commit `4b3a75bf4a0e0e3b9d8850d735bc9f2715323b63`.
- Portanto, três das quatro branches citadas estão integradas. A pendência é a PR #568 (`save-failure-diagnostics`).

## Próxima integração pendente — PR #568

- URL: https://github.com/rlcancian/Genesys-Simulator/pull/568
- Branch remota: `WiP202611/save-failure-diagnostics`.
- HEAD atual da branch/PR: `c2005e561f800d7c17e36076cd33b949d589b162`.
- Estado: aberta, Draft; mergeability indicada como `MERGEABLE`, `mergeStateStatus=UNSTABLE` enquanto checks executam.
- A branch foi reconciliada em worktree isolado com o então-current `origin/WorkInProgress` e com a branch da PR #570; conflitos da backlog foram resolvidos preservando as entradas. O delta da PR contra a base anterior contém o diagnóstico de save e SignalData; agora que #570 foi integrada, o delta funcional restante é o diagnóstico de save.
- O corpo da PR foi atualizado para registrar que a instrução explícita do mantenedor em 2026-11-09 autoriza merge após validação, supersedendo o “NO MERGE” anterior da fase inicial.
- Checks do HEAD `c2005e561f800d7c17e36076cd33b949d589b162` observados na última consulta:
  - PASS: Worker, Shell, Data Analyzer, Optimizer, GUI GMDD diagnostics, plugin archives/link graph (present/absent), plugin target introspection, documentation governance, plugin lifetime, launcher, smoke, unit tests, kernel tests e build principal Qt6/issue form.
  - PENDING: build Debian package, AI Assistant GUI startup, `integrated-tools`.
- Não faça merge da #568 até todos os checks obrigatórios da revisão final terminarem e serem inspecionados. Quando verdes: marque Ready, faça merge commit (governança desabilita squash/rebase), confirme SHA e atualize `WorkInProgress` local.

## Worktrees locais

- `/tmp/genesys-wait-signal`: detached no candidato `e9f8ef84ec89a8415f2ceacef18db11c66aff6c0`; remoto da branch #570 continua existindo.
- `/tmp/genesys-save-diagnostics`: detached no candidato `c2005e561f800d7c17e36076cd33b949d589b162`, atualizado com WIP e #570; remoto da branch #568 está nesse SHA.
- Há outros worktrees locais em `/home/rafaelcancian/Repositories/Genesys-*`; antes de removê-los, verificar a lista de worktrees e não descartar trabalho.
- A branch remota `WiP202611/wait-signal-data-property` ainda existe apesar do merge. As branches #571 e #572 também foram vistas no remoto. Não apagar branches locais/worktrees sem verificar dependências e estado.

## Comandos imediatos ao retomar

```bash
cd /home/rafaelcancian/Repositories/Genesys-Simulator
git status --short --branch
git fetch origin --prune
git rev-parse HEAD origin/WorkInProgress
gh pr checks 568 --repo rlcancian/Genesys-Simulator
gh pr view 568 --repo rlcancian/Genesys-Simulator --json headRefOid,state,isDraft,mergeable,mergeStateStatus,statusCheckRollup,url
```

Depois dos checks:

1. confirmar que o HEAD da PR continua `c2005e561...`; se mudou, reavaliar checks;
2. inspecionar o resultado de cada workflow, especialmente `integrated-tools`, `tests-unit`, GUI AI Assistant e pacote Debian;
3. se tudo obrigatório estiver verde, marcar a PR pronta e fazer merge commit #568;
4. `git fetch origin --prune`, `git pull --ff-only origin WorkInProgress` no checkout local;
5. verificar `HEAD == origin/WorkInProgress` e preservar o tarball não rastreado;
6. confirmar os quatro resultados de PRs #568/#570/#571/#572;
7. atualizar/encerrar este handoff se apropriado. Não alegar que a #568 foi integrada antes de confirmar o merge SHA.
