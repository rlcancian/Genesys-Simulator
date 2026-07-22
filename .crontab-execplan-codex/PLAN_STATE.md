# PLAN_STATE.md — Estado da execução autônoma (prompt de cada invocação)

RUN_STATE: EM_EXECUCAO
HALT_REASON:
CURRENT_STEP:
NOTIFIED: NAO

## Etapas

| ID | Descrição curta | Fonte em PLAN.md | Depende de | Requer decisão humana | Status | Última atualização |
|---|---|---|---|---|---|---|
| MAN-M0-001 | Registrar baseline do update manual | §22 "Proposed first PR", §17.1, §17.2/MAN-M0-001 | - | Não | done | 2026-07-22T19:43:38-0300 |
| MAN-M1-001 | Corrigir comandos, presets, alvos e baseline de build | §22 itens 1-4, §17.1, §17.2/MAN-M1-001 | MAN-M0-001 | Não | done | 2026-07-22T20:05:00-0300 |
| MAN-M1-002 | Inserir aviso de evidência, maturidade e segurança | §22 itens 7 e 14, §17.1, §17.2/MAN-M1-002 | MAN-M0-001 | Não | done | 2026-07-22T20:24:59-0300 |
| MAN-M1-003 | Corrigir o texto de plugins estáticos | §22 item 6, §17.1, §17.2/MAN-M1-003 | MAN-M0-001 | Não | done | 2026-07-22T20:31:03-0300 |
| MAN-M1-004 | Remover referências obsoletas ao terminal | §22 item 5, §17.1, §17.2/MAN-M1-004 | MAN-M0-001 | Não | done | 2026-07-22T20:51:53-0300 |
| MAN-M1-005 | Adicionar aviso obrigatório contra deploy público | §22 item 8, §17.1, §17.2/MAN-M1-005 | MAN-M0-001 | Não | pending | - |
| MAN-M1-006 | Corrigir exageros de maturidade e linguagem científica | §22 itens 7 e 14, §17.1, §17.2/MAN-M1-006 | MAN-M0-001 | Não | pending | - |
| MAN-M1-007 | Validar e corrigir a configuração de `hyperref` | §22 item 10, §17.1, §17.2/MAN-M1-007 | MAN-M1-001, MAN-M1-002, MAN-M1-003, MAN-M1-004, MAN-M1-005, MAN-M1-006, MAN-M1-008 | Não | pending | - |
| MAN-M1-008 | Limpar português residual nos trechos tocados | §22 item 9, §17.1, §17.2/MAN-M1-008 | MAN-M1-001, MAN-M1-002, MAN-M1-003, MAN-M1-004, MAN-M1-005, MAN-M1-006 | Não | pending | - |
| MAN-M7-001 | Construir e inspecionar o PDF atualizado | §22 itens 11-12, §17.1, §17.2/MAN-M7-001 | MAN-M1-007 | Não | pending | - |

## Log resumido (últimas execuções)

- 2026-07-22T20:51:53-0300: etapa MAN-M1-004 concluída. Atualizado o capítulo de aplicações para trocar `source/applications/terminal` por `source/applications/shell` e `source/applications/modelSpecific`, distinguindo `genesys_shell` de `genesys_modelspecific_app`. Validados `cmake --preset genesys_shell`, `cmake --preset genesys_modelspecific_app`, `cmake --build --preset genesys_shell --parallel "$(nproc)"` e `cmake --build --preset genesys_modelspecific_app --parallel "$(nproc)"`.
- 2026-07-22T19:43:38-0300: baseline de MAN-M0-001 registrado. Branch `WorkInProgress`, SHA `f6a86cd57b573056c2d17424983e9b083d21e059`, data `2026-07-22T19:43:38-0300`. Fontes-base confirmadas nesta invocação: `PLAN.md`, `docs/ai_assistants/{README.md,GOVERNANCE.md,ARCHITECTURE.md,STATUS.md,BACKLOG_AUTONOMOUS.md,BACKLOG_HUMAN.md,runbooks/GITHUB_AGENT.md,reference/manual_figure_automation_plan.md}`, `docs/developers/ManualGenESyS_source/{README.md,ManualGenESyS.tex,book_content.tex,preface.tex,capa.tex,copyright.tex,structure.tex,make.sh,chapter_*.tex}`, `CMakeLists.txt`, `CMakePresets.json`, `source/**/CMakeLists.txt`, `debian/{control,rules}`, `.github/workflows/genesys-ci.yml`, `docs/ManualGenESyS.pdf`. Evidências e limitações mantidas conforme o plano.
- 2026-07-22T20:05:00-0300: etapa MAN-M1-001 concluída. Validado `cmake --preset gui-app`, `cmake --build --preset gui-app --parallel "$(nproc)"` e a presença de `build/gui-app/source/applications/gui/genesys/genesys-gui`. Ajuste mantido no capítulo de instalação/build com os nomes reais de preset, alvo e executável.
- 2026-07-22T20:24:59-0300: etapa MAN-M1-002 concluída. Atualizado o `preface.tex` com vocabulário controlado de evidência e maturidade, limites de leitura para build/startup/validação científica e aviso de `Worker` local/private only. Validado com `./make.sh` em `docs/developers/ManualGenESyS_source`; a compilação XeLaTeX/PDF concluiu com warnings já existentes do manual.
- 2026-07-22T20:31:03-0300: etapa MAN-M1-003 concluída. Ajustado o capítulo de plugins para explicitar bibliotecas estáticas, metadados e conectores, sem sugerir pacotes runtime independentes nem definir ABI futuro. Validado com `./make.sh` em `docs/developers/ManualGenESyS_source`; o manual recompilou com warnings já existentes.
- 2026-07-22T19:43:38-0300: etapa MAN-M0-001 concluída sem mutação externa; baseline ficou apenas registrado no estado do plano.
