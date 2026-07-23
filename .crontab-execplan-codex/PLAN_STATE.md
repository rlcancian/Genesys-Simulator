# PLAN_STATE.md — Estado da execução autônoma (prompt de cada invocação)

RUN_STATE: EM_EXECUCAO
HALT_REASON:
CURRENT_STEP:
NOTIFIED: NAO

## Etapas

| ID | Descrição curta | Fonte em PLAN.md | Depende de | Requer decisão humana | Status | Última atualização |
|---|---|---|---|---|---|---|
| MAN-F0-001 | Congelar baseline do manual | §8, §§50-51 | - | Não | done | 2026-07-23 |
| MAN-F1-001 | Corrigir defeitos técnicos do PDF atual | §9, §§50-51 | MAN-F0-001 | Não | done | 2026-07-23 |
| MAN-F2-001 | Criar infraestrutura LaTeX e política de figuras | §10, §§50-51 | MAN-F1-001 | Não | done | 2026-07-23 |
| MAN-F3-001 | Implementar a nova árvore editorial | §11, §§50-51 | MAN-F2-001 | Não | done | 2026-07-23 |
| MAN-F4-001 | Front matter, rastreabilidade e evidência | §12, §§50-51 | MAN-F3-001 | Não | pending | - |
| MAN-F5-001 | Documentar instalação para usuários | §13, §§50-51 | MAN-F4-001 | Não | pending | - |
| MAN-F6-001 | Documentar a visão geral das aplicações | §14, §§50-51 | MAN-F5-001 | Não | pending | - |
| MAN-F7-001 | Documentar a aplicação gráfica principal | §15, §§50-51 | MAN-F6-001 | Não | pending | - |
| MAN-F8-001 | Criar o primeiro modelo reproduzível | §16, §§50-51 | MAN-F7-001 | Não | pending | - |
| MAN-F9-001 | Documentar edição de modelos na GUI | §17, §§50-51 | MAN-F8-001 | Não | pending | - |
| MAN-F10-001 | Documentar execução de simulações | §18, §§50-51 | MAN-F9-001 | Não | pending | - |
| MAN-F11-001 | Documentar resultados, traces e relatórios | §19, §§50-51 | MAN-F10-001 | Não | pending | - |
| MAN-F12-001 | Documentar a linguagem para usuários | §20, §§50-51 | MAN-F11-001 | Não | pending | - |
| MAN-F13-001 | Documentar o Shell | §21, §§50-51 | MAN-F12-001 | Não | pending | - |
| MAN-F14-001 | Documentar Worker e segurança | §22, §§50-51, §54 | MAN-F13-001 | Não | pending | - |
| MAN-F15-001 | Documentar ferramentas independentes | §23, §§50-51 | MAN-F14-001 | Não | pending | - |
| MAN-F16-001 | Documentar modelos específicos e exemplos avançados | §24, §§50-51 | MAN-F15-001 | Não | pending | - |
| MAN-F17-001 | Documentar troubleshooting e limitações | §25, §§50-51 | MAN-F16-001 | Não | pending | - |
| MAN-F18-001 | Documentar ambiente de desenvolvimento e build | §26, §§50-51 | MAN-F17-001 | Não | pending | - |
| MAN-F19-001 | Documentar a arquitetura do repositório | §27, §§50-51 | MAN-F18-001 | Não | pending | - |
| MAN-F20-001 | Documentar kernel, lifecycle e managers | §28, §§50-51 | MAN-F19-001 | Não | pending | - |
| MAN-F21-001 | Documentar eventos e replicações | §29, §§50-51 | MAN-F20-001 | Não | pending | - |
| MAN-F22-001 | Documentar representação e persistência de modelos | §30, §§50-51 | MAN-F21-001 | Não | pending | - |
| MAN-F23-001 | Documentar Components e ModelData | §31, §§50-51 | MAN-F22-001 | Não | pending | - |
| MAN-F24-001 | Documentar arquitetura atual e futura de plugins | §32, §§50-51 | MAN-F23-001 | Não | pending | - |
| MAN-F25-001 | Documentar a arquitetura do parser | §33, §§50-51 | MAN-F24-001 | Não | pending | - |
| MAN-F26-001 | Publicar a referência da linguagem | §34, §§50-51 | MAN-F25-001 | Não | pending | - |
| MAN-F27-001 | Documentar tools e algoritmos | §35, §§50-51 | MAN-F26-001 | Não | pending | - |
| MAN-F28-001 | Documentar simulação contínua, modal e híbrida | §36, §§50-51, §54 | MAN-F27-001 | Não | pending | - |
| MAN-F29-001 | Documentar a arquitetura das aplicações | §37, §§50-51 | MAN-F28-001 | Não | pending | - |
| MAN-F30-001 | Documentar a arquitetura da GUI | §38, §§50-51 | MAN-F29-001 | Não | pending | - |
| MAN-F31-001 | Documentar Worker e execução remota | §39, §§50-51, §54 | MAN-F30-001 | Não | pending | - |
| MAN-F32-001 | Documentar IA e integrações externas | §40, §§50-51 | MAN-F31-001 | Não | pending | - |
| MAN-F33-001 | Documentar extensões biológicas e whole-cell | §41, §§50-51, §53, §54 | MAN-F32-001 | Não | pending | - |
| MAN-F34-001 | Documentar testes e validação | §42, §§50-51 | MAN-F33-001 | Não | pending | - |
| MAN-F35-001 | Documentar packaging e CI | §43, §§50-51 | MAN-F34-001 | Não | pending | - |
| MAN-F36-001 | Documentar governança e contribuição | §44, §§50-51 | MAN-F35-001 | Não | pending | - |
| MAN-F37-001 | Produzir figuras definitivas | §45, §46, §§50-51 | MAN-F36-001 | Não | pending | - |
| MAN-F38-001 | Consolidar bibliografia, glossário, índice e apêndices | §47, §§50-51, §53 | MAN-F37-001 | Não | pending | - |
| MAN-F39-001 | Validar integralmente o manual até o ponto de release | §48.1-§48.4, §§50-51, §54 | MAN-F38-001 | Não | pending | - |
| MAN-F39-002 | Autorizar e executar a publicação final do manual | §48.5, §53, §54 | MAN-F39-001 | Sim | blocked_human | - |
| MAN-F40-001 | Gerar handoff e manutenção contínua | §49, §§50-51 | MAN-F39-002 | Não | pending | - |

## Log resumido (últimas execuções)

- 2026-07-23: MAN-F3-001 concluida. A nova arvore editorial foi adicionada com 32 stubs compilaveis do User/Developer Manual, a tabela de migracao foi incluida e o `book_content.tex` passou a expor a estrutura final antes de reter os capitulos legados em `Legacy Chapters (Temporary)`; `docs/ManualGenESyS.pdf` recompilado com 156 paginas. [revisao sugerida] a retencao temporaria da arvore legada e a tabela de migracao sao escolhas conservadoras para preservar rastreabilidade antes da limpeza futura.
- 2026-07-23: MAN-F2-001 concluida. Infraestrutura LaTeX de figuras criada com macro `\figureplaceholder`, politica documentada no `README.md`, validacao reproduzivel em `validate_figure_specs.sh`, diretorios de destino materializados e exemplo compilavel adicionado em `preface.tex`; `docs/ManualGenESyS.pdf` recompilado com 86 paginas e a referencia da figura placeholder ficou resolvida. [revisao sugerida] a solucao adotou um placeholder estrutural conservador no proprio LaTeX e um verificador leve de `FIGURE-SPEC` para manter a politica auditavel sem introduzir dependencias novas.
- 2026-07-23: MAN-F1-001 concluida. `extbook` substituiu `book` para manter `9pt` sem warning de opcao global, metadata do PDF foi corrigida, bibliografia temporaria/ficticia foi removida do fluxo, e os overfulls objetivos foram eliminados ou reduzidos a underfulls residuais; `docs/ManualGenESyS.pdf` recompilado com 84 paginas. [revisao sugerida] a remocao temporaria da bibliografia e o uso de `extbook` foram escolhas conservadoras permitidas pela fase para preservar o layout e eliminar warnings objetivos sem reintroduzir refs ficticias.
- 2026-07-23: MAN-F0-001 concluida. Baseline do manual registrado em `../docs/developers/ManualGenESyS_source/MANUAL_BASELINE.md` com branch `WorkInProgress`, commit `ab34b2c9033a4869acc955e8ab710eda09598400`, inventario dos capitulos/figuras, metadados atuais do PDF e warnings objetivos do log. [revisao sugerida] o worktree ja estava sujo por alteracoes preexistentes do usuario, entao o baseline documenta esse estado sem tentar limpa-lo.
