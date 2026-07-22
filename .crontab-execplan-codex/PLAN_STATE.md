# PLAN_STATE.md — Estado da execução autônoma (prompt de cada invocação)

RUN_STATE: EM_EXECUCAO
HALT_REASON:
CURRENT_STEP: MAN-M1-001
NOTIFIED: NAO

## Etapas

| ID | Descrição curta | Fonte em PLAN.md | Depende de | Requer decisão humana | Status | Última atualização |
|---|---|---|---|---|---|---|
| MAN-M0-001 | Registrar baseline de evidências | §17.1 MAN-M0-001, §18 M0, §19.1 | - | Não | done | 2026-07-22T17:02:44-03:00 |
| MAN-M1-001 | Corrigir comandos e alvos de instalação | §17.1 MAN-M1-001, §17.2 MAN-M1-001, §18 M1, §19.2, §22 | MAN-M0-001 | Não | pending | - |
| MAN-M1-002 | Adicionar aviso de maturidade e evidências | §17.1 MAN-M1-002, §17.2 MAN-M1-002, §18 M1, §20 item 1, §22 | MAN-M0-001 | Sim (§20 item 1) | blocked_human | - |
| MAN-M1-003 | Corrigir texto de plugins estáticos | §17.1 MAN-M1-003, §17.2 MAN-M1-003, §18 M1, §22 | MAN-M0-001 | Não | pending | - |
| MAN-M1-004 | Remover caminhos obsoletos do terminal | §17.1 MAN-M1-004, §17.2 MAN-M1-004, §18 M1, §22 | MAN-M0-001 | Não | pending | - |
| MAN-M1-005 | Adicionar aviso de segurança do worker | §17.1 MAN-M1-005, §17.2 MAN-M1-005, §18 M1, §20 item 5, §22 | MAN-M0-001 | Sim (§20 item 5) | blocked_human | - |
| MAN-M1-006 | Corrigir exageros sobre otimizador, IA e ciência | §17.1 MAN-M1-006, §18 M1, §21.1, §22 | MAN-M0-001 | Não | pending | - |
| MAN-M1-007 | Validar e corrigir `hyperref` | §17.1 MAN-M1-007, §17.2 MAN-M1-007, §18 M1, §19.2, §22 | MAN-M1-001, MAN-M1-002, MAN-M1-003, MAN-M1-004, MAN-M1-005, MAN-M1-006 | Não | pending | - |
| MAN-M1-008 | Remover português residual ativo | §17.1 MAN-M1-008, §18 M1, §19.3 | MAN-M1-007 | Não | pending | - |
| MAN-M2-001 | Aprovar mapa editorial novo | §17.1 MAN-M2-001, §17.2 MAN-M2-001, §18 M2, §20 item 2 | MAN-M1-008 | Sim (§20 item 2) | blocked_human | - |
| MAN-M2-002 | Separar capítulos do Manual do Usuário | §17.1 MAN-M2-002, §18 M2 | MAN-M2-001 | Não | pending | - |
| MAN-M2-003 | Separar capítulos do Manual do Desenvolvedor | §17.1 MAN-M2-003, §18 M2 | MAN-M2-001 | Não | pending | - |
| MAN-M3-001 | Selecionar e validar o primeiro modelo | §17.1 MAN-M3-001, §17.2 MAN-M3-001, §18 M3, §20 item 4 | MAN-M2-003 | Sim (§20 item 4) | blocked_human | - |
| MAN-M3-002 | Documentar a GUI principal validada | §17.1 MAN-M3-002, §18 M3 | MAN-M3-001 | Não | pending | - |
| MAN-M3-003 | Adicionar fluxo validado do Shell | §17.1 MAN-M3-003, §18 M3, §19.4 | MAN-M3-001 | Não | pending | - |
| MAN-M3-004 | Adicionar fluxo local limitado do worker | §17.1 MAN-M3-004, §18 M3, §20 item 5 | MAN-M3-001 | Sim (§20 item 5) | blocked_human | - |
| MAN-M3-005 | Documentar apps stand-alone com evidência | §17.1 MAN-M3-005, §18 M3 | MAN-M3-001 | Não | pending | - |
| MAN-M4-001 | Documentar ciclo de vida e ownership do kernel | §17.1 MAN-M4-001, §17.2 MAN-M4-001, §18 M4 | MAN-M2-003 | Não | pending | - |
| MAN-M4-002 | Adicionar arquitetura de persistência | §17.1 MAN-M4-002, §18 M4 | MAN-M4-001 | Não | pending | - |
| MAN-M4-003 | Montar referência da linguagem de expressão | §17.1 MAN-M4-003, §17.2 MAN-M4-003, §18 M4 | MAN-M4-001 | Não | pending | - |
| MAN-M4-004 | Documentar registro estático de plugins | §17.1 MAN-M4-004, §18 M4 | MAN-M4-001 | Não | pending | - |
| MAN-M4-005 | Separar testes de validação científica | §17.1 MAN-M4-005, §17.2 MAN-M4-005, §18 M4, §20 item 10 | MAN-M2-003 | Sim (§20 item 10) | blocked_human | - |
| MAN-M4-006 | Atualizar presets e CI atuais | §17.1 MAN-M4-006, §18 M4, §19.4 | MAN-M1-008 | Não | pending | - |
| MAN-M4-007 | Documentar ciclo de pacotes Debian | §17.1 MAN-M4-007, §18 M4 | MAN-M4-006 | Não | pending | - |
| MAN-M5-001 | Documentar contratos e testes de domínios contínuos | §17.1 MAN-M5-001, §18 M5, §20 item 10 | MAN-M4-005 | Sim (§20 item 10) | blocked_human | - |
| MAN-M5-002 | Documentar semântica modal e híbrida atual | §17.1 MAN-M5-002, §18 M5, §20 item 9 | MAN-M5-001 | Sim (§20 item 9) | blocked_human | - |
| MAN-M5-003 | Adicionar exemplos validados de CA/Petri | §17.1 MAN-M5-003, §18 M5 | MAN-M5-001, MAN-M5-002 | Não | pending | - |
| MAN-M5-004 | Documentar capítulo biológico/WCM | §17.1 MAN-M5-004, §17.2 MAN-M5-004, §18 M5, §20 item 10, 12 | MAN-M5-001, MAN-M5-002 | Sim (§20 item 10, 12) | blocked_human | - |
| MAN-M5-005 | Documentar arquitetura de confiança e segurança de IA | §17.1 MAN-M5-005, §18 M5, §20 item 13 | MAN-M5-001 | Sim (§20 item 13) | blocked_human | - |
| MAN-M6-001 | Inserir placeholders de figuras com fonte | §17.1 MAN-M6-001, §17.2 MAN-M6-001, §18 M6, §20 item 14 | MAN-M2-003 | Sim (§20 item 14) | blocked_human | - |
| MAN-M6-002 | Capturar estados validados das aplicações | §17.1 MAN-M6-002, §18 M6, §19.5, §19.9 | MAN-M6-001 | Não | pending | - |
| MAN-M6-003 | Gerar diagramas de arquitetura validados | §17.1 MAN-M6-003, §18 M6 | MAN-M4-001, MAN-M4-002, MAN-M4-003, MAN-M4-004, MAN-M4-005, MAN-M4-006, MAN-M4-007 | Não | pending | - |
| MAN-M6-004 | Gerar gráficos numéricos validados | §17.1 MAN-M6-004, §18 M6, §20 item 14 | MAN-M5-001, MAN-M5-002, MAN-M5-003, MAN-M5-004, MAN-M5-005 | Sim (§20 item 14) | blocked_human | - |
| MAN-M7-001 | Construir e inspecionar o PDF final | §17.1 MAN-M7-001, §17.2 MAN-M7-001, §18 M7, §19.2, §19.9, §20 item 2 | MAN-M6-002, MAN-M6-003, MAN-M6-004 | Sim (§20 item 2) | blocked_human | - |
| MAN-M7-002 | Atualizar mapa e status da fonte do manual | §17.1 MAN-M7-002, §18 M7 | MAN-M7-001 | Não | pending | - |
| MAN-M7-003 | Submeter PR pequeno de documentação | §17.1 MAN-M7-003, §18 M7 | MAN-M7-001 | Não | pending | - |

## Baseline de evidências (MAN-M0-001)

- Data: 2026-07-22T17:02:44-03:00
- Branch: `WorkInProgress`
- SHA: `2a529facb96922f266c0c431452c464b52ade6b3`
- Fontes: `git status --short`, `git branch --show-current`, `git rev-parse HEAD`
- Referências do plano: §17.1 MAN-M0-001, §18 M0, §19.1
- Limite de evidência: baseline apenas; nenhuma alteração de código ou documento externo foi feita nesta etapa.

## Log resumido (últimas execuções)

- 2026-07-22T17:02:44-03:00: baseline registrado com branch `WorkInProgress`, SHA `2a529facb96922f266c0c431452c464b52ade6b3` e fontes `git status --short`/`git branch --show-current`/`git rev-parse HEAD`; validação de §19.1 conferida.
