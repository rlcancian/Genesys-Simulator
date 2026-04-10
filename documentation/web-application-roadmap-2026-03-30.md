# GenESyS — Roadmap para novo tipo de aplicação Web (Webhook/API HTTP)

## Audit Status (WiP20261)
- Branch audited: `WiP20261`
- Audit scope: reauditoria do roadmap web contra implementação atual em `source/applications/web`
- Status legend: `DONE`, `PARTIAL`, `OPEN`, `UNCERTAIN`, `SUPERSEDED`
- Data desta reauditoria documental: `2026-04-10`

## Objetivo histórico
Adicionar um terceiro tipo de aplicação em `source/applications`, além de Terminal e GUI: **Web**.

## Estado atual revalidado

### Build e estrutura de aplicação
### Audit status
`DONE`

### Evidence
- Existe `source/applications/web/CMakeLists.txt`.
- Existe biblioteca `genesys_web_core`.
- Existe executável `genesys_webhook`.
- Existe `main.cpp` e `BaseGenesysWebApplication.cpp` operando como entrypoint da aplicação web.

### Remaining gaps
- Sem gap estrutural principal para “novo tipo Web disponível”.

## Reclassificação por fases do roadmap original

### Fase 0 — Preparação de build
### Audit status
`DONE`

### Evidence
- Alvos web dedicados estão presentes e compiláveis em separado.
- Link com runtime do kernel está configurado.

### Remaining gaps
- Nenhum gap crítico desta fase.

### Fase 1 — Esqueleto da aplicação Web
### Audit status
`DONE`

### Evidence
- Processo HTTP sobe via `SimpleHttpServer`.
- Endpoint `GET /health` já implementado em `ApiRouter.cpp`.
- Porta é configurável por CLI (`--port`) com default 8080.

### Remaining gaps
- Nenhum gap crítico desta fase.

### Fase 2 — Bridge HTTP → Kernel (v1)
### Audit status
`DONE`

### Evidence
- Endpoints implementados:
  - `/api/v1/auth/session`
  - `/api/v1/simulator/info`
  - `/api/v1/models`
  - `/api/v1/models/current`
  - `/api/v1/models/save`
  - `/api/v1/models/load`
  - `/api/v1/simulation/status`
  - `/api/v1/simulation/config`
  - `/api/v1/simulation/run`
  - `/api/v1/simulation/step`
- `SimulatorSessionService` encapsula operações de sessão/modelo/simulação.

### Remaining gaps
- Rota histórica de `plugins/autoload` não foi localizada na API atual.

### Fase 3 — Segurança e robustez mínima
### Audit status
`PARTIAL`

### Evidence
- Há autenticação por bearer token de sessão.
- Há validações de método HTTP, token, corpo mínimo e mapeamento de vários erros JSON.
- Há validação de filename no service (`_isSafeFilename`) para operações de persistência.

### Remaining gaps
- Parsing de JSON ainda é baseado em regex simples, sem parser JSON robusto.
- Não há evidência de limites de payload/timeout diretamente no roteador/servidor nesta auditoria.

### Fase 4 — Simulação remota (controle de execução)
### Audit status
`PARTIAL`

### Evidence
- Existem endpoints de execução: `run` e `step`.
- Existe endpoint de status e configuração.

### Remaining gaps
- Não foram encontrados endpoints `pause`, `resume` e `stop`.
- Roadmap antigo citava `start`; implementação atual usa `run` (equivalência parcial de intenção, não 1:1 de contrato).

## Itens superados por mudanças posteriores

### Audit status
`SUPERSEDED`

### Evidence
- O roadmap original sugeria que o tipo Web ainda precisava ser criado; hoje o tipo já está materializado e funcional em múltiplos endpoints.
- Requisito inicial de “porta 80” foi superado por abordagem mais operacional (`--port`, default 8080), alinhada à mitigação prevista no próprio roadmap.

### Remaining gaps
- Atualizar histórico para refletir que as fases iniciais já foram concluídas.

## Conclusão da reauditoria
O roadmap de 2026-03-30 está amplamente superado pela implementação atual: a aplicação Web já existe, possui build dedicado e uma superfície API v1 relevante. O backlog real migrou para robustez (parsing/segurança operacional) e completude de controles de simulação (`pause/resume/stop`) além da eventual rota de autoload de plugins.

## Remaining Work
- `OPEN` — Adicionar endpoint de `plugins/autoload` (se ainda fizer parte do contrato alvo).
- `OPEN` — Adicionar endpoints de controle de simulação `pause`, `resume` e `stop`.
- `PARTIAL` — Reforçar robustez de parsing/segurança (evitar regex ad-hoc para JSON; revisar limites operacionais de request).
- `UNCERTAIN` — Confirmar em teste integrado se há limitação de payload/timeout no servidor HTTP atual (não evidenciado diretamente nesta leitura documental).
